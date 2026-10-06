#include "attack_masks.hpp"
#include "../core/bitwise.hpp"

namespace heavensgate {

std::array<Bitboard, 64> AttackMasks::pawn_attacks_table[2]{};
std::array<Bitboard, 64> AttackMasks::knight_attacks_table{};
std::array<Bitboard, 64> AttackMasks::king_attacks_table{};
std::array<std::array<Bitboard, 64>, 64> AttackMasks::between_table{};
std::array<std::array<Bitboard, 64>, 64> AttackMasks::line_table{};

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

    for (int sq1 = 0; sq1 < 64; ++sq1) {
        for (int sq2 = 0; sq2 < 64; ++sq2) {
            between_table[sq1][sq2] = EmptyBB;
            line_table[sq1][sq2] = EmptyBB;
            if (sq1 == sq2) continue;

            Square s1 = static_cast<Square>(sq1);
            Square s2 = static_cast<Square>(sq2);
            int f1 = static_cast<int>(file_of(s1)), r1 = static_cast<int>(rank_of(s1));
            int f2 = static_cast<int>(file_of(s2)), r2 = static_cast<int>(rank_of(s2));
            int df = f2 - f1;
            int dr = r2 - r1;

            if (df == 0 || dr == 0 || std::abs(df) == std::abs(dr)) {
                int step_f = (df == 0) ? 0 : (df > 0 ? 1 : -1);
                int step_r = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);

                // Between squares (strictly between s1 and s2)
                int curr_f = f1 + step_f;
                int curr_r = r1 + step_r;
                while (curr_f != f2 || curr_r != r2) {
                    between_table[sq1][sq2] |= square_bb(make_square(static_cast<File>(curr_f), static_cast<Rank>(curr_r)));
                    curr_f += step_f;
                    curr_r += step_r;
                }

                // Line through (entire ray extending across whole board)
                for (int f = 0; f < 8; ++f) {
                    for (int r = 0; r < 8; ++r) {
                        int test_df = f - f1;
                        int test_dr = r - r1;
                        if (test_df * step_r == test_dr * step_f) {
                            line_table[sq1][sq2] |= square_bb(make_square(static_cast<File>(f), static_cast<Rank>(r)));
                        }
                    }
                }
            }
        }
    }
}

} // namespace heavensgate
