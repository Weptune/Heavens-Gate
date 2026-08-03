#pragma once

#include "../core/types.hpp"
#include "nnue.hpp"
#include <vector>
#include <array>
#include <string>
#include <cstdint>

namespace heavensgate {

class Board; // Forward declaration

// =============================================================================
// TensorNNUE — Tensor-Decomposed Neural Network Evaluation Engine
// =============================================================================
//
// Represents the 40,960 HalfKP sparse feature matrix using a Tensor-Ring / CP
// Canonical Tensor Decomposition:
//
//   W_features [40960, 256]  ==>  U_factor [40960, R] × V_core [R, 256]
//
//   Where R = Bond Rank (32 or 64).
//
// Benefits:
//   1. 0-Blunder Sparse Feature Lookups (HalfKP Piece-Square Indexing)
//   2. 8x Parameter Compression & High-Order Quantum Tensor Factorization
//   3. Extremely Fast Accumulator Add/Sub Updates
//
// =============================================================================

class TensorNNUE {
public:
    static constexpr size_t NUM_FEATURES = 40960;
    static constexpr size_t BOND_RANK    = 32;    // Tensor rank R
    static constexpr size_t HIDDEN_DIM   = 256;   // Hidden layer dimension
    static constexpr size_t OUTPUT_DIM   = 512;   // 2 * HIDDEN_DIM (us + them)

    static constexpr uint32_t MAGIC = 0x544E4E55; // "TNNU" in little-endian

    struct Accumulator {
        alignas(64) std::array<float, BOND_RANK> factors;
    };

    explicit TensorNNUE(size_t rank = BOND_RANK);

    // Feedforward evaluation (returns score in centipawns relative to side_to_move)
    int evaluate(const Board& board) const;

    // Fast accumulator computation from board state
    void compute_accumulator(const Board& board, Color perspective, Accumulator& acc) const;

    // Weight initialization & binary I/O
    void initialize_weights(uint32_t seed = 42);
    bool save_weights(const std::string& path) const;
    bool load_weights(const std::string& path);

    // Accessors
    float* u_factor_data() { return U_factor_.data(); }
    float* v_core_data() { return V_core_.data(); }
    float* w_out_data() { return W_out_.data(); }
    const float* u_factor_data() const { return U_factor_.data(); }
    const float* v_core_data() const { return V_core_.data(); }
    const float* w_out_data() const { return W_out_.data(); }

    static TensorNNUE& instance();

private:
    size_t R_; // Tensor rank

    // Factorized Tensor Storage:
    // U_factor_: Shape [NUM_FEATURES, R_]
    // V_core_:   Shape [R_, HIDDEN_DIM]
    // W_out_:    Shape [OUTPUT_DIM]
    // Bias_out_: Scalar bias
    std::vector<float> U_factor_;
    std::vector<float> V_core_;
    std::vector<float> W_out_;
    float bias_out_ = 0.0f;
};

} // namespace heavensgate
