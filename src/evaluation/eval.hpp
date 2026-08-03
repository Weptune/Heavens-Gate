#pragma once

#include "../core/types.hpp"
#include "eval_features.hpp"
#include "nnue.hpp"

namespace heavensgate {

class Board; // Forward declaration

constexpr int PawnValue   = 100;
constexpr int KnightValue = 320;
constexpr int BishopValue = 330;
constexpr int RookValue   = 500;
constexpr int QueenValue  = 900;

enum class EvalMode {
    MaterialOnly,
    MasterPositional,
    NNUE,
    TensorNetwork
};

class Evaluator {
public:
    static void init();
    static int evaluate(const Board& board);

    static void set_mode(EvalMode mode) { current_mode_ = mode; }
    static EvalMode mode() { return current_mode_; }

private:
    static int evaluate_side(const Board& board, Color side);

    static EvalMode current_mode_;
};

} // namespace heavensgate
