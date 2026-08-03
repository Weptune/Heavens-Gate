#pragma once

#include "../board/board.hpp"
#include "move_list.hpp"

namespace heavensgate {

class MoveGenerator {
public:
    static void init();
    static void generate_legal_moves(const Board& board, MoveList& moves);
    static void generate_capture_moves(const Board& board, MoveList& moves);
    static bool in_check(const Board& board, Color c);
    static bool is_square_attacked(const Board& board, Square sq, Color by_color);
};

} // namespace heavensgate
