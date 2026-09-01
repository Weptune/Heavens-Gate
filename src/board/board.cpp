#include "board.hpp"
#include "../core/fen.hpp"
#include "../core/zobrist.hpp"
#include "../evaluation/pst.hpp"
#include <algorithm>

namespace heavensgate {

Board::Board() {
    clear();
}

void Board::clear() {
    square_pieces_.fill(Piece::None);
    piece_bitboards_.fill(EmptyBB);
    color_bitboards_.fill(EmptyBB);

    king_squares_[0] = Square::None;
    king_squares_[1] = Square::None;

    mg_material_ = { 0, 0 };
    eg_material_ = { 0, 0 };
    mg_pst_ = { 0, 0 };
    eg_pst_ = { 0, 0 };
    game_phase_ = 0;

    side_to_move_ = Color::White;
    castling_rights_ = CastlingNone;
    ep_square_ = Square::None;

    halfmove_clock_ = 0;
    fullmove_number_ = 1;
    zobrist_key_ = 0ULL;

    history_.clear();
    pos_history_.clear();
    accumulator_ = Accumulator{};
}

void Board::reset() {
    load_fen(std::string(FEN::StartPOS));
}

void Board::load_fen(const std::string& fen_str) {
    clear();
    FEN::parse(fen_str, *this);
    pos_history_.push_back(zobrist_key_);
    accumulator_.computed[0] = false;
    accumulator_.computed[1] = false;
}

void Board::recalculate_zobrist_key() {
    zobrist_key_ = Zobrist::compute_hash(*this);
}

void Board::set_piece(Square sq, Piece p) {
    if (p == Piece::None) return;

    square_pieces_[static_cast<size_t>(sq)] = p;

    size_t p_idx = static_cast<size_t>(p);
    piece_bitboards_[p_idx] |= square_bb(sq);

    Color c = color_of(p);
    size_t c_idx = static_cast<size_t>(c);
    color_bitboards_[c_idx] |= square_bb(sq);

    PieceType pt = piece_type_of(p);
    if (pt == PieceType::King) {
        king_squares_[c_idx] = sq;
    }

    int mg_val = 0, eg_val = 0, phase_w = 0;
    switch (pt) {
        case PieceType::Pawn:   mg_val = 100; eg_val = 120; phase_w = 0; break;
        case PieceType::Knight: mg_val = 320; eg_val = 310; phase_w = 1; break;
        case PieceType::Bishop: mg_val = 330; eg_val = 340; phase_w = 1; break;
        case PieceType::Rook:   mg_val = 500; eg_val = 530; phase_w = 2; break;
        case PieceType::Queen:  mg_val = 900; eg_val = 950; phase_w = 4; break;
        default: break;
    }

    mg_material_[c_idx] += mg_val;
    eg_material_[c_idx] += eg_val;
    mg_pst_[c_idx] += PieceSquareTables::get_mg(pt, c, sq);
    eg_pst_[c_idx] += PieceSquareTables::get_eg(pt, c, sq);
    game_phase_ += phase_w;
}

void Board::remove_piece(Square sq) {
    Piece p = square_pieces_[static_cast<size_t>(sq)];
    if (p == Piece::None) return;

    square_pieces_[static_cast<size_t>(sq)] = Piece::None;

    size_t p_idx = static_cast<size_t>(p);
    piece_bitboards_[p_idx] &= ~square_bb(sq);

    Color c = color_of(p);
    size_t c_idx = static_cast<size_t>(c);
    color_bitboards_[c_idx] &= ~square_bb(sq);

    PieceType pt = piece_type_of(p);
    if (pt == PieceType::King) {
        king_squares_[c_idx] = Square::None;
    }

    int mg_val = 0, eg_val = 0, phase_w = 0;
    switch (pt) {
        case PieceType::Pawn:   mg_val = 100; eg_val = 120; phase_w = 0; break;
        case PieceType::Knight: mg_val = 320; eg_val = 310; phase_w = 1; break;
        case PieceType::Bishop: mg_val = 330; eg_val = 340; phase_w = 1; break;
        case PieceType::Rook:   mg_val = 500; eg_val = 530; phase_w = 2; break;
        case PieceType::Queen:  mg_val = 900; eg_val = 950; phase_w = 4; break;
        default: break;
    }

    mg_material_[c_idx] -= mg_val;
    eg_material_[c_idx] -= eg_val;
    mg_pst_[c_idx] -= PieceSquareTables::get_mg(pt, c, sq);
    eg_pst_[c_idx] -= PieceSquareTables::get_eg(pt, c, sq);
    game_phase_ -= phase_w;
}

bool Board::has_non_pawn_material(Color c) const {
    Bitboard knights = pieces(make_piece(c, PieceType::Knight));
    Bitboard bishops = pieces(make_piece(c, PieceType::Bishop));
    Bitboard rooks   = pieces(make_piece(c, PieceType::Rook));
    Bitboard queens  = pieces(make_piece(c, PieceType::Queen));

    return (knights | bishops | rooks | queens) != EmptyBB;
}

bool Board::is_repetition(int fold) const {
    int count = 0;
    int limit = std::max(0, static_cast<int>(pos_history_.size()) - halfmove_clock_ - 1);
    for (int i = static_cast<int>(pos_history_.size()) - 1; i >= limit; --i) {
        if (pos_history_[static_cast<size_t>(i)] == zobrist_key_) {
            count++;
            if (count >= fold) return true;
        }
    }
    return false;
}

bool Board::is_insufficient_material() const {
    Bitboard pawns   = pieces(Piece::WhitePawn)   | pieces(Piece::BlackPawn);
    Bitboard rooks   = pieces(Piece::WhiteRook)   | pieces(Piece::BlackRook);
    Bitboard queens  = pieces(Piece::WhiteQueen)  | pieces(Piece::BlackQueen);

    if (pawns || rooks || queens) return false;

    Bitboard w_knights = pieces(Piece::WhiteKnight);
    Bitboard b_knights = pieces(Piece::BlackKnight);
    Bitboard w_bishops = pieces(Piece::WhiteBishop);
    Bitboard b_bishops = pieces(Piece::BlackBishop);

    int w_minor = popcount(w_knights | w_bishops);
    int b_minor = popcount(b_knights | b_bishops);

    if (w_minor == 0 && b_minor == 0) return true;
    if (w_minor == 1 && b_minor == 0) return true;
    if (w_minor == 0 && b_minor == 1) return true;

    if (w_minor == 1 && b_minor == 1 && w_bishops && b_bishops) {
        Square w_sq = lsb(w_bishops);
        Square b_sq = lsb(b_bishops);
        bool w_light = (static_cast<int>(file_of(w_sq)) + static_cast<int>(rank_of(w_sq))) % 2 != 0;
        bool b_light = (static_cast<int>(file_of(b_sq)) + static_cast<int>(rank_of(b_sq))) % 2 != 0;
        if (w_light == b_light) return true;
    }

    return false;
}

void Board::make_move(const Move& m) {
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.ep_square = ep_square_;
    state.halfmove_clock = halfmove_clock_;
    state.zobrist_key = zobrist_key_;

    Color us = side_to_move_;
    Color them = ~us;

    Square from = m.from();
    Square to = m.to();
    Piece p = piece_at(from);
    PieceType pt = piece_type_of(p);
    Piece captured = piece_at(to);

    state.captured_piece = captured;
    history_.push_back(state);

    zobrist_key_ ^= Zobrist::side_to_move();

    if (ep_square_ != Square::None) {
        zobrist_key_ ^= Zobrist::en_passant(file_of(ep_square_));
        ep_square_ = Square::None;
    }

    halfmove_clock_++;

    if (pt == PieceType::Pawn) {
        halfmove_clock_ = 0;
    }

    if (captured != Piece::None) {
        halfmove_clock_ = 0;
        remove_piece(to);
        zobrist_key_ ^= Zobrist::piece(to, captured);
    }

    remove_piece(from);
    zobrist_key_ ^= Zobrist::piece(from, p);

    if (m.is_ep()) {
        Square cap_sq = make_square(file_of(to), rank_of(from));
        Piece ep_cap = piece_at(cap_sq);
        remove_piece(cap_sq);
        zobrist_key_ ^= Zobrist::piece(cap_sq, ep_cap);
    }

    if (m.is_promotion()) {
        Piece promo_p = make_piece(us, m.promotion_piece_type());
        set_piece(to, promo_p);
        zobrist_key_ ^= Zobrist::piece(to, promo_p);
    } else {
        set_piece(to, p);
        zobrist_key_ ^= Zobrist::piece(to, p);
    }

    if (m.is_castle()) {
        Square rfrom = Square::None;
        Square rto = Square::None;

        if (to == Square::g1) { rfrom = Square::h1; rto = Square::f1; }
        else if (to == Square::c1) { rfrom = Square::a1; rto = Square::d1; }
        else if (to == Square::g8) { rfrom = Square::h8; rto = Square::f8; }
        else if (to == Square::c8) { rfrom = Square::a8; rto = Square::d8; }

        Piece rook = piece_at(rfrom);
        remove_piece(rfrom);
        set_piece(rto, rook);

        zobrist_key_ ^= Zobrist::piece(rfrom, rook);
        zobrist_key_ ^= Zobrist::piece(rto, rook);
    }

    if (pt == PieceType::Pawn && std::abs(static_cast<int>(rank_of(to)) - static_cast<int>(rank_of(from))) == 2) {
        ep_square_ = make_square(file_of(from), (us == Color::White) ? Rank::Rank3 : Rank::Rank6);
        zobrist_key_ ^= Zobrist::en_passant(file_of(ep_square_));
    }

    zobrist_key_ ^= Zobrist::castling(castling_rights_);

    if (pt == PieceType::King) {
        if (us == Color::White) {
            castling_rights_ &= ~(WhiteOO | WhiteOOO);
        } else {
            castling_rights_ &= ~(BlackOO | BlackOOO);
        }
    }

    if (from == Square::a1 || to == Square::a1) castling_rights_ &= ~WhiteOOO;
    if (from == Square::h1 || to == Square::h1) castling_rights_ &= ~WhiteOO;
    if (from == Square::a8 || to == Square::a8) castling_rights_ &= ~BlackOOO;
    if (from == Square::h8 || to == Square::h8) castling_rights_ &= ~BlackOO;

    zobrist_key_ ^= Zobrist::castling(castling_rights_);

    side_to_move_ = them;
    pos_history_.push_back(zobrist_key_);

    accumulator_.computed[0] = false;
    accumulator_.computed[1] = false;
}

void Board::unmake_move(const Move& m) {
    if (history_.empty()) return;

    StateInfo state = history_.back();
    history_.pop_back();

    pos_history_.pop_back();

    Color us = side_to_move_;
    Color them = ~us;
    side_to_move_ = them;

    Square from = m.from();
    Square to = m.to();

    Piece p = piece_at(to);
    if (m.is_promotion()) {
        remove_piece(to);
        p = make_piece(them, PieceType::Pawn);
        set_piece(from, p);
    } else {
        remove_piece(to);
        set_piece(from, p);
    }

    if (state.captured_piece != Piece::None && !m.is_ep()) {
        set_piece(to, state.captured_piece);
    }

    if (m.is_ep()) {
        Square cap_sq = make_square(file_of(to), rank_of(from));
        Piece ep_cap = make_piece(us, PieceType::Pawn);
        set_piece(cap_sq, ep_cap);
    }

    if (m.is_castle()) {
        Square rfrom = Square::None;
        Square rto = Square::None;

        if (to == Square::g1) { rfrom = Square::h1; rto = Square::f1; }
        else if (to == Square::c1) { rfrom = Square::a1; rto = Square::d1; }
        else if (to == Square::g8) { rfrom = Square::h8; rto = Square::f8; }
        else if (to == Square::c8) { rfrom = Square::a8; rto = Square::d8; }

        Piece rook = piece_at(rto);
        remove_piece(rto);
        set_piece(rfrom, rook);
    }

    castling_rights_ = state.castling_rights;
    ep_square_ = state.ep_square;
    halfmove_clock_ = state.halfmove_clock;
    zobrist_key_ = state.zobrist_key;

    accumulator_.computed[0] = false;
    accumulator_.computed[1] = false;
}

void Board::make_null_move() {
    StateInfo state;
    state.castling_rights = castling_rights_;
    state.ep_square = ep_square_;
    state.halfmove_clock = halfmove_clock_;
    state.zobrist_key = zobrist_key_;
    state.captured_piece = Piece::None;

    history_.push_back(state);

    zobrist_key_ ^= Zobrist::side_to_move();

    if (ep_square_ != Square::None) {
        zobrist_key_ ^= Zobrist::en_passant(file_of(ep_square_));
        ep_square_ = Square::None;
    }

    side_to_move_ = ~side_to_move_;
    pos_history_.push_back(zobrist_key_);
}

void Board::unmake_null_move() {
    if (history_.empty()) return;

    StateInfo state = history_.back();
    history_.pop_back();

    pos_history_.pop_back();

    side_to_move_ = ~side_to_move_;

    castling_rights_ = state.castling_rights;
    ep_square_ = state.ep_square;
    halfmove_clock_ = state.halfmove_clock;
    zobrist_key_ = state.zobrist_key;
}

std::string Board::to_ascii() const {
    std::string s = "+---+---+---+---+---+---+---+---+\n";
    for (int r = 7; r >= 0; --r) {
        s += "|";
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = piece_at(sq);
            char c = ' ';
            if (p != Piece::None) {
                c = piece_to_char(p);
            }
            s += " ";
            s += c;
            s += " |";
        }
        s += " " + std::to_string(r + 1) + "\n+---+---+---+---+---+---+---+---+\n";
    }
    s += "  a   b   c   d   e   f   g   h\n";
    s += "Side to move: " + std::string(side_to_move_ == Color::White ? "White" : "Black") + "\n";
    return s;
}

} // namespace heavensgate
