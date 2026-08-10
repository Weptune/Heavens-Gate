#include "tropical_eval.hpp"
#include "pst.hpp"
#include "../board/board.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace heavensgate {

TropicalEvaluator::TropicalEvaluator() {
    sectors_t1_.resize(TOTAL_SECTORS_T1);
    sectors_t2_.resize(TOTAL_SECTORS_T2);
    initialize_weights(42);
}

TropicalEvaluator& TropicalEvaluator::instance() {
    static TropicalEvaluator inst = []() {
        TropicalEvaluator obj;
        if (!obj.load_weights("heavensgate_tropical.trm")) {
            obj.initialize_weights(42);
        }
        return obj;
    }();
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
    sectors_t1_.resize(TOTAL_SECTORS_T1);
    sectors_t2_.resize(TOTAL_SECTORS_T2);

    // Initialize T1 (Advantage / Positional Dominance Surface)
    for (size_t b = 0; b < NUM_KING_BUCKETS; b++) {
        for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
            size_t sec_idx = b * SECTORS_PER_SURFACE + j;
            auto& sec = sectors_t1_[sec_idx];
            float jf = static_cast<float>(j);
            float bf = static_cast<float>(b);

            sec.b = -5.0f + 0.3f * jf + 0.5f * bf;

            sec.w[0]  = 1.0f; // Material
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
            sec.w[16] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 0.9f + bf));   // BatXCenter
            sec.w[17] = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 1.1f + bf));   // FiedXPWN
            sec.w[18] = 0.6f  + 0.2f  * std::fabs(std::sin(jf * 0.6f + bf));   // EG_Mobility
            sec.w[19] = 0.5f  + 0.2f  * std::fabs(std::cos(jf * 0.7f + bf));   // PassXCenter
            sec.w[20] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 1.3f + bf));   // KingXBat
            sec.w[21] = 0.3f  + 0.1f  * std::fabs(std::cos(jf * 0.8f + bf));   // ShldXPWN
            sec.w[22] = 0.4f  + 0.15f * std::fabs(std::sin(jf * 0.9f));        // Cheb T2 Us
            sec.w[23] = 0.3f  + 0.10f * std::fabs(std::cos(jf * 1.1f));        // Cheb T2 Them
            sec.w[24] = 0.5f  + 0.20f * std::fabs(std::sin(jf * 1.3f));        // Cheb King Threat
        }
    }

    // Initialize T2 (Vulnerability / King Exposure Surface)
    for (size_t b = 0; b < NUM_KING_BUCKETS; b++) {
        for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
            size_t sec_idx = b * SECTORS_PER_SURFACE + j;
            auto& sec = sectors_t2_[sec_idx];
            float jf = static_cast<float>(j);
            float bf = static_cast<float>(b);

            sec.b = -15.0f + 0.2f * jf + 0.3f * bf;

            sec.w[0]  = 0.0f; // Material handled in T1
            sec.w[1]  = 0.2f  + 0.1f  * std::fabs(std::sin(jf * 0.4f + bf));   // Fiedler Vulnerability
            sec.w[2]  = 0.15f + 0.08f * std::fabs(std::cos(jf * 0.6f));        // Subgraph Cohesion
            sec.w[3]  = 0.1f  + 0.05f * std::fabs(std::sin(jf * 1.0f));        // Gap
            sec.w[4]  = 0.2f  + 0.1f  * std::fabs(std::cos(jf * 0.2f));        // PST
            sec.w[5]  = 0.4f  + 0.15f * std::fabs(std::sin(jf * 0.8f));        // Enemy King Press
            sec.w[6]  = 0.25f + 0.1f  * std::fabs(std::cos(jf * 1.2f));        // Battery Exposure
            sec.w[7]  = 0.2f  + 0.1f  * std::fabs(std::sin(jf * 0.5f));        // Pawn Weakness
            sec.w[8]  = 0.1f  + 0.05f * std::fabs(std::cos(jf * 0.3f));        // Trace
            sec.w[9]  = 0.15f + 0.08f * std::fabs(std::sin(jf * 0.7f));        // Opponent Mobility
            sec.w[10] = 0.2f  + 0.1f  * std::fabs(std::cos(jf * 0.9f));        // Center Loss
            sec.w[11] = 0.1f  + 0.05f * std::fabs(std::sin(jf * 1.1f));        // Phase
            sec.w[12] = 0.3f  + 0.12f * std::fabs(std::sin(jf * 0.6f));        // Shield Breach
            sec.w[13] = 0.2f  + 0.1f  * std::fabs(std::cos(jf * 0.4f));        // Opponent Passed
            sec.w[14] = 0.25f + 0.1f  * std::fabs(std::sin(jf * 0.3f));        // Opponent EG Passed
            sec.w[15] = 0.2f  + 0.1f  * std::fabs(std::cos(jf * 0.7f));        // Opponent Attack
            sec.w[16] = 0.15f + 0.08f * std::fabs(std::sin(jf * 0.8f));
            sec.w[17] = 0.1f  + 0.05f * std::fabs(std::cos(jf * 1.0f));
            sec.w[18] = 0.2f  + 0.1f  * std::fabs(std::sin(jf * 0.5f));
            sec.w[19] = 0.15f + 0.08f * std::fabs(std::cos(jf * 0.6f));
            sec.w[20] = 0.2f  + 0.1f  * std::fabs(std::sin(jf * 1.2f));
            sec.w[21] = 0.1f  + 0.05f * std::fabs(std::cos(jf * 0.7f));
            sec.w[22] = 0.15f + 0.08f * std::fabs(std::sin(jf * 0.9f));
            sec.w[23] = 0.10f + 0.05f * std::fabs(std::cos(jf * 1.1f));
            sec.w[24] = 0.20f + 0.10f * std::fabs(std::sin(jf * 1.3f));
        }
    }
}

std::array<float, TropicalEvaluator::NUM_FEATURES> TropicalEvaluator::extract_features(const Board& board) const {
    int white_mat = 0, black_mat = 0;
    int white_pst = 0, black_pst = 0;
    int white_passed = 0, black_passed = 0;
    int white_king_shield = 0, black_king_shield = 0;

    Square w_king_sq = board.king_square(Color::White);
    Square b_king_sq = board.king_square(Color::Black);

    auto rank_of_sq = [](Square sq) { return static_cast<int>(sq) / 8; };
    auto file_of_sq = [](Square sq) { return static_cast<int>(sq) % 8; };

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            Square sq = static_cast<Square>(r * 8 + f);
            Piece p = board.piece_at(sq);
            if (p == Piece::None) continue;

            PieceType pt = piece_type_of(p);
            Color pc = color_of(p);

            int val = 0;
            switch (pt) {
                case PieceType::Pawn:   val = 100; break;
                case PieceType::Knight: val = 320; break;
                case PieceType::Bishop: val = 330; break;
                case PieceType::Rook:   val = 500; break;
                case PieceType::Queen:  val = 900; break;
                default: break;
            }

            int pst_val = PST::get_pst_value(pt, pc, sq, false);

            if (pc == Color::White) {
                white_mat += val;
                white_pst += pst_val;
                if (pt == PieceType::Pawn) {
                    bool passed = true;
                    for (int check_f = std::max(0, f - 1); check_f <= std::min(7, f + 1); check_f++) {
                        for (int check_r = r + 1; check_r < 8; check_r++) {
                            Piece enemy_p = board.piece_at(static_cast<Square>(check_r * 8 + check_f));
                            if (enemy_p == Piece::BlackPawn) { passed = false; break; }
                        }
                        if (!passed) break;
                    }
                    if (passed) white_passed++;
                }
                if (w_king_sq != Square::None) {
                    int k_rank = rank_of_sq(w_king_sq);
                    int k_file = file_of_sq(w_king_sq);
                    if (std::abs(r - k_rank) <= 1 && std::abs(f - k_file) <= 1 && pt == PieceType::Pawn) {
                        white_king_shield++;
                    }
                }
            } else {
                black_mat += val;
                black_pst += pst_val;
                if (pt == PieceType::Pawn) {
                    bool passed = true;
                    for (int check_f = std::max(0, f - 1); check_f <= std::min(7, f + 1); check_f++) {
                        for (int check_r = r - 1; check_r >= 0; check_r--) {
                            Piece enemy_p = board.piece_at(static_cast<Square>(check_r * 8 + check_f));
                            if (enemy_p == Piece::WhitePawn) { passed = false; break; }
                        }
                        if (!passed) break;
                    }
                    if (passed) black_passed++;
                }
                if (b_king_sq != Square::None) {
                    int k_rank = rank_of_sq(b_king_sq);
                    int k_file = file_of_sq(b_king_sq);
                    if (std::abs(r - k_rank) <= 1 && std::abs(f - k_file) <= 1 && pt == PieceType::Pawn) {
                        black_king_shield++;
                    }
                }
            }
        }
    }

    Color stm = board.side_to_move();

    SpectralFeatures feat = SpectralGraph::compute_spectrum(board);

    std::array<float, TropicalEvaluator::NUM_FEATURES> x{};

    float mat_us   = (stm == Color::White) ? static_cast<float>(white_mat) : static_cast<float>(black_mat);
    float mat_them = (stm == Color::White) ? static_cast<float>(black_mat) : static_cast<float>(white_mat);
    x[0] = (mat_us - mat_them) / 100.0f;

    x[1] = feat.fiedler_val * 10.0f;
    x[2] = (feat.cohesion_us - feat.cohesion_them) * 0.1f;
    x[3] = feat.spectral_gap * 10.0f;

    float pst_us   = (stm == Color::White) ? static_cast<float>(white_pst) : static_cast<float>(black_pst);
    float pst_them = (stm == Color::White) ? static_cast<float>(black_pst) : static_cast<float>(white_pst);
    x[4] = (pst_us - pst_them) / 100.0f;

    x[5] = (feat.king_pressure_us - feat.king_pressure_them) * 0.2f;
    x[6] = (feat.battery_energy_us - feat.battery_energy_them) * 0.2f;
    x[7] = (feat.pawn_cohesion_us - feat.pawn_cohesion_them) * 0.2f;
    x[8] = feat.laplacian_trace * 0.05f;

    x[9]  = (feat.mobility_us - feat.mobility_them) * 0.1f;
    x[10] = (feat.center_control_us - feat.center_control_them) * 0.3f;
    x[11] = feat.game_phase * 10.0f;

    float shld_us   = (stm == Color::White) ? static_cast<float>(white_king_shield) : static_cast<float>(black_king_shield);
    float shld_them = (stm == Color::White) ? static_cast<float>(black_king_shield) : static_cast<float>(white_king_shield);
    x[12] = (shld_us - shld_them) * 0.5f;

    float pass_us   = (stm == Color::White) ? static_cast<float>(white_passed) : static_cast<float>(black_passed);
    float pass_them = (stm == Color::White) ? static_cast<float>(black_passed) : static_cast<float>(white_passed);
    x[13] = (pass_us - pass_them) * 0.8f;
    x[14] = x[13] * (1.0f - feat.game_phase);

    float att_us   = (feat.king_pressure_us + feat.battery_energy_us * 0.5f);
    float att_them = (feat.king_pressure_them + feat.battery_energy_them * 0.5f);
    x[15] = (att_us - att_them) * 0.2f;

    float pos_center   = std::max(0.0f, x[10]);
    float pos_pawncoh  = std::max(0.0f, x[7]);
    float pos_battery  = std::max(0.0f, x[6]);

    x[16] = x[6]  * (pos_center / 10.0f);
    x[17] = x[1]  * (pos_pawncoh / 10.0f);
    x[18] = x[9]  * (1.0f - feat.game_phase);
    x[19] = x[13] * (pos_center / 10.0f);
    x[20] = x[5]  * (pos_battery / 10.0f);
    x[21] = x[12] * (pos_pawncoh / 10.0f);

    // Phase 5: Chebyshev 2-Hop Graph Convolutions (x22..x24)
    x[22] = feat.chebyshev_t2_us;
    x[23] = feat.chebyshev_t2_them;
    x[24] = feat.chebyshev_king_threat;

    return x;
}

int TropicalEvaluator::evaluate(const Board& board) const {
    auto res = evaluate_detailed(board);
    return res.score + 15; // +15 cp Tempo Bonus
}

TropicalEvaluator::EvalResult TropicalEvaluator::evaluate_detailed(const Board& board) const {
    Square opp_king_sq = board.king_square(~board.side_to_move());
    int bucket = get_king_bucket(opp_king_sq, board.side_to_move());
    return evaluate_detailed_from_features(extract_features(board), static_cast<size_t>(bucket));
}

TropicalEvaluator::EvalResult TropicalEvaluator::evaluate_detailed_from_features(const std::array<float, NUM_FEATURES>& x, size_t bucket) const {
    size_t base_sec_idx = bucket * SECTORS_PER_SURFACE;

    // 1. Evaluate T1 (Advantage Surface)
    std::array<float, SECTORS_PER_SURFACE> t1_vals;
    float max_t1 = -1e9f;
    size_t win_t1 = 0;
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        const auto& sec = sectors_t1_[base_sec_idx + j];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) val += sec.w[i] * x[i];
        t1_vals[j] = val;
        if (val > max_t1) { max_t1 = val; win_t1 = j; }
    }

    float sum_exp_t1 = 0.0f;
    std::array<float, SECTORS_PER_SURFACE> softmax_t1{};
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        float exp_val = std::exp((t1_vals[j] - max_t1) / SMOOTH_TAU);
        softmax_t1[j] = exp_val;
        sum_exp_t1 += exp_val;
    }
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) softmax_t1[j] /= sum_exp_t1;
    float t1_smooth = max_t1 + SMOOTH_TAU * (std::log(sum_exp_t1) - std::log(static_cast<float>(SECTORS_PER_SURFACE)));

    // 2. Evaluate T2 (Vulnerability Surface)
    std::array<float, SECTORS_PER_SURFACE> t2_vals;
    float max_t2 = -1e9f;
    size_t win_t2 = 0;
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        const auto& sec = sectors_t2_[base_sec_idx + j];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) val += sec.w[i] * x[i];
        t2_vals[j] = val;
        if (val > max_t2) { max_t2 = val; win_t2 = j; }
    }

    float sum_exp_t2 = 0.0f;
    std::array<float, SECTORS_PER_SURFACE> softmax_t2{};
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        float exp_val = std::exp((t2_vals[j] - max_t2) / SMOOTH_TAU);
        softmax_t2[j] = exp_val;
        sum_exp_t2 += exp_val;
    }
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) softmax_t2[j] /= sum_exp_t2;
    float t2_smooth = max_t2 + SMOOTH_TAU * (std::log(sum_exp_t2) - std::log(static_cast<float>(SECTORS_PER_SURFACE)));

    // 3. Phase 4 Tropical Rational Function: f(x) = T1(x) - T2(x)
    float rational_eval_units = t1_smooth - t2_smooth;
    float smooth_eval = rational_eval_units * 10.0f;
    int score = static_cast<int>(std::max(-30000.0f, std::min(30000.0f, smooth_eval)));

    return {score, static_cast<int>(bucket), t1_smooth, t2_smooth, base_sec_idx + win_t1, base_sec_idx + win_t2, softmax_t1, softmax_t2};
}

bool TropicalEvaluator::save_weights(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    uint32_t num_sec = static_cast<uint32_t>(TOTAL_SECTORS);
    uint32_t num_feat = static_cast<uint32_t>(NUM_FEATURES);

    out.write(reinterpret_cast<const char*>(&num_sec), sizeof(num_sec));
    out.write(reinterpret_cast<const char*>(&num_feat), sizeof(num_feat));

    // Save T1 (Advantage sectors)
    for (const auto& sec : sectors_t1_) {
        out.write(reinterpret_cast<const char*>(sec.w.data()), NUM_FEATURES * sizeof(float));
        out.write(reinterpret_cast<const char*>(&sec.b), sizeof(sec.b));
    }

    // Save T2 (Vulnerability sectors)
    for (const auto& sec : sectors_t2_) {
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
        std::remove(path.c_str());
        return false;
    }

    sectors_t1_.resize(TOTAL_SECTORS_T1);
    for (auto& sec : sectors_t1_) {
        in.read(reinterpret_cast<char*>(sec.w.data()), NUM_FEATURES * sizeof(float));
        in.read(reinterpret_cast<char*>(&sec.b), sizeof(sec.b));
    }

    sectors_t2_.resize(TOTAL_SECTORS_T2);
    for (auto& sec : sectors_t2_) {
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
