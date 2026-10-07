#pragma once

#include "../board/board.hpp"
#include "../movegen/move_list.hpp"
#include <array>
#include <memory>

namespace heavensgate {

struct ContHistoryTables {
    std::array<std::array<std::array<std::array<int16_t, 64>, 14>, 64>, 14> cont_history{};   // 1-ply (opp move 1 ply ago)
    std::array<std::array<std::array<std::array<int16_t, 64>, 14>, 64>, 14> cont_history_2{}; // 2-ply (self move 2 plies ago)
    std::array<std::array<std::array<std::array<int16_t, 64>, 14>, 64>, 14> cont_history_4{}; // 4-ply (self move 4 plies ago)
    std::array<std::array<std::array<std::array<int16_t, 64>, 14>, 64>, 14> cont_history_6{}; // 6-ply (self move 6 plies ago)
    std::array<std::array<std::array<int16_t, 6>, 64>, 14> capture_history{};
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
    Move get_killer_move(int ply, size_t slot) const noexcept {
        if (ply >= 256 || slot >= 2) return Move();
        return killer_moves_[static_cast<size_t>(ply)][slot];
    }
    void add_history_score(Color c, Move m, int depth) noexcept;
    void sub_history_score(Color c, Move m, int depth) noexcept;
    int get_history_score(Color c, Move m) const noexcept;
    void add_countermove(Move prev_move, Move countermove) noexcept;
    Move get_countermove(Move prev_move) const noexcept {
        if (!static_cast<bool>(prev_move)) return Move();
        size_t prev_from = static_cast<size_t>(prev_move.from());
        size_t prev_to = static_cast<size_t>(prev_move.to());
        if (prev_from < 64 && prev_to < 64) {
            return countermoves_[prev_from][prev_to];
        }
        return Move();
    }
    void add_continuation_history(const Board& board, Move prev_move, Move curr_move, int depth, Piece prev_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    void sub_continuation_history(const Board& board, Move prev_move, Move curr_move, int depth, Piece prev_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    int get_continuation_history(const Board& board, Move prev_move, Move curr_move, Piece prev_p = Piece::None, Piece curr_p = Piece::None) const noexcept;
    void add_continuation_history_2(const Board& board, Move prev2_move, Move curr_move, int depth, Piece prev2_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    void sub_continuation_history_2(const Board& board, Move prev2_move, Move curr_move, int depth, Piece prev2_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    int get_continuation_history_2(const Board& board, Move prev2_move, Move curr_move, Piece prev2_p = Piece::None, Piece curr_p = Piece::None) const noexcept;
    void add_continuation_history_4(const Board& board, Move prev4_move, Move curr_move, int depth, Piece prev4_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    void sub_continuation_history_4(const Board& board, Move prev4_move, Move curr_move, int depth, Piece prev4_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    int get_continuation_history_4(const Board& board, Move prev4_move, Move curr_move, Piece prev4_p = Piece::None, Piece curr_p = Piece::None) const noexcept;
    void add_continuation_history_6(const Board& board, Move prev6_move, Move curr_move, int depth, Piece prev6_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    void sub_continuation_history_6(const Board& board, Move prev6_move, Move curr_move, int depth, Piece prev6_p = Piece::None, Piece curr_p = Piece::None) noexcept;
    int get_continuation_history_6(const Board& board, Move prev6_move, Move curr_move, Piece prev6_p = Piece::None, Piece curr_p = Piece::None) const noexcept;
    void add_capture_history(Piece attacker, Square to, PieceType victim, int depth) noexcept;
    void sub_capture_history(Piece attacker, Square to, PieceType victim, int depth) noexcept;
    int get_capture_history(Piece attacker, Square to, PieceType victim) const noexcept;

    void score_moves(const Board& board, MoveList& moves, std::array<int, 256>& scores, int ply, Move pv_move = Move(), Move prev_move = Move(), Move prev2_move = Move(), Move prev4_move = Move(), Move prev6_move = Move(), Piece prev_piece = Piece::None, Piece prev2_piece = Piece::None, Piece prev4_piece = Piece::None, Piece prev6_piece = Piece::None) const noexcept;
    void score_captures_only(const Board& board, MoveList& moves, std::array<int, 256>& scores, Move pv_move = Move()) const noexcept;
    static void pick_best(MoveList& moves, std::array<int, 256>& scores, size_t start_idx) noexcept;

    void score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move = Move(), Move prev_move = Move(), Move prev2_move = Move(), Move prev4_move = Move(), Move prev6_move = Move(), Piece prev_piece = Piece::None, Piece prev2_piece = Piece::None, Piece prev4_piece = Piece::None, Piece prev6_piece = Piece::None) const noexcept;
    static bool see_ge(const Board& board, Move m, int threshold) noexcept;
};

} // namespace heavensgate
