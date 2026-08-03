#pragma once

#include "../board/board.hpp"
#include "pst.hpp"
#include "eval_features.hpp"

namespace heavensgate {

constexpr int PawnValue   = 100;
constexpr int KnightValue = 320;
constexpr int BishopValue = 330;
constexpr int RookValue   = 500;
constexpr int QueenValue  = 900;
constexpr int KingValue   = 20000;

class Evaluator {
public:
    static void init();
    static int evaluate(const Board& board);
    static int evaluate_side(const Board& board, Color side);
};

} // namespace heavensgate
