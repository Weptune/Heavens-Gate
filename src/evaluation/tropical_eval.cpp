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
        sec.b = 0.0f;
        sec.w.fill(0.0f);
        // Base positional weights tailored across 32 tropical sectors
        sec.w[0] = 1.0f; // Material
        sec.w[1] = 1.0f + 0.1f * (j % 5); // Fiedler
        sec.w[2] = 1.0f + 0.08f * (j % 4); // Cohesion
        sec.w[3] = 0.5f + 0.05f * (j % 3); // Spectral Gap
        sec.w[4] = 1.0f; // PST
        sec.w[5] = 1.5f + 0.2f * (j % 6); // King Pressure
        sec.w[6] = 1.2f + 0.15f * (j % 4); // Battery Energy
        sec.w[7] = 1.5f + 0.1f * (j % 5); // Pawn Cohesion
        sec.w[8] = 0.5f; // Trace Energy
    }
}

int TropicalEvaluator::evaluate(const Board& board) const {
    int white_mat = 0;
    int black_mat = 0;
    int white_pst = 0;
    int black_pst = 0;

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
    }

    Color us = board.side_to_move();
    int material_diff = (us == Color::White) ? (white_mat - black_mat) : (black_mat - white_mat);
    int pst_diff      = (us == Color::White) ? (white_pst - black_pst) : (black_pst - white_pst);

    SpectralFeatures feat = SpectralGraph::compute_spectrum(board);

    // Construct High-Resolution 12-Dimensional Spectral-Tropical Feature Vector x
    std::array<float, NUM_FEATURES> x;
    x[0]  = static_cast<float>(material_diff);                                          // Material diff
    x[1]  = (feat.fiedler_us - feat.fiedler_them) * 15.0f;                                // Relative Fiedler
    x[2]  = (feat.cohesion_us - feat.cohesion_them) * 5.0f;                              // Relative Subgraph Cohesion
    x[3]  = feat.spectral_gap * 2.0f;                                                     // Relative Control Bottleneck
    x[4]  = static_cast<float>(pst_diff);                                                // Relative PST
    x[5]  = (feat.king_pressure_us - feat.king_pressure_them) * 10.0f;                    // Relative King Attack Pressure
    x[6]  = (feat.battery_energy_us - feat.battery_energy_them) * 8.0f;                   // Relative Ray Alignment Battery
    x[7]  = (feat.pawn_cohesion_us - feat.pawn_cohesion_them) * 12.0f;                     // Relative Pawn Structure
    x[8]  = feat.laplacian_trace / 10.0f;                                               // Total Energy Density
    x[9]  = 0.0f;
    x[10] = 0.0f;
    x[11] = 0.0f;

    // Tropical (max, +) Semiring Minimax Surface for Positional Correlations
    float max_positional_sector = -1e9f;

    for (size_t j = 0; j < NUM_SECTORS; j++) {
        const auto& sec = sectors_[j];
        float sector_val = sec.b;
        // Evaluate positional terms (indices 1..7, omitting raw material index 0)
        for (size_t i = 1; i < NUM_FEATURES; i++) {
            sector_val += sec.w[i] * x[i];
        }
        if (sector_val > max_positional_sector) {
            max_positional_sector = sector_val;
        }
    }

    // Material Dominance Principle: Positional bonuses are clamped to [-250 cp, +250 cp]
    // so tactical blunders (losing Queen/Rook) can NEVER be offset by positional cohesion.
    float clamped_positional = std::max(-250.0f, std::min(250.0f, max_positional_sector));

    float total_eval = static_cast<float>(material_diff) + clamped_positional + static_cast<float>(pst_diff);

    return static_cast<int>(std::max(-30000.0f, std::min(30000.0f, total_eval)));
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
