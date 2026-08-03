#include "tensor_eval.hpp"
#include "../board/board.hpp"
#include <fstream>
#include <random>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace heavensgate {

// =============================================================================
// Construction
// =============================================================================

TensorMPS::TensorMPS(int bond_dim) : D_(bond_dim) {
    // Allocate tensor storage
    T_stm_.resize(LOCAL_DIM_STM * D_, 0.0f);
    T_bulk_.resize(NUM_BULK * LOCAL_DIM_SQ * D_ * D_, 0.0f);
    T_right_.resize(LOCAL_DIM_SQ * D_, 0.0f);
}

// =============================================================================
// Singleton
// =============================================================================

TensorMPS& TensorMPS::instance() {
    static TensorMPS mps(16);
    static bool initialized = false;
    if (!initialized) {
        mps.initialize_random(42);
        initialized = true;
    }
    return mps;
}

// =============================================================================
// Parameter Count
// =============================================================================

int TensorMPS::num_parameters() const {
    // STM boundary: 2 * D
    // Bulk sites:   63 * 13 * D * D
    // Right boundary: 13 * D
    // Scale: 1
    return LOCAL_DIM_STM * D_
         + NUM_BULK * LOCAL_DIM_SQ * D_ * D_
         + LOCAL_DIM_SQ * D_
         + 1;
}

// =============================================================================
// Random Initialization
// =============================================================================
//
// Critical for numerical stability: with 64 matrix multiplications, naive
// random initialization causes the product to explode or vanish.
//
// Strategy:
//   - Bulk matrices initialized to Identity + small noise
//   - The "empty square" (index 0) slice is EXACT identity
//   - Boundary vectors initialized to 1/sqrt(D)
//   - This ensures initial eval ≈ 0 for symmetric positions
//
// =============================================================================

void TensorMPS::initialize_random(uint32_t seed) {
    std::mt19937 rng(seed);
    float epsilon = 0.01f / std::sqrt(static_cast<float>(D_));
    std::normal_distribution<float> noise(0.0f, epsilon);

    // --- STM boundary vectors [2, D] ---
    // Initialize both to 1/sqrt(D) + noise
    float base_val = 1.0f / std::sqrt(static_cast<float>(D_));
    for (int stm = 0; stm < LOCAL_DIM_STM; stm++) {
        for (int j = 0; j < D_; j++) {
            T_stm_[stm * D_ + j] = base_val + noise(rng) * 0.1f;
        }
    }

    // --- Bulk tensors [63, 13, D, D] ---
    for (int site = 0; site < NUM_BULK; site++) {
        for (int piece = 0; piece < LOCAL_DIM_SQ; piece++) {
            float* mat = bulk_matrix_mut(site, piece);
            for (int i = 0; i < D_; i++) {
                for (int j = 0; j < D_; j++) {
                    if (piece == 0) {
                        // Empty square: exact identity
                        // This ensures empty squares are "transparent" —
                        // they pass information through without modification
                        mat[i * D_ + j] = (i == j) ? 1.0f : 0.0f;
                    } else {
                        // Occupied square: identity + noise
                        // The noise breaks symmetry and allows learning
                        mat[i * D_ + j] = (i == j ? 1.0f : 0.0f) + noise(rng);
                    }
                }
            }
        }
    }

    // --- Right boundary [13, D] ---
    for (int piece = 0; piece < LOCAL_DIM_SQ; piece++) {
        for (int j = 0; j < D_; j++) {
            if (piece == 0) {
                T_right_[piece * D_ + j] = base_val;
            } else {
                T_right_[piece * D_ + j] = base_val + noise(rng) * 0.1f;
            }
        }
    }

    scale_ = 100.0f;
}

// =============================================================================
// Evaluation — Full MPS Contraction
// =============================================================================
//
// Computes: eval = v_L × A[sq_0] × A[sq_1] × ... × A[sq_62] · v_R[sq_63]
//
// Where:
//   v_L       = T_stm[side_to_move]         (shape [D])
//   A[sq_i]   = T_bulk[i][piece_on_sq_i]    (shape [D, D])
//   v_R[sq]   = T_right[piece_on_last_sq]   (shape [D])
//
// The contraction proceeds left-to-right as a sequence of matrix-vector
// multiplications, maintaining a running vector v of shape [D].
//
// Total cost: 63 × D² + D multiply-adds
//
// =============================================================================

int TensorMPS::evaluate(const Board& board) const {
    // 1. Calculate Base Material Score
    int white_mat = 0;
    int black_mat = 0;
    for (int sq = 0; sq < 64; sq++) {
        Piece p = board.piece_at(static_cast<Square>(sq));
        switch (p) {
            case Piece::WhitePawn:   white_mat += 100; break;
            case Piece::WhiteKnight: white_mat += 320; break;
            case Piece::WhiteBishop: white_mat += 330; break;
            case Piece::WhiteRook:   white_mat += 500; break;
            case Piece::WhiteQueen:  white_mat += 900; break;
            case Piece::BlackPawn:   black_mat += 100; break;
            case Piece::BlackKnight: black_mat += 320; break;
            case Piece::BlackBishop: black_mat += 330; break;
            case Piece::BlackRook:   black_mat += 500; break;
            case Piece::BlackQueen:  black_mat += 900; break;
            default: break;
        }
    }
    int material_cp = (board.side_to_move() == Color::White) ? (white_mat - black_mat) : (black_mat - white_mat);

    const auto& hilbert = HilbertCurve::order();

    // 2. Initialize running vector from side-to-move
    int stm_idx = (board.side_to_move() == Color::White) ? 0 : 1;
    const float* stm_vec = stm_vector(stm_idx);

    float v[128];
    float v_new[128];

    for (int j = 0; j < D_; j++) {
        v[j] = stm_vec[j];
    }

    // 3. Contract through 63 bulk sites with per-site L2 normalization
    for (int site = 0; site < NUM_BULK; site++) {
        Square sq = static_cast<Square>(hilbert[site]);
        Piece p = board.piece_at(sq);
        int local_idx = piece_to_local_index(p);

        const float* mat = bulk_matrix(site, local_idx);

        float norm_sq = 0.0f;
        for (int j = 0; j < D_; j++) {
            float sum = 0.0f;
            for (int k = 0; k < D_; k++) {
                sum += v[k] * mat[k * D_ + j];
            }
            v_new[j] = sum;
            norm_sq += sum * sum;
        }

        float norm = std::sqrt(norm_sq);
        if (norm > 1e-6f) {
            float inv_norm = 1.0f / norm;
            for (int j = 0; j < D_; j++) {
                v_new[j] *= inv_norm;
            }
        }

        std::memcpy(v, v_new, D_ * sizeof(float));
    }

    // 4. Contract with right boundary
    Square last_sq = static_cast<Square>(hilbert[63]);
    Piece last_p = board.piece_at(last_sq);
    int last_idx = piece_to_local_index(last_p);

    const float* rv = right_vector(last_idx);
    float raw_output = 0.0f;
    for (int j = 0; j < D_; j++) {
        raw_output += v[j] * rv[j];
    }

    // 5. Combine Base Material + Tensor MPS Residual Positional Correlation
    float mps_residual = std::max(-400.0f, std::min(400.0f, raw_output * scale_));
    float final_eval = static_cast<float>(material_cp) + mps_residual;

    return static_cast<int>(std::max(-30000.0f, std::min(30000.0f, final_eval)));
}

// =============================================================================
// Incremental Evaluation using Left Environment Caching
// =============================================================================

int TensorMPS::evaluate_incremental(const Board& board, Environment& env) const {
    if (env.L.size() != static_cast<size_t>(NUM_SITES * D_)) {
        env.L.resize(NUM_SITES * D_, 0.0f);
        env.valid_up_to = -1;
    }

    int white_mat = 0;
    int black_mat = 0;
    for (int sq = 0; sq < 64; sq++) {
        Piece p = board.piece_at(static_cast<Square>(sq));
        switch (p) {
            case Piece::WhitePawn:   white_mat += 100; break;
            case Piece::WhiteKnight: white_mat += 320; break;
            case Piece::WhiteBishop: white_mat += 330; break;
            case Piece::WhiteRook:   white_mat += 500; break;
            case Piece::WhiteQueen:  white_mat += 900; break;
            case Piece::BlackPawn:   black_mat += 100; break;
            case Piece::BlackKnight: black_mat += 320; break;
            case Piece::BlackBishop: black_mat += 330; break;
            case Piece::BlackRook:   black_mat += 500; break;
            case Piece::BlackQueen:  black_mat += 900; break;
            default: break;
        }
    }
    int material_cp = (board.side_to_move() == Color::White) ? (white_mat - black_mat) : (black_mat - white_mat);

    const auto& hilbert = HilbertCurve::order();
    int start_site = std::max(0, env.valid_up_to);

    if (start_site == 0) {
        int stm_idx = (board.side_to_move() == Color::White) ? 0 : 1;
        const float* stm_vec = stm_vector(stm_idx);
        std::memcpy(&env.L[0], stm_vec, D_ * sizeof(float));
    }

    for (int site = start_site; site < NUM_BULK; site++) {
        Square sq = static_cast<Square>(hilbert[site]);
        Piece p = board.piece_at(sq);
        int local_idx = piece_to_local_index(p);

        const float* mat = bulk_matrix(site, local_idx);
        const float* v_curr = &env.L[site * D_];
        float* v_next = &env.L[(site + 1) * D_];

        float norm_sq = 0.0f;
        for (int j = 0; j < D_; j++) {
            float sum = 0.0f;
            for (int k = 0; k < D_; k++) {
                sum += v_curr[k] * mat[k * D_ + j];
            }
            v_next[j] = sum;
            norm_sq += sum * sum;
        }

        float norm = std::sqrt(norm_sq);
        if (norm > 1e-6f) {
            float inv_norm = 1.0f / norm;
            for (int j = 0; j < D_; j++) {
                v_next[j] *= inv_norm;
            }
        }
    }

    env.valid_up_to = NUM_BULK;

    Square last_sq = static_cast<Square>(hilbert[63]);
    Piece last_p = board.piece_at(last_sq);
    int last_idx = piece_to_local_index(last_p);

    const float* rv = right_vector(last_idx);
    const float* v_final = &env.L[NUM_BULK * D_];

    float raw_output = 0.0f;
    for (int j = 0; j < D_; j++) {
        raw_output += v_final[j] * rv[j];
    }

    float mps_residual = std::max(-400.0f, std::min(400.0f, raw_output * scale_));
    float final_eval = static_cast<float>(material_cp) + mps_residual;

    return static_cast<int>(std::max(-30000.0f, std::min(30000.0f, final_eval)));
}

// =============================================================================
// Weight I/O
// =============================================================================
//
// Binary format (.tnw):
//   Header (16 bytes):
//     [4 bytes] Magic: 0x4E544748 ("HGTN")
//     [4 bytes] Version: 1
//     [4 bytes] Bond dimension D
//     [4 bytes] Num sites (65)
//   Body:
//     T_stm:   2 * D          floats
//     T_bulk:  63 * 13 * D*D  floats
//     T_right: 13 * D         floats
//     Scale:   1              float
//
// =============================================================================

bool TensorMPS::save_weights(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    // Header
    uint32_t magic = MAGIC;
    uint32_t version = 1;
    uint32_t bond = static_cast<uint32_t>(D_);
    uint32_t sites = static_cast<uint32_t>(NUM_SITES);

    out.write(reinterpret_cast<const char*>(&magic),   4);
    out.write(reinterpret_cast<const char*>(&version), 4);
    out.write(reinterpret_cast<const char*>(&bond),    4);
    out.write(reinterpret_cast<const char*>(&sites),   4);

    // Body
    out.write(reinterpret_cast<const char*>(T_stm_.data()),
              T_stm_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(T_bulk_.data()),
              T_bulk_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(T_right_.data()),
              T_right_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(&scale_), sizeof(float));

    return out.good();
}

bool TensorMPS::load_weights(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    // Header
    uint32_t magic, version, bond, sites;
    in.read(reinterpret_cast<char*>(&magic),   4);
    in.read(reinterpret_cast<char*>(&version), 4);
    in.read(reinterpret_cast<char*>(&bond),    4);
    in.read(reinterpret_cast<char*>(&sites),   4);

    if (magic != MAGIC || version != 1 || sites != NUM_SITES) {
        std::cerr << "[TensorMPS] Invalid weight file: " << path << "\n";
        return false;
    }

    // Resize if bond dimension changed
    D_ = static_cast<int>(bond);
    T_stm_.resize(LOCAL_DIM_STM * D_);
    T_bulk_.resize(NUM_BULK * LOCAL_DIM_SQ * D_ * D_);
    T_right_.resize(LOCAL_DIM_SQ * D_);

    // Body
    in.read(reinterpret_cast<char*>(T_stm_.data()),
            T_stm_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(T_bulk_.data()),
            T_bulk_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(T_right_.data()),
            T_right_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(&scale_), sizeof(float));

    if (!in.good()) {
        std::cerr << "[TensorMPS] Failed to read weight data from: " << path << "\n";
        return false;
    }

    std::cout << "[TensorMPS] Loaded weights: D=" << D_
              << ", params=" << num_parameters()
              << ", scale=" << scale_ << "\n";
    return true;
}

} // namespace heavensgate
