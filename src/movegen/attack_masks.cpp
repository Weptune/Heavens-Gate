#include "attack_masks.hpp"
#include "../core/bitwise.hpp"

namespace heavensgate {

std::array<Bitboard, 64> AttackMasks::pawn_attacks_table[2]{};
std::array<Bitboard, 64> AttackMasks::knight_attacks_table{};
std::array<Bitboard, 64> AttackMasks::king_attacks_table{};

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

void AttackMasks::init() {
    MagicBitboards::init();

    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        Bitboard b = square_bb(s);

        // White Pawn Attacks (Northeast + Northwest)
        pawn_attacks_table[static_cast<size_t>(Color::White)][sq] =
            shift<Direction::NorthEast>(b) | shift<Direction::NorthWest>(b);

        // Black Pawn Attacks (Southeast + Southwest)
        pawn_attacks_table[static_cast<size_t>(Color::Black)][sq] =
            shift<Direction::SouthEast>(b) | shift<Direction::SouthWest>(b);

        // Knight Attacks (8 L-shaped jumps)
        knight_attacks_table[sq] =
            shift<Direction::North>(shift<Direction::NorthEast>(b)) |
            shift<Direction::North>(shift<Direction::NorthWest>(b)) |
            shift<Direction::South>(shift<Direction::SouthEast>(b)) |
            shift<Direction::South>(shift<Direction::SouthWest>(b)) |
            shift<Direction::East>(shift<Direction::NorthEast>(b)) |
            shift<Direction::East>(shift<Direction::SouthEast>(b)) |
            shift<Direction::West>(shift<Direction::NorthWest>(b)) |
            shift<Direction::West>(shift<Direction::SouthWest>(b));

        // King Attacks (8 surrounding squares)
        king_attacks_table[sq] =
            shift<Direction::North>(b) | shift<Direction::South>(b) |
            shift<Direction::East>(b)  | shift<Direction::West>(b)  |
            shift<Direction::NorthEast>(b) | shift<Direction::NorthWest>(b) |
            shift<Direction::SouthEast>(b) | shift<Direction::SouthWest>(b);
    }
}

} // namespace heavensgate
