#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include <array>

namespace heavensgate {

class AttackMasks {
private:
    static std::array<std::array<Bitboard, 64>, 2> pawn_attacks_;
    static std::array<Bitboard, 64> knight_attacks_;
    static std::array<Bitboard, 64> king_attacks_;
    static bool initialized_;

public:
    static void init();

    static Bitboard pawn_attacks(Color c, Square sq) noexcept {
        if (c == Color::None || sq == Square::None) return EmptyBB;
        return pawn_attacks_[static_cast<size_t>(c)][static_cast<size_t>(sq)];
    }

    static Bitboard knight_attacks(Square sq) noexcept {
        if (sq == Square::None) return EmptyBB;
        return knight_attacks_[static_cast<size_t>(sq)];
    }

    static Bitboard king_attacks(Square sq) noexcept {
        if (sq == Square::None) return EmptyBB;
        return king_attacks_[static_cast<size_t>(sq)];
    }

    // Sliding piece attack generators
    static Bitboard bishop_attacks(Square sq, Bitboard occupied) noexcept;
    static Bitboard rook_attacks(Square sq, Bitboard occupied) noexcept;
    static Bitboard queen_attacks(Square sq, Bitboard occupied) noexcept {
        return bishop_attacks(sq, occupied) | rook_attacks(sq, occupied);
    }
};

} // namespace heavensgate
