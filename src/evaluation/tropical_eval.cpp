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
    // Sector 1: Standard Material Dominance
    sectors_[0].w = { 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f };
    sectors_[0].b = 0.0f;

    // Sector 2: Piece Coordination & Fiedler Connectivity Attack
    sectors_[1].w = { 1.0f, 2.5f, 1.5f, 1.0f, 1.2f, 0.0f, 0.0f, 0.0f };
    sectors_[1].b = 0.0f;

    // Sector 3: Spectral Gap Compression & Central Bottleneck Control
    sectors_[2].w = { 1.0f, 1.2f, 2.0f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f };
    sectors_[2].b = 0.0f;

    // Sector 4: Total Activity & Laplacian Trace Expansion
    sectors_[3].w = { 1.0f, 1.5f, 1.0f, 2.0f, 1.5f, 0.0f, 0.0f, 0.0f };
    sectors_[3].b = 0.0f;

    // Sector 5: Subgraph Cohesion & Defensive Fortress
    sectors_[4].w = { 1.0f, 1.0f, 2.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f };
    sectors_[4].b = 0.0f;

    // Sector 6: Counter-Attack Regime
    sectors_[5].w = { 1.0f, 2.0f, 2.0f, 1.0f, 1.5f, 0.0f, 0.0f, 0.0f };
    sectors_[5].b = 0.0f;

    // Sector 7: High Pressure Dynamic Tactical Scramble
    sectors_[6].w = { 1.0f, 3.0f, 2.5f, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f };
    sectors_[6].b = 0.0f;

    // Sector 8: Equilibrium Strategic Sector
    sectors_[7].w = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    sectors_[7].b = 0.0f;
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

    // Construct Spectral-Tropical Feature Vector x (All relative: Us - Them)
    std::array<float, NUM_FEATURES> x;
    x[0] = static_cast<float>(material_diff);                     // Material difference in cp
    x[1] = (feat.fiedler_us - feat.fiedler_them) * 15.0f;           // Relative Fiedler Coordination
    x[2] = (feat.cohesion_us - feat.cohesion_them) * 5.0f;         // Relative Subgraph Cohesion
    x[3] = feat.spectral_gap * 2.0f;                                // Relative Control Bottleneck
    x[4] = static_cast<float>(pst_diff);                           // Relative PST Positional bonus
    x[5] = 0.0f;
    x[6] = 0.0f;
    x[7] = 0.0f;                                                   // Zero Bias at symmetry

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
