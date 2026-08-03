#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include "magic.hpp"
#include <array>

namespace heavensgate {

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
