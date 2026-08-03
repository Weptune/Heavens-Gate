#pragma once

#include "../board/board.hpp"
#include "../movegen/attack_masks.hpp"
#include <array>

namespace heavensgate {

class EvalFeatures {
public:
    static std::array<Bitboard, 64> PassedPawnMask[2];
    static std::array<Bitboard, 8> IsolatedPawnMask;
    static std::array<int, 32> KingDangerTable;

    static void init();

    static int evaluate_pawn_structure(const Board& board, Color us);
    static int evaluate_passed_pawns(const Board& board, Color us);
    static int evaluate_king_safety(const Board& board, Color us);
    static int evaluate_piece_activity(const Board& board, Color us);
    static int evaluate_mobility(const Board& board, Color us);
};

} // namespace heavensgate
