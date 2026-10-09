#include "movegen.hpp"
#include "attack_masks.hpp"
#include <iostream>

namespace heavensgate {

void MoveGenerator::init() {
    AttackMasks::init();
}

bool MoveGenerator::is_square_attacked(const Board& board, Square sq, Color by_color) {
    if (sq == Square::None || by_color == Color::None) return false;

    // 1. Pawn attacks
    Color victim_color = ~by_color;
    Bitboard pawn_atks = AttackMasks::pawn_attacks(victim_color, sq);
    if (pawn_atks & board.pieces(make_piece(by_color, PieceType::Pawn))) return true;

    // 2. Knight attacks
    Bitboard knight_atks = AttackMasks::knight_attacks(sq);
    if (knight_atks & board.pieces(make_piece(by_color, PieceType::Knight))) return true;

    // 3. Bishop / Queen diagonal attacks
    Bitboard occ = board.occupied();
    Bitboard bishop_atks = AttackMasks::bishop_attacks(sq, occ);
    Bitboard b_q = board.pieces(make_piece(by_color, PieceType::Bishop)) |
                   board.pieces(make_piece(by_color, PieceType::Queen));
    if (bishop_atks & b_q) return true;

    // 4. Rook / Queen orthogonal attacks
    Bitboard rook_atks = AttackMasks::rook_attacks(sq, occ);
    Bitboard r_q = board.pieces(make_piece(by_color, PieceType::Rook)) |
                   board.pieces(make_piece(by_color, PieceType::Queen));
    if (rook_atks & r_q) return true;

    // 5. King attacks
    Bitboard king_atks = AttackMasks::king_attacks(sq);
    if (king_atks & board.pieces(make_piece(by_color, PieceType::King))) return true;

    return false;
}

bool MoveGenerator::in_check(const Board& board, Color c) {
    Square ksq = board.king_square(c);
    if (ksq == Square::None) return false;
    return is_square_attacked(board, ksq, ~c);
}

void MoveGenerator::generate_pseudo_legal_captures(const Board& board, MoveList& moves) {
    moves.clear();
    Color us = board.side_to_move();
    Color them = ~us;

    Bitboard them_pieces = board.pieces(them);
    Bitboard empty_sqs   = ~board.occupied();

    // 1. Pawns (Captures, En Passant, Promotions)
    Piece pawn = make_piece(us, PieceType::Pawn);
    Bitboard pawns = board.pieces(pawn);

    Rank promo_rank = (us == Color::White) ? Rank::Rank8 : Rank::Rank1;

    while (pawns) {
        Square from = pop_lsb(pawns);
        Bitboard from_bb = square_bb(from);

        // Promotion pushes (Tactical queen/under-promotions to 8th/1st rank)
        Bitboard push_bb = (us == Color::White) ? (shift<Direction::North>(from_bb) & empty_sqs)
                                                : (shift<Direction::South>(from_bb) & empty_sqs);
        if (push_bb) {
            Square to = lsb(push_bb);
            if (rank_of(to) == promo_rank) {
                moves.push_back(Move(from, to, MoveType::PromoQueen));
                moves.push_back(Move(from, to, MoveType::PromoRook));
                moves.push_back(Move(from, to, MoveType::PromoKnight));
                moves.push_back(Move(from, to, MoveType::PromoBishop));
            }
        }

        // Pawn Captures (and Promo-Captures)
        Bitboard atk_bb = AttackMasks::pawn_attacks(us, from);
        Bitboard cap_bb = atk_bb & them_pieces;

        while (cap_bb) {
            Square to = pop_lsb(cap_bb);
            if (rank_of(to) == promo_rank) {
                moves.push_back(Move(from, to, MoveType::PromoCaptureQueen));
                moves.push_back(Move(from, to, MoveType::PromoCaptureRook));
                moves.push_back(Move(from, to, MoveType::PromoCaptureKnight));
                moves.push_back(Move(from, to, MoveType::PromoCaptureBishop));
            } else {
                moves.push_back(Move(from, to, MoveType::Capture));
            }
        }

        // En Passant
        Square ep_sq = board.en_passant_sq();
        if (ep_sq != Square::None) {
            Bitboard ep_atk = AttackMasks::pawn_attacks(us, from) & square_bb(ep_sq);
            if (ep_atk) {
                moves.push_back(Move(from, ep_sq, MoveType::EnPassant));
            }
        }
    }

    // Helper for piece captures
    auto gen_piece_captures = [&](PieceType pt, auto attack_fn) {
        Piece p = make_piece(us, pt);
        Bitboard bb = board.pieces(p);
        while (bb) {
            Square from = pop_lsb(bb);
            Bitboard cap_atks = attack_fn(from, board.occupied()) & them_pieces;

            while (cap_atks) {
                Square to = pop_lsb(cap_atks);
                moves.push_back(Move(from, to, MoveType::Capture));
            }
        }
    };

    gen_piece_captures(PieceType::Knight, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    gen_piece_captures(PieceType::Bishop, [](Square s, Bitboard occ) { return AttackMasks::bishop_attacks(s, occ); });
    gen_piece_captures(PieceType::Rook,   [](Square s, Bitboard occ) { return AttackMasks::rook_attacks(s, occ); });
    gen_piece_captures(PieceType::Queen,  [](Square s, Bitboard occ) { return AttackMasks::queen_attacks(s, occ); });
    gen_piece_captures(PieceType::King,   [](Square s, Bitboard) { return AttackMasks::king_attacks(s); });
}

void MoveGenerator::generate_pawn_pushes_to_7th(const Board& board, MoveList& moves) {
    Color us = board.side_to_move();
    Piece pawn = make_piece(us, PieceType::Pawn);
    Bitboard pawns = board.pieces(pawn);
    Bitboard empty_sqs = ~board.occupied();
    Rank target_rank = (us == Color::White) ? Rank::Rank7 : Rank::Rank2;

    while (pawns) {
        Square from = pop_lsb(pawns);
        Bitboard from_bb = square_bb(from);
        Bitboard push_bb = (us == Color::White) ? (shift<Direction::North>(from_bb) & empty_sqs)
                                                : (shift<Direction::South>(from_bb) & empty_sqs);
        if (push_bb) {
            Square to = lsb(push_bb);
            if (rank_of(to) == target_rank) {
                moves.push_back(Move(from, to, MoveType::Quiet));
            }
        }
    }
}

void MoveGenerator::generate_pseudo_legal_quiets(const Board& board, MoveList& moves) {
    moves.clear();
    Color us = board.side_to_move();

    Bitboard empty_sqs = ~board.occupied();

    // 1. Pawns (Single non-promo pushes, Double pushes)
    Piece pawn = make_piece(us, PieceType::Pawn);
    Bitboard pawns = board.pieces(pawn);

    Rank promo_rank = (us == Color::White) ? Rank::Rank8 : Rank::Rank1;
    Rank double_rank = (us == Color::White) ? Rank::Rank2 : Rank::Rank7;

    while (pawns) {
        Square from = pop_lsb(pawns);
        Bitboard from_bb = square_bb(from);

        Bitboard push_bb = (us == Color::White) ? (shift<Direction::North>(from_bb) & empty_sqs)
                                                : (shift<Direction::South>(from_bb) & empty_sqs);
        if (push_bb) {
            Square to = lsb(push_bb);
            if (rank_of(to) != promo_rank) {
                moves.push_back(Move(from, to, MoveType::Quiet));

                // Double push
                if (rank_of(from) == double_rank) {
                    Bitboard dbl_bb = (us == Color::White) ? (shift<Direction::North>(push_bb) & empty_sqs)
                                                           : (shift<Direction::South>(push_bb) & empty_sqs);
                    if (dbl_bb) {
                        moves.push_back(Move(from, lsb(dbl_bb), MoveType::DoublePawnPush));
                    }
                }
            }
        }
    }

    // Helper for piece quiets
    auto gen_piece_quiets = [&](PieceType pt, auto attack_fn) {
        Piece p = make_piece(us, pt);
        Bitboard bb = board.pieces(p);
        while (bb) {
            Square from = pop_lsb(bb);
            Bitboard quiet_atks = attack_fn(from, board.occupied()) & empty_sqs;

            while (quiet_atks) {
                Square to = pop_lsb(quiet_atks);
                moves.push_back(Move(from, to, MoveType::Quiet));
            }
        }
    };

    gen_piece_quiets(PieceType::Knight, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    gen_piece_quiets(PieceType::Bishop, [](Square s, Bitboard occ) { return AttackMasks::bishop_attacks(s, occ); });
    gen_piece_quiets(PieceType::Rook,   [](Square s, Bitboard occ) { return AttackMasks::rook_attacks(s, occ); });
    gen_piece_quiets(PieceType::Queen,  [](Square s, Bitboard occ) { return AttackMasks::queen_attacks(s, occ); });
    gen_piece_quiets(PieceType::King,   [](Square s, Bitboard) { return AttackMasks::king_attacks(s); });

    // Castling
    if (!in_check(board, us)) {
        CastlingRights cr = board.castling_rights();
        if (us == Color::White) {
            if ((cr & WhiteOO) && !test_bit(board.occupied(), Square::f1) && !test_bit(board.occupied(), Square::g1)) {
                if (!is_square_attacked(board, Square::f1, Color::Black) && !is_square_attacked(board, Square::g1, Color::Black)) {
                    moves.push_back(Move(Square::e1, Square::g1, MoveType::KingCastle));
                }
            }
            if ((cr & WhiteOOO) && !test_bit(board.occupied(), Square::d1) && !test_bit(board.occupied(), Square::c1) && !test_bit(board.occupied(), Square::b1)) {
                if (!is_square_attacked(board, Square::d1, Color::Black) && !is_square_attacked(board, Square::c1, Color::Black)) {
                    moves.push_back(Move(Square::e1, Square::c1, MoveType::QueenCastle));
                }
            }
        } else {
            if ((cr & BlackOO) && !test_bit(board.occupied(), Square::f8) && !test_bit(board.occupied(), Square::g8)) {
                if (!is_square_attacked(board, Square::f8, Color::White) && !is_square_attacked(board, Square::g8, Color::White)) {
                    moves.push_back(Move(Square::e8, Square::g8, MoveType::KingCastle));
                }
            }
            if ((cr & BlackOOO) && !test_bit(board.occupied(), Square::d8) && !test_bit(board.occupied(), Square::c8) && !test_bit(board.occupied(), Square::b8)) {
                if (!is_square_attacked(board, Square::d8, Color::White) && !is_square_attacked(board, Square::c8, Color::White)) {
                    moves.push_back(Move(Square::e8, Square::c8, MoveType::QueenCastle));
                }
            }
        }
    }
}

void MoveGenerator::generate_pseudo_legal_moves(const Board& board, MoveList& moves) {
    moves.clear();
    MoveList quiets;
    generate_pseudo_legal_captures(board, moves);
    generate_pseudo_legal_quiets(board, quiets);
    for (size_t i = 0; i < quiets.size(); ++i) {
        moves.push_back(quiets[i]);
    }
}

static inline Bitboard pinned_pieces(const Board& board, Color us, Square ksq) {
    const Color them = ~us;
    const Bitboard occ = board.occupied();
    const Bitboard diag_snipers = (board.pieces(make_piece(them, PieceType::Bishop)) | board.pieces(make_piece(them, PieceType::Queen))) & MagicBitboards::bishop_attacks(ksq, EmptyBB);
    const Bitboard ortho_snipers = (board.pieces(make_piece(them, PieceType::Rook)) | board.pieces(make_piece(them, PieceType::Queen))) & MagicBitboards::rook_attacks(ksq, EmptyBB);
    Bitboard snipers = diag_snipers | ortho_snipers;
    Bitboard pinned = EmptyBB;
    while (snipers) {
        const Square sniper_sq = pop_lsb(snipers);
        const Bitboard blockers = AttackMasks::between(ksq, sniper_sq) & occ;
        if (popcount(blockers) == 1) pinned |= blockers & board.pieces(us);
    }
    return pinned;
}

static inline void filter_legal_moves(const Board& board, const MoveList& pseudo_moves, MoveList& legal_moves,
                                     bool stop_after_first = false) {
    legal_moves.clear();
    Color us = board.side_to_move();
    Board& mut_board = const_cast<Board&>(board);

    if (MoveGenerator::in_check(board, us)) {
        for (size_t i = 0; i < pseudo_moves.size(); ++i) {
            mut_board.make_move(pseudo_moves[i]);
            if (!MoveGenerator::in_check(mut_board, us)) {
                legal_moves.push_back(pseudo_moves[i]);
            }
            mut_board.unmake_move(pseudo_moves[i]);
            if (stop_after_first && !legal_moves.empty()) return;
        }
        return;
    }

    Square ksq = board.king_square(us);
    if (static_cast<size_t>(ksq) >= 64) {
        for (size_t i = 0; i < pseudo_moves.size(); ++i) {
            mut_board.make_move(pseudo_moves[i]);
            if (!MoveGenerator::in_check(mut_board, us)) {
                legal_moves.push_back(pseudo_moves[i]);
            }
            mut_board.unmake_move(pseudo_moves[i]);
            if (stop_after_first && !legal_moves.empty()) return;
        }
        return;
    }

    const Bitboard pinned = pinned_pieces(board, us, ksq);

    for (size_t i = 0; i < pseudo_moves.size(); ++i) {
        const Move& m = pseudo_moves[i];
        Square from = m.from();

        // 1. King moves: verify with make_move (king cannot step into check)
        if (from == ksq) {
            mut_board.make_move(m);
            if (!MoveGenerator::in_check(mut_board, us)) {
                legal_moves.push_back(m);
            }
            mut_board.unmake_move(m);
            if (stop_after_first && !legal_moves.empty()) return;
            continue;
        }

        // 2. En passant: can uncover horizontal rank check, verify with make_move
        if (m.is_ep()) {
            mut_board.make_move(m);
            if (!MoveGenerator::in_check(mut_board, us)) {
                legal_moves.push_back(m);
            }
            mut_board.unmake_move(m);
            if (stop_after_first && !legal_moves.empty()) return;
            continue;
        }

        // 3. Pinned pieces: can only move along the pin line
        if (test_bit(pinned, from)) {
            if (test_bit(AttackMasks::line(ksq, from), m.to())) {
                legal_moves.push_back(m);
                if (stop_after_first) return;
            }
            continue;
        }

        // 4. Unpinned pieces: 100% mathematically guaranteed to be legal!
        legal_moves.push_back(m);
        if (stop_after_first) return;
    }
}

void MoveGenerator::generate_legal_moves(const Board& board, MoveList& moves) {
    MoveList pseudo_moves;
    generate_pseudo_legal_moves(board, pseudo_moves);
    filter_legal_moves(board, pseudo_moves, moves);
}

bool MoveGenerator::has_legal_move(const Board& board) {
    const Color us = board.side_to_move();
    const Square ksq = board.king_square(us);
    if (static_cast<size_t>(ksq) < 64 && !in_check(board, us)) {
        // Outside check, an unpinned non-king move cannot expose its king.
        // This is the same exact legality proof used by filter_legal_moves,
        // not a material-count or mobility-score approximation to stalemate.
        const Bitboard pinned = pinned_pieces(board, us, ksq);
        const Bitboard occ = board.occupied();
        const Bitboard pawns = board.pieces(make_piece(us, PieceType::Pawn)) & ~pinned;
        const Bitboard pushes = us == Color::White ? pawns << 8 : pawns >> 8;
        if (pushes & ~occ) return true; // Includes legal quiet promotions.
        const Bitboard destinations = ~board.pieces(us);
        for (PieceType type : {PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen}) {
            Bitboard pieces = board.pieces(make_piece(us, type)) & ~pinned;
            while (pieces) {
                const Square from = pop_lsb(pieces);
                Bitboard attacks = EmptyBB;
                switch (type) {
                    case PieceType::Knight: attacks = AttackMasks::knight_attacks(from); break;
                    case PieceType::Bishop: attacks = AttackMasks::bishop_attacks(from, occ); break;
                    case PieceType::Rook: attacks = AttackMasks::rook_attacks(from, occ); break;
                    case PieceType::Queen: attacks = AttackMasks::queen_attacks(from, occ); break;
                    default: break;
                }
                if (attacks & destinations) return true;
            }
        }
    }
    // King-only mobility, pins, pawn captures, EP and checked nodes retain
    // the shared full legality filter, stopping after its first legal move.
    MoveList pseudo_moves, legal_moves;
    generate_pseudo_legal_moves(board, pseudo_moves);
    filter_legal_moves(board, pseudo_moves, legal_moves, true);
    return !legal_moves.empty();
}

void MoveGenerator::generate_capture_moves(const Board& board, MoveList& moves) {
    MoveList pseudo_caps;
    generate_pseudo_legal_captures(board, pseudo_caps);
    filter_legal_moves(board, pseudo_caps, moves);
}

bool MoveGenerator::is_pseudo_legal(const Board& board, Move m) {
    Square from = m.from();
    Square to = m.to();
    if (from == Square::None || to == Square::None || from == to) return false;

    Piece p = board.piece_at(from);
    if (p == Piece::None) return false;
    Color us = board.side_to_move();
    if (color_of(p) != us) return false;

    Piece target_p = board.piece_at(to);
    if (target_p != Piece::None && color_of(target_p) == us) return false;

    PieceType pt = piece_type_of(p);
    Bitboard occ = board.occupied();

    if (pt == PieceType::Pawn) {
        Rank promo_rank = (us == Color::White) ? Rank::Rank8 : Rank::Rank1;
        Rank start_rank = (us == Color::White) ? Rank::Rank2 : Rank::Rank7;
        int forward_dir = (us == Color::White) ? 8 : -8;

        if (m.is_ep()) {
            Square ep_sq = board.en_passant_sq();
            if (ep_sq == Square::None || to != ep_sq) return false;
            return (AttackMasks::pawn_attacks(us, from) & square_bb(to)) != EmptyBB;
        }

        // Push
        if (file_of(from) == file_of(to)) {
            if (target_p != Piece::None) return false;
            if (m.is_capture()) return false;
            if (static_cast<int>(to) - static_cast<int>(from) == forward_dir) {
                if (rank_of(to) == promo_rank) return m.is_promotion();
                return !m.is_promotion();
            }
            if (rank_of(from) == start_rank && static_cast<int>(to) - static_cast<int>(from) == 2 * forward_dir) {
                Square mid_sq = static_cast<Square>(static_cast<int>(from) + forward_dir);
                return board.piece_at(mid_sq) == Piece::None && !m.is_promotion();
            }
            return false;
        }

        // Capture
        if ((AttackMasks::pawn_attacks(us, from) & square_bb(to)) && target_p != Piece::None) {
            if (!m.is_capture()) return false;
            if (rank_of(to) == promo_rank) return m.is_promotion();
            return !m.is_promotion();
        }
        return false;
    }

    // For non-pawn pieces, target square occupancy must match capture flag
    if (target_p != Piece::None) {
        if (!m.is_capture()) return false;
    } else {
        if (m.is_capture()) return false;
    }
    if (m.is_promotion()) return false;

    if (pt == PieceType::Knight) {
        return (AttackMasks::knight_attacks(from) & square_bb(to)) != EmptyBB;
    }

    if (pt == PieceType::Bishop) {
        return (AttackMasks::bishop_attacks(from, occ) & square_bb(to)) != EmptyBB;
    }

    if (pt == PieceType::Rook) {
        return (AttackMasks::rook_attacks(from, occ) & square_bb(to)) != EmptyBB;
    }

    if (pt == PieceType::Queen) {
        return (AttackMasks::queen_attacks(from, occ) & square_bb(to)) != EmptyBB;
    }

    if (pt == PieceType::King) {
        if (m.is_castle()) {
            if (in_check(board, us)) return false;
            if (from != (us == Color::White ? Square::e1 : Square::e8)) return false;
            CastlingRights cr = board.castling_rights();
            if (us == Color::White) {
                if (to == Square::g1) {
                    return (cr & WhiteOO) && !test_bit(occ, Square::f1) && !test_bit(occ, Square::g1)
                        && !is_square_attacked(board, Square::f1, Color::Black)
                        && !is_square_attacked(board, Square::g1, Color::Black);
                }
                if (to == Square::c1) {
                    return (cr & WhiteOOO) && !test_bit(occ, Square::d1) && !test_bit(occ, Square::c1) && !test_bit(occ, Square::b1)
                        && !is_square_attacked(board, Square::d1, Color::Black)
                        && !is_square_attacked(board, Square::c1, Color::Black);
                }
            } else {
                if (to == Square::g8) {
                    return (cr & BlackOO) && !test_bit(occ, Square::f8) && !test_bit(occ, Square::g8)
                        && !is_square_attacked(board, Square::f8, Color::White)
                        && !is_square_attacked(board, Square::g8, Color::White);
                }
                if (to == Square::c8) {
                    return (cr & BlackOOO) && !test_bit(occ, Square::d8) && !test_bit(occ, Square::c8) && !test_bit(occ, Square::b8)
                        && !is_square_attacked(board, Square::d8, Color::White)
                        && !is_square_attacked(board, Square::c8, Color::White);
                }
            }
            return false;
        }
        return (AttackMasks::king_attacks(from) & square_bb(to)) != EmptyBB;
    }

    return false;
}

bool MoveGenerator::gives_check(const Board& board, Move m) {
    Square from = m.from();
    Square to = m.to();
    Piece p = board.piece_at(from);
    if (p == Piece::None) return false;

    Color us = board.side_to_move();
    Color them = ~us;
    Square ksq = board.king_square(them);
    if (ksq == Square::None) return false;

    PieceType pt = m.is_promotion() ? m.promotion_piece_type() : piece_type_of(p);
    Bitboard occ_after = (board.occupied() ^ square_bb(from)) | square_bb(to);

    // 1. En Passant: Clear the captured pawn's square from occupancy
    if (m.is_ep()) {
        Square cap_sq = make_square(file_of(to), rank_of(from));
        occ_after ^= square_bb(cap_sq);
    }

    // 2. Castling: Update rook's movement in occupancy and test direct rook check
    if (m.is_castle()) {
        Square rfrom = Square::None;
        Square rto = Square::None;
        if (to == Square::g1) { rfrom = Square::h1; rto = Square::f1; }
        else if (to == Square::c1) { rfrom = Square::a1; rto = Square::d1; }
        else if (to == Square::g8) { rfrom = Square::h8; rto = Square::f8; }
        else if (to == Square::c8) { rfrom = Square::a8; rto = Square::d8; }

        occ_after = (occ_after ^ square_bb(rfrom)) | square_bb(rto);
        if (AttackMasks::rook_attacks(rto, occ_after) & square_bb(ksq)) return true;
    }

    // 3. Direct check by moved piece (or promoted piece)
    if (pt == PieceType::Pawn) {
        if (AttackMasks::pawn_attacks(us, to) & square_bb(ksq)) return true;
    } else if (pt == PieceType::Knight) {
        if (AttackMasks::knight_attacks(to) & square_bb(ksq)) return true;
    } else if (pt == PieceType::Bishop) {
        if (AttackMasks::bishop_attacks(to, occ_after) & square_bb(ksq)) return true;
    } else if (pt == PieceType::Rook) {
        if (AttackMasks::rook_attacks(to, occ_after) & square_bb(ksq)) return true;
    } else if (pt == PieceType::Queen) {
        if (AttackMasks::queen_attacks(to, occ_after) & square_bb(ksq)) return true;
    }

    // 4. Discovered check from friendly slider unmasked by moving 'from'
    Bitboard rooks_queens = board.pieces(make_piece(us, PieceType::Rook)) | board.pieces(make_piece(us, PieceType::Queen));
    rooks_queens &= ~square_bb(from);
    if (m.is_castle()) {
        Square rfrom = (to == Square::g1) ? Square::h1 : (to == Square::c1) ? Square::a1 : (to == Square::g8) ? Square::h8 : Square::a8;
        Square rto = (to == Square::g1) ? Square::f1 : (to == Square::c1) ? Square::d1 : (to == Square::g8) ? Square::f8 : Square::d8;
        rooks_queens &= ~square_bb(rfrom);
        rooks_queens |= square_bb(rto);
    }
    if (rooks_queens && (AttackMasks::rook_attacks(ksq, occ_after) & rooks_queens)) return true;

    Bitboard bishops_queens = board.pieces(make_piece(us, PieceType::Bishop)) | board.pieces(make_piece(us, PieceType::Queen));
    bishops_queens &= ~square_bb(from);
    if (bishops_queens && (AttackMasks::bishop_attacks(ksq, occ_after) & bishops_queens)) return true;

    return false;
}

} // namespace heavensgate
