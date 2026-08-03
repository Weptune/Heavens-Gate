#include "movegen.hpp"

namespace heavensgate {

void MoveGenerator::init() {
    AttackMasks::init();
}

bool MoveGenerator::is_square_attacked(const Board& board, Square sq, Color attacker_color) {
    if (sq == Square::None) return false;

    Color victim_color = ~attacker_color;
    Bitboard occ = board.occupied();

    // 1. Pawn attacks
    Bitboard pawns = board.pieces(make_piece(attacker_color, PieceType::Pawn));
    if (pawns & AttackMasks::pawn_attacks(victim_color, sq)) return true;

    // 2. Knight attacks
    Bitboard knights = board.pieces(make_piece(attacker_color, PieceType::Knight));
    if (knights & AttackMasks::knight_attacks(sq)) return true;

    // 3. King attacks
    Bitboard king = board.pieces(make_piece(attacker_color, PieceType::King));
    if (king & AttackMasks::king_attacks(sq)) return true;

    // 4. Bishop/Queen diagonal attacks
    Bitboard bishops_queens = board.pieces(make_piece(attacker_color, PieceType::Bishop)) |
                             board.pieces(make_piece(attacker_color, PieceType::Queen));
    if (bishops_queens & AttackMasks::bishop_attacks(sq, occ)) return true;

    // 5. Rook/Queen orthogonal attacks
    Bitboard rooks_queens = board.pieces(make_piece(attacker_color, PieceType::Rook)) |
                           board.pieces(make_piece(attacker_color, PieceType::Queen));
    if (rooks_queens & AttackMasks::rook_attacks(sq, occ)) return true;

    return false;
}

bool MoveGenerator::in_check(const Board& board, Color c) {
    Square ksq = board.king_square(c);
    if (ksq == Square::None) return false;
    return is_square_attacked(board, ksq, ~c);
}

void MoveGenerator::generate_pseudo_legal_moves(const Board& board, MoveList& moves, MoveGenType type) {
    Color us = board.side_to_move();
    Color them = ~us;

    Bitboard us_bb = board.pieces(us);
    Bitboard them_bb = board.pieces(them);
    Bitboard empty_bb = ~board.occupied();
    Bitboard occ = board.occupied();

    // 1. Pawn Moves
    Piece pawn_piece = make_piece(us, PieceType::Pawn);
    Bitboard pawns = board.pieces(pawn_piece);

    Rank promo_rank = (us == Color::White) ? Rank::Rank8 : Rank::Rank1;
    Rank start_rank = (us == Color::White) ? Rank::Rank2 : Rank::Rank7;

    while (pawns) {
        Square from = pop_lsb(pawns);
        int r = static_cast<int>(rank_of(from));
        int f = static_cast<int>(file_of(from));

        // Single push
        int push_r = (us == Color::White) ? r + 1 : r - 1;
        Square push_sq = make_square(static_cast<File>(f), static_cast<Rank>(push_r));
        if (test_bit(empty_bb, push_sq)) {
            if (rank_of(push_sq) == promo_rank) {
                if (type != MoveGenType::Captures) {
                    moves.push_back(Move(from, push_sq, MoveType::PromoQueen));
                    moves.push_back(Move(from, push_sq, MoveType::PromoRook));
                    moves.push_back(Move(from, push_sq, MoveType::PromoBishop));
                    moves.push_back(Move(from, push_sq, MoveType::PromoKnight));
                }
            } else {
                if (type != MoveGenType::Captures) {
                    moves.push_back(Move(from, push_sq, MoveType::Quiet));
                }

                // Double push from starting rank
                if (rank_of(from) == start_rank) {
                    int dbl_r = (us == Color::White) ? r + 2 : r - 2;
                    Square dbl_sq = make_square(static_cast<File>(f), static_cast<Rank>(dbl_r));
                    if (test_bit(empty_bb, dbl_sq)) {
                        if (type != MoveGenType::Captures) {
                            moves.push_back(Move(from, dbl_sq, MoveType::DoublePawnPush));
                        }
                    }
                }
            }
        }

        // Standard Pawn Captures
        Bitboard attacks = AttackMasks::pawn_attacks(us, from) & them_bb;
        while (attacks) {
            Square to = pop_lsb(attacks);
            if (rank_of(to) == promo_rank) {
                if (type != MoveGenType::Quiets) {
                    moves.push_back(Move(from, to, MoveType::PromoCaptureQueen));
                    moves.push_back(Move(from, to, MoveType::PromoCaptureRook));
                    moves.push_back(Move(from, to, MoveType::PromoCaptureBishop));
                    moves.push_back(Move(from, to, MoveType::PromoCaptureKnight));
                }
            } else {
                if (type != MoveGenType::Quiets) {
                    moves.push_back(Move(from, to, MoveType::Capture));
                }
            }
        }

        // En Passant Capture
        Square ep_sq = board.en_passant_sq();
        if (ep_sq != Square::None) {
            Bitboard ep_atk = AttackMasks::pawn_attacks(us, from) & square_bb(ep_sq);
            if (ep_atk) {
                if (type != MoveGenType::Quiets) {
                    moves.push_back(Move(from, ep_sq, MoveType::EnPassant));
                }
            }
        }
    }

    // Helper lambda for piece moves
    auto gen_piece_moves = [&](PieceType pt, auto attack_fn) {
        Bitboard pieces_bb = board.pieces(make_piece(us, pt));
        while (pieces_bb) {
            Square from = pop_lsb(pieces_bb);
            Bitboard targets = attack_fn(from, occ) & ~us_bb;
            while (targets) {
                Square to = pop_lsb(targets);
                bool is_cap = test_bit(them_bb, to);
                if (is_cap) {
                    if (type != MoveGenType::Quiets) {
                        moves.push_back(Move(from, to, MoveType::Capture));
                    }
                } else {
                    if (type != MoveGenType::Captures) {
                        moves.push_back(Move(from, to, MoveType::Quiet));
                    }
                }
            }
        }
    };

    // 2. Knights
    gen_piece_moves(PieceType::Knight, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });

    // 3. Bishops
    gen_piece_moves(PieceType::Bishop, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });

    // 4. Rooks
    gen_piece_moves(PieceType::Rook, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });

    // 5. Queens
    gen_piece_moves(PieceType::Queen, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

    // 6. Kings
    gen_piece_moves(PieceType::King, [](Square s, Bitboard) { return AttackMasks::king_attacks(s); });

    // 7. Castling Moves
    if (type != MoveGenType::Captures) {
        Square ksq = board.king_square(us);
        if (ksq != Square::None && !in_check(board, us)) {
            CastlingRights cr = board.castling_rights();
            if (us == Color::White) {
                // White Kingside (e1 -> g1)
                if (cr & WhiteOO) {
                    if (board.piece_at(Square::f1) == Piece::None && board.piece_at(Square::g1) == Piece::None) {
                        if (!is_square_attacked(board, Square::f1, Color::Black) &&
                            !is_square_attacked(board, Square::g1, Color::Black)) {
                            moves.push_back(Move(Square::e1, Square::g1, MoveType::KingCastle));
                        }
                    }
                }
                // White Queenside (e1 -> c1)
                if (cr & WhiteOOO) {
                    if (board.piece_at(Square::d1) == Piece::None &&
                        board.piece_at(Square::c1) == Piece::None &&
                        board.piece_at(Square::b1) == Piece::None) {
                        if (!is_square_attacked(board, Square::d1, Color::Black) &&
                            !is_square_attacked(board, Square::c1, Color::Black)) {
                            moves.push_back(Move(Square::e1, Square::c1, MoveType::QueenCastle));
                        }
                    }
                }
            } else {
                // Black Kingside (e8 -> g8)
                if (cr & BlackOO) {
                    if (board.piece_at(Square::f8) == Piece::None && board.piece_at(Square::g8) == Piece::None) {
                        if (!is_square_attacked(board, Square::f8, Color::White) &&
                            !is_square_attacked(board, Square::g8, Color::White)) {
                            moves.push_back(Move(Square::e8, Square::g8, MoveType::KingCastle));
                        }
                    }
                }
                // Black Queenside (e8 -> c8)
                if (cr & BlackOOO) {
                    if (board.piece_at(Square::d8) == Piece::None &&
                        board.piece_at(Square::c8) == Piece::None &&
                        board.piece_at(Square::b8) == Piece::None) {
                        if (!is_square_attacked(board, Square::d8, Color::White) &&
                            !is_square_attacked(board, Square::c8, Color::White)) {
                            moves.push_back(Move(Square::e8, Square::c8, MoveType::QueenCastle));
                        }
                    }
                }
            }
        }
    }
}

void MoveGenerator::generate_legal_moves(Board& board, MoveList& moves, MoveGenType type) {
    MoveList pseudo_moves;
    generate_pseudo_legal_moves(board, pseudo_moves, type);

    Color us = board.side_to_move();
    moves.clear();

    for (const auto& m : pseudo_moves) {
        board.make_move(m);
        // Verify king of side that moved is NOT in check
        if (!in_check(board, us)) {
            moves.push_back(m);
        }
        board.unmake_move(m);
    }
}

} // namespace heavensgate
