#include "attack_masks.hpp"

namespace heavensgate {

std::array<std::array<Bitboard, 64>, 2> AttackMasks::pawn_attacks_{};
std::array<Bitboard, 64> AttackMasks::knight_attacks_{};
std::array<Bitboard, 64> AttackMasks::king_attacks_{};
bool AttackMasks::initialized_ = false;

void AttackMasks::init() {
    if (initialized_) return;

    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        int r = static_cast<int>(rank_of(s));
        int f = static_cast<int>(file_of(s));

        // 1. Pawn Attacks
        // White Pawns capture (+1 rank, +/-1 file)
        Bitboard w_atk = EmptyBB;
        if (r < 7) {
            if (f > 0) set_bit(w_atk, make_square(static_cast<File>(f - 1), static_cast<Rank>(r + 1)));
            if (f < 7) set_bit(w_atk, make_square(static_cast<File>(f + 1), static_cast<Rank>(r + 1)));
        }
        pawn_attacks_[static_cast<size_t>(Color::White)][sq] = w_atk;

        // Black Pawns capture (-1 rank, +/-1 file)
        Bitboard b_atk = EmptyBB;
        if (r > 0) {
            if (f > 0) set_bit(b_atk, make_square(static_cast<File>(f - 1), static_cast<Rank>(r - 1)));
            if (f < 7) set_bit(b_atk, make_square(static_cast<File>(f + 1), static_cast<Rank>(r - 1)));
        }
        pawn_attacks_[static_cast<size_t>(Color::Black)][sq] = b_atk;

        // 2. Knight Attacks (8 L-shaped offsets)
        Bitboard k_atk = EmptyBB;
        constexpr int knight_dr[8] = { 2,  1, -1, -2, -2, -1,  1,  2};
        constexpr int knight_df[8] = { 1,  2,  2,  1, -1, -2, -2, -1};
        for (int i = 0; i < 8; ++i) {
            int nr = r + knight_dr[i];
            int nf = f + knight_df[i];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                set_bit(k_atk, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
            }
        }
        knight_attacks_[sq] = k_atk;

        // 3. King Attacks (8 adjacent squares)
        Bitboard king_atk = EmptyBB;
        constexpr int king_dr[8] = { 1, 1, 1, 0, 0, -1, -1, -1};
        constexpr int king_df[8] = {-1, 0, 1,-1, 1, -1,  0,  1};
        for (int i = 0; i < 8; ++i) {
            int nr = r + king_dr[i];
            int nf = f + king_df[i];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                set_bit(king_atk, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
            }
        }
        king_attacks_[sq] = king_atk;
    }

    initialized_ = true;
}

Bitboard AttackMasks::bishop_attacks(Square sq, Bitboard occupied) noexcept {
    Bitboard attacks = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    // 4 Diagonal directions: (+1,+1), (+1,-1), (-1,+1), (-1,-1)
    constexpr int dr[4] = {1, 1, -1, -1};
    constexpr int df[4] = {1, -1, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nf = f + df[i];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            Square target = make_square(static_cast<File>(nf), static_cast<Rank>(nr));
            set_bit(attacks, target);
            if (test_bit(occupied, target)) break; // Blocked by piece
            nr += dr[i];
            nf += df[i];
        }
    }
    return attacks;
}

Bitboard AttackMasks::rook_attacks(Square sq, Bitboard occupied) noexcept {
    Bitboard attacks = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    // 4 Orthogonal directions: (+1,0), (-1,0), (0,+1), (0,-1)
    constexpr int dr[4] = {1, -1, 0, 0};
    constexpr int df[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nf = f + df[i];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            Square target = make_square(static_cast<File>(nf), static_cast<Rank>(nr));
            set_bit(attacks, target);
            if (test_bit(occupied, target)) break; // Blocked by piece
            nr += dr[i];
            nf += df[i];
        }
    }
    return attacks;
}

} // namespace heavensgate
