#pragma once

#include "../board/board.hpp"
#include "pst.hpp"
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
    // Tapered evaluation combining material, PSTs, mobility, and pawn structure
    static int evaluate(const Board& board);

    // Material counting only
    static int evaluate_material(const Board& board);

    // Game phase calculation (24 = Midgame, 0 = Endgame)
    static int calculate_game_phase(const Board& board) noexcept;
};

} // namespace heavensgate
