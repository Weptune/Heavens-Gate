#pragma once

#include "../core/types.hpp"
#include "hilbert.hpp"
#include <array>
#include <vector>
#include <string>
#include <cstdint>

namespace heavensgate {

class Board; // Forward declaration

// =============================================================================
// TensorMPS — Matrix Product State Evaluation Engine
// =============================================================================
//
// Evaluates chess positions by contracting a chain of tensors (Matrix Product
// State) over the 64 squares of the board, ordered by a Hilbert space-filling
// curve for maximum locality preservation.
//
// Architecture:
//   Site 0:      Side-to-move virtual site, shape [2, 1, D]
//   Sites 1-63:  Bulk square sites (Hilbert-ordered), shape [13, D, D]
//   Site 64:     Right boundary square site, shape [13, D, 1]
//
// Evaluation:
//   eval = v_L × A[sq_0] × A[sq_1] × ... × A[sq_63] × v_R
//   where v_L is selected by side-to-move, each A[sq_i] is a D×D matrix
//   selected by the piece on that square, and v_R contracts to a scalar.
//
// Computational cost: 63 × D² multiply-adds per evaluation.
//   D=16: ~16K ops,  D=32: ~65K ops,  D=64: ~262K ops
//
// =============================================================================

class TensorMPS {
public:
    // ----- Constants -----
    static constexpr int NUM_SQUARES   = 64;
    static constexpr int NUM_SITES     = 65;   // 1 virtual + 64 squares
    static constexpr int LOCAL_DIM_STM = 2;    // {White, Black} to move
    static constexpr int LOCAL_DIM_SQ  = 13;   // {Empty, WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK}
    static constexpr int NUM_BULK      = 63;   // bulk sites (sites 1 through 63)

    // File format magic number
    static constexpr uint32_t MAGIC = 0x4E544748; // "HGTN" in little-endian

    // ----- Construction -----
    explicit TensorMPS(int bond_dim = 16);

    // ----- Evaluation -----
    // Returns evaluation in centipawns from White's perspective.
    int evaluate(const Board& board) const;

    // ----- Incremental Evaluation Environment -----
    struct Environment {
        std::vector<float> L;  // Left environment vectors [65 * D]
        int valid_up_to = -1;  // Site index up to which L is valid
    };

    // Incremental evaluation using cached left environment
    int evaluate_incremental(const Board& board, Environment& env) const;

    // ----- Weight Management -----
    void initialize_random(uint32_t seed = 42);
    bool save_weights(const std::string& path) const;
    bool load_weights(const std::string& path);

    // ----- Accessors -----
    int bond_dim() const { return D_; }
    int num_parameters() const;
    float scale() const { return scale_; }
    void set_scale(float s) { scale_ = s; }

    // Access raw tensor data (for training)
    float* stm_data() { return T_stm_.data(); }
    float* bulk_data() { return T_bulk_.data(); }
    float* right_data() { return T_right_.data(); }
    const float* stm_data() const { return T_stm_.data(); }
    const float* bulk_data() const { return T_bulk_.data(); }
    const float* right_data() const { return T_right_.data(); }

    // ----- Singleton -----
    static TensorMPS& instance();

    // ----- Piece Index Mapping -----
    // Maps engine Piece enum to local tensor dimension index [0..12]
    // Piece::None → 0 (empty), WhitePawn(0) → 1, ..., BlackKing(11) → 12
    static int piece_to_local_index(Piece p) {
        if (p == Piece::None) return 0;
        return static_cast<int>(p) + 1;
    }

private:
    int D_; // Bond dimension

    // Tensor storage (float32, row-major):
    //
    // T_stm_:  Side-to-move boundary. Shape [LOCAL_DIM_STM, D].
    //          Index: stm_idx * D + j
    //          Conceptually: T_stm[stm][j] is the j-th element of the
    //          left boundary vector for side-to-move stm.
    std::vector<float> T_stm_;

    // T_bulk_: Bulk site tensors. Shape [NUM_BULK, LOCAL_DIM_SQ, D, D].
    //          Index: site * (LOCAL_DIM_SQ * D * D) + piece * (D * D) + row * D + col
    //          T_bulk[site][piece][row][col] selects a D×D matrix for the
    //          given piece on the given site. The contraction multiplies
    //          the running vector v (shape [D]) by this matrix.
    std::vector<float> T_bulk_;

    // T_right_: Right boundary tensor. Shape [LOCAL_DIM_SQ, D].
    //           Index: piece * D + j
    //           T_right[piece][j] selects a D-element column vector.
    //           The final dot product v · T_right[piece] yields the scalar output.
    std::vector<float> T_right_;

    // Output scaling: eval_cp = clamp(raw_output * scale_, -30000, 30000)
    float scale_ = 100.0f;

    // ----- Internal Helpers -----
    // Get pointer to the D×D matrix for a given bulk site and piece
    const float* bulk_matrix(int site, int piece_idx) const {
        return &T_bulk_[(site * LOCAL_DIM_SQ + piece_idx) * D_ * D_];
    }
    float* bulk_matrix_mut(int site, int piece_idx) {
        return &T_bulk_[(site * LOCAL_DIM_SQ + piece_idx) * D_ * D_];
    }

    // Get pointer to the D-element boundary vector for STM
    const float* stm_vector(int stm_idx) const {
        return &T_stm_[stm_idx * D_];
    }

    // Get pointer to the D-element right boundary vector for a piece
    const float* right_vector(int piece_idx) const {
        return &T_right_[piece_idx * D_];
    }
};

} // namespace heavensgate
