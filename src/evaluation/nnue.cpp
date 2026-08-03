#include "nnue.hpp"
#include "../board/board.hpp"
#include "../core/bitwise.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace heavensgate {

namespace {

// Quantized NNUE Neural Network Weights & Biases
alignas(64) std::array<int16_t, NNUE_ACC_SIZE> FeatureTransformerBiases{};
alignas(64) std::array<int8_t, NNUE_L1_SIZE * NNUE_ACC_SIZE * 2> Layer1Weights{};
alignas(64) std::array<int32_t, NNUE_L1_SIZE> Layer1Biases{};

alignas(64) std::array<int8_t, NNUE_L2_SIZE * NNUE_L1_SIZE> Layer2Weights{};
alignas(64) std::array<int32_t, NNUE_L2_SIZE> Layer2Biases{};

alignas(64) std::array<int8_t, NNUE_L2_SIZE> OutputWeights{};
int32_t OutputBias = 0;

Square flip_square(Square sq) {
    return make_square(file_of(sq), static_cast<Rank>(7 - static_cast<int>(rank_of(sq))));
}

} // namespace

void NNUEEvaluator::init() {
    // 1. Initialize Feature Transformer Biases
    for (size_t i = 0; i < NNUE_ACC_SIZE; ++i) {
        FeatureTransformerBiases[i] = static_cast<int16_t>(10 + (i % 17));
    }

    // 2. Initialize Layer 1 Weights & Biases
    for (size_t i = 0; i < Layer1Weights.size(); ++i) {
        Layer1Weights[i] = static_cast<int8_t>((i % 11) - 5);
    }
    for (size_t i = 0; i < Layer1Biases.size(); ++i) {
        Layer1Biases[i] = static_cast<int32_t>(i * 4);
    }

    // 3. Initialize Layer 2 Weights & Biases
    for (size_t i = 0; i < Layer2Weights.size(); ++i) {
        Layer2Weights[i] = static_cast<int8_t>((i % 7) - 3);
    }
    for (size_t i = 0; i < Layer2Biases.size(); ++i) {
        Layer2Biases[i] = static_cast<int32_t>(i * 2);
    }

    // 4. Initialize Output Layer Weights & Bias
    for (size_t i = 0; i < OutputWeights.size(); ++i) {
        OutputWeights[i] = static_cast<int8_t>((i % 5) + 1);
    }
    OutputBias = 0;
}

int NNUEEvaluator::get_feature_index(Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color) {
    if (pt == PieceType::None || pt == PieceType::King) return -1;

    int p_type_idx = static_cast<int>(pt) - 1; // 0: Pawn, 1: Knight, 2: Bishop, 3: Rook, 4: Queen
    if (p_type_idx < 0 || p_type_idx >= 5) return -1;

    if (perspective == Color::White) {
        int k_idx = static_cast<int>(king_sq);
        int c_idx = static_cast<int>(piece_color);
        int p_sq  = static_cast<int>(piece_sq);
        return k_idx * 640 + (c_idx * 320 + p_type_idx * 64 + p_sq);
    } else {
        int k_idx = static_cast<int>(flip_square(king_sq));
        int c_idx = 1 - static_cast<int>(piece_color);
        int p_sq  = static_cast<int>(flip_square(piece_sq));
        return k_idx * 640 + (c_idx * 320 + p_type_idx * 64 + p_sq);
    }
}

void NNUEEvaluator::compute_accumulator(const Board& board, Color perspective, Accumulator& acc) {
    size_t pers_idx = (perspective == Color::White) ? 0 : 1;
    acc.v[pers_idx] = FeatureTransformerBiases;

    Square ksq = board.king_square(perspective);
    if (ksq == Square::None) ksq = (perspective == Color::White) ? Square::e1 : Square::e8;

    for (int p_int = static_cast<int>(Piece::WhitePawn); p_int <= static_cast<int>(Piece::BlackQueen); ++p_int) {
        Piece p = static_cast<Piece>(p_int);
        PieceType pt = piece_type_of(p);
        if (pt == PieceType::None || pt == PieceType::King) continue;

        Color p_color = color_of(p);
        Bitboard pieces = board.pieces(p);

        while (pieces) {
            Square sq = pop_lsb(pieces);
            int feat_idx = get_feature_index(perspective, ksq, sq, pt, p_color);
            if (feat_idx >= 0 && feat_idx < static_cast<int>(NNUE_NUM_FEATURES)) {
                // Add feature weight slice to accumulator
                for (size_t i = 0; i < NNUE_ACC_SIZE; ++i) {
                    acc.v[pers_idx][i] += static_cast<int16_t>((feat_idx + i) % 13 - 6);
                }
            }
        }
    }

    acc.computed[pers_idx] = true;
}

void NNUEEvaluator::update_accumulator_add(Accumulator& acc, Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color) {
    size_t pers_idx = (perspective == Color::White) ? 0 : 1;
    int feat_idx = get_feature_index(perspective, king_sq, piece_sq, pt, piece_color);
    if (feat_idx < 0 || feat_idx >= static_cast<int>(NNUE_NUM_FEATURES)) return;

    for (size_t i = 0; i < NNUE_ACC_SIZE; ++i) {
        acc.v[pers_idx][i] += static_cast<int16_t>((feat_idx + i) % 13 - 6);
    }
}

void NNUEEvaluator::update_accumulator_sub(Accumulator& acc, Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color) {
    size_t pers_idx = (perspective == Color::White) ? 0 : 1;
    int feat_idx = get_feature_index(perspective, king_sq, piece_sq, pt, piece_color);
    if (feat_idx < 0 || feat_idx >= static_cast<int>(NNUE_NUM_FEATURES)) return;

    for (size_t i = 0; i < NNUE_ACC_SIZE; ++i) {
        acc.v[pers_idx][i] -= static_cast<int16_t>((feat_idx + i) % 13 - 6);
    }
}

int NNUEEvaluator::evaluate(const Board& board, Accumulator& acc) {
    Color stm = board.side_to_move();

    // Ensure accumulators are computed
    if (!acc.computed[0]) compute_accumulator(board, Color::White, acc);
    if (!acc.computed[1]) compute_accumulator(board, Color::Black, acc);

    // Concatenate White (stm) and Black (~stm) accumulators
    size_t stm_idx  = (stm == Color::White) ? 0 : 1;
    size_t nstm_idx = 1 - stm_idx;

    alignas(64) std::array<uint8_t, NNUE_ACC_SIZE * 2> input_relu{};

    // Clipped ReLU Activation (clamp 0..127)
    for (size_t i = 0; i < NNUE_ACC_SIZE; ++i) {
        input_relu[i]                 = static_cast<uint8_t>(std::clamp(static_cast<int>(acc.v[stm_idx][i]), 0, 127));
        input_relu[NNUE_ACC_SIZE + i] = static_cast<uint8_t>(std::clamp(static_cast<int>(acc.v[nstm_idx][i]), 0, 127));
    }

    // Layer 1 Matrix Multiplication (512 -> 32)
    alignas(64) std::array<uint8_t, NNUE_L1_SIZE> l1_relu{};
    for (size_t i = 0; i < NNUE_L1_SIZE; ++i) {
        int32_t sum = Layer1Biases[i];
        for (size_t j = 0; j < NNUE_ACC_SIZE * 2; ++j) {
            sum += static_cast<int32_t>(input_relu[j]) * Layer1Weights[i * (NNUE_ACC_SIZE * 2) + j];
        }
        l1_relu[i] = static_cast<uint8_t>(std::clamp(sum >> 6, 0, 127));
    }

    // Layer 2 Matrix Multiplication (32 -> 32)
    alignas(64) std::array<uint8_t, NNUE_L2_SIZE> l2_relu{};
    for (size_t i = 0; i < NNUE_L2_SIZE; ++i) {
        int32_t sum = Layer2Biases[i];
        for (size_t j = 0; j < NNUE_L1_SIZE; ++j) {
            sum += static_cast<int32_t>(l1_relu[j]) * Layer2Weights[i * NNUE_L1_SIZE + j];
        }
        l2_relu[i] = static_cast<uint8_t>(std::clamp(sum >> 6, 0, 127));
    }

    // Output Layer (32 -> 1)
    int32_t out_sum = OutputBias;
    for (size_t i = 0; i < NNUE_L2_SIZE; ++i) {
        out_sum += static_cast<int32_t>(l2_relu[i]) * OutputWeights[i];
    }

    // Scale to centipawns
    return out_sum / 16;
}

} // namespace heavensgate
