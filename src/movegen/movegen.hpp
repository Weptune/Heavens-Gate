#pragma once

#include "../board/board.hpp"
#include "move_list.hpp"
#include "attack_masks.hpp"

namespace heavensgate {

enum class MoveGenType {
    All,
    Captures,
    Quiets
};

class MoveGenerator {
public:
    static void init();

    // Check if a square is attacked by attacker_color
    static bool is_square_attacked(const Board& board, Square sq, Color attacker_color);

    // Check if color c is currently in check
    static bool in_check(const Board& board, Color c);

    // Generate pseudo-legal moves
    static void generate_pseudo_legal_moves(const Board& board, MoveList& moves, MoveGenType type = MoveGenType::All);

    // Generate strictly legal moves
    static void generate_legal_moves(Board& board, MoveList& moves, MoveGenType type = MoveGenType::All);
};

} // namespace heavensgate
