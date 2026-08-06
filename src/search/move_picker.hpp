#pragma once

#include "../board/board.hpp"
#include "../movegen/move_list.hpp"
#include <array>

namespace heavensgate {

class MovePicker {
private:
    std::array<std::array<Move, 2>, 128> killer_moves_{};
    std::array<std::array<std::array<int, 64>, 64>, 2> history_scores_{};
    std::array<std::array<Move, 64>, 64> countermoves_{};

public:
    MovePicker();

    void clear() noexcept;
    void add_killer_move(int ply, Move m) noexcept;
    void add_history_score(Color c, Move m, int depth) noexcept;
    int get_history_score(Color c, Move m) const noexcept;
    void add_countermove(Move prev_move, Move countermove) noexcept;

    void score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move = Move(), Move prev_move = Move()) const noexcept;
};

} // namespace heavensgate
