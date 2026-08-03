#pragma once

#include "../core/types.hpp"
#include <array>
#include <cstdint>

namespace heavensgate {

class Board; // Forward declaration
struct Accumulator; // Forward declaration

constexpr size_t NNUE_ACC_SIZE = 256;
constexpr size_t NNUE_L1_SIZE  = 32;
constexpr size_t NNUE_L2_SIZE  = 32;
constexpr size_t NNUE_NUM_FEATURES = 40960; // 64 king squares * (2 colors * 5 piece types * 64 piece squares)

class NNUEEvaluator {
public:
    static void init();

    // Feature Index Calculator
    static int get_feature_index(Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color);

    // Full Accumulator Calculation from Board
    static void compute_accumulator(const Board& board, Color perspective, Accumulator& acc);

    // Incremental Accumulator Update
    static void update_accumulator_add(Accumulator& acc, Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color);
    static void update_accumulator_sub(Accumulator& acc, Color perspective, Square king_sq, Square piece_sq, PieceType pt, Color piece_color);

    // Feedforward Evaluation (Returns score in centipawns from side_to_move perspective)
    static int evaluate(const Board& board, Accumulator& acc);
};

} // namespace heavensgate
