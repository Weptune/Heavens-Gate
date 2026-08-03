#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include "../core/zobrist.hpp"
#include <array>
#include <vector>
#include <string>

namespace heavensgate {

struct StateInfo {
    CastlingRights castling_rights{CastlingNone};
    Square en_passant_sq{Square::None};
    uint16_t halfmove_clock{0};
    Piece captured_piece{Piece::None};
    Bitboard zobrist_key{0ULL};
};

class Board {
private:
    std::array<Bitboard, 13> piece_bb_{};
    std::array<Bitboard, 3> color_bb_{};
    std::array<Piece, 64> mailbox_{};

    Color side_to_move_{Color::White};
    CastlingRights castling_rights_{CastlingNone};
    Square en_passant_sq_{Square::None};
    uint16_t halfmove_clock_{0};
    uint16_t fullmove_number_{1};
    Bitboard zobrist_key_{0ULL};

    std::vector<StateInfo> history_{};

public:
    Board();
    
    // Position manipulation
    void clear();
    void set_piece(Square sq, Piece p);
    void remove_piece(Square sq);
    void recalculate_zobrist_key() noexcept;

    // Getters
    constexpr Color side_to_move() const noexcept { return side_to_move_; }
    constexpr CastlingRights castling_rights() const noexcept { return castling_rights_; }
    constexpr Square en_passant_sq() const noexcept { return en_passant_sq_; }
    constexpr uint16_t halfmove_clock() const noexcept { return halfmove_clock_; }
    constexpr uint16_t fullmove_number() const noexcept { return fullmove_number_; }
    constexpr Bitboard zobrist_key() const noexcept { return zobrist_key_; }

    constexpr Bitboard pieces(Piece p) const noexcept {
        if (p == Piece::None) return EmptyBB;
        return piece_bb_[static_cast<size_t>(p)];
    }
    constexpr Bitboard pieces(Color c) const noexcept {
        if (c == Color::None) return EmptyBB;
        return color_bb_[static_cast<size_t>(c)];
    }
    constexpr Bitboard pieces(PieceType pt) const noexcept {
        if (pt == PieceType::None) return EmptyBB;
        return piece_bb_[static_cast<size_t>(make_piece(Color::White, pt))] |
               piece_bb_[static_cast<size_t>(make_piece(Color::Black, pt))];
    }
    constexpr Bitboard occupied() const noexcept { return color_bb_[2]; }
    constexpr Piece piece_at(Square sq) const noexcept {
        if (sq == Square::None) return Piece::None;
        return mailbox_[static_cast<size_t>(sq)];
    }

    constexpr Square king_square(Color c) const noexcept {
        if (c == Color::None) return Square::None;
        Bitboard k = piece_bb_[static_cast<size_t>(make_piece(c, PieceType::King))];
        return k ? lsb(k) : Square::None;
    }

    // Mutators
    void set_side_to_move(Color c) noexcept { side_to_move_ = c; }
    void set_castling_rights(CastlingRights cr) noexcept { castling_rights_ = cr; }
    void set_en_passant_sq(Square sq) noexcept { en_passant_sq_ = sq; }
    void set_halfmove_clock(uint16_t hm) noexcept { halfmove_clock_ = hm; }
    void set_fullmove_number(uint16_t fm) noexcept { fullmove_number_ = fm; }

    // Make & Unmake move
    void make_move(Move m);
    void unmake_move(Move m);
    void make_null_move();
    void unmake_null_move();

    // String formatting
    std::string to_ascii() const;
};

constexpr std::string_view StartposFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

} // namespace heavensgate
