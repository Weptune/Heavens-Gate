#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include "magic.hpp"
#include <array>

namespace heavensgate {

enum class Direction {
    North, South, East, West,
    NorthEast, NorthWest, SouthEast, SouthWest
};

template<Direction D>
constexpr Bitboard shift(Bitboard b) noexcept {
    constexpr Bitboard FileA = 0x0101010101010101ULL;
    constexpr Bitboard FileH = 0x8080808080808080ULL;

    if constexpr (D == Direction::North)     return b << 8;
    if constexpr (D == Direction::South)     return b >> 8;
    if constexpr (D == Direction::East)      return (b & ~FileH) << 1;
    if constexpr (D == Direction::West)      return (b & ~FileA) >> 1;
    if constexpr (D == Direction::NorthEast) return (b & ~FileH) << 9;
    if constexpr (D == Direction::NorthWest) return (b & ~FileA) << 7;
    if constexpr (D == Direction::SouthEast) return (b & ~FileH) >> 7;
    if constexpr (D == Direction::SouthWest) return (b & ~FileA) >> 9;
    return EmptyBB;
}

class AttackMasks {
public:
    static std::array<Bitboard, 64> pawn_attacks_table[2];
    static std::array<Bitboard, 64> knight_attacks_table;
    static std::array<Bitboard, 64> king_attacks_table;

    static void init();

    static Bitboard pawn_attacks(Color c, Square sq) noexcept {
        if (c == Color::None || sq == Square::None) return EmptyBB;
        return pawn_attacks_table[static_cast<size_t>(c)][static_cast<size_t>(sq)];
    }

    static Bitboard knight_attacks(Square sq) noexcept {
        if (sq == Square::None) return EmptyBB;
        return knight_attacks_table[static_cast<size_t>(sq)];
    }

    static Bitboard king_attacks(Square sq) noexcept {
        if (sq == Square::None) return EmptyBB;
        return king_attacks_table[static_cast<size_t>(sq)];
    }

    static Bitboard bishop_attacks(Square sq, Bitboard occupied) noexcept {
        if (sq == Square::None) return EmptyBB;
        return MagicBitboards::bishop_attacks(sq, occupied);
    }

    static Bitboard rook_attacks(Square sq, Bitboard occupied) noexcept {
        if (sq == Square::None) return EmptyBB;
        return MagicBitboards::rook_attacks(sq, occupied);
    }

    static Bitboard queen_attacks(Square sq, Bitboard occupied) noexcept {
        if (sq == Square::None) return EmptyBB;
        return MagicBitboards::queen_attacks(sq, occupied);
    }
};

} // namespace heavensgate
