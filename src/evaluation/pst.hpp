#pragma once

#include "../core/types.hpp"
#include <array>

namespace heavensgate {

// Piece-Square Tables (PST) for Midgame (MG) and Endgame (EG)
class PieceSquareTables {
public:
    static const std::array<int, 64> PawnMG;
    static const std::array<int, 64> PawnEG;
    static const std::array<int, 64> KnightMG;
    static const std::array<int, 64> KnightEG;
    static const std::array<int, 64> BishopMG;
    static const std::array<int, 64> BishopEG;
    static const std::array<int, 64> RookMG;
    static const std::array<int, 64> RookEG;
    static const std::array<int, 64> QueenMG;
    static const std::array<int, 64> QueenEG;
    static const std::array<int, 64> KingMG;
    static const std::array<int, 64> KingEG;

    static constexpr Square flip_sq(Square sq) noexcept {
        return static_cast<Square>(static_cast<uint8_t>(sq) ^ 56);
    }

    static void init() noexcept {}

    static int get_pst_value(PieceType pt, Color c, Square sq, bool is_endgame) noexcept;

    static inline int get_mg(PieceType pt, Color c, Square sq) noexcept {
        return get_pst_value(pt, c, sq, false);
    }

    static inline int get_eg(PieceType pt, Color c, Square sq) noexcept {
        return get_pst_value(pt, c, sq, true);
    }
};

using PST = PieceSquareTables;

} // namespace heavensgate
