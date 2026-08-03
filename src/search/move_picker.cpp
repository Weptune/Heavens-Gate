#include "move_picker.hpp"
#include "../evaluation/eval.hpp"
#include <algorithm>

namespace heavensgate {

static constexpr std::array<int, 6> PieceValuesMVV = {
    PawnValue, KnightValue, BishopValue, RookValue, QueenValue, KingValue
};

int MovePicker::score_move(const Board& board, Move m, int ply, Move pv_move) const noexcept {
    if (m == pv_move) return ScorePVMove;

    if (m.is_capture()) {
        Piece victim = board.piece_at(m.to());
        Piece attacker = board.piece_at(m.from());

        if (m.type() == MoveType::EnPassant) {
            victim = make_piece(~board.side_to_move(), PieceType::Pawn);
        }

        int victim_val = PieceValuesMVV[static_cast<size_t>(piece_type_of(victim))];
        int attacker_val = PieceValuesMVV[static_cast<size_t>(piece_type_of(attacker))];

        return ScoreMVV_LVA + (victim_val * 10 - attacker_val);
    }

    // Quiet moves heuristics
    if (ply < MaxSearchDepth) {
        if (m == killer_moves_[ply][0]) return ScoreKiller1;
        if (m == killer_moves_[ply][1]) return ScoreKiller2;
    }

    size_t color_idx = static_cast<size_t>(board.side_to_move());
    size_t from_idx  = static_cast<size_t>(m.from());
    size_t to_idx    = static_cast<size_t>(m.to());

    return history_table_[color_idx][from_idx][to_idx];
}

void MovePicker::score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move pv_move) const {
    if (moves.size() <= 1) return;

    std::array<int, 256> scores{};
    for (size_t i = 0; i < moves.size(); ++i) {
        scores[i] = score_move(board, moves[i], ply, pv_move);
    }

    // In-place insertion sort (fastest for small N <= 35)
    for (size_t i = 1; i < moves.size(); ++i) {
        Move key_move = moves[i];
        int key_score = scores[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0 && scores[j] < key_score) {
            moves[j + 1] = moves[j];
            scores[j + 1] = scores[j];
            j--;
        }
        moves[j + 1] = key_move;
        scores[j + 1] = key_score;
    }
}

} // namespace heavensgate
