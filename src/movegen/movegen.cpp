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

void MoveGenerator::generate_legal_moves(const Board& board, MoveList& moves) {
    moves.clear();
    Color us = board.side_to_move();
    Color them = ~us;

    Bitboard us_pieces   = board.pieces(us);
    Bitboard them_pieces = board.pieces(them);
    Bitboard empty_sqs   = ~board.occupied();

    // 1. Pawns
    Piece pawn = make_piece(us, PieceType::Pawn);
    Bitboard pawns = board.pieces(pawn);

    Rank promo_rank = (us == Color::White) ? Rank::Rank8 : Rank::Rank1;
    Rank double_rank = (us == Color::White) ? Rank::Rank2 : Rank::Rank7;

    while (pawns) {
        Square from = pop_lsb(pawns);
        Bitboard from_bb = square_bb(from);

        // Single push
        Bitboard push_bb = (us == Color::White) ? (shift<Direction::North>(from_bb) & empty_sqs)
                                                : (shift<Direction::South>(from_bb) & empty_sqs);
        if (push_bb) {
            Square to = lsb(push_bb);
            if (rank_of(to) == promo_rank) {
                moves.push_back(Move(from, to, MoveType::PromoQueen));
                moves.push_back(Move(from, to, MoveType::PromoRook));
                moves.push_back(Move(from, to, MoveType::PromoKnight));
                moves.push_back(Move(from, to, MoveType::PromoBishop));
            } else {
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

        // Captures
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

    // Helper for piece moves
    auto gen_piece_moves = [&](PieceType pt, auto attack_fn) {
        Piece p = make_piece(us, pt);
        Bitboard bb = board.pieces(p);
        while (bb) {
            Square from = pop_lsb(bb);
            Bitboard atks = attack_fn(from, board.occupied());
            Bitboard valid_atks = atks & ~us_pieces;

            while (valid_atks) {
                Square to = pop_lsb(valid_atks);
                MoveType type = test_bit(them_pieces, to) ? MoveType::Capture : MoveType::Quiet;
                moves.push_back(Move(from, to, type));
            }
        }
    };

    gen_piece_moves(PieceType::Knight, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    gen_piece_moves(PieceType::Bishop, [](Square s, Bitboard occ) { return AttackMasks::bishop_attacks(s, occ); });
    gen_piece_moves(PieceType::Rook,   [](Square s, Bitboard occ) { return AttackMasks::rook_attacks(s, occ); });
    gen_piece_moves(PieceType::Queen,  [](Square s, Bitboard occ) { return AttackMasks::queen_attacks(s, occ); });
    gen_piece_moves(PieceType::King,   [](Square s, Bitboard) { return AttackMasks::king_attacks(s); });

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

    // Filter out illegal moves that leave king in check
    MoveList legal_moves;
    Board& mut_board = const_cast<Board&>(board);
    for (size_t i = 0; i < moves.size(); ++i) {
        mut_board.make_move(moves[i]);
        if (!in_check(mut_board, us)) {
            legal_moves.push_back(moves[i]);
        }
        mut_board.unmake_move(moves[i]);
    }

    moves = legal_moves;
}

void MoveGenerator::generate_capture_moves(const Board& board, MoveList& moves) {
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

    // Filter out illegal captures that leave king in check
    MoveList legal_captures;
    Board& mut_board = const_cast<Board&>(board);
    for (size_t i = 0; i < moves.size(); ++i) {
        mut_board.make_move(moves[i]);
        if (!in_check(mut_board, us)) {
            legal_captures.push_back(moves[i]);
        }
        mut_board.unmake_move(moves[i]);
    }

    moves = legal_captures;
}

} // namespace heavensgate
