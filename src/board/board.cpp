#include "board.hpp"
#include <sstream>
#include <cassert>

namespace heavensgate {

Board::Board() {
    clear();
}

void Board::clear() {
    piece_bb_.fill(EmptyBB);
    color_bb_.fill(EmptyBB);
    mailbox_.fill(Piece::None);
    side_to_move_ = Color::White;
    castling_rights_ = CastlingNone;
    en_passant_sq_ = Square::None;
    halfmove_clock_ = 0;
    fullmove_number_ = 1;
    history_.clear();
}

void Board::set_piece(Square sq, Piece p) {
    if (sq == Square::None) return;
    
    // Clear existing piece at sq if any
    Piece old_p = mailbox_[static_cast<size_t>(sq)];
    if (old_p != Piece::None) {
        remove_piece(sq);
    }

    if (p != Piece::None) {
        size_t p_idx = static_cast<size_t>(p);
        Color c = color_of(p);
        size_t c_idx = static_cast<size_t>(c);

        set_bit(piece_bb_[p_idx], sq);
        set_bit(color_bb_[c_idx], sq);
        set_bit(color_bb_[2], sq); // Both colors
        mailbox_[static_cast<size_t>(sq)] = p;
    }
}

void Board::remove_piece(Square sq) {
    if (sq == Square::None) return;
    Piece p = mailbox_[static_cast<size_t>(sq)];
    if (p == Piece::None) return;

    size_t p_idx = static_cast<size_t>(p);
    Color c = color_of(p);
    size_t c_idx = static_cast<size_t>(c);

    clear_bit(piece_bb_[p_idx], sq);
    clear_bit(color_bb_[c_idx], sq);
    clear_bit(color_bb_[2], sq);
    mailbox_[static_cast<size_t>(sq)] = Piece::None;
}

void Board::make_move(Move m) {
    Square from = m.from();
    Square to = m.to();
    MoveType type = m.type();
    Color us = side_to_move_;
    Color them = ~us;
    Piece p = mailbox_[static_cast<size_t>(from)];

    // Save history state
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.en_passant_sq = en_passant_sq_;
    state.halfmove_clock = halfmove_clock_;
    state.captured_piece = Piece::None;

    // Handle captured piece square
    Square cap_sq = to;
    if (type == MoveType::EnPassant) {
        cap_sq = (us == Color::White) ? make_square(file_of(to), Rank::Rank5) 
                                      : make_square(file_of(to), Rank::Rank4);
    }
    state.captured_piece = mailbox_[static_cast<size_t>(cap_sq)];

    history_.push_back(state);

    // Reset EP square and increment clocks
    en_passant_sq_ = Square::None;
    halfmove_clock_++;
    if (us == Color::Black) {
        fullmove_number_++;
    }

    if (p == Piece::WhitePawn || p == Piece::BlackPawn || m.is_capture()) {
        halfmove_clock_ = 0;
    }

    // Remove captured piece
    if (state.captured_piece != Piece::None) {
        remove_piece(cap_sq);
    }

    // Move piece from -> to
    remove_piece(from);

    // Promotion or standard move
    if (m.is_promotion()) {
        Piece promo_piece = make_piece(us, m.promotion_piece_type());
        set_piece(to, promo_piece);
    } else {
        set_piece(to, p);
    }

    // Special moves logic
    if (type == MoveType::DoublePawnPush) {
        en_passant_sq_ = (us == Color::White) ? make_square(file_of(from), Rank::Rank3)
                                              : make_square(file_of(from), Rank::Rank6);
    } else if (type == MoveType::KingCastle) {
        Square rook_from = (us == Color::White) ? Square::h1 : Square::h8;
        Square rook_to   = (us == Color::White) ? Square::f1 : Square::f8;
        Piece rook = mailbox_[static_cast<size_t>(rook_from)];
        remove_piece(rook_from);
        set_piece(rook_to, rook);
    } else if (type == MoveType::QueenCastle) {
        Square rook_from = (us == Color::White) ? Square::a1 : Square::a8;
        Square rook_to   = (us == Color::White) ? Square::d1 : Square::d8;
        Piece rook = mailbox_[static_cast<size_t>(rook_from)];
        remove_piece(rook_from);
        set_piece(rook_to, rook);
    }

    // Update castling rights
    if (p == Piece::WhiteKing) {
        castling_rights_ &= ~WhiteCastling;
    } else if (p == Piece::BlackKing) {
        castling_rights_ &= ~BlackCastling;
    }

    if (from == Square::a1 || to == Square::a1) castling_rights_ &= ~WhiteOOO;
    if (from == Square::h1 || to == Square::h1) castling_rights_ &= ~WhiteOO;
    if (from == Square::a8 || to == Square::a8) castling_rights_ &= ~BlackOOO;
    if (from == Square::h8 || to == Square::h8) castling_rights_ &= ~BlackOO;

    // Switch turn
    side_to_move_ = them;
}

void Board::unmake_move(Move m) {
    assert(!history_.empty());
    StateInfo state = history_.back();
    history_.pop_back();

    Color them = side_to_move_;
    Color us = ~them;
    side_to_move_ = us;

    Square from = m.from();
    Square to = m.to();
    MoveType type = m.type();

    Piece moved_piece = mailbox_[static_cast<size_t>(to)];
    if (m.is_promotion()) {
        moved_piece = make_piece(us, PieceType::Pawn);
    }

    // Remove piece from 'to' and restore to 'from'
    remove_piece(to);
    set_piece(from, moved_piece);

    // Special moves undo
    if (type == MoveType::KingCastle) {
        Square rook_from = (us == Color::White) ? Square::h1 : Square::h8;
        Square rook_to   = (us == Color::White) ? Square::f1 : Square::f8;
        Piece rook = mailbox_[static_cast<size_t>(rook_to)];
        remove_piece(rook_to);
        set_piece(rook_from, rook);
    } else if (type == MoveType::QueenCastle) {
        Square rook_from = (us == Color::White) ? Square::a1 : Square::a8;
        Square rook_to   = (us == Color::White) ? Square::d1 : Square::d8;
        Piece rook = mailbox_[static_cast<size_t>(rook_to)];
        remove_piece(rook_to);
        set_piece(rook_from, rook);
    }

    // Restore captured piece
    if (state.captured_piece != Piece::None) {
        Square cap_sq = to;
        if (type == MoveType::EnPassant) {
            cap_sq = (us == Color::White) ? make_square(file_of(to), Rank::Rank5)
                                          : make_square(file_of(to), Rank::Rank4);
        }
        set_piece(cap_sq, state.captured_piece);
    }

    // Restore board state variables
    castling_rights_ = state.castling_rights;
    en_passant_sq_ = state.en_passant_sq;
    halfmove_clock_ = state.halfmove_clock;
    if (us == Color::Black) {
        fullmove_number_--;
    }
}

void Board::make_null_move() {
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.en_passant_sq = en_passant_sq_;
    state.halfmove_clock = halfmove_clock_;
    state.captured_piece = Piece::None;
    history_.push_back(state);

    en_passant_sq_ = Square::None;
    side_to_move_ = ~side_to_move_;
}

void Board::unmake_null_move() {
    assert(!history_.empty());
    StateInfo state = history_.back();
    history_.pop_back();

    castling_rights_ = state.castling_rights;
    en_passant_sq_ = state.en_passant_sq;
    halfmove_clock_ = state.halfmove_clock;
    side_to_move_ = ~side_to_move_;
}

std::string Board::to_ascii() const {
    std::string out;
    out += "  +-----------------+\n";
    for (int r = 7; r >= 0; --r) {
        out += std::to_string(r + 1) + " | ";
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            out += piece_to_char(mailbox_[static_cast<size_t>(sq)]);
            out += ' ';
        }
        out += "|\n";
    }
    out += "  +-----------------+\n";
    out += "    a b c d e f g h\n\n";
    out += "Side to move : " + std::string(side_to_move_ == Color::White ? "White" : "Black") + "\n";
    out += "Castling     : ";
    if (castling_rights_ == CastlingNone) out += "-";
    if (castling_rights_ & WhiteOO)  out += "K";
    if (castling_rights_ & WhiteOOO) out += "Q";
    if (castling_rights_ & BlackOO)  out += "k";
    if (castling_rights_ & BlackOOO) out += "q";
    out += "\n";
    out += "En Passant   : " + square_to_string(en_passant_sq_) + "\n";
    out += "Halfmove     : " + std::to_string(halfmove_clock_) + "\n";
    out += "Fullmove     : " + std::to_string(fullmove_number_) + "\n";
    return out;
}

} // namespace heavensgate
