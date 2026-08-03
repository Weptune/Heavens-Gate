#include "move_picker.hpp"
#include "../evaluation/eval.hpp"
#include <algorithm>

namespace heavensgate {

MovePicker::MovePicker() {
    clear();
}

void MovePicker::clear() noexcept {
    for (auto& ply_killers : killer_moves_) {
        ply_killers[0] = Move();
        ply_killers[1] = Move();
    }
    for (auto& c_table : history_table_) {
        for (auto& from_table : c_table) {
            from_table.fill(0);
        }
    }
    for (auto& from_table : countermove_table_) {
        from_table.fill(Move());
    }
}

void MovePicker::add_killer_move(int ply, Move m) noexcept {
    if (ply < 128 && !m.is_capture()) {
        if (killer_moves_[ply][0] != m) {
            killer_moves_[ply][1] = killer_moves_[ply][0];
            killer_moves_[ply][0] = m;
        }
    }
}

void MovePicker::add_history_score(Color c, Move m, int depth) noexcept {
    if (c != Color::None && !m.is_capture()) {
        size_t c_idx = static_cast<size_t>(c);
        size_t from  = static_cast<size_t>(m.from());
        size_t to    = static_cast<size_t>(m.to());
        history_table_[c_idx][from][to] += depth * depth;
    }
}

void MovePicker::add_countermove(Move prev_move, Move countermove) noexcept {
    if (static_cast<bool>(prev_move) && static_cast<bool>(countermove)) {
        size_t from = static_cast<size_t>(prev_move.from());
        size_t to   = static_cast<size_t>(prev_move.to());
        countermove_table_[from][to] = countermove;
    }
}

int MovePicker::score_move(const Board& board, Move m, Move pv_move, Move countermove, int ply) const noexcept {
    // 1. PV Move from TT gets highest priority
    if (m == pv_move) {
        return 1'000'000;
    }

    // 2. MVV-LVA for Captures
    if (m.is_capture()) {
        Piece attacker = board.piece_at(m.from());
        Piece victim   = (m.type() == MoveType::EnPassant)
            ? make_piece(~board.side_to_move(), PieceType::Pawn)
            : board.piece_at(m.to());

        int victim_val = 0;
        switch (piece_type_of(victim)) {
            case PieceType::Pawn:   victim_val = PawnValue; break;
            case PieceType::Knight: victim_val = KnightValue; break;
            case PieceType::Bishop: victim_val = BishopValue; break;
            case PieceType::Rook:   victim_val = RookValue; break;
            case PieceType::Queen:  victim_val = QueenValue; break;
            default: break;
        }

        int attacker_val = 0;
        switch (piece_type_of(attacker)) {
            case PieceType::Pawn:   attacker_val = PawnValue; break;
            case PieceType::Knight: attacker_val = KnightValue; break;
            case PieceType::Bishop: attacker_val = BishopValue; break;
            case PieceType::Rook:   attacker_val = RookValue; break;
            case PieceType::Queen:  attacker_val = QueenValue; break;
            default: break;
        }

        return 100'000 + (victim_val * 10 - attacker_val);
    }

    // 3. Killer Moves
    if (ply < 128) {
        if (m == killer_moves_[ply][0]) return 90'000;
        if (m == killer_moves_[ply][1]) return 80'000;
    }

    // 4. Countermove Heuristic
    if (m == countermove) {
        return 70'000;
    }

    // 5. History Heuristic
    Color us = board.side_to_move();
    if (us != Color::None) {
        size_t c_idx = static_cast<size_t>(us);
        size_t from  = static_cast<size_t>(m.from());
        size_t to    = static_cast<size_t>(m.to());
        return history_table_[c_idx][from][to];
    }

    return 0;
}

void MovePicker::score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move, Move prev_move) const noexcept {
    if (moves.empty()) return;

    Move countermove = Move();
    if (static_cast<bool>(prev_move)) {
        size_t from = static_cast<size_t>(prev_move.from());
        size_t to   = static_cast<size_t>(prev_move.to());
        countermove = countermove_table_[from][to];
    }

    std::vector<std::pair<int, Move>> scored_moves;
    scored_moves.reserve(moves.size());

    for (const auto& m : moves) {
        int score = score_move(board, m, pv_move, countermove, ply);
        scored_moves.push_back({score, m});
    }

    std::sort(scored_moves.begin(), scored_moves.end(),
        [](const auto& a, const auto& b) {
            return a.first > b.first;
        });

    moves.clear();
    for (const auto& item : scored_moves) {
        moves.push_back(item.second);
    }
}

} // namespace heavensgate
