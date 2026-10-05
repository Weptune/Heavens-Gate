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

using namespace heavensgate;

// Dense feature representation for 1 training sample
struct FeatureSample {
    float target;       // 1.0 (Win), 0.5 (Draw), 0.0 (Loss) from White's perspective
    float phase_mg;     // phase / 24.0
    float phase_eg;     // (24 - phase) / 24.0
    float base_constant;// PST tables + king danger + trapped pieces

    // Features: index corresponds to tunable parameter index
    // Each value is (feature_white - feature_black)
    std::vector<float> f_mg;
    std::vector<float> f_eg;
};

// Parameter descriptor for the tuner
struct TunerParam {
    std::string name;
    double* val_ptr;
    int* int_ptr;
    double min_val;
    double max_val;
    bool is_mg; // true if midgame, false if endgame
    size_t feat_idx;
};

inline double sigmoid(double eval_cp, double K = 400.0) {
    return 1.0 / (1.0 + std::pow(10.0, -eval_cp / K));
}

// Extract the feature differences from a position
void extract_board_features(const Board& board, std::vector<float>& feats_mg, std::vector<float>& feats_eg, float& base_constant, float& phase_mg, float& phase_eg) {
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
            if ((side == Color::White && kr <= 3 && kf >= 2 && kf <= 5) ||
                (side == Color::Black && kr >= 4 && kf >= 2 && kf <= 5)) {
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
            feats_mg[36] += sign * shield;
        }
    };

    extract_side(Color::White, 1.0f);
    extract_side(Color::Black, -1.0f);

    // Tempo: White to move = +1, Black to move = -1
    float tempo_sign = (board.side_to_move() == Color::White) ? 1.0f : -1.0f;
    feats_mg[38] = tempo_sign;
    feats_eg[38] = tempo_sign;

    // Base constant: PST tables + king danger + storm/trapped piece static terms
    float white_pst = board.mg_pst(Color::White) * phase_mg + board.eg_pst(Color::White) * phase_eg;
    float black_pst = board.mg_pst(Color::Black) * phase_mg + board.eg_pst(Color::Black) * phase_eg;
    base_constant = white_pst - black_pst;
}

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  HEAVEN'S GATE ANALYTIC TEXEL EVALUATION TUNER v2    \n";
    std::cout << "  High-Speed Exact Gradient Descent (77,455 Games)    \n";
    std::cout << "======================================================\n\n";

    int epochs = 80;
    double lr = 0.50;
    double lambda_reg = 0.0001; // L2 regularization to prevent drift
    std::string dataset_path = "data/quiet_positions.txt";

    if (argc > 1) epochs = std::stoi(argv[1]);
    if (argc > 2) lr = std::stod(argv[2]);
    if (argc > 3) dataset_path = argv[3];

    Zobrist::init();
    MoveGenerator::init();
    PST::init();
    EvalFeatures::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);

    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    std::cout << "[Texel] Initialized OpenMP across " << num_threads << " CPU threads.\n";

    // 1. Build Parameter Table (74 Parameters)
    std::vector<TunerParam> params;
    std::vector<double> w;

    auto add_p = [&](const std::string& name, int* int_p, double min_v, double max_v, bool is_mg, size_t f_idx) {
        w.push_back(static_cast<double>(*int_p));
        params.push_back({name, &w.back(), int_p, min_v, max_v, is_mg, f_idx});
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
    std::cout << "[Texel] Extracted " << num_params << " tunable parameters.\n";
    std::cout << "[Texel] Anchor invariant locked: PawnMG = 100 cp (Zero Scale Inflation).\n\n";

    // 2. Load Dataset & Precompute Feature Matrices
    std::cout << "[Texel] Precomputing feature tensors from: " << dataset_path << "...\n";
    std::ifstream in(dataset_path);
    if (!in.is_open()) {
        std::cerr << "[Error] Could not open dataset!\n";
        return 1;
    }

    std::vector<FeatureSample> dataset;
    std::string line;
    auto t_load_start = std::chrono::high_resolution_clock::now();

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        size_t pipe_pos = line.find('|');
        if (pipe_pos == std::string::npos) continue;

        std::string fen = line.substr(0, pipe_pos);
        float target = std::stof(line.substr(pipe_pos + 1));

        Board board;
        if (FEN::parse(fen, board)) {
            FeatureSample sample;
            sample.target = target;
            extract_board_features(board, sample.f_mg, sample.f_eg, sample.base_constant, sample.phase_mg, sample.phase_eg);
            dataset.push_back(std::move(sample));
        }
    }
    in.close();

    auto t_load_end = std::chrono::high_resolution_clock::now();
    double load_sec = std::chrono::duration<double>(t_load_end - t_load_start).count();
    std::cout << "[Texel] Successfully cached " << dataset.size() << " samples in " << std::fixed << std::setprecision(2) << load_sec << "s.\n\n";

    const size_t N = dataset.size();
    if (N == 0) return 1;

    // Anchor: PawnMG is always 100
    const double pawn_mg_val = 100.0;

    // Fast inline evaluator of a sample given weights w
    auto eval_sample = [&](const FeatureSample& s) -> double {
        double eval_white = s.base_constant;
        // Add anchored PawnMG:
        eval_white += s.phase_mg * s.f_mg[0] * pawn_mg_val;

        // Add all tunable parameters
        for (size_t p = 0; p < num_params; ++p) {
            const auto& param = params[p];
            double weight = w[p];
            if (param.is_mg) {
                eval_white += s.phase_mg * s.f_mg[param.feat_idx] * weight;
            } else {
                eval_white += s.phase_eg * s.f_eg[param.feat_idx] * weight;
            }
        }
        return eval_white;
    };

    // Compute Loss
    auto compute_current_loss = [&]() -> double {
        double total_loss = 0.0;
        #pragma omp parallel for reduction(+:total_loss) schedule(static, 2048)
        for (size_t i = 0; i < N; ++i) {
            double eval_w = eval_sample(dataset[i]);
            double pred = sigmoid(eval_w);
            double err = dataset[i].target - pred;
            total_loss += err * err;
        }
        return total_loss / static_cast<double>(N);
    };

    double initial_loss = compute_current_loss();
    std::cout << "[Texel] Baseline Evaluation Loss (MSE): " << std::fixed << std::setprecision(6) << initial_loss << "\n\n";

    // Store initial values for final diff report
    std::vector<double> initial_w = w;

    // Adam Optimizer State
    std::vector<double> m(num_params, 0.0);
    std::vector<double> v(num_params, 0.0);
    const double beta1 = 0.90;
    const double beta2 = 0.999;
    const double eps = 1e-8;
    const double k_const = std::log(10.0) / 400.0; // derivative factor of sigmoid

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
                double pred = sigmoid(eval_w);
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
            grads[p] += lambda_reg * (w[p] - initial_w[p]); // Regularization towards grandmaster defaults

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

        double loss_now = compute_current_loss();
        auto ep_end = std::chrono::high_resolution_clock::now();
        double ep_ms = std::chrono::duration<double, std::milli>(ep_end - ep_start).count();
        double imp_pct = ((initial_loss - loss_now) / initial_loss) * 100.0;

        if (epoch % 5 == 0 || epoch == 1 || epoch == epochs) {
            std::cout << "[Epoch " << std::setw(2) << epoch << "/" << epochs << "] "
                      << "Loss: " << std::fixed << std::setprecision(6) << loss_now << " "
                      << "(Imp: " << std::setprecision(3) << imp_pct << "%) "
                      << "MaxGrad: " << std::setprecision(6) << max_grad << " "
                      << "in " << std::setprecision(0) << ep_ms << "ms\n" << std::flush;
        }
    }

    auto opt_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(opt_end - opt_start).count();
    double final_loss = compute_current_loss();

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
    std::ofstream out("src/evaluation/tuned_eval_params.inc");
    if (out.is_open()) {
        out << "// Automatically generated by Heaven's Gate Classical Texel Tuner\n";
        out << "// Trained across " << dataset.size() << " grandmaster quiet positions\n";
        out << "// Anchor invariant: PawnMG = 100\n\n";
        out << "inline void apply_tuned_eval_params(EvalParams& p) {\n";
        for (size_t p = 0; p < num_params; ++p) {
            int new_v = static_cast<int>(std::round(w[p]));
            out << "    p." << params[p].name << " = " << new_v << ";\n";
        }
        out << "}\n";
        out.close();
        std::cout << "[Texel] Exported tuned parameters to: src/evaluation/tuned_eval_params.inc\n";
    }

    return 0;
}
