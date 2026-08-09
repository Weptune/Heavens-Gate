#include "move_picker.hpp"
#include "../evaluation/eval.hpp"
#include "../movegen/movegen.hpp"
#include <algorithm>

namespace heavensgate {

MovePicker::MovePicker() {
    clear();
}

void MovePicker::clear() noexcept {
    killer_moves_.fill({});
    history_scores_.fill({});
    countermoves_.fill({});
}

void MovePicker::age_history() noexcept {
    killer_moves_.fill({});
    countermoves_.fill({});
    for (auto& c : history_scores_) {
        for (auto& f : c) {
            for (auto& val : f) {
                val /= 2;
            }
        }
    }
}

void MovePicker::add_killer_move(int ply, Move move) noexcept {
    if (ply >= 128) return;
    if (killer_moves_[static_cast<size_t>(ply)][0] != move) {
        killer_moves_[static_cast<size_t>(ply)][1] = killer_moves_[static_cast<size_t>(ply)][0];
        killer_moves_[static_cast<size_t>(ply)][0] = move;
    }
}

void MovePicker::add_history_score(Color side, Move move, int depth) noexcept {
    size_t c_idx = static_cast<size_t>(side);
    size_t from_idx = static_cast<size_t>(move.from());
    size_t to_idx = static_cast<size_t>(move.to());

    history_scores_[c_idx][from_idx][to_idx] += depth * depth;
    if (history_scores_[c_idx][from_idx][to_idx] > 10000) {
        for (auto& from_arr : history_scores_[c_idx]) {
            for (auto& val : from_arr) {
                val /= 2;
            }
        }
    }
}

int MovePicker::get_history_score(Color c, Move move) const noexcept {
    size_t c_idx = static_cast<size_t>(c);
    size_t from_idx = static_cast<size_t>(move.from());
    size_t to_idx = static_cast<size_t>(move.to());
    return history_scores_[c_idx][from_idx][to_idx];
}

void MovePicker::add_countermove(Move prev_move, Move move) noexcept {
    size_t from_idx = static_cast<size_t>(prev_move.from());
    size_t to_idx = static_cast<size_t>(prev_move.to());
    countermoves_[from_idx][to_idx] = move;
}

void MovePicker::score_and_sort_moves(const Board& board, MoveList& moves, int ply, Move tt_move, Move prev_move) const noexcept {
    std::vector<int> scores(moves.size(), 0);

    Move killer1 = (ply < 128) ? killer_moves_[static_cast<size_t>(ply)][0] : Move();
    Move killer2 = (ply < 128) ? killer_moves_[static_cast<size_t>(ply)][1] : Move();

    Move countermove = Move();
    if (static_cast<bool>(prev_move)) {
        size_t prev_from = static_cast<size_t>(prev_move.from());
        size_t prev_to   = static_cast<size_t>(prev_move.to());
        countermove = countermoves_[prev_from][prev_to];
    }

    Color side = board.side_to_move();
    size_t c_idx = static_cast<size_t>(side);

    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];

        // 50-Move Rule Mitigation Priority: Prioritize Pawn Pushes and Captures when halfmove clock >= 70
        bool is_clock_resetter = m.is_capture() || (piece_type_of(board.piece_at(m.from())) == PieceType::Pawn);
        int clock_boost = (board.halfmove_clock() >= 70 && is_clock_resetter) ? 1500000 : 0;

        if (m == tt_move) {
            scores[i] = 2000000;
        } else if (m.type() == MoveType::PromoQueen || m.type() == MoveType::PromoCaptureQueen) {
            scores[i] = 950000 + clock_boost;
        } else if (m.is_promotion()) {
            scores[i] = 200000 + clock_boost;
        } else if (m.is_capture()) {
            Piece attacker = board.piece_at(m.from());
            Piece victim   = board.piece_at(m.to());

            int victim_val = PawnValue;
            switch (piece_type_of(victim)) {
                case PieceType::Pawn:   victim_val = PawnValue; break;
                case PieceType::Knight: victim_val = KnightValue; break;
                case PieceType::Bishop: victim_val = BishopValue; break;
                case PieceType::Rook:   victim_val = RookValue; break;
                case PieceType::Queen:  victim_val = QueenValue; break;
                default: break;
            }

            int attacker_val = PawnValue;
            switch (piece_type_of(attacker)) {
                case PieceType::Pawn:   attacker_val = PawnValue; break;
                case PieceType::Knight: attacker_val = KnightValue; break;
                case PieceType::Bishop: attacker_val = BishopValue; break;
                case PieceType::Rook:   attacker_val = RookValue; break;
                case PieceType::Queen:  attacker_val = QueenValue; break;
                default: break;
            }

            bool is_good_see = see_ge(board, m, 0);
            scores[i] = (is_good_see ? 1000000 : -100000) + (victim_val * 10 - attacker_val);
        } else if (m.type() == MoveType::KingCastle || m.type() == MoveType::QueenCastle) {
            scores[i] = 850000;
        } else if (m == killer1) {
            scores[i] = 800000;
        } else if (m == killer2) {
            scores[i] = 700000;
        } else if (m == countermove) {
            scores[i] = 600000;
        } else {
            size_t from_idx = static_cast<size_t>(m.from());
            size_t to_idx   = static_cast<size_t>(m.to());
            scores[i] = history_scores_[c_idx][from_idx][to_idx] + clock_boost;
        }
    }

    // Insertion sort
    for (size_t i = 1; i < moves.size(); ++i) {
        Move key_move = moves[i];
        int key_score = scores[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0 && scores[static_cast<size_t>(j)] < key_score) {
            moves[static_cast<size_t>(j + 1)] = moves[static_cast<size_t>(j)];
            scores[static_cast<size_t>(j + 1)] = scores[static_cast<size_t>(j)];
            j--;
        }

        moves[static_cast<size_t>(j + 1)] = key_move;
        scores[static_cast<size_t>(j + 1)] = key_score;
    }
}

bool MovePicker::see_ge(const Board& board, Move m, int threshold) noexcept {
    if (!m.is_capture()) return threshold <= 0;

    Piece victim = board.piece_at(m.to());
    Piece attacker = board.piece_at(m.from());
    if (victim == Piece::None && !m.is_ep()) return threshold <= 0;

    int victim_val = PawnValue;
    switch (piece_type_of(victim)) {
        case PieceType::Pawn:   victim_val = PawnValue; break;
        case PieceType::Knight: victim_val = KnightValue; break;
        case PieceType::Bishop: victim_val = BishopValue; break;
        case PieceType::Rook:   victim_val = RookValue; break;
        case PieceType::Queen:  victim_val = QueenValue; break;
        default: break;
    }

    int attacker_val = PawnValue;
    switch (piece_type_of(attacker)) {
        case PieceType::Pawn:   attacker_val = PawnValue; break;
        case PieceType::Knight: attacker_val = KnightValue; break;
        case PieceType::Bishop: attacker_val = BishopValue; break;
        case PieceType::Rook:   attacker_val = RookValue; break;
        case PieceType::Queen:  attacker_val = QueenValue; break;
        default: break;
    }

    // Static balance: If victim value minus attacker value exceeds threshold, capture is good!
    int swap = victim_val - threshold;
    if (swap < 0) return false;
    if (swap - attacker_val >= 0) return true;

    return swap >= 0;
}

} // namespace heavensgate
