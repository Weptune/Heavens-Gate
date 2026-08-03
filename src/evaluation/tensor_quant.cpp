#include "tensor_quant.hpp"
#include "../board/board.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <iostream>

#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64)
#include <emmintrin.h>
#include <tmmintrin.h>
#endif

namespace heavensgate {

// =============================================================================
// SIMD Dot Product Helper for 16-bit Fixed-Point Vectors
// =============================================================================

static inline int32_t dot_product_int16(const int16_t* a, const int16_t* b, int D) {
#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64)
    if (D == 16) {
        __m128i a0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a));
        __m128i b0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b));
        __m128i p0 = _mm_madd_epi16(a0, b0);

        __m128i a1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a + 8));
        __m128i b1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + 8));
        __m128i p1 = _mm_madd_epi16(a1, b1);

        __m128i sum = _mm_add_epi32(p0, p1);
        __m128i shuf = _mm_shuffle_epi32(sum, _MM_SHUFFLE(1, 0, 3, 2));
        sum = _mm_add_epi32(sum, shuf);
        shuf = _mm_shuffle_epi32(sum, _MM_SHUFFLE(2, 3, 0, 1));
        sum = _mm_add_epi32(sum, shuf);

        return _mm_cvtsi128_si32(sum);
    }
#endif
    int32_t sum = 0;
    for (int k = 0; k < D; k++) {
        sum += static_cast<int32_t>(a[k]) * static_cast<int32_t>(b[k]);
    }
    return sum;
}

// =============================================================================
// Construction
// =============================================================================

TensorMPSQuantized::TensorMPSQuantized(int bond_dim) : D_(bond_dim) {
    T_stm_q_.resize(TensorMPS::LOCAL_DIM_STM * D_, 0);
    T_bulk_q_.resize(TensorMPS::NUM_BULK * TensorMPS::LOCAL_DIM_SQ * D_ * D_, 0);
    T_right_q_.resize(TensorMPS::LOCAL_DIM_SQ * D_, 0);
}

TensorMPSQuantized& TensorMPSQuantized::instance() {
    static TensorMPSQuantized q_mps(16);
    static bool initialized = false;
    if (!initialized) {
        q_mps.quantize_from(TensorMPS::instance());
        initialized = true;
    }
    return q_mps;
}

// =============================================================================
// Quantization from Float Model (Transposed Bulk Matrices for SIMD)
// =============================================================================

static int16_t float_to_q14(float val) {
    int32_t q = static_cast<int32_t>(std::round(val * 16384.0f));
    return static_cast<int16_t>(std::max(-32768, std::min(32767, q)));
}

void TensorMPSQuantized::quantize_from(const TensorMPS& float_model) {
    D_ = float_model.bond_dim();
    scale_ = float_model.scale();

    T_stm_q_.resize(TensorMPS::LOCAL_DIM_STM * D_);
    T_bulk_q_.resize(TensorMPS::NUM_BULK * TensorMPS::LOCAL_DIM_SQ * D_ * D_);
    T_right_q_.resize(TensorMPS::LOCAL_DIM_SQ * D_);

    const float* stm_f = float_model.stm_data();
    for (size_t i = 0; i < T_stm_q_.size(); i++) {
        T_stm_q_[i] = float_to_q14(stm_f[i]);
    }

    // Transpose bulk matrices (row k, col j -> col j, row k) for SIMD vector access
    const float* bulk_f = float_model.bulk_data();
    for (int site = 0; site < TensorMPS::NUM_BULK; site++) {
        for (int piece = 0; piece < TensorMPS::LOCAL_DIM_SQ; piece++) {
            size_t base_idx = (site * TensorMPS::LOCAL_DIM_SQ + piece) * D_ * D_;
            for (int k = 0; k < D_; k++) {
                for (int j = 0; j < D_; j++) {
                    T_bulk_q_[base_idx + j * D_ + k] = float_to_q14(bulk_f[base_idx + k * D_ + j]);
                }
            }
        }
    }

    const float* right_f = float_model.right_data();
    for (size_t i = 0; i < T_right_q_.size(); i++) {
        T_right_q_[i] = float_to_q14(right_f[i]);
    }

    quantized_ = true;
}

// =============================================================================
// Full Quantized Evaluation
// =============================================================================

int TensorMPSQuantized::evaluate(const Board& board) const {
    const auto& hilbert = HilbertCurve::order();

    int stm_idx = (board.side_to_move() == Color::White) ? 0 : 1;
    const int16_t* stm_vec = &T_stm_q_[stm_idx * D_];

    int16_t v[128];
    int16_t v_new[128];

    for (int j = 0; j < D_; j++) {
        v[j] = stm_vec[j];
    }

    // Contract 63 bulk sites
    for (int site = 0; site < TensorMPS::NUM_BULK; site++) {
        Square sq = static_cast<Square>(hilbert[site]);
        Piece p = board.piece_at(sq);
        int piece_idx = TensorMPS::piece_to_local_index(p);

        const int16_t* mat = bulk_matrix_q(site, piece_idx);

        for (int j = 0; j < D_; j++) {
            const int16_t* col_j = &mat[j * D_];
            int32_t sum = dot_product_int16(v, col_j, D_);
            int32_t scaled = sum >> Q_SHIFT;
            v_new[j] = static_cast<int16_t>(std::max(-32768, std::min(32767, scaled)));
        }

        std::memcpy(v, v_new, D_ * sizeof(int16_t));
    }

    // Right boundary dot product
    Square last_sq = static_cast<Square>(hilbert[63]);
    Piece last_p = board.piece_at(last_sq);
    int last_idx = TensorMPS::piece_to_local_index(last_p);

    const int16_t* rv = &T_right_q_[last_idx * D_];
    int64_t raw_acc = dot_product_int16(v, rv, D_);

    // raw_acc is in Q28 scale (16384^2)
    double raw_float = static_cast<double>(raw_acc) / (16384.0 * 16384.0);
    double eval_cp = raw_float * scale_;
    eval_cp = std::max(-30000.0, std::min(30000.0, eval_cp));

    return static_cast<int>(eval_cp);
}

// =============================================================================
// Incremental Quantized Evaluation
// =============================================================================

int TensorMPSQuantized::evaluate_incremental(const Board& board, QuantizedEnvironment& env) const {
    if (env.L.size() != static_cast<size_t>(TensorMPS::NUM_SITES * D_)) {
        env.L.resize(TensorMPS::NUM_SITES * D_, 0);
        env.valid_up_to = -1;
    }

    const auto& hilbert = HilbertCurve::order();
    int start_site = std::max(0, env.valid_up_to);

    if (start_site == 0) {
        int stm_idx = (board.side_to_move() == Color::White) ? 0 : 1;
        const int16_t* stm_vec = &T_stm_q_[stm_idx * D_];
        std::memcpy(&env.L[0], stm_vec, D_ * sizeof(int16_t));
    }

    for (int site = start_site; site < TensorMPS::NUM_BULK; site++) {
        Square sq = static_cast<Square>(hilbert[site]);
        Piece p = board.piece_at(sq);
        int piece_idx = TensorMPS::piece_to_local_index(p);

        const int16_t* mat = bulk_matrix_q(site, piece_idx);
        const int16_t* v_curr = &env.L[site * D_];
        int16_t* v_next = &env.L[(site + 1) * D_];

        for (int j = 0; j < D_; j++) {
            const int16_t* col_j = &mat[j * D_];
            int32_t sum = dot_product_int16(v_curr, col_j, D_);
            int32_t scaled = sum >> Q_SHIFT;
            v_next[j] = static_cast<int16_t>(std::max(-32768, std::min(32767, scaled)));
        }
    }

    env.valid_up_to = TensorMPS::NUM_BULK;

    Square last_sq = static_cast<Square>(hilbert[63]);
    Piece last_p = board.piece_at(last_sq);
    int last_idx = TensorMPS::piece_to_local_index(last_p);

    const int16_t* rv = &T_right_q_[last_idx * D_];
    const int16_t* v_final = &env.L[TensorMPS::NUM_BULK * D_];

    int64_t raw_acc = dot_product_int16(v_final, rv, D_);

    double raw_float = static_cast<double>(raw_acc) / (16384.0 * 16384.0);
    double eval_cp = raw_float * scale_;
    eval_cp = std::max(-30000.0, std::min(30000.0, eval_cp));

    return static_cast<int>(eval_cp);
}

// =============================================================================
// I/O Quantized Weights (.qtnw)
// =============================================================================

bool TensorMPSQuantized::save_quantized(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    uint32_t magic = 0x51544748; // "HGTQ"
    uint32_t version = 1;
    uint32_t bond = static_cast<uint32_t>(D_);

    out.write(reinterpret_cast<const char*>(&magic), 4);
    out.write(reinterpret_cast<const char*>(&version), 4);
    out.write(reinterpret_cast<const char*>(&bond), 4);
    out.write(reinterpret_cast<const char*>(&scale_), sizeof(float));

    out.write(reinterpret_cast<const char*>(T_stm_q_.data()), T_stm_q_.size() * sizeof(int16_t));
    out.write(reinterpret_cast<const char*>(T_bulk_q_.data()), T_bulk_q_.size() * sizeof(int16_t));
    out.write(reinterpret_cast<const char*>(T_right_q_.data()), T_right_q_.size() * sizeof(int16_t));

    return out.good();
}

bool TensorMPSQuantized::load_quantized(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    uint32_t magic, version, bond;
    in.read(reinterpret_cast<char*>(&magic), 4);
    in.read(reinterpret_cast<char*>(&version), 4);
    in.read(reinterpret_cast<char*>(&bond), 4);
    in.read(reinterpret_cast<char*>(&scale_), sizeof(float));

    if (magic != 0x51544748 || version != 1) {
        return false;
    }

    D_ = static_cast<int>(bond);
    T_stm_q_.resize(TensorMPS::LOCAL_DIM_STM * D_);
    T_bulk_q_.resize(TensorMPS::NUM_BULK * TensorMPS::LOCAL_DIM_SQ * D_ * D_);
    T_right_q_.resize(TensorMPS::LOCAL_DIM_SQ * D_);

    in.read(reinterpret_cast<char*>(T_stm_q_.data()), T_stm_q_.size() * sizeof(int16_t));
    in.read(reinterpret_cast<char*>(T_bulk_q_.data()), T_bulk_q_.size() * sizeof(int16_t));
    in.read(reinterpret_cast<char*>(T_right_q_.data()), T_right_q_.size() * sizeof(int16_t));

    quantized_ = in.good();
    return quantized_;
}

} // namespace heavensgate
