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
    static constexpr size_t NUM_KING_BUCKETS       = 10;
    static constexpr size_t NUM_SECTORS_PER_BUCKET = 32;
    static constexpr size_t TOTAL_SECTORS          = NUM_KING_BUCKETS * NUM_SECTORS_PER_BUCKET; // 320 sectors
    static constexpr size_t NUM_FEATURES           = 16;
    static constexpr float  SMOOTH_TAU             = 3.0f; // Log-Sum-Exp smoothing temperature in normalized unit scale

    struct SectorWeights {
        std::array<float, NUM_FEATURES> w;
        float b;
    };

    explicit TropicalEvaluator();

    // King-Bucket spatial partitioning lookup (0..9) with horizontal symmetry
    static int get_king_bucket(Square opp_king_sq, Color us);

    // Evaluates board position (returns score in centipawns relative to side_to_move)
    int evaluate(const Board& board) const;

    // Training support: extract 16D spectral-tropical feature vector from board
    std::array<float, NUM_FEATURES> extract_features(const Board& board) const;

    // Training support: evaluate with smooth Log-Sum-Exp max and return (score, bucket_index, winning_sector_index, sector_softmax_probs)
    struct EvalResult {
        int score;
        int bucket;
        size_t winning_sector;
        std::array<float, NUM_SECTORS_PER_BUCKET> softmax_probs;
    };
    EvalResult evaluate_detailed(const Board& board) const;
    std::pair<int, size_t> evaluate_with_sector(const Board& board) const;

    // Weight management
    void initialize_weights(uint32_t seed = 42);
    bool save_weights(const std::string& path) const;
    bool load_weights(const std::string& path);

    // Accessors for training (320 sectors = 10 buckets * 32 sectors)
    std::vector<SectorWeights>& sectors() { return sectors_; }
    const std::vector<SectorWeights>& sectors() const { return sectors_; }

    static TropicalEvaluator& instance();

private:
    std::vector<SectorWeights> sectors_; // 320 sectors (5,440 parameters)
};

} // namespace heavensgate
