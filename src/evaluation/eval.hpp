#pragma once

#include "../board/board.hpp"
#include <cstdint>

namespace heavensgate {

constexpr int ScoreDraw     = 0;
constexpr int ScoreMate     = 30000;
constexpr int ScoreInfinity = 32000;

// Material values in centipawns
constexpr int PawnValue   = 100;
constexpr int KnightValue = 300;
constexpr int BishopValue = 325;
constexpr int RookValue   = 500;
constexpr int QueenValue  = 900;
constexpr int KingValue   = 20000;

class Evaluator {
public:
    // Returns evaluation score in centipawns relative to side to move
    static int evaluate(const Board& board);

    // Pure material counting relative to side to move
    static int evaluate_material(const Board& board);
};

} // namespace heavensgate
