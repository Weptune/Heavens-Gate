#pragma once

#include "../board/board.hpp"
#include "../movegen/move_list.hpp"
#include <array>
#include <memory>

namespace heavensgate {

struct ContHistoryTables {
    std::array<std::array<std::array<std::array<int, 64>, 14>, 64>, 14> cont_history{};
    std::array<std::array<std::array<std::array<int, 64>, 14>, 64>, 14> cont_history_2{};
    std::array<std::array<std::array<int, 6>, 64>, 14> capture_history{};
};

class MovePicker {
private:
    std::array<std::array<Move, 2>, 256> killer_moves_{};
    std::array<std::array<std::array<int, 64>, 64>, 2> history_scores_{};
    std::array<std::array<Move, 64>, 64> countermoves_{};
    std::unique_ptr<ContHistoryTables> cont_tables_;

public:
    MovePicker();
    ~MovePicker();
    MovePicker(const MovePicker& other);
    MovePicker& operator=(const MovePicker& other);
    MovePicker(MovePicker&&) noexcept;
    MovePicker& operator=(MovePicker&&) noexcept;

    void clear() noexcept;
    void age_history() noexcept;
    void add_killer_move(int ply, Move m) noexcept;
    void add_history_score(Color c, Move m, int depth) noexcept;
    void sub_history_score(Color c, Move m, int depth) noexcept;
    int get_history_score(Color c, Move m) const noexcept;
    void add_countermove(Move prev_move, Move countermove) noexcept;
    void add_continuation_history(const Board& board, Move prev_move, Move curr_move, int depth) noexcept;
    void sub_continuation_history(const Board& board, Move prev_move, Move curr_move, int depth) noexcept;
    int get_continuation_history(const Board& board, Move prev_move, Move curr_move) const noexcept;
    void add_continuation_history_2(const Board& board, Move prev2_move, Move curr_move, int depth) noexcept;
    void sub_continuation_history_2(const Board& board, Move prev2_move, Move curr_move, int depth) noexcept;
    int get_continuation_history_2(const Board& board, Move prev2_move, Move curr_move) const noexcept;
    void add_capture_history(Piece attacker, Square to, PieceType victim, int depth) noexcept;
    int get_capture_history(Piece attacker, Square to, PieceType victim) const noexcept;

    void score_moves(const Board& board, MoveList& moves, std::array<int, 256>& scores, int ply, Move pv_move = Move(), Move prev_move = Move(), Move prev2_move = Move()) const noexcept;
    void score_captures_only(const Board& board, MoveList& moves, std::array<int, 256>& scores, Move pv_move = Move()) const noexcept;
    static void pick_best(MoveList& moves, std::array<int, 256>& scores, size_t start_idx) noexcept;

    void score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move = Move(), Move prev_move = Move(), Move prev2_move = Move()) const noexcept;
    static bool see_ge(const Board& board, Move m, int threshold) noexcept;
};

} // namespace heavensgate
