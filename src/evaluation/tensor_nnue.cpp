#include "tensor_nnue.hpp"
#include "../board/board.hpp"
#include "../movegen/movegen.hpp"
#include <fstream>
#include <iostream>
#include <random>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace heavensgate {

TensorNNUE::TensorNNUE(size_t rank) : R_(rank) {
    U_factor_.resize(NUM_FEATURES * R_, 0.0f);
    V_core_.resize(R_ * HIDDEN_DIM, 0.0f);
    W_out_.resize(OUTPUT_DIM, 0.0f);
    initialize_weights(42);
}

TensorNNUE& TensorNNUE::instance() {
    static TensorNNUE instance(BOND_RANK);
    static bool initialized = false;
    if (!initialized) {
        instance.load_weights("heavensgate_tnnue.nnue");
        initialized = true;
    }
    return instance;
}

void TensorNNUE::initialize_weights(uint32_t seed) {
    std::mt19937 rng(seed);
    std::normal_distribution<float> norm_u(0.0f, std::sqrt(2.0f / static_cast<float>(R_)));
    std::normal_distribution<float> norm_v(0.0f, std::sqrt(2.0f / static_cast<float>(HIDDEN_DIM)));
    std::normal_distribution<float> norm_w(0.0f, std::sqrt(2.0f / static_cast<float>(OUTPUT_DIM)));

    for (auto& val : U_factor_) val = norm_u(rng) * 0.05f;
    for (auto& val : V_core_)   val = norm_v(rng) * 0.1f;
    for (auto& val : W_out_)    val = norm_w(rng) * 0.5f;

    bias_out_ = 0.0f;
}

void TensorNNUE::compute_accumulator(const Board& board, Color perspective, Accumulator& acc) const {
    acc.factors.fill(0.0f);

    Piece king_piece = (perspective == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
    Bitboard king_bb = board.pieces(king_piece);
    if (!king_bb) return;

    Square king_sq = lsb(king_bb);

    for (int p_idx = 0; p_idx < 12; p_idx++) {
        Piece p = static_cast<Piece>(p_idx);
        if (p == Piece::WhiteKing || p == Piece::BlackKing) continue;

        Color piece_color = color_of(p);
        PieceType pt = piece_type_of(p);
        Bitboard bb = board.pieces(p);

        while (bb) {
            Square piece_sq = pop_lsb(bb);
            int idx = NNUEEvaluator::get_feature_index(perspective, king_sq, piece_sq, pt, piece_color);
            if (idx >= 0 && idx < static_cast<int>(NUM_FEATURES)) {
                const float* u_vec = &U_factor_[idx * R_];
                for (size_t r = 0; r < R_; r++) {
                    acc.factors[r] += u_vec[r];
                }
            }
        }
    }
}

int TensorNNUE::evaluate(const Board& board) const {
    Color us = board.side_to_move();
    Color them = ~us;

    Accumulator acc_us;
    Accumulator acc_them;

    compute_accumulator(board, us, acc_us);
    compute_accumulator(board, them, acc_them);

    // Layer 1: ClippedReLU Activation over V_core tensor
    float h_us[HIDDEN_DIM];
    float h_them[HIDDEN_DIM];

    for (size_t d = 0; d < HIDDEN_DIM; d++) {
        float sum_us = 0.0f;
        float sum_them = 0.0f;
        for (size_t r = 0; r < R_; r++) {
            float v_val = V_core_[r * HIDDEN_DIM + d];
            sum_us   += acc_us.factors[r] * v_val;
            sum_them += acc_them.factors[r] * v_val;
        }
        h_us[d]   = std::max(0.0f, std::min(1.0f, sum_us));
        h_them[d] = std::max(0.0f, std::min(1.0f, sum_them));
    }

    // Output Layer Dot Product
    float score = bias_out_;
    for (size_t d = 0; d < HIDDEN_DIM; d++) {
        score += h_us[d] * W_out_[d];
        score += h_them[d] * W_out_[HIDDEN_DIM + d];
    }

    // Scale to centipawns (1.0 = ~100 cp)
    float eval_cp = score * 400.0f;
    return static_cast<int>(std::max(-30000.0f, std::min(30000.0f, eval_cp)));
}

bool TensorNNUE::save_weights(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    uint32_t magic = MAGIC;
    uint32_t num_f = static_cast<uint32_t>(NUM_FEATURES);
    uint32_t rank  = static_cast<uint32_t>(R_);
    uint32_t hdim  = static_cast<uint32_t>(HIDDEN_DIM);

    out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    out.write(reinterpret_cast<const char*>(&num_f), sizeof(num_f));
    out.write(reinterpret_cast<const char*>(&rank), sizeof(rank));
    out.write(reinterpret_cast<const char*>(&hdim), sizeof(hdim));

    out.write(reinterpret_cast<const char*>(U_factor_.data()), U_factor_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(V_core_.data()), V_core_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(W_out_.data()), W_out_.size() * sizeof(float));
    out.write(reinterpret_cast<const char*>(&bias_out_), sizeof(bias_out_));

    return out.good();
}

bool TensorNNUE::load_weights(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;

    uint32_t magic = 0, num_f = 0, rank = 0, hdim = 0;
    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != MAGIC) return false;

    in.read(reinterpret_cast<char*>(&num_f), sizeof(num_f));
    in.read(reinterpret_cast<char*>(&rank), sizeof(rank));
    in.read(reinterpret_cast<char*>(&hdim), sizeof(hdim));

    R_ = rank;
    U_factor_.resize(num_f * rank);
    V_core_.resize(rank * hdim);
    W_out_.resize(2 * hdim);

    in.read(reinterpret_cast<char*>(U_factor_.data()), U_factor_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(V_core_.data()), V_core_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(W_out_.data()), W_out_.size() * sizeof(float));
    in.read(reinterpret_cast<char*>(&bias_out_), sizeof(bias_out_));

    return in.good();
}

} // namespace heavensgate
