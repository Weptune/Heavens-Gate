#include "tropical_eval.hpp"
#include "pst.hpp"
#include "../board/board.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace heavensgate {

TropicalEvaluator::TropicalEvaluator() {
    sectors_.resize(NUM_SECTORS);
    initialize_weights(42);
}

TropicalEvaluator& TropicalEvaluator::instance() {
    static TropicalEvaluator inst;
    static bool initialized = false;
    if (!initialized) {
        inst.load_weights("heavensgate_tropical.trm");
        initialized = true;
    }
    return inst;
}

void TropicalEvaluator::initialize_weights(uint32_t /*seed*/) {
    sectors_.resize(NUM_SECTORS);
    for (size_t j = 0; j < NUM_SECTORS; j++) {
        auto& sec = sectors_[j];
        float jf = static_cast<float>(j);

        // Spread sector biases across the feature space for diverse coverage
        sec.b = -5.0f + 0.3f * jf;

        // Material weight = 0 (material is added separately, not through tropical surface)
        sec.w[0]  = 0.0f;
        // Diversified positional weights using trig functions for sector specialization
        sec.w[1]  = 0.5f  + 0.3f  * std::sin(jf * 0.5f);   // Fiedler
        sec.w[2]  = 0.4f  + 0.2f  * std::cos(jf * 0.7f);   // Cohesion
        sec.w[3]  = 0.3f  + 0.15f * std::sin(jf * 1.1f);   // Spectral Gap
        sec.w[4]  = 0.8f  + 0.3f  * std::cos(jf * 0.3f);   // PST
        sec.w[5]  = 1.0f  + 0.4f  * std::sin(jf * 0.9f);   // King Pressure
        sec.w[6]  = 0.7f  + 0.25f * std::cos(jf * 1.3f);   // Battery Energy
        sec.w[7]  = 0.6f  + 0.2f  * std::sin(jf * 0.6f);   // Pawn Cohesion
        sec.w[8]  = 0.3f  + 0.1f  * std::cos(jf * 0.4f);   // Trace Energy
        sec.w[9]  = 0.5f  + 0.2f  * std::sin(jf * 0.8f);   // Mobility
        sec.w[10] = 0.4f  + 0.15f * std::cos(jf * 1.0f);   // Center Control
        sec.w[11] = 0.3f  + 0.1f  * std::sin(jf * 1.2f);   // Game Phase
    }
}

// =============================================================================
// Extract the 14-dimensional spectral-tropical feature vector from a position
// =============================================================================
std::array<float, TropicalEvaluator::NUM_FEATURES> TropicalEvaluator::extract_features(const Board& board) const {
    int white_mat = 0, black_mat = 0;
    int white_pst = 0, black_pst = 0;
    int white_passed = 0, black_passed = 0;

    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        Piece p = board.piece_at(s);
        if (p == Piece::None) continue;

        Color c = color_of(p);
        PieceType pt = piece_type_of(p);

        int val = 100;
        switch (pt) {
            case PieceType::Pawn:   val = 100; break;
            case PieceType::Knight: val = 320; break;
            case PieceType::Bishop: val = 330; break;
            case PieceType::Rook:   val = 500; break;
            case PieceType::Queen:  val = 900; break;
            default: break;
        }

        if (c == Color::White) {
            white_mat += val;
            white_pst += PST::get_mg(pt, Color::White, s);
        } else {
            black_mat += val;
            black_pst += PST::get_mg(pt, Color::Black, s);
        }

        // Passed pawn detection (simplified: no enemy pawn on same or adjacent files ahead)
        if (pt == PieceType::Pawn) {
            int file = static_cast<int>(file_of(s));
            int rank = static_cast<int>(rank_of(s));
            bool passed = true;
            for (int sq2 = 0; sq2 < 64 && passed; sq2++) {
                Piece p2 = board.piece_at(static_cast<Square>(sq2));
                if (p2 == Piece::None || piece_type_of(p2) != PieceType::Pawn) continue;
                if (color_of(p2) == c) continue; // Same color pawn, skip
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

    // Lazy Spectral Evaluation: skip expensive Laplacian eigensolver when material is decisive (> 1200 cp = Queen advantage)
    if (std::abs(material_diff) > 1200) {
        std::array<float, NUM_FEATURES> x{};
        x[0] = static_cast<float>(material_diff);
        x[4] = static_cast<float>(pst_diff);
        x[13] = static_cast<float>(passed_diff) * 30.0f;
        return x;
    }

    SpectralFeatures feat = SpectralGraph::compute_spectrum(board);

    float our_shield   = (us == Color::White) ? feat.king_shield_us : feat.king_shield_them;
    float their_shield = (us == Color::White) ? feat.king_shield_them : feat.king_shield_us;
    float our_pressure = (us == Color::White) ? feat.king_pressure_us : feat.king_pressure_them;

    // Construct 16-Dimensional Spectral-Tropical Feature Vector
    std::array<float, NUM_FEATURES> x;
    x[0]  = static_cast<float>(material_diff);                                     // Material diff (RAW pass-through)
    x[1]  = (feat.fiedler_us - feat.fiedler_them) * 15.0f;                         // Relative Fiedler (per-side)
    x[2]  = (feat.cohesion_us - feat.cohesion_them) * 5.0f;                        // Relative Subgraph Cohesion
    x[3]  = feat.spectral_gap * 2.0f;                                              // Global Control Bottleneck
    x[4]  = static_cast<float>(pst_diff);                                          // Relative PST
    x[5]  = (feat.king_pressure_us - feat.king_pressure_them) * 10.0f;             // Relative King Attack Pressure
    x[6]  = (feat.battery_energy_us - feat.battery_energy_them) * 8.0f;            // Relative Ray Alignment Battery
    x[7]  = (feat.pawn_cohesion_us - feat.pawn_cohesion_them) * 12.0f;             // Relative Pawn Structure
    x[8]  = feat.laplacian_trace / 10.0f;                                          // Total Energy Density
    x[9]  = (feat.mobility_us - feat.mobility_them) * 3.0f;                        // Relative Mobility
    x[10] = (feat.center_control_us - feat.center_control_them) * 8.0f;            // Relative Center Control
    x[11] = feat.game_phase * 50.0f;                                               // Game Phase
    x[12] = (feat.king_shield_us - feat.king_shield_them) * 10.0f;                 // King Shield Energy
    x[13] = static_cast<float>(passed_diff) * 30.0f;                               // Passed Pawn Advantage
    x[14] = static_cast<float>(passed_diff) * (1.0f - feat.game_phase) * 40.0f;    // Cross-Term 1: Endgame Passed Pawn Multiplier
    x[15] = (our_pressure / (their_shield + 1.0f)) * 15.0f;                        // Cross-Term 2: Unshielded King Attack Ratio

    return x;
}

// =============================================================================
// Evaluate position using the Tropical (max, +) Minimax Surface
// =============================================================================
int TropicalEvaluator::evaluate(const Board& board) const {
    auto [score, _sector] = evaluate_with_sector(board);
    return score;
}

// =============================================================================
// Evaluate and return both score and winning sector index (for training)
// =============================================================================
std::pair<int, size_t> TropicalEvaluator::evaluate_with_sector(const Board& board) const {
    std::array<float, NUM_FEATURES> x = extract_features(board);

    float material_diff = x[0];
    float pst_diff = x[4];

    // Tropical (max, +) Semiring Minimax Surface for Positional Correlations
    // ONLY positional features go through sectors. x[0] (Material) and x[4] (PST)
    // are raw pass-through terms to prevent double-counting.
    float max_positional_sector = -1e9f;
    size_t winning_sector = 0;

    for (size_t j = 0; j < NUM_SECTORS; j++) {
        const auto& sec = sectors_[j];
        float sector_val = sec.b;
        // Evaluate positional features (indices 1..13, skip x[0] raw material)
        for (size_t i = 1; i < NUM_FEATURES; i++) {
            sector_val += sec.w[i] * x[i];
        }
        if (sector_val > max_positional_sector) {
            max_positional_sector = sector_val;
            winning_sector = j;
        }
    }

    // Material Dominance Principle: Positional bonuses are clamped to [-250 cp, +250 cp]
    // so tactical blunders (losing Queen/Rook) can NEVER be offset by positional cohesion.
    float clamped_positional = std::max(-250.0f, std::min(250.0f, max_positional_sector));

    float total_eval = material_diff + clamped_positional + pst_diff;

    int score = static_cast<int>(std::max(-30000.0f, std::min(30000.0f, total_eval)));
    return {score, winning_sector};
}

bool TropicalEvaluator::save_weights(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    uint32_t num_sec = static_cast<uint32_t>(NUM_SECTORS);
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

    if (num_sec != NUM_SECTORS || num_feat != NUM_FEATURES) return false;

    sectors_.resize(NUM_SECTORS);
    for (auto& sec : sectors_) {
        in.read(reinterpret_cast<char*>(sec.w.data()), NUM_FEATURES * sizeof(float));
        in.read(reinterpret_cast<char*>(&sec.b), sizeof(sec.b));
    }

    return in.good();
}

} // namespace heavensgate
