#include "../src/evaluation/eval.hpp"
#include "../src/evaluation/eval_params.hpp"
#include "../src/evaluation/pst.hpp"
#include "../src/evaluation/eval_features.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/board/board.hpp"
#include "../src/movegen/attack_masks.hpp"
#include "../src/movegen/movegen.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <omp.h>
#include <algorithm>
#include <filesystem>
#include <sstream>

using namespace heavensgate;

// Dense feature representation for 1 training sample
struct FeatureSample {
    std::string fen;    // Retained only in parity-check mode.
    float target;       // 1.0 (Win), 0.5 (Draw), 0.0 (Loss) from White's perspective
    float phase_mg;     // phase / 24.0
    float phase_eg;     // (24 - phase) / 24.0
    double base_constant; // Runtime white-perspective score minus modeled tunable terms
    double runtime_eval_white;
    double unrounded_eval_white;

    // Features: index corresponds to tunable parameter index
    // Each value is (feature_white - feature_black)
    std::vector<float> f_mg;
    std::vector<float> f_eg;
};

// Parameter descriptor for the tuner
struct TunerParam {
    std::string name;
    int* int_ptr;
    double min_val;
    double max_val;
    bool is_mg; // true if midgame, false if endgame
    size_t feat_idx;
};

// Independent slope oracle: assemble the production features without the two
// per-side integer divisions. This separates feature errors from quantization.
double unrounded_runtime_white(const Board& board) {
    const int phase = std::min(24, board.game_phase());
    auto side = [&](Color color) {
        const ScorePair positional = EvalFeatures::evaluate_pawn_structure(board, color)
            + EvalFeatures::evaluate_passed_pawns(board, color)
            + EvalFeatures::evaluate_king_safety(board, color)
            + EvalFeatures::evaluate_piece_activity(board, color)
            + EvalFeatures::evaluate_threats(board, color)
            + EvalFeatures::evaluate_mobility(board, color)
            + EvalFeatures::evaluate_material_imbalances(board, color);
        return ((board.mg_material(color) + board.mg_pst(color) + positional.mg) * phase
            + (board.eg_material(color) + board.eg_pst(color) + positional.eg) * (24 - phase)) / 24.0;
    };
    const double tempo = (g_eval_params.tempo_mg * phase + g_eval_params.tempo_eg * (24 - phase)) / 24.0;
    return side(Color::White) - side(Color::Black) + (board.side_to_move() == Color::White ? tempo : -tempo);
}

inline double sigmoid(double eval_cp, double K = 400.0) {
    return 1.0 / (1.0 + std::pow(10.0, -eval_cp / K));
}

// Extract the feature differences from a position
void extract_board_features(const Board& board, std::vector<float>& feats_mg, std::vector<float>& feats_eg, float& phase_mg, float& phase_eg) {
    int phase = std::min(24, board.game_phase());
    phase_mg = phase / 24.0f;
    phase_eg = (24 - phase) / 24.0f;

    // Feature indices:
    // 0: pawn, 1: knight, 2: bishop, 3: rook, 4: queen
    // 5: doubled, 6: connected, 7: phalanx, 8: isolated, 9: backward, 10: central_lever, 11: pawn_tension
    // 12..17: passed ranks 2..7, 18: protected_passed
    // 19: minor_undev, 20: bishop_pair, 21: knight_outpost, 22: central_knight_outpost, 23: bishop_outpost
    // 24: rook_7th, 25: rook_open, 26: rook_semi_open
    // 27: minor_threat_rook, 28: minor_threat_queen, 29: rook_threat_queen, 30: pawn_push_threat, 31: pawn_attack_threat
    // 32: knight_mob, 33: bishop_mob, 34: rook_mob, 35: queen_mob
    // 36: pawn_shield, 37: uncastled_king, 38: tempo
    constexpr size_t NUM_FEATS = 39;
    feats_mg.assign(NUM_FEATS, 0.0f);
    feats_eg.assign(NUM_FEATS, 0.0f);

    auto extract_side = [&](Color side, float sign) {
        Color opp = ~side;
        Bitboard occ = board.occupied();

        // 1. Material
        Bitboard pawns   = board.pieces(make_piece(side, PieceType::Pawn));
        Bitboard knights = board.pieces(make_piece(side, PieceType::Knight));
        Bitboard bishops = board.pieces(make_piece(side, PieceType::Bishop));
        Bitboard rooks   = board.pieces(make_piece(side, PieceType::Rook));
        Bitboard queens  = board.pieces(make_piece(side, PieceType::Queen));

        feats_mg[0] += sign * popcount(pawns);
        feats_eg[0] += sign * popcount(pawns);
        feats_mg[1] += sign * popcount(knights);
        feats_eg[1] += sign * popcount(knights);
        feats_mg[2] += sign * popcount(bishops);
        feats_eg[2] += sign * popcount(bishops);
        feats_mg[3] += sign * popcount(rooks);
        feats_eg[3] += sign * popcount(rooks);
        feats_mg[4] += sign * popcount(queens);
        feats_eg[4] += sign * popcount(queens);

        // 2. Pawn Structure
        Bitboard opp_pawns = board.pieces(make_piece(opp, PieceType::Pawn));
        Bitboard not_file_a = ~file_bb(File::FileA);
        Bitboard not_file_h = ~file_bb(File::FileH);

        // Doubled
        int doubled = 0;
        for (int f = 0; f < 8; ++f) {
            int cnt = popcount(pawns & file_bb(static_cast<File>(f)));
            if (cnt > 1) doubled += (cnt - 1);
        }
        feats_mg[5] -= sign * doubled;
        feats_eg[5] -= sign * doubled;

        // Connected & Phalanx
        Bitboard pawn_att = (side == Color::White)
            ? (((pawns & not_file_a) << 7) | ((pawns & not_file_h) << 9))
            : (((pawns & not_file_a) >> 9) | ((pawns & not_file_h) >> 7));
        int connected = popcount(pawns & pawn_att);
        feats_mg[6] += sign * connected;
        feats_eg[6] += sign * connected;

        Bitboard phalanx = pawns & (((pawns & not_file_a) >> 1) | ((pawns & not_file_h) << 1));
        int phalanx_cnt = popcount(phalanx) / 2;
        feats_mg[7] += sign * phalanx_cnt;
        feats_eg[7] += sign * phalanx_cnt;

        // Isolated
        Bitboard p_copy = pawns;
        int isolated = 0;
        while (p_copy) {
            Square sq = pop_lsb(p_copy);
            File f = file_of(sq);
            if ((pawns & EvalFeatures::IsolatedPawnMask[static_cast<size_t>(f)]) == EmptyBB) isolated++;
        }
        feats_mg[8] -= sign * isolated;
        feats_eg[8] -= sign * isolated;

        // Backward
        Bitboard opp_attacks = (side == Color::White)
            ? (((opp_pawns & not_file_a) >> 9) | ((opp_pawns & not_file_h) >> 7))
            : (((opp_pawns & not_file_a) << 7) | ((opp_pawns & not_file_h) << 9));
        p_copy = pawns;
        int backward = 0;
        while (p_copy) {
            Square sq = pop_lsb(p_copy);
            File f = file_of(sq);
            Rank r = rank_of(sq);
            Bitboard adj = pawns & EvalFeatures::IsolatedPawnMask[static_cast<size_t>(f)];
            bool is_behind = true;
            Bitboard adj_copy = adj;
            while (adj_copy) {
                Rank adj_r = rank_of(pop_lsb(adj_copy));
                if (side == Color::White && adj_r <= r) is_behind = false;
                if (side == Color::Black && adj_r >= r) is_behind = false;
            }
            if (is_behind && adj != EmptyBB) {
                Square stop_sq = make_square(f, (side == Color::White) ? static_cast<Rank>(static_cast<int>(r) + 1) : static_cast<Rank>(static_cast<int>(r) - 1));
                if (stop_sq != Square::None && (opp_attacks & square_bb(stop_sq))) backward++;
            }
        }
        feats_mg[9] -= sign * backward;
        feats_eg[9] -= sign * backward;

        // Central levers
        Bitboard single_pushes = (side == Color::White) ? ((pawns << 8) & ~occ) : ((pawns >> 8) & ~occ);
        Bitboard push_attacks = (side == Color::White)
            ? (((single_pushes & not_file_a) << 7) | ((single_pushes & not_file_h) << 9))
            : (((single_pushes & not_file_a) >> 9) | ((single_pushes & not_file_h) >> 7));
        Bitboard lever_targets = push_attacks & opp_pawns;
        Bitboard central_files = file_bb(File::FileC) | file_bb(File::FileD) | file_bb(File::FileE) | file_bb(File::FileF);
        int central_levers = popcount(lever_targets & central_files);
        feats_mg[10] += sign * central_levers;
        feats_eg[10] += sign * central_levers;

        // Tension
        int tension = popcount(pawn_att & opp_pawns);
        feats_mg[11] += sign * tension;
        feats_eg[11] += sign * tension;

        // 3. Passed Pawns
        size_t pers_idx = (side == Color::White) ? 0 : 1;
        p_copy = pawns;
        while (p_copy) {
            Square sq = pop_lsb(p_copy);
            if ((opp_pawns & EvalFeatures::PassedPawnMask[pers_idx][static_cast<size_t>(sq)]) == EmptyBB) {
                Rank r = rank_of(sq);
                int rank_idx = (side == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));
                if (rank_idx >= 1 && rank_idx <= 6) {
                    feats_mg[12 + rank_idx - 1] += sign;
                    feats_eg[12 + rank_idx - 1] += sign;
                }
                if (pawns & AttackMasks::pawn_attacks(~side, sq)) {
                    feats_mg[18] += sign;
                    feats_eg[18] += sign;
                }
            }
        }

        // 4. Piece Activity & Outposts
        // Bishop pair
        if (popcount(bishops) >= 2) {
            feats_mg[20] += sign;
            feats_eg[20] += sign;
        }

        // Undeveloped minors
        Bitboard home_minors = (side == Color::White)
            ? ((knights & (square_bb(Square::b1) | square_bb(Square::g1))) | (bishops & (square_bb(Square::c1) | square_bb(Square::f1))))
            : ((knights & (square_bb(Square::b8) | square_bb(Square::g8))) | (bishops & (square_bb(Square::c8) | square_bb(Square::f8))));
        feats_mg[19] -= sign * popcount(home_minors);

        // Outposts
        Bitboard central_mask = square_bb(Square::d4) | square_bb(Square::e4) | square_bb(Square::d5) | square_bb(Square::e5);
        Bitboard k_copy = knights;
        while (k_copy) {
            Square nsq = pop_lsb(k_copy);
            Rank nr = rank_of(nsq);
            bool adv = (side == Color::White) ? (nr >= Rank::Rank4 && nr <= Rank::Rank6) : (nr >= Rank::Rank3 && nr <= Rank::Rank5);
            if (adv && (pawns & AttackMasks::pawn_attacks(~side, nsq)) && !(opp_pawns & EvalFeatures::OutpostMask[pers_idx][static_cast<size_t>(nsq)])) {
                feats_mg[21] += sign;
                feats_eg[21] += sign;
                if (square_bb(nsq) & central_mask) {
                    feats_mg[22] += sign;
                    feats_eg[22] += sign;
                }
            }
        }

        Bitboard b_copy = bishops;
        while (b_copy) {
            Square bsq = pop_lsb(b_copy);
            Rank br = rank_of(bsq);
            bool adv = (side == Color::White) ? (br >= Rank::Rank4 && br <= Rank::Rank6) : (br >= Rank::Rank3 && br <= Rank::Rank5);
            if (adv && (pawns & AttackMasks::pawn_attacks(~side, bsq)) && !(opp_pawns & EvalFeatures::OutpostMask[pers_idx][static_cast<size_t>(bsq)])) {
                feats_mg[23] += sign;
                feats_eg[23] += sign;
            }
        }

        // Rooks
        Bitboard all_pawns = board.pieces(Piece::WhitePawn) | board.pieces(Piece::BlackPawn);
        Rank SeventhRank = (side == Color::White) ? Rank::Rank7 : Rank::Rank2;
        Bitboard r_copy = rooks;
        while (r_copy) {
            Square rsq = pop_lsb(r_copy);
            File f = file_of(rsq);
            Rank r = rank_of(rsq);
            if (r == SeventhRank) {
                feats_mg[24] += sign;
                feats_eg[24] += sign;
            }
            if ((all_pawns & file_bb(f)) == EmptyBB) {
                feats_mg[25] += sign;
                feats_eg[25] += sign;
            } else if ((pawns & file_bb(f)) == EmptyBB) {
                feats_mg[26] += sign;
                feats_eg[26] += sign;
            }
        }

        // 5. Threats
        Bitboard opp_rooks  = board.pieces(make_piece(opp, PieceType::Rook));
        Bitboard opp_queens = board.pieces(make_piece(opp, PieceType::Queen));
        Bitboard opp_minors = board.pieces(make_piece(opp, PieceType::Knight)) | board.pieces(make_piece(opp, PieceType::Bishop));

        Bitboard minor_att = EmptyBB;
        Bitboard kn_copy = knights;
        while (kn_copy) minor_att |= AttackMasks::knight_attacks(pop_lsb(kn_copy));
        Bitboard bi_copy = bishops;
        while (bi_copy) minor_att |= AttackMasks::bishop_attacks(pop_lsb(bi_copy), occ);

        feats_mg[27] += sign * popcount(minor_att & opp_rooks);
        feats_eg[27] += sign * popcount(minor_att & opp_rooks);
        feats_mg[28] += sign * popcount(minor_att & opp_queens);
        feats_eg[28] += sign * popcount(minor_att & opp_queens);

        Bitboard r_att = EmptyBB;
        Bitboard rk_copy = rooks;
        while (rk_copy) r_att |= AttackMasks::rook_attacks(pop_lsb(rk_copy), occ);
        feats_mg[29] += sign * popcount(r_att & opp_queens);
        feats_eg[29] += sign * popcount(r_att & opp_queens);

        int pawn_threats = popcount(push_attacks & (opp_minors | opp_rooks | opp_queens));
        feats_mg[30] += sign * pawn_threats;
        feats_eg[30] += sign * pawn_threats;

        // 6. Safe Mobility
        Bitboard opp_pawn_att = (opp == Color::White)
            ? (((opp_pawns & not_file_a) << 7) | ((opp_pawns & not_file_h) << 9))
            : (((opp_pawns & not_file_a) >> 9) | ((opp_pawns & not_file_h) >> 7));
        Bitboard safe_mask = ~(board.pieces(side) | opp_pawn_att);

        int n_mob = 0; kn_copy = knights;
        while (kn_copy) n_mob += popcount(AttackMasks::knight_attacks(pop_lsb(kn_copy)) & safe_mask);
        feats_mg[32] += sign * n_mob; feats_eg[32] += sign * n_mob;

        int b_mob = 0; bi_copy = bishops;
        while (bi_copy) b_mob += popcount(AttackMasks::bishop_attacks(pop_lsb(bi_copy), occ) & safe_mask);
        feats_mg[33] += sign * b_mob; feats_eg[33] += sign * b_mob;

        int r_mob = 0; rk_copy = rooks;
        while (rk_copy) r_mob += popcount(AttackMasks::rook_attacks(pop_lsb(rk_copy), occ) & safe_mask);
        feats_mg[34] += sign * r_mob; feats_eg[34] += sign * r_mob;

        int q_mob = 0; Bitboard q_copy = queens;
        while (q_copy) q_mob += popcount(AttackMasks::queen_attacks(pop_lsb(q_copy), occ) & safe_mask);
        feats_mg[35] += sign * q_mob; feats_eg[35] += sign * q_mob;

        // 7. King Safety
        Square ksq = board.king_square(side);
        if (ksq != Square::None) {
            int kr = static_cast<int>(rank_of(ksq));
            int kf = static_cast<int>(file_of(ksq));
            const bool queens_present = board.pieces(Piece::WhiteQueen) || board.pieces(Piece::BlackQueen);
            if (queens_present && ((side == Color::White && kr <= 3 && kf >= 2 && kf <= 5) ||
                (side == Color::Black && kr >= 4 && kf >= 2 && kf <= 5))) {
                feats_mg[37] -= sign;
                feats_eg[37] -= sign;
            }

            File kf_enum = file_of(ksq);
            Rank kr_enum = rank_of(ksq);
            Bitboard shield_mask = EmptyBB;
            if (side == Color::White && kr_enum <= Rank::Rank3) {
                for (int df = -1; df <= 1; ++df) {
                    int f_idx = static_cast<int>(kf_enum) + df;
                    if (f_idx >= 0 && f_idx < 8) {
                        shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank2));
                        shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank3));
                    }
                }
            } else if (side == Color::Black && kr_enum >= Rank::Rank6) {
                for (int df = -1; df <= 1; ++df) {
                    int f_idx = static_cast<int>(kf_enum) + df;
                    if (f_idx >= 0 && f_idx < 8) {
                        shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank7));
                        shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank6));
                    }
                }
            }
            int shield = popcount(pawns & shield_mask);
            if (queens_present) feats_mg[36] += sign * shield;
        }
    };

    extract_side(Color::White, 1.0f);
    extract_side(Color::Black, -1.0f);

    // Tempo: White to move = +1, Black to move = -1
    float tempo_sign = (board.side_to_move() == Color::White) ? 1.0f : -1.0f;
    feats_mg[38] = tempo_sign;
    feats_eg[38] = tempo_sign;

}

// The feature model is the local linear part of runtime evaluation. Any
// non-tunable terms and integer tapering residue belong in base_constant.
double tunable_score(const FeatureSample& sample, const std::vector<TunerParam>& params,
                     const std::vector<double>& weights) {
    double score = sample.phase_mg * sample.f_mg[0] * 100.0; // Pawn MG anchor.
    for (size_t p = 0; p < params.size(); ++p) {
        const auto& param = params[p];
        score += (param.is_mg ? sample.phase_mg * sample.f_mg[param.feat_idx]
                              : sample.phase_eg * sample.f_eg[param.feat_idx]) * weights[p];
    }
    return score;
}

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  HEAVEN'S GATE ANALYTIC TEXEL EVALUATION TUNER v2    \n";
    std::cout << "  Candidate-only fitting with held-out validation   \n";
    std::cout << "======================================================\n\n";

    int epochs = 80;
    double lr = 0.50;
    double lambda_reg = 0.0001; // L2 regularization to prevent drift
    std::string dataset_path = "data/quiet_positions.txt";

    bool parity_only = false;
    std::string validation_path, test_path, output_path, report_path;
    int patience = 10, requested_threads = 1;
    double sigmoid_k = 400.0;
    try {
        if (argc > 1) epochs = std::stoi(argv[1]);
        if (argc > 2) lr = std::stod(argv[2]);
        if (argc > 3) dataset_path = argv[3];
        for (int argument = 4; argument < argc; ++argument) {
            const std::string flag = argv[argument];
            if (flag == "--parity-only") { parity_only = true; continue; }
            if (argument + 1 == argc) throw std::invalid_argument("Missing option value");
            const std::string value = argv[++argument];
            if (flag == "--validation") validation_path = value;
            else if (flag == "--test") test_path = value;
            else if (flag == "--output") output_path = value;
            else if (flag == "--report") report_path = value;
            else if (flag == "--patience") patience = std::stoi(value);
            else if (flag == "--threads") requested_threads = std::stoi(value);
            else if (flag == "--k") sigmoid_k = std::stod(value);
            else throw std::invalid_argument("Unknown option: " + flag);
        }
        if (epochs < 0 || !std::isfinite(lr) || lr <= 0 || patience < 1 || requested_threads < 1 ||
            !std::isfinite(sigmoid_k) || sigmoid_k <= 0) throw std::invalid_argument("Invalid optimizer configuration");
        if (!parity_only) {
            if (validation_path.empty() || test_path.empty() || output_path.empty() || report_path.empty())
                throw std::invalid_argument("Training requires --validation, --test, --output and --report; use run_texel_candidate.py");
            auto absolute = [](const std::string& value) { return std::filesystem::weakly_canonical(value); };
            const auto production = absolute("src/evaluation/tuned_eval_params.inc");
            if (absolute(output_path) == production || absolute(report_path) == production ||
                absolute(output_path) == absolute(report_path) || std::filesystem::exists(output_path) ||
                std::filesystem::exists(report_path)) throw std::invalid_argument("Output must be a fresh, non-production candidate and report");
            if (absolute(dataset_path) == absolute(validation_path) || absolute(dataset_path) == absolute(test_path) ||
                absolute(validation_path) == absolute(test_path)) throw std::invalid_argument("Held-out inputs must be separate files");
        }
    } catch (const std::exception& error) {
        std::cerr << "[Error] " << error.what() << '\n';
        return 1;
    }

    Zobrist::init();
    MoveGenerator::init();
    PST::init();
    EvalFeatures::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);

    int num_threads = std::min(requested_threads, omp_get_max_threads());
    omp_set_num_threads(num_threads);
    std::cout << "[Texel] Initialized OpenMP across " << num_threads << " CPU threads.\n";

    // 1. Build Parameter Table (Pawn MG remains anchored).
    std::vector<TunerParam> params;
    std::vector<double> w;

    auto add_p = [&](const std::string& name, int* int_p, double min_v, double max_v, bool is_mg, size_t f_idx) {
        w.push_back(static_cast<double>(*int_p));
        params.push_back({name, int_p, min_v, max_v, is_mg, f_idx});
    };

    // Material (PawnMG anchored at 100 - NOT in params!)
    add_p("pawn_eg", &g_eval_params.pawn_eg, 80, 160, false, 0);
    add_p("knight_mg", &g_eval_params.knight_mg, 260, 380, true, 1);
    add_p("knight_eg", &g_eval_params.knight_eg, 250, 370, false, 1);
    add_p("bishop_mg", &g_eval_params.bishop_mg, 270, 390, true, 2);
    add_p("bishop_eg", &g_eval_params.bishop_eg, 270, 400, false, 2);
    add_p("rook_mg", &g_eval_params.rook_mg, 420, 580, true, 3);
    add_p("rook_eg", &g_eval_params.rook_eg, 440, 620, false, 3);
    add_p("queen_mg", &g_eval_params.queen_mg, 780, 1050, true, 4);
    add_p("queen_eg", &g_eval_params.queen_eg, 800, 1100, false, 4);

    // Pawn Structure
    add_p("doubled_mg", &g_eval_params.doubled_mg, 8, 30, true, 5);
    add_p("doubled_eg", &g_eval_params.doubled_eg, 12, 40, false, 5);
    add_p("connected_mg", &g_eval_params.connected_mg, 6, 25, true, 6);
    add_p("connected_eg", &g_eval_params.connected_eg, 10, 35, false, 6);
    add_p("phalanx_mg", &g_eval_params.phalanx_mg, 6, 25, true, 7);
    add_p("phalanx_eg", &g_eval_params.phalanx_eg, 10, 35, false, 7);
    add_p("isolated_mg", &g_eval_params.isolated_mg, 6, 30, true, 8);
    add_p("isolated_eg", &g_eval_params.isolated_eg, 10, 40, false, 8);
    add_p("backward_mg", &g_eval_params.backward_mg, 6, 30, true, 9);
    add_p("backward_eg", &g_eval_params.backward_eg, 10, 40, false, 9);
    add_p("central_lever_mg", &g_eval_params.central_lever_mg, 6, 30, true, 10);
    add_p("central_lever_eg", &g_eval_params.central_lever_eg, 2, 25, false, 10);
    add_p("pawn_tension_mg", &g_eval_params.pawn_tension_mg, 4, 20, true, 11);
    add_p("pawn_tension_eg", &g_eval_params.pawn_tension_eg, 2, 15, false, 11);

    // Passed Pawns
    const double passed_mg_min[6] = { 0, 5, 15, 30, 50, 90 };
    const double passed_mg_max[6] = { 20, 40, 80, 120, 180, 250 };
    const double passed_eg_min[6] = { 5, 15, 35, 70, 120, 200 };
    const double passed_eg_max[6] = { 30, 60, 120, 180, 260, 380 };
    for (int r = 0; r < 6; ++r) {
        add_p("passed_mg[" + std::to_string(r) + "]", &g_eval_params.passed_mg[r], passed_mg_min[r], passed_mg_max[r], true, 12 + r);
        add_p("passed_eg[" + std::to_string(r) + "]", &g_eval_params.passed_eg[r], passed_eg_min[r], passed_eg_max[r], false, 12 + r);
    }
    add_p("protected_passed_mg", &g_eval_params.protected_passed_mg, 8, 40, true, 18);
    add_p("protected_passed_eg", &g_eval_params.protected_passed_eg, 15, 60, false, 18);

    // Activity & Outposts
    add_p("minor_undeveloped_mg", &g_eval_params.minor_undeveloped_mg, 8, 30, true, 19);
    add_p("bishop_pair_mg", &g_eval_params.bishop_pair_mg, 25, 75, true, 20);
    add_p("bishop_pair_eg", &g_eval_params.bishop_pair_eg, 40, 95, false, 20);
    add_p("knight_outpost_mg", &g_eval_params.knight_outpost_mg, 15, 50, true, 21);
    add_p("knight_outpost_eg", &g_eval_params.knight_outpost_eg, 20, 60, false, 21);
    add_p("central_knight_outpost_mg", &g_eval_params.central_knight_outpost_mg, 6, 25, true, 22);
    add_p("central_knight_outpost_eg", &g_eval_params.central_knight_outpost_eg, 8, 30, false, 22);
    add_p("bishop_outpost_mg", &g_eval_params.bishop_outpost_mg, 10, 40, true, 23);
    add_p("bishop_outpost_eg", &g_eval_params.bishop_outpost_eg, 15, 50, false, 23);
    add_p("rook_7th_mg", &g_eval_params.rook_7th_mg, 15, 50, true, 24);
    add_p("rook_7th_eg", &g_eval_params.rook_7th_eg, 25, 75, false, 24);
    add_p("rook_open_mg", &g_eval_params.rook_open_mg, 12, 40, true, 25);
    add_p("rook_open_eg", &g_eval_params.rook_open_eg, 15, 45, false, 25);
    add_p("rook_semi_open_mg", &g_eval_params.rook_semi_open_mg, 8, 25, true, 26);
    add_p("rook_semi_open_eg", &g_eval_params.rook_semi_open_eg, 10, 30, false, 26);

    // Threats
    add_p("minor_threat_rook_mg", &g_eval_params.minor_threat_rook_mg, 20, 65, true, 27);
    add_p("minor_threat_rook_eg", &g_eval_params.minor_threat_rook_eg, 25, 75, false, 27);
    add_p("minor_threat_queen_mg", &g_eval_params.minor_threat_queen_mg, 25, 85, true, 28);
    add_p("minor_threat_queen_eg", &g_eval_params.minor_threat_queen_eg, 30, 105, false, 28);
    add_p("rook_threat_queen_mg", &g_eval_params.rook_threat_queen_mg, 15, 60, true, 29);
    add_p("rook_threat_queen_eg", &g_eval_params.rook_threat_queen_eg, 20, 70, false, 29);
    add_p("pawn_push_threat_mg", &g_eval_params.pawn_push_threat_mg, 8, 35, true, 30);
    add_p("pawn_push_threat_eg", &g_eval_params.pawn_push_threat_eg, 8, 40, false, 30);
    add_p("pawn_attack_threat_mg", &g_eval_params.pawn_attack_threat_mg, 12, 40, true, 31);
    add_p("pawn_attack_threat_eg", &g_eval_params.pawn_attack_threat_eg, 16, 50, false, 31);

    // Mobility
    add_p("knight_mob_mg", &g_eval_params.knight_mob_mg, 2, 10, true, 32);
    add_p("knight_mob_eg", &g_eval_params.knight_mob_eg, 2, 10, false, 32);
    add_p("bishop_mob_mg", &g_eval_params.bishop_mob_mg, 2, 10, true, 33);
    add_p("bishop_mob_eg", &g_eval_params.bishop_mob_eg, 2, 10, false, 33);
    add_p("rook_mob_mg", &g_eval_params.rook_mob_mg, 1, 8, true, 34);
    add_p("rook_mob_eg", &g_eval_params.rook_mob_eg, 2, 8, false, 34);
    add_p("queen_mob_mg", &g_eval_params.queen_mob_mg, 1, 5, true, 35);
    add_p("queen_mob_eg", &g_eval_params.queen_mob_eg, 1, 5, false, 35);

    // King Safety & Tempo
    add_p("pawn_shield_mg", &g_eval_params.pawn_shield_mg, 8, 30, true, 36);
    add_p("uncastled_king_mg", &g_eval_params.uncastled_king_mg, 50, 150, true, 37);
    add_p("uncastled_king_eg", &g_eval_params.uncastled_king_eg, 0, 30, false, 37);
    add_p("tempo_mg", &g_eval_params.tempo_mg, 4, 25, true, 38);
    add_p("tempo_eg", &g_eval_params.tempo_eg, 0, 10, false, 38);

    const size_t num_params = params.size();
    if (g_eval_params.pawn_mg != 100) {
        std::cerr << "[Error] Pawn MG anchor differs from runtime evaluation.\n";
        return 1;
    }
    std::cout << "[Texel] Extracted " << num_params << " tunable parameters.\n";
    std::cout << "[Texel] Anchor invariant locked: PawnMG = 100 cp (Zero Scale Inflation).\n\n";

    // 2. Load Dataset & Precompute Feature Matrices
    std::cout << "[Texel] Precomputing features from: " << dataset_path << "...\n";
    std::vector<FeatureSample> dataset, validation, test;
    auto t_load_start = std::chrono::high_resolution_clock::now();
    auto load_dataset = [&](const std::string& filename, std::vector<FeatureSample>& destination) {
        std::ifstream input(filename);
        if (!input) throw std::runtime_error("Cannot open dataset: " + filename);
        std::string line;
        size_t line_number = 0;
        while (std::getline(input, line)) {
            ++line_number;
            if (line.empty() || line == "\r") continue;
            const size_t pipe_pos = line.find('|');
            if (pipe_pos == std::string::npos || line.find('|', pipe_pos + 1) != std::string::npos)
                throw std::runtime_error("Malformed dataset line " + std::to_string(line_number));
            const std::string fen = line.substr(0, pipe_pos);
            const std::string target_text = line.substr(pipe_pos + 1);
            size_t consumed = 0;
            const double target = std::stod(target_text, &consumed);
            if (!std::isfinite(target) || target < 0 || target > 1 ||
                target_text.find_first_not_of(" \t\r", consumed) != std::string::npos)
                throw std::runtime_error("Invalid target at line " + std::to_string(line_number));
            Board board;
            if (!FEN::parse(fen, board) || board.king_square(Color::White) == Square::None ||
                board.king_square(Color::Black) == Square::None)
                throw std::runtime_error("Invalid FEN at line " + std::to_string(line_number));
            FeatureSample sample;
            if (parity_only) sample.fen = fen;
            sample.target = target;
            extract_board_features(board, sample.f_mg, sample.f_eg, sample.phase_mg, sample.phase_eg);
            const int runtime_stm = Evaluator::evaluate_fast(board);
            sample.runtime_eval_white = board.side_to_move() == Color::White ? runtime_stm : -runtime_stm;
            sample.base_constant = sample.runtime_eval_white - tunable_score(sample, params, w);
            if (parity_only) sample.unrounded_eval_white = unrounded_runtime_white(board);
            destination.push_back(std::move(sample));
        }
        if (!input.eof() || destination.empty()) throw std::runtime_error("Unreadable or empty dataset: " + filename);
    };
    try {
        load_dataset(dataset_path, dataset);
        if (!parity_only) {
            load_dataset(validation_path, validation);
            load_dataset(test_path, test);
        }
    } catch (const std::exception& error) {
        std::cerr << "[Error] " << error.what() << '\n';
        return 1;
    }

    auto t_load_end = std::chrono::high_resolution_clock::now();
    double load_sec = std::chrono::duration<double>(t_load_end - t_load_start).count();
    std::cout << "[Texel] Successfully cached " << dataset.size() << " samples in " << std::fixed << std::setprecision(2) << load_sec << "s.\n\n";

    const size_t N = dataset.size();
    if (N == 0) return 1;

    // Fast inline evaluator of a sample given weights w
    auto eval_sample = [&](const FeatureSample& s) -> double {
        return s.base_constant + tunable_score(s, params, w);
    };

    // Abort before optimization if the baseline model diverges from the engine.
    double max_parity_error = 0.0;
    for (const auto& sample : dataset)
        max_parity_error = std::max(max_parity_error, std::abs(eval_sample(sample) - sample.runtime_eval_white));
    std::cout << "[Texel] Runtime baseline parity: max error " << std::fixed
              << std::setprecision(9) << max_parity_error << " cp across " << N << " positions.\n";
    if (max_parity_error > 1e-6) {
        std::cerr << "[Error] Feature reconstruction does not match evaluate_fast.\n";
        return 1;
    }
    if (parity_only) {
        // Check local slopes against the real evaluator too. Baseline residual
        // parity alone would hide a feature extractor with incorrect gradients.
        double max_slope_error = 0.0;
        double max_unrounded_error = 0.0;
        std::string worst_parameter;
        for (const auto& sample : dataset) {
            for (size_t p = 0; p < num_params; ++p) {
                const auto& param = params[p];
                const int original = *param.int_ptr;
                const double coefficient = param.is_mg
                    ? sample.phase_mg * sample.f_mg[param.feat_idx]
                    : sample.phase_eg * sample.f_eg[param.feat_idx];
                for (int delta : {-8, -1, 1, 8}) {
                    if (original + delta < param.min_val || original + delta > param.max_val) continue;
                    *param.int_ptr = original + delta;
                    Board perturbed;
                    if (!FEN::parse(sample.fen, perturbed)) {
                        *param.int_ptr = original;
                        std::cerr << "[Error] Parity fixture became invalid.\n";
                        return 1;
                    }
                    Evaluator::reset_incremental_cache();
                    const int score_stm = Evaluator::evaluate_fast(perturbed);
                    const double score_white = perturbed.side_to_move() == Color::White ? score_stm : -score_stm;
                    const double error = std::abs(score_white - (sample.runtime_eval_white + delta * coefficient));
                    max_unrounded_error = std::max(max_unrounded_error, std::abs(
                        unrounded_runtime_white(perturbed) - (sample.unrounded_eval_white + delta * coefficient)));
                    if (error > max_slope_error) {
                        max_slope_error = error;
                        worst_parameter = param.name;
                    }
                }
                *param.int_ptr = original;
            }
        }
        Evaluator::reset_incremental_cache();
        std::cout << "[Texel] Local slope parity: max error " << max_slope_error
                  << " cp (" << worst_parameter << ").\n";
        std::cout << "[Texel] Unrounded production slope error: " << max_unrounded_error << " cp.\n";
        // The two separately truncated side scores can change the residual by
        // less than 2 cp. The unrounded oracle must still match the feature slope.
        return max_slope_error <= 2.01 && max_unrounded_error <= 1e-4 ? 0 : 1;
    }

    // Compute Loss
    auto compute_loss = [&](const std::vector<FeatureSample>& samples) -> double {
        double total_loss = 0.0;
        #pragma omp parallel for reduction(+:total_loss) schedule(static, 2048)
        for (size_t i = 0; i < samples.size(); ++i) {
            double eval_w = eval_sample(samples[i]);
            double pred = sigmoid(eval_w, sigmoid_k);
            double err = samples[i].target - pred;
            total_loss += err * err;
        }
        return total_loss / static_cast<double>(samples.size());
    };

    double initial_loss = compute_loss(dataset);
    const double initial_validation_loss = compute_loss(validation);
    const double initial_test_loss = compute_loss(test);
    std::cout << "[Texel] Validation baseline MSE: " << initial_validation_loss
              << "; samples: train=" << N << " validation=" << validation.size() << " test=" << test.size() << '\n';
    std::cout << "[Texel] Baseline Evaluation Loss (MSE): " << std::fixed << std::setprecision(6) << initial_loss << "\n\n";

    // Store initial values for final diff report
    std::vector<double> initial_w = w;
    std::vector<double> best_w = w;
    double best_validation_loss = initial_validation_loss;
    int best_epoch = 0, stale_epochs = 0, epochs_completed = 0;

    // Adam Optimizer State
    std::vector<double> m(num_params, 0.0);
    std::vector<double> v(num_params, 0.0);
    const double beta1 = 0.90;
    const double beta2 = 0.999;
    const double eps = 1e-8;
    const double k_const = std::log(10.0) / sigmoid_k;

    auto opt_start = std::chrono::high_resolution_clock::now();

    // 3. Optimization Loop (Analytic Gradient Descent)
    for (int epoch = 1; epoch <= epochs; ++epoch) {
        auto ep_start = std::chrono::high_resolution_clock::now();
        std::vector<double> grads(num_params, 0.0);

        #pragma omp parallel
        {
            std::vector<double> local_grads(num_params, 0.0);

            #pragma omp for schedule(static, 2048)
            for (size_t i = 0; i < N; ++i) {
                const auto& s = dataset[i];
                double eval_w = eval_sample(s);
                double pred = sigmoid(eval_w, sigmoid_k);
                double residual = s.target - pred;

                // dL / dE = -2 * residual * sigma'(E)
                // sigma'(E) = k * pred * (1 - pred)
                double dL_dE = -2.0 * residual * (k_const * pred * (1.0 - pred));

                for (size_t p = 0; p < num_params; ++p) {
                    const auto& param = params[p];
                    double dE_dw = param.is_mg ? (s.phase_mg * s.f_mg[param.feat_idx]) : (s.phase_eg * s.f_eg[param.feat_idx]);
                    local_grads[p] += dL_dE * dE_dw;
                }
            }

            #pragma omp critical
            {
                for (size_t p = 0; p < num_params; ++p) {
                    grads[p] += local_grads[p];
                }
            }
        }

        // Normalize gradient by N and apply L2 regularization
        double max_grad = 0.0;
        for (size_t p = 0; p < num_params; ++p) {
            grads[p] /= static_cast<double>(N);
            grads[p] += lambda_reg * (w[p] - initial_w[p]); // Regularize towards the frozen baseline.

            if (std::abs(grads[p]) > max_grad) max_grad = std::abs(grads[p]);

            // Adam update
            m[p] = beta1 * m[p] + (1.0 - beta1) * grads[p];
            v[p] = beta2 * v[p] + (1.0 - beta2) * (grads[p] * grads[p]);

            double m_hat = m[p] / (1.0 - std::pow(beta1, epoch));
            double v_hat = v[p] / (1.0 - std::pow(beta2, epoch));

            double step = (lr * m_hat) / (std::sqrt(v_hat) + eps);
            w[p] -= step;

            // Clamping to sane range
            w[p] = std::clamp(w[p], params[p].min_val, params[p].max_val);
        }

        double loss_now = compute_loss(dataset);
        const double validation_loss = compute_loss(validation);
        epochs_completed = epoch;
        if (std::isfinite(validation_loss) && validation_loss < best_validation_loss - 1e-8) {
            best_validation_loss = validation_loss;
            best_w = w;
            best_epoch = epoch;
            stale_epochs = 0;
        } else ++stale_epochs;
        auto ep_end = std::chrono::high_resolution_clock::now();
        double ep_ms = std::chrono::duration<double, std::milli>(ep_end - ep_start).count();
        double imp_pct = ((initial_loss - loss_now) / initial_loss) * 100.0;

        if (epoch % 5 == 0 || epoch == 1 || epoch == epochs) {
            std::cout << "[Epoch " << std::setw(2) << epoch << "/" << epochs << "] "
                      << "Loss: " << std::fixed << std::setprecision(6) << loss_now << " "
                      << "(Imp: " << std::setprecision(3) << imp_pct << "%) "
                      << "Validation: " << validation_loss << " MaxGrad: " << std::setprecision(6) << max_grad << " "
                      << "in " << std::setprecision(0) << ep_ms << "ms\n" << std::flush;
        }
        if (stale_epochs >= patience) {
            std::cout << "[Texel] Early stop after " << stale_epochs << " epochs without held-out improvement.\n";
            break;
        }
    }

    w = best_w;
    for (double& weight : w) weight = std::round(weight);

    auto opt_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(opt_end - opt_start).count();
    double final_loss = compute_loss(dataset);
    const double final_validation_loss = compute_loss(validation);
    const double final_test_loss = compute_loss(test);
    const bool validation_improved = std::isfinite(final_validation_loss) &&
        final_validation_loss < initial_validation_loss - 1e-8;
    std::ofstream report(report_path);
    if (!report) { std::cerr << "[Error] Cannot create candidate report.\n"; return 1; }
    report << std::setprecision(17) << "{\"schema\":1,\"best_epoch\":" << best_epoch
           << ",\"epochs_completed\":" << epochs_completed << ",\"sigmoid_k\":" << sigmoid_k
           << ",\"train_baseline_mse\":" << initial_loss << ",\"train_candidate_mse\":" << final_loss
           << ",\"validation_baseline_mse\":" << initial_validation_loss
           << ",\"validation_candidate_mse\":" << final_validation_loss
           << ",\"test_baseline_mse\":" << initial_test_loss << ",\"test_candidate_mse\":" << final_test_loss
           << ",\"validation_improved\":" << (validation_improved ? "true" : "false")
           << ",\"exported\":" << (validation_improved ? "true" : "false") << "}\n";
    report.close();
    if (!report) return 1;
    if (!validation_improved) {
        std::cout << "[Texel] Rounded candidate did not improve held-out validation; no parameter file exported.\n";
        return 2;
    }

    std::cout << "\n======================================================\n";
    std::cout << "  TEXEL TUNING COMPLETE in " << std::fixed << std::setprecision(2) << total_sec << "s\n";
    std::cout << "  Initial Loss : " << std::setprecision(6) << initial_loss << "\n";
    std::cout << "  Tuned Loss   : " << std::setprecision(6) << final_loss << "\n";
    std::cout << "  Total Loss Reduction: " << std::setprecision(3) << ((initial_loss - final_loss) / initial_loss) * 100.0 << "%\n";
    std::cout << "======================================================\n\n";

    // 4. Parameter Diff Report
    std::cout << "PARAMETER COMPARISON REPORT:\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(28) << "Parameter Name" 
              << std::right << std::setw(12) << "Old Value" 
              << std::setw(14) << "Tuned Value" 
              << std::setw(12) << "Delta" << "\n";
    std::cout << "----------------------------------------------------------------------\n";

    for (size_t p = 0; p < num_params; ++p) {
        int old_v = static_cast<int>(std::round(initial_w[p]));
        int new_v = static_cast<int>(std::round(w[p]));
        int diff = new_v - old_v;
        std::cout << std::left << std::setw(28) << params[p].name
                  << std::right << std::setw(12) << old_v
                  << std::setw(14) << new_v
                  << std::setw(12) << ((diff >= 0 ? "+" : "") + std::to_string(diff)) << "\n";
        // Also update runtime ptr
        *params[p].int_ptr = new_v;
    }
    std::cout << "----------------------------------------------------------------------\n\n";

    // 5. Export tuned C++ header include
    std::ofstream out(output_path);
    if (out.is_open()) {
        out << "// Automatically generated by Heaven's Gate Classical Texel Tuner\n";
        out << "// Training positions: " << dataset.size() << "; provenance lives in the dataset manifest\n";
        out << "// Candidate only. Requires paired-game validation before promotion.\n";
        out << "// Anchor invariant: PawnMG = 100\n\nnamespace heavensgate {\n\n";
        out << "inline void apply_tuned_eval_params(EvalParams& p) {\n";
        for (size_t p = 0; p < num_params; ++p) {
            int new_v = static_cast<int>(std::round(w[p]));
            out << "    p." << params[p].name << " = " << new_v << ";\n";
        }
        out << "}\n\n} // namespace heavensgate\n";
        out.close();
        if (!out) return 1;
        std::cout << "[Texel] Exported candidate parameters to: " << output_path << '\n';
    } else { std::cerr << "[Error] Cannot create candidate output.\n"; return 1; }

    return 0;
}
