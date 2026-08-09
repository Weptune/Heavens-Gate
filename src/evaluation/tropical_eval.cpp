#include "tropical_eval.hpp"
#include "pst.hpp"
#include "../board/board.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace heavensgate {

TropicalEvaluator::TropicalEvaluator() {
    sectors_.resize(TOTAL_SECTORS);
    initialize_weights(42);
}

TropicalEvaluator& TropicalEvaluator::instance() {
    static TropicalEvaluator inst;
    static bool initialized = false;
    if (!initialized) {
        if (!inst.load_weights("heavensgate_tropical.trm")) {
            inst.initialize_weights(42);
        }
        initialized = true;
    }
    return inst;
}

int TropicalEvaluator::get_king_bucket(Square opp_king_sq, Color us) {
    if (opp_king_sq == Square::None) return (us == Color::White) ? 0 : 5;

    int rank = static_cast<int>(rank_of(opp_king_sq));
    int file = static_cast<int>(file_of(opp_king_sq));

    // Normalize Black's perspective to White's (vertical flip)
    if (us == Color::Black) {
        rank = 7 - rank;
    }

    // Apply horizontal symmetry (mirror files e-h onto files d-a)
    if (file > 3) {
        file = 7 - file;
    }

    // Map (rank, file) into 5 White buckets (0..4)
    int base_bucket = 0;
    if (rank <= 1) {
        base_bucket = (file <= 1) ? 0 : 1; // 0: Flank Back-Rank, 1: Central Back-Rank
    } else if (rank <= 3) {
        base_bucket = (file <= 1) ? 2 : 3; // 2: Flank Midgame, 3: Central Midgame
    } else {
        base_bucket = 4;                   // 4: Enemy Infiltration
    }

    return (us == Color::White) ? base_bucket : (5 + base_bucket);
}

void TropicalEvaluator::initialize_weights(uint32_t /*seed*/) {
    sectors_.resize(TOTAL_SECTORS);
    for (size_t b = 0; b < NUM_KING_BUCKETS; b++) {
        for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
            size_t sec_idx = b * NUM_SECTORS_PER_BUCKET + j;
            auto& sec = sectors_[sec_idx];
            float jf = static_cast<float>(j);
            float bf = static_cast<float>(b);

            // Diverse sector biases per bucket
            sec.b = -10.0f + 0.3f * jf + 0.5f * bf;

            // Material weight = 1.0 (learned inside sectors, bounded [0.8, 1.2])
            sec.w[0]  = 1.0f;
            // Positive positional weights ensuring every sector starts with positive feature guidance
            sec.w[1]  = 0.5f  + 0.2f  * std::fabs(std::sin(jf * 0.5f + bf));   // Fiedler Cohesion
            sec.w[2]  = 0.4f  + 0.15f * std::fabs(std::cos(jf * 0.7f + bf));   // Subgraph Cohesion
            sec.w[3]  = 0.3f  + 0.1f  * std::fabs(std::sin(jf * 1.1f));        // Spectral Gap
            sec.w[4]  = 0.8f  + 0.2f  * std::fabs(std::cos(jf * 0.3f));        // PST
            sec.w[5]  = 1.0f  + 0.3f  * std::fabs(std::sin(jf * 0.9f));        // King Pressure
            sec.w[6]  = 0.7f  + 0.2f  * std::fabs(std::cos(jf * 1.3f));        // Battery Energy
            sec.w[7]  = 0.6f  + 0.15f * std::fabs(std::sin(jf * 0.6f));        // Pawn Cohesion
            sec.w[8]  = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 0.4f));        // Trace Energy
            sec.w[9]  = 0.5f  + 0.2f  * std::fabs(std::sin(jf * 0.8f));        // Mobility
            sec.w[10] = 0.6f  + 0.2f  * std::fabs(std::cos(jf * 1.0f));        // Center Control
            sec.w[11] = 0.3f  + 0.1f  * std::fabs(std::sin(jf * 1.2f));        // Game Phase
            sec.w[12] = 0.5f  + 0.2f  * std::fabs(std::sin(jf * 0.7f));        // King Shield
            sec.w[13] = 0.8f  + 0.2f  * std::fabs(std::cos(jf * 0.5f));        // Passed Pawns
            sec.w[14] = 0.9f  + 0.2f  * std::fabs(std::sin(jf * 0.4f));        // EG Passed Pawns
            sec.w[15] = 0.7f  + 0.2f  * std::fabs(std::cos(jf * 0.8f));        // Attack Ratio

            // Phase 2 Non-Linear Cross-Terms (x16..x21)
            sec.w[16] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 0.9f + bf));   // BatXCenter
            sec.w[17] = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 1.1f + bf));   // FiedXPWN
            sec.w[18] = 0.6f  + 0.2f  * std::fabs(std::sin(jf * 0.6f + bf));   // EG_Mobility
            sec.w[19] = 0.5f  + 0.2f  * std::fabs(std::cos(jf * 0.7f + bf));   // PassXCenter
            sec.w[20] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 1.3f + bf));   // KingXBat
            sec.w[21] = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 0.8f + bf));   // ShldXPWN

            // Phase 3 4-Zone Localized Spatial Fiedler Features (x22..x27)
            sec.w[22] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 0.5f + bf));   // KS_Fiedler
            sec.w[23] = 0.4f  + 0.15f * std::fabs(std::cos(jf * 0.6f + bf));   // QS_Fiedler
            sec.w[24] = 0.5f  + 0.2f  * std::fabs(std::sin(jf * 0.7f + bf));   // CTR_Fiedler
            sec.w[25] = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 0.8f + bf));   // KSFiedXPress
            sec.w[26] = 0.3f  + 0.1f  * std::fabs(std::sin(jf * 0.9f + bf));   // CTRFiedXCenter
            sec.w[27] = 0.4f  + 0.15f * std::fabs(std::cos(jf * 1.0f + bf));   // BR_Fiedler
        }
    }
}

std::array<float, TropicalEvaluator::NUM_FEATURES> TropicalEvaluator::extract_features(const Board& board) const {
    int white_mat = 0, black_mat = 0;
    int white_pst = 0, black_pst = 0;
    int white_passed = 0, black_passed = 0;

    int knights = popcount(board.pieces(make_piece(Color::White, PieceType::Knight))) + popcount(board.pieces(make_piece(Color::Black, PieceType::Knight)));
    int bishops = popcount(board.pieces(make_piece(Color::White, PieceType::Bishop))) + popcount(board.pieces(make_piece(Color::Black, PieceType::Bishop)));
    int rooks   = popcount(board.pieces(make_piece(Color::White, PieceType::Rook)))   + popcount(board.pieces(make_piece(Color::Black, PieceType::Rook)));
    int queens  = popcount(board.pieces(make_piece(Color::White, PieceType::Queen)))  + popcount(board.pieces(make_piece(Color::Black, PieceType::Queen)));
    int raw_phase = knights * 1 + bishops * 1 + rooks * 2 + queens * 4;
    int phase_weight = std::min(raw_phase, 24);

    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        Piece p = board.piece_at(s);
        if (p == Piece::None) continue;

        PieceType pt = piece_type_of(p);
        Color c = color_of(p);

        int val = 100;
        switch (pt) {
            case PieceType::Pawn:   val = 100; break;
            case PieceType::Knight: val = 320; break;
            case PieceType::Bishop: val = 330; break;
            case PieceType::Rook:   val = 500; break;
            case PieceType::Queen:  val = 900; break;
            default: break;
        }

        int mg_pst = PST::get_mg(pt, c, s);
        int eg_pst = PST::get_eg(pt, c, s);
        int interpolated_pst = (mg_pst * phase_weight + eg_pst * (24 - phase_weight)) / 24;

        if (c == Color::White) {
            white_mat += val;
            white_pst += interpolated_pst;
        } else {
            black_mat += val;
            black_pst += interpolated_pst;
        }

        if (pt == PieceType::Pawn) {
            int file = static_cast<int>(file_of(s));
            int rank = static_cast<int>(rank_of(s));
            bool passed = true;
            for (int sq2 = 0; sq2 < 64 && passed; sq2++) {
                Piece p2 = board.piece_at(static_cast<Square>(sq2));
                if (p2 == Piece::None || piece_type_of(p2) != PieceType::Pawn) continue;
                if (color_of(p2) == c) continue;
                int f2 = sq2 % 8;
                int r2 = sq2 / 8;
                if (std::abs(file - f2) <= 1) {
                    if (c == Color::White && r2 > rank) passed = false;
                    if (c == Color::Black && r2 < rank) passed = false;
                }
            }
            if (passed) {
                if (c == Color::White) white_passed++;
                else black_passed++;
            }
        }
    }

    Color us = board.side_to_move();
    int material_diff = (us == Color::White) ? (white_mat - black_mat) : (black_mat - white_mat);
    int pst_diff      = (us == Color::White) ? (white_pst - black_pst) : (black_pst - white_pst);
    int passed_diff   = (us == Color::White) ? (white_passed - black_passed) : (black_passed - white_passed);

    // Lazy Spectral Evaluation: skip expensive Laplacian eigensolver when material is decisive (> 1200 cp)
    if (std::abs(material_diff) > 1200) {
        std::array<float, NUM_FEATURES> x{};
        x[0]  = static_cast<float>(material_diff) / 10.0f;
        x[4]  = static_cast<float>(pst_diff) / 10.0f;
        x[13] = static_cast<float>(passed_diff) * 4.0f;
        return x;
    }

    SpectralFeatures feat = SpectralGraph::compute_spectrum(board);

    float our_shield   = (us == Color::White) ? feat.king_shield_us : feat.king_shield_them;
    float their_shield = (us == Color::White) ? feat.king_shield_them : feat.king_shield_us;
    float our_pressure = (us == Color::White) ? feat.king_pressure_us : feat.king_pressure_them;

    // Feature Scale Normalization: All features scaled into a balanced dynamic band (~[-50, +50])
    // so gradient magnitudes (dLoss/dw_i = error * x_i) are equal across material and positional features.
    std::array<float, NUM_FEATURES> x;
    x[0]  = static_cast<float>(material_diff) / 10.0f;                       // Material: 100 cp -> 10.0
    x[1]  = (feat.fiedler_us - feat.fiedler_them) * 5.0f;                     // Fiedler cohesion
    x[2]  = (feat.cohesion_us - feat.cohesion_them) * 2.0f;                    // Subgraph cohesion
    x[3]  = feat.spectral_gap * 2.0f;                                          // Spectral gap
    x[4]  = static_cast<float>(pst_diff) / 10.0f;                             // PST
    x[5]  = (feat.king_pressure_us - feat.king_pressure_them) * 3.0f;          // King pressure
    x[6]  = (feat.battery_energy_us - feat.battery_energy_them) * 3.0f;        // Battery energy
    x[7]  = (feat.pawn_cohesion_us - feat.pawn_cohesion_them) * 3.0f;          // Pawn cohesion
    x[8]  = (feat.laplacian_trace - 30.0f) / 5.0f;                             // Relative trace energy
    x[9]  = (feat.mobility_us - feat.mobility_them) * 2.0f;                    // Mobility
    x[10] = (feat.center_control_us - feat.center_control_them) * 3.0f;        // Center control
    x[11] = feat.game_phase * 5.0f;                                            // Game phase
    x[12] = (feat.king_shield_us - feat.king_shield_them) * 3.0f;              // King shield
    x[13] = static_cast<float>(passed_diff) * 4.0f;                           // Passed pawns
    x[14] = static_cast<float>(passed_diff) * (1.0f - feat.game_phase) * 5.0f; // EG Passed pawns
    x[15] = (feat.king_pressure_us / (feat.king_pressure_them + 1.0f)) * 5.0f; // Attack Ratio

    // Phase 2 Residual Skip-Connection Cross-Terms (x16..x21)
    float pos_center   = std::max(0.0f, x[10]);
    float pos_pawncoh  = std::max(0.0f, x[7]);
    float pos_battery  = std::max(0.0f, x[6]);

    x[16] = x[6]  * (pos_center / 10.0f);                                       // BatXCenter Residual Boost
    x[17] = x[1]  * (pos_pawncoh / 10.0f);                                      // FiedXPWN Residual Boost
    x[18] = x[9]  * (1.0f - feat.game_phase);                                   // EG_Mobility Residual Boost
    x[19] = x[13] * (pos_center / 10.0f);                                       // PassXCenter Residual Boost
    x[20] = x[5]  * (pos_battery / 10.0f);                                      // KingXBat Residual Boost
    x[21] = x[12] * (pos_pawncoh / 10.0f);                                      // ShldXPWN Residual Boost

    return x;
}

int TropicalEvaluator::evaluate(const Board& board) const {
    auto res = evaluate_detailed(board);
    return res.score + 15; // +15 cp Tempo Bonus for side to move!
}

TropicalEvaluator::EvalResult TropicalEvaluator::evaluate_detailed(const Board& board) const {
    Color us = board.side_to_move();
    Color them = ~us;
    Piece opp_king = (them == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
    Square opp_king_sq = board.pieces(opp_king) ? lsb(board.pieces(opp_king)) : Square::None;

    int bucket = get_king_bucket(opp_king_sq, us);
    size_t base_sec_idx = static_cast<size_t>(bucket) * NUM_SECTORS_PER_BUCKET;

    std::array<float, NUM_FEATURES> x = extract_features(board);

    std::array<float, NUM_SECTORS_PER_BUCKET> sector_vals;
    float max_val = -1e9f;
    size_t winning_sector = 0;

    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        const auto& sec = sectors_[base_sec_idx + j];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) {
            val += sec.w[i] * x[i];
        }
        sector_vals[j] = val;
        if (val > max_val) {
            max_val = val;
            winning_sector = j;
        }
    }

    // Smooth Log-Sum-Exp Tropical Semiring Evaluation (Centered zero offset)
    float sum_exp = 0.0f;
    std::array<float, NUM_SECTORS_PER_BUCKET> softmax_probs{};
    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        float exp_val = std::exp((sector_vals[j] - max_val) / SMOOTH_TAU);
        softmax_probs[j] = exp_val;
        sum_exp += exp_val;
    }
    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        softmax_probs[j] /= sum_exp;
    }

    // Subtract tau * log(NUM_SECTORS_PER_BUCKET) so baseline evaluation centered at max_val
    float smooth_eval_units = max_val + SMOOTH_TAU * (std::log(sum_exp) - std::log(static_cast<float>(NUM_SECTORS_PER_BUCKET)));
    float smooth_eval = smooth_eval_units * 10.0f;
    int score = static_cast<int>(std::max(-30000.0f, std::min(30000.0f, smooth_eval)));

    return {score, bucket, base_sec_idx + winning_sector, softmax_probs};
}

size_t TropicalEvaluator::get_king_bucket(Square opp_king_sq) {
    if (opp_king_sq == Square::None) return 0;
    int rank = static_cast<int>(rank_of(opp_king_sq));
    int file = static_cast<int>(file_of(opp_king_sq));
    if (file >= 4) file = 7 - file;
    int b = (rank / 2) * 2 + (file / 2);
    return static_cast<size_t>(std::max(0, std::min(9, b)));
}

TropicalEvaluator::EvalResult TropicalEvaluator::evaluate_detailed_from_features(const std::array<float, NUM_FEATURES>& x, size_t bucket) const {
    size_t base_sec_idx = bucket * NUM_SECTORS_PER_BUCKET;

    std::array<float, NUM_SECTORS_PER_BUCKET> sector_vals;
    float max_val = -1e9f;
    size_t winning_sector = 0;

    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        const auto& sec = sectors_[base_sec_idx + j];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) {
            val += sec.w[i] * x[i];
        }
        sector_vals[j] = val;
        if (val > max_val) {
            max_val = val;
            winning_sector = j;
        }
    }

    float sum_exp = 0.0f;
    std::array<float, NUM_SECTORS_PER_BUCKET> softmax_probs{};
    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        float exp_val = std::exp((sector_vals[j] - max_val) / SMOOTH_TAU);
        softmax_probs[j] = exp_val;
        sum_exp += exp_val;
    }
    for (size_t j = 0; j < NUM_SECTORS_PER_BUCKET; j++) {
        softmax_probs[j] /= sum_exp;
    }

    float smooth_eval_units = max_val + SMOOTH_TAU * (std::log(sum_exp) - std::log(static_cast<float>(NUM_SECTORS_PER_BUCKET)));
    float smooth_eval = smooth_eval_units * 10.0f;
    int score = static_cast<int>(std::max(-30000.0f, std::min(30000.0f, smooth_eval)));

    return {score, static_cast<int>(bucket), base_sec_idx + winning_sector, softmax_probs};
}

std::pair<int, size_t> TropicalEvaluator::evaluate_with_sector(const Board& board) const {
    auto res = evaluate_detailed(board);
    return {res.score, res.winning_sector};
}

bool TropicalEvaluator::save_weights(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    uint32_t num_sec = static_cast<uint32_t>(TOTAL_SECTORS);
    uint32_t num_feat = static_cast<uint32_t>(NUM_FEATURES);

    out.write(reinterpret_cast<const char*>(&num_sec), sizeof(num_sec));
    out.write(reinterpret_cast<const char*>(&num_feat), sizeof(num_feat));

    for (const auto& sec : sectors_) {
        out.write(reinterpret_cast<const char*>(sec.w.data()), NUM_FEATURES * sizeof(float));
        out.write(reinterpret_cast<const char*>(&sec.b), sizeof(sec.b));
    }

    return out.good();
}

bool TropicalEvaluator::load_weights(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;

    uint32_t num_sec = 0, num_feat = 0;
    in.read(reinterpret_cast<char*>(&num_sec), sizeof(num_sec));
    in.read(reinterpret_cast<char*>(&num_feat), sizeof(num_feat));

    if (num_sec != TOTAL_SECTORS || num_feat != NUM_FEATURES) {
        in.close();
        std::remove(path.c_str()); // Auto-clean incompatible weights from old phase
        return false;
    }

    sectors_.resize(TOTAL_SECTORS);
    for (auto& sec : sectors_) {
        in.read(reinterpret_cast<char*>(sec.w.data()), NUM_FEATURES * sizeof(float));
        in.read(reinterpret_cast<char*>(&sec.b), sizeof(sec.b));
    }

    bool ok = in.good();
    in.close();
    if (!ok) {
        std::remove(path.c_str());
        return false;
    }
    return true;
}

} // namespace heavensgate
