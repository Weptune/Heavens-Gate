#pragma once

#include "../board/board.hpp"
#include "../movegen/move_list.hpp"
#include "pv.hpp"

namespace heavensgate {

class MovePicker {
private:
    std::array<std::array<Move, 2>, MaxSearchDepth> killer_moves_{};
    std::array<std::array<std::array<int, 64>, 64>, 2> history_table_{};

    static constexpr int ScorePVMove    = 2000000;
    static constexpr int ScoreMVV_LVA   = 1000000;
    static constexpr int ScoreKiller1   =  900000;
    static constexpr int ScoreKiller2   =  800000;

    int score_move(const Board& board, Move m, int ply, Move pv_move) const noexcept;

public:
    void clear() noexcept {
        killer_moves_.fill({Move(), Move()});
        for (auto& color_hist : history_table_) {
            for (auto& from_hist : color_hist) {
                from_hist.fill(0);
            }
        }
    }

    void add_killer_move(int ply, Move m) noexcept {
        if (ply >= MaxSearchDepth || m.is_capture()) return;
        if (killer_moves_[ply][0] != m) {
            killer_moves_[ply][1] = killer_moves_[ply][0];
            killer_moves_[ply][0] = m;
        }
    }

    void add_history_score(Color c, Move m, int depth) noexcept {
        if (m.is_capture()) return;
        size_t color_idx = static_cast<size_t>(c);
        size_t from_idx  = static_cast<size_t>(m.from());
        size_t to_idx    = static_cast<size_t>(m.to());

        history_table_[color_idx][from_idx][to_idx] += depth * depth;
        if (history_table_[color_idx][from_idx][to_idx] > 500000) {
            history_table_[color_idx][from_idx][to_idx] /= 2;
        }
    }

    void score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move = Move()) const;
};

} // namespace heavensgate
