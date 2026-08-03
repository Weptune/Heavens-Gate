#pragma once

#include "../board/board.hpp"
#include <array>

namespace heavensgate {

class EvalFeatures {
public:
    static std::array<Bitboard, 64> PassedPawnMask[2];
    static std::array<Bitboard, 8>  IsolatedPawnMask;
    static std::array<int, 32>      KingDangerTable;
    static Bitboard LightSquaresBB;
    static Bitboard DarkSquaresBB;

    static void init();

    // Feature Evaluation Functions (Returns score in centipawns from perspective of 'us')
    static int evaluate_pawn_structure(const Board& board, Color us);
    static int evaluate_passed_pawns(const Board& board, Color us);
    static int evaluate_king_safety(const Board& board, Color us);
    static int evaluate_piece_activity(const Board& board, Color us);
    static int evaluate_bad_bishops(const Board& board, Color us);
    static int evaluate_pin_threats(const Board& board, Color us);
    static int evaluate_mobility(const Board& board, Color us);
};

} // namespace heavensgate
