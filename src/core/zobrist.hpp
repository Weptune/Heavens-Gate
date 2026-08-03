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
};

} // namespace heavensgate
