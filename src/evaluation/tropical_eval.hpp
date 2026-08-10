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
    static constexpr size_t NUM_KING_BUCKETS         = 10;
    static constexpr size_t SECTORS_PER_SURFACE     = 16; // 16 for T1 (Advantage), 16 for T2 (Vulnerability)
    static constexpr size_t TOTAL_SECTORS_T1        = NUM_KING_BUCKETS * SECTORS_PER_SURFACE; // 160 sectors
    static constexpr size_t TOTAL_SECTORS_T2        = NUM_KING_BUCKETS * SECTORS_PER_SURFACE; // 160 sectors
    static constexpr size_t TOTAL_SECTORS           = TOTAL_SECTORS_T1 + TOTAL_SECTORS_T2;    // 320 sectors total
    static constexpr size_t NUM_FEATURES             = 25;
    static constexpr float  SMOOTH_TAU               = 3.0f; // Log-Sum-Exp smoothing temperature

    struct SectorWeights {
        std::array<float, NUM_FEATURES> w;
        float b;
    };

    explicit TropicalEvaluator();

    // King-Bucket spatial partitioning lookup (0..9) with horizontal symmetry
    static int get_king_bucket(Square opp_king_sq, Color us);

    // Evaluates board position using Tropical Rational Function: f(x) = T1(x) - T2(x) + Tempo
    int evaluate(const Board& board) const;

    // Training support: extract 22D spectral-tropical feature vector from board
    std::array<float, NUM_FEATURES> extract_features(const Board& board) const;

    // Phase 4 Detailed Evaluation Result (Dual Softmax Distributions over T1 and T2)
    struct EvalResult {
        int score;
        int bucket;
        float t1_val;
        float t2_val;
        size_t winning_sector_t1;
        size_t winning_sector_t2;
        std::array<float, SECTORS_PER_SURFACE> softmax_t1;
        std::array<float, SECTORS_PER_SURFACE> softmax_t2;
    };
    EvalResult evaluate_detailed(const Board& board) const;
    EvalResult evaluate_detailed_from_features(const std::array<float, NUM_FEATURES>& features, size_t bucket) const;

    // Weight management
    void initialize_weights(uint32_t seed = 42);
    bool save_weights(const std::string& path) const;
    bool load_weights(const std::string& path);

    // Accessors for T1 (Advantage) and T2 (Vulnerability) sector surfaces
    std::vector<SectorWeights>& sectors_t1() { return sectors_t1_; }
    const std::vector<SectorWeights>& sectors_t1() const { return sectors_t1_; }

    std::vector<SectorWeights>& sectors_t2() { return sectors_t2_; }
    const std::vector<SectorWeights>& sectors_t2() const { return sectors_t2_; }

    static TropicalEvaluator& instance();

private:
    std::vector<SectorWeights> sectors_t1_; // 160 T1 Advantage sectors
    std::vector<SectorWeights> sectors_t2_; // 160 T2 Vulnerability sectors
};

} // namespace heavensgate
