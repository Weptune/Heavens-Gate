#pragma once

#include "types.hpp"
#include "bitwise.hpp"
#include <array>

namespace heavensgate {

class Board; // Forward declaration

class Zobrist {
public:
    static std::array<std::array<Bitboard, 64>, 12> PieceKeys;
    static Bitboard SideKey;
    static std::array<Bitboard, 16> CastlingKeys;
    static std::array<Bitboard, 64> EnPassantKeys;

    static void init();
    static Bitboard compute_hash(const Board& board) noexcept;

    static Bitboard piece(Square sq, Piece p) noexcept {
        if (p == Piece::None || sq == Square::None) return 0;
        size_t p_idx = static_cast<size_t>(p);
        size_t sq_idx = static_cast<size_t>(sq);
        return (p_idx < 12 && sq_idx < 64) ? PieceKeys[p_idx][sq_idx] : 0ULL;
    }

    static Bitboard side_to_move() noexcept { return SideKey; }
    static Bitboard castling(CastlingRights cr) noexcept { return CastlingKeys[static_cast<size_t>(cr) & 0x0F]; }
    static Bitboard en_passant(File f) noexcept {
        if (f == File::None) return 0ULL;
        return EnPassantKeys[static_cast<size_t>(f)];
    }
};

} // namespace heavensgate
