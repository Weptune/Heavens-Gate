#pragma once

#include "tensor_eval.hpp"
#include "../core/types.hpp"
#include "hilbert.hpp"
#include <vector>
#include <cstdint>
#include <string>

namespace heavensgate {

// =============================================================================
// TensorMPSQuantized — Int16 Q14 Fixed-Point Quantized Evaluator
// =============================================================================
//
// Quantizes float32 MPS tensors to int16 Q14 fixed-point format (scale factor 2^14 = 16384).
//
// Key Advantages:
//   1. 2x Memory Reduction: 820KB -> 410KB (fits entirely in CPU L2 cache)
//   2. Fast SIMD Contraction: Uses 16-bit integer matrix-vector multiplication
//   3. Scale Preservation: Bit-shift by 14 per site maintains magnitude across 64 sites
//
// Fixed-Point Formula:
//   W_q = round(W_float * 16384)
//   v_raw[j] = sum_k (v_curr[k] * W_q[k, j])   (int32 accumulator)
//   v_next[j] = clamp16(v_raw[j] >> 14)        (int16 vector)
//
// =============================================================================

class TensorMPSQuantized {
public:
    static constexpr int Q_SHIFT = 14;
    static constexpr int Q_SCALE = 1 << Q_SHIFT; // 16384

    explicit TensorMPSQuantized(int bond_dim = 16);

    // Quantize from floating-point TensorMPS model
    void quantize_from(const TensorMPS& float_model);

    // Evaluate position using int16 fixed-point contraction
    int evaluate(const Board& board) const;

    // Fast incremental evaluation with quantized environment cache
    struct QuantizedEnvironment {
        std::vector<int16_t> L; // [65 * D] int16 left environment vectors
        int valid_up_to = -1;
    };

    int evaluate_incremental(const Board& board, QuantizedEnvironment& env) const;

    // I/O for quantized binary weights (.qtnw)
    bool save_quantized(const std::string& path) const;
    bool load_quantized(const std::string& path);

    int bond_dim() const { return D_; }
    bool is_quantized() const { return quantized_; }

    static TensorMPSQuantized& instance();

private:
    int D_;
    bool quantized_ = false;
    float scale_ = 100.0f;

    // Int16 tensor buffers (contiguous row-major)
    std::vector<int16_t> T_stm_q_;   // [2, D]
    std::vector<int16_t> T_bulk_q_;  // [63, 13, D, D]
    std::vector<int16_t> T_right_q_; // [13, D]

    const int16_t* bulk_matrix_q(int site, int piece_idx) const {
        return &T_bulk_q_[(site * TensorMPS::LOCAL_DIM_SQ + piece_idx) * D_ * D_];
    }
};

} // namespace heavensgate
