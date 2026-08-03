#include "board.hpp"
#include "../core/zobrist.hpp"
#include <sstream>
#include <iostream>

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
    zobrist_key_ = 0ULL;
    history_.clear();
}

void Board::recalculate_zobrist_key() noexcept {
    zobrist_key_ = Zobrist::compute_hash(*this);
}

bool Board::is_repetition() const noexcept {
    if (history_.empty()) return false;
    int count = 0;
    int search_limit = std::min(static_cast<int>(history_.size()), static_cast<int>(halfmove_clock_));
    int size = static_cast<int>(history_.size());

    for (int i = 1; i <= search_limit; ++i) {
        if (history_[size - i].zobrist_key == zobrist_key_) {
            count++;
            if (count >= 1) return true;
        }
    }
    return false;
}

bool Board::is_insufficient_material() const noexcept {
    // If pawns, rooks, or queens exist, material is sufficient for checkmate
    if (pieces(PieceType::Pawn) || pieces(PieceType::Rook) || pieces(PieceType::Queen)) {
        return false;
    }

    int white_knights = popcount(pieces(make_piece(Color::White, PieceType::Knight)));
    int black_knights = popcount(pieces(make_piece(Color::Black, PieceType::Knight)));
    int white_bishops = popcount(pieces(make_piece(Color::White, PieceType::Bishop)));
    int black_bishops = popcount(pieces(make_piece(Color::Black, PieceType::Bishop)));

    int total_minors = white_knights + black_knights + white_bishops + black_bishops;

    // K vs K
    if (total_minors == 0) return true;

    // K+N vs K or K+B vs K
    if (total_minors == 1) return true;

    // K+B vs K+B (bishops of same color)
    if (white_bishops == 1 && black_bishops == 1 && white_knights == 0 && black_knights == 0) {
        Square w_bsq = lsb(pieces(make_piece(Color::White, PieceType::Bishop)));
        Square b_bsq = lsb(pieces(make_piece(Color::Black, PieceType::Bishop)));

        bool w_is_light = (static_cast<int>(file_of(w_bsq)) + static_cast<int>(rank_of(w_bsq))) % 2 != 0;
        bool b_is_light = (static_cast<int>(file_of(b_bsq)) + static_cast<int>(rank_of(b_bsq))) % 2 != 0;

        if (w_is_light == b_is_light) return true; // Same color bishops -> Draw!
    }

    return false;
}

void Board::set_piece(Square sq, Piece p) {
    if (sq == Square::None) return;

    Piece old_p = mailbox_[static_cast<size_t>(sq)];
    if (old_p != Piece::None) {
        remove_piece(sq);
    }

    if (p == Piece::None) return;

    size_t s_idx = static_cast<size_t>(sq);
    size_t p_idx = static_cast<size_t>(p);
    Color c = color_of(p);
    size_t c_idx = static_cast<size_t>(c);

    set_bit(piece_bb_[p_idx], sq);
    set_bit(color_bb_[c_idx], sq);
    set_bit(color_bb_[2], sq);

    mailbox_[s_idx] = p;

    zobrist_key_ ^= Zobrist::PieceKeys[p_idx][s_idx];
}

void Board::remove_piece(Square sq) {
    if (sq == Square::None) return;

    size_t s_idx = static_cast<size_t>(sq);
    Piece p = mailbox_[s_idx];
    if (p == Piece::None) return;

    size_t p_idx = static_cast<size_t>(p);
    Color c = color_of(p);
    size_t c_idx = static_cast<size_t>(c);

    clear_bit(piece_bb_[p_idx], sq);
    clear_bit(color_bb_[c_idx], sq);
    clear_bit(color_bb_[2], sq);

    mailbox_[s_idx] = Piece::None;

    zobrist_key_ ^= Zobrist::PieceKeys[p_idx][s_idx];
}

void Board::make_move(Move m) {
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.en_passant_sq   = en_passant_sq_;
    state.halfmove_clock  = halfmove_clock_;
    state.zobrist_key     = zobrist_key_;

    Square from = m.from();
    Square to   = m.to();
    MoveType type = m.type();

    Piece moving_piece = piece_at(from);
    Piece captured_piece = (type == MoveType::EnPassant)
        ? make_piece(~side_to_move_, PieceType::Pawn)
        : piece_at(to);

    state.captured_piece = captured_piece;
    history_.push_back(state);

    halfmove_clock_++;
    if (side_to_move_ == Color::Black) {
        fullmove_number_++;
    }

    if (piece_type_of(moving_piece) == PieceType::Pawn || captured_piece != Piece::None) {
        halfmove_clock_ = 0;
    }

    if (en_passant_sq_ != Square::None) {
        zobrist_key_ ^= Zobrist::EnPassantKeys[static_cast<size_t>(en_passant_sq_)];
        en_passant_sq_ = Square::None;
    }

    if (captured_piece != Piece::None) {
        Square cap_sq = (type == MoveType::EnPassant)
            ? make_square(file_of(to), rank_of(from))
            : to;
        remove_piece(cap_sq);
    }

    remove_piece(from);

    if (m.is_promotion()) {
        set_piece(to, make_piece(side_to_move_, m.promotion_piece_type()));
    } else {
        set_piece(to, moving_piece);
    }

    if (type == MoveType::DoublePawnPush) {
        Rank ep_rank = (side_to_move_ == Color::White) ? Rank::Rank3 : Rank::Rank6;
        en_passant_sq_ = make_square(file_of(from), ep_rank);
        zobrist_key_ ^= Zobrist::EnPassantKeys[static_cast<size_t>(en_passant_sq_)];
    } else if (type == MoveType::KingCastle) {
        Square rook_from = (side_to_move_ == Color::White) ? Square::h1 : Square::h8;
        Square rook_to   = (side_to_move_ == Color::White) ? Square::f1 : Square::f8;
        Piece rook = piece_at(rook_from);
        remove_piece(rook_from);
        set_piece(rook_to, rook);
    } else if (type == MoveType::QueenCastle) {
        Square rook_from = (side_to_move_ == Color::White) ? Square::a1 : Square::a8;
        Square rook_to   = (side_to_move_ == Color::White) ? Square::d1 : Square::d8;
        Piece rook = piece_at(rook_from);
        remove_piece(rook_from);
        set_piece(rook_to, rook);
    }

    zobrist_key_ ^= Zobrist::CastlingKeys[static_cast<size_t>(castling_rights_)];

    if (moving_piece == Piece::WhiteKing) {
        castling_rights_ &= ~WhiteCastling;
    } else if (moving_piece == Piece::BlackKing) {
        castling_rights_ &= ~BlackCastling;
    }

    if (from == Square::a1 || to == Square::a1) castling_rights_ &= ~WhiteOOO;
    if (from == Square::h1 || to == Square::h1) castling_rights_ &= ~WhiteOO;
    if (from == Square::a8 || to == Square::a8) castling_rights_ &= ~BlackOOO;
    if (from == Square::h8 || to == Square::h8) castling_rights_ &= ~BlackOO;

    zobrist_key_ ^= Zobrist::CastlingKeys[static_cast<size_t>(castling_rights_)];

    side_to_move_ = ~side_to_move_;
    zobrist_key_ ^= Zobrist::SideKey;
}

void Board::unmake_move(Move m) {
    if (history_.empty()) return;

    StateInfo state = history_.back();
    history_.pop_back();

    side_to_move_ = ~side_to_move_;
    if (side_to_move_ == Color::Black) {
        fullmove_number_--;
    }

    castling_rights_ = state.castling_rights;
    en_passant_sq_   = state.en_passant_sq;
    halfmove_clock_  = state.halfmove_clock;

    Square from = m.from();
    Square to   = m.to();
    MoveType type = m.type();

    Piece moved_piece = piece_at(to);

    remove_piece(to);

    if (m.is_promotion()) {
        set_piece(from, make_piece(side_to_move_, PieceType::Pawn));
    } else {
        set_piece(from, moved_piece);
    }

    if (state.captured_piece != Piece::None) {
        Square cap_sq = (type == MoveType::EnPassant)
            ? make_square(file_of(to), rank_of(from))
            : to;
        set_piece(cap_sq, state.captured_piece);
    }

    if (type == MoveType::KingCastle) {
        Square rook_from = (side_to_move_ == Color::White) ? Square::h1 : Square::h8;
        Square rook_to   = (side_to_move_ == Color::White) ? Square::f1 : Square::f8;
        Piece rook = piece_at(rook_to);
        remove_piece(rook_to);
        set_piece(rook_from, rook);
    } else if (type == MoveType::QueenCastle) {
        Square rook_from = (side_to_move_ == Color::White) ? Square::a1 : Square::a8;
        Square rook_to   = (side_to_move_ == Color::White) ? Square::d1 : Square::d8;
        Piece rook = piece_at(rook_to);
        remove_piece(rook_to);
        set_piece(rook_from, rook);
    }

    zobrist_key_ = state.zobrist_key;
}

void Board::make_null_move() {
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.en_passant_sq   = en_passant_sq_;
    state.halfmove_clock  = halfmove_clock_;
    state.captured_piece  = Piece::None;
    state.zobrist_key     = zobrist_key_;

    history_.push_back(state);

    if (en_passant_sq_ != Square::None) {
        zobrist_key_ ^= Zobrist::EnPassantKeys[static_cast<size_t>(en_passant_sq_)];
        en_passant_sq_ = Square::None;
    }

    side_to_move_ = ~side_to_move_;
    zobrist_key_ ^= Zobrist::SideKey;
}

void Board::unmake_null_move() {
    if (history_.empty()) return;

    StateInfo state = history_.back();
    history_.pop_back();

    side_to_move_ = ~side_to_move_;
    castling_rights_ = state.castling_rights;
    en_passant_sq_   = state.en_passant_sq;
    halfmove_clock_  = state.halfmove_clock;
    zobrist_key_     = state.zobrist_key;
}

std::string Board::to_ascii() const {
    std::string out;
    out.reserve(256);
    out += "  +-----------------+\n";
    for (int r = 7; r >= 0; --r) {
        out += std::to_string(r + 1) + " | ";
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = piece_at(sq);
            out += piece_to_char(p);
            out += ' ';
        }
        out += "|\n";
    }
    out += "  +-----------------+\n";
    out += "    a b c d e f g h\n";
    out += "Side to move: " + std::string(side_to_move_ == Color::White ? "White" : "Black") + "\n";
    out += "Zobrist Key : " + std::to_string(zobrist_key_) + "\n";
    return out;
}

} // namespace heavensgate
