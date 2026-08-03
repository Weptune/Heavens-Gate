#include "attack_masks.hpp"
#include "../core/bitwise.hpp"

namespace heavensgate {

std::array<Bitboard, 64> AttackMasks::pawn_attacks_table[2]{};
std::array<Bitboard, 64> AttackMasks::knight_attacks_table{};
std::array<Bitboard, 64> AttackMasks::king_attacks_table{};

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
