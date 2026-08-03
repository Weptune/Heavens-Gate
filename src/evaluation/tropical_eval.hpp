#pragma once

#include "../core/types.hpp"
#include "spectral_graph.hpp"
#include <vector>
#include <array>
#include <string>
#include <cstdint>

namespace heavensgate {

class Board; // Forward declaration

// =============================================================================
// TropicalEvaluator — (max, +) Tropical Minimax Surface Evaluator
// =============================================================================
//
// Combines Spectral Graph Theory features with a Tropical Semiring (max, +)
// Piecewise-Linear Minimax Surface:
//
//   T(x) = ⨁_{j=1}^M (w_j ⊗ x_1^{a_{j1}} ⊗ ... ⊗ x_k^{a_{jk}})
//        = max_{j ∈ {1..M}} (w_j^T x + b_j)
//
// Where:
//   ⊕ is Tropical Addition (max)
//   ⊗ is Tropical Multiplication (classical addition)
//   x contains [Material, λ₂ (Fiedler), λ_N - λ₂ (Gap), Tr(L), Cohesion]
//
// =============================================================================

class TropicalEvaluator {
public:
    static constexpr size_t NUM_SECTORS = 8;
    static constexpr size_t NUM_FEATURES = 8;

    struct SectorWeights {
        std::array<float, NUM_FEATURES> w;
        float b;
    };

    explicit TropicalEvaluator();

    // Evaluates board position (returns score in centipawns relative to side_to_move)
    int evaluate(const Board& board) const;

    // Weight management
    void initialize_weights(uint32_t seed = 42);
    bool save_weights(const std::string& path) const;
    bool load_weights(const std::string& path);

    // Accessors for training
    std::vector<SectorWeights>& sectors() { return sectors_; }
    const std::vector<SectorWeights>& sectors() const { return sectors_; }

    static TropicalEvaluator& instance();

private:
    std::vector<SectorWeights> sectors_;
};

} // namespace heavensgate
