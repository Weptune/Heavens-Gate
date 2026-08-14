#include "move_picker.hpp"
#include "../evaluation/eval.hpp"
#include "../movegen/movegen.hpp"
#include "../movegen/attack_masks.hpp"
#include <algorithm>

namespace heavensgate {

MovePicker::MovePicker() {
    clear();
}

void MovePicker::clear() noexcept {
    killer_moves_.fill({});
    history_scores_.fill({});
    countermoves_.fill({});
    cont_history_.fill({});
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
    for (auto& a1 : cont_history_) {
        for (auto& a2 : a1) {
            for (auto& a3 : a2) {
                for (auto& val : a3) val /= 2;
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

void MovePicker::add_continuation_history(const Board& board, Move prev_move, Move curr_move, int depth) noexcept {
    if (!static_cast<bool>(prev_move) || !static_cast<bool>(curr_move)) return;

    Piece curr_p = board.piece_at(curr_move.from());
    Piece prev_p = board.piece_at(prev_move.to());
    if (curr_p == Piece::None || prev_p == Piece::None) return;

    size_t curr_pt = static_cast<size_t>(piece_type_of(curr_p));
    size_t curr_to = static_cast<size_t>(curr_move.to());
    size_t prev_pt = static_cast<size_t>(piece_type_of(prev_p));
    size_t prev_to = static_cast<size_t>(prev_move.to());

    if (curr_pt < 6 && prev_pt < 6) {
        cont_history_[curr_pt][curr_to][prev_pt][prev_to] += depth * depth;
        if (cont_history_[curr_pt][curr_to][prev_pt][prev_to] > 10000) {
            for (auto& a1 : cont_history_) {
                for (auto& a2 : a1) {
                    for (auto& a3 : a2) {
                        for (auto& val : a3) val /= 2;
                    }
                }
            }
        }
    }
}

int MovePicker::get_continuation_history(const Board& board, Move prev_move, Move curr_move) const noexcept {
    if (!static_cast<bool>(prev_move) || !static_cast<bool>(curr_move)) return 0;

    Piece curr_p = board.piece_at(curr_move.from());
    Piece prev_p = board.piece_at(prev_move.to());
    if (curr_p == Piece::None || prev_p == Piece::None) return 0;

    size_t curr_pt = static_cast<size_t>(piece_type_of(curr_p));
    size_t curr_to = static_cast<size_t>(curr_move.to());
    size_t prev_pt = static_cast<size_t>(piece_type_of(prev_p));
    size_t prev_to = static_cast<size_t>(prev_move.to());

    if (curr_pt < 6 && prev_pt < 6) {
        return cont_history_[curr_pt][curr_to][prev_pt][prev_to];
    }
    return 0;
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

        if (m == tt_move) {
            scores[i] = 2000000;
        } else if (m.type() == MoveType::PromoQueen || m.type() == MoveType::PromoCaptureQueen) {
            scores[i] = 950000;
        } else if (m.is_promotion()) {
            scores[i] = 200000;
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
            int hist = history_scores_[c_idx][from_idx][to_idx];
            int cont = get_continuation_history(board, prev_move, m);
            scores[i] = hist + cont;
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

    Square from = m.from();
    Square to = m.to();

    Piece victim = board.piece_at(to);
    Piece attacker = board.piece_at(from);
    if (victim == Piece::None && !m.is_ep()) return threshold <= 0;

    auto get_val = [](PieceType pt) -> int {
        switch (pt) {
            case PieceType::Pawn:   return PawnValue;
            case PieceType::Knight: return KnightValue;
            case PieceType::Bishop: return BishopValue;
            case PieceType::Rook:   return RookValue;
            case PieceType::Queen:  return QueenValue;
            case PieceType::King:   return 20000;
            default: return 0;
        }
    };

    int gain[32];
    int d = 0;
    gain[d] = m.is_ep() ? PawnValue : get_val(piece_type_of(victim));

    Bitboard occ = board.occupied();
    occ ^= square_bb(from); // Remove initial attacker

    Color side = ~board.side_to_move();
    PieceType curr_piece = piece_type_of(attacker);

    while (true) {
        d++;
        gain[d] = get_val(curr_piece) - gain[d - 1];
        if (std::max(-gain[d - 1], gain[d]) < 0) break; // Stand-pat cutoff

        // Find attackers to target square 'to' for 'side'
        Bitboard attackers = EmptyBB;
        Bitboard side_occ = board.pieces(side) & occ;

        // Pawns
        Bitboard p_att = AttackMasks::pawn_attacks(~side, to) & board.pieces(make_piece(side, PieceType::Pawn)) & occ;
        if (p_att) { curr_piece = PieceType::Pawn; attackers = p_att; }
        else {
            // Knights
            Bitboard n_att = AttackMasks::knight_attacks(to) & board.pieces(make_piece(side, PieceType::Knight)) & occ;
            if (n_att) { curr_piece = PieceType::Knight; attackers = n_att; }
            else {
                // Bishops
                Bitboard b_att = AttackMasks::bishop_attacks(to, occ) & (board.pieces(make_piece(side, PieceType::Bishop)) | board.pieces(make_piece(side, PieceType::Queen))) & occ;
                if (b_att) {
                    Bitboard b_only = b_att & board.pieces(make_piece(side, PieceType::Bishop));
                    if (b_only) { curr_piece = PieceType::Bishop; attackers = b_only; }
                    else { curr_piece = PieceType::Queen; attackers = b_att; }
                } else {
                    // Rooks
                    Bitboard r_att = AttackMasks::rook_attacks(to, occ) & (board.pieces(make_piece(side, PieceType::Rook)) | board.pieces(make_piece(side, PieceType::Queen))) & occ;
                    if (r_att) {
                        Bitboard r_only = r_att & board.pieces(make_piece(side, PieceType::Rook));
                        if (r_only) { curr_piece = PieceType::Rook; attackers = r_only; }
                        else { curr_piece = PieceType::Queen; attackers = r_att; }
                    } else {
                        // King
                        Bitboard k_att = AttackMasks::king_attacks(to) & board.pieces(make_piece(side, PieceType::King)) & occ;
                        if (k_att) { curr_piece = PieceType::King; attackers = k_att; }
                    }
                }
            }
        }

        if (!attackers) break; // No more attackers for this side

        Square att_sq = pop_lsb(attackers);
        occ ^= square_bb(att_sq); // Remove used attacker
        side = ~side;
    }

    // Minimax back up the gain sequence
    while (--d > 0) {
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
    }

    return gain[0] >= threshold;
}

} // namespace heavensgate
