#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"
#include "move_list.hpp"
#include <array>
#include <vector>

namespace heavensgate {

class MoveGenerator {
public:
    static void init();

    static void generate_legal_moves(const Board& board, MoveList& moves);
    static void generate_capture_moves(const Board& board, MoveList& moves);
    static bool in_check(const Board& board, Color side);
    static bool gives_check(const Board& board, Move m);
    static bool is_square_attacked(const Board& board, Square sq, Color attacker_color);

private:
    static void generate_pawn_moves(const Board& board, Color side, MoveList& moves);
    static void generate_knight_moves(const Board& board, Color side, MoveList& moves);
    static void generate_bishop_moves(const Board& board, Color side, MoveList& moves);
    static void generate_rook_moves(const Board& board, Color side, MoveList& moves);
    static void generate_queen_moves(const Board& board, Color side, MoveList& moves);
    static void generate_king_moves(const Board& board, Color side, MoveList& moves);
    static void generate_castling_moves(const Board& board, Color side, MoveList& moves);
};

} // namespace heavensgate
