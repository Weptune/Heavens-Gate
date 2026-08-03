#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include <array>
#include <string>
#include <vector>

namespace heavensgate {

struct Accumulator {
    std::array<int16_t, 256> v[2]; // 0: White, 1: Black
    bool computed[2]{ false, false };
};

struct StateInfo {
    Piece captured_piece = Piece::None;
    CastlingRights castling_rights = CastlingNone;
    Square ep_square = Square::None;
    int halfmove_clock = 0;
    uint64_t zobrist_key = 0ULL;
};

class Board {
    friend class FEN;

public:
    Board();

    void reset();
    void load_fen(const std::string& fen_str);

    // Piece accessors
    Piece piece_at(Square sq) const { return square_pieces_[static_cast<size_t>(sq)]; }
    Bitboard pieces(Piece p) const {
        size_t idx = static_cast<size_t>(p);
        return idx < 12 ? piece_bitboards_[idx] : EmptyBB;
    }
    Bitboard pieces(Color c) const {
        size_t idx = static_cast<size_t>(c);
        return idx < 2 ? color_bitboards_[idx] : EmptyBB;
    }
    Bitboard occupied() const { return color_bitboards_[0] | color_bitboards_[1]; }

    Color side_to_move() const { return side_to_move_; }
    CastlingRights castling_rights() const { return castling_rights_; }
    Square ep_square() const { return ep_square_; }
    Square en_passant_sq() const { return ep_square_; }
    int halfmove_clock() const { return halfmove_clock_; }
    int fullmove_number() const { return fullmove_number_; }
    uint64_t zobrist_key() const { return zobrist_key_; }
    Square king_square(Color c) const { return king_squares_[static_cast<size_t>(c)]; }
    bool has_non_pawn_material(Color c) const;

    // FEN helper setters
    void set_side_to_move(Color c) { side_to_move_ = c; }
    void set_castling_rights(CastlingRights cr) { castling_rights_ = cr; }
    void set_en_passant_sq(Square sq) { ep_square_ = sq; }
    void set_halfmove_clock(int h) { halfmove_clock_ = h; }
    void set_fullmove_number(int f) { fullmove_number_ = f; }
    void recalculate_zobrist_key();

    // Move execution
    void make_move(const Move& m);
    void unmake_move(const Move& m);

    void make_null_move();
    void unmake_null_move();

    // Game end & draw checks
    bool is_repetition() const;
    bool is_insufficient_material() const;
    std::string to_ascii() const;

    // NNUE Accumulator accessor
    Accumulator& accumulator() { return accumulator_; }
    const Accumulator& accumulator() const { return accumulator_; }

    void set_piece(Square sq, Piece p);
    void remove_piece(Square sq);
    void clear();

private:
    std::array<Piece, 64> square_pieces_{};
    std::array<Bitboard, 12> piece_bitboards_{};
    std::array<Bitboard, 2>  color_bitboards_{};
    std::array<Square, 2>    king_squares_{ Square::None, Square::None };

    Color side_to_move_ = Color::White;
    CastlingRights castling_rights_ = CastlingNone;
    Square ep_square_ = Square::None;
    int halfmove_clock_ = 0;
    int fullmove_number_ = 1;

    uint64_t zobrist_key_ = 0ULL;

    std::vector<StateInfo> history_;
    std::vector<uint64_t> pos_history_;

    Accumulator accumulator_;
};

} // namespace heavensgate
