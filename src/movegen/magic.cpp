#include "magic.hpp"
#include <random>
#include <iostream>
#include <cassert>

namespace heavensgate {

std::array<MagicEntry, 64> MagicBitboards::rook_magics_{};
std::array<MagicEntry, 64> MagicBitboards::bishop_magics_{};
std::vector<Bitboard> MagicBitboards::rook_table_{};
std::vector<Bitboard> MagicBitboards::bishop_table_{};
bool MagicBitboards::initialized_ = false;

// Number of relevant bits for Rook on each square 0..63
static constexpr std::array<int, 64> RookBits = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

// Number of relevant bits for Bishop on each square 0..63
static constexpr std::array<int, 64> BishopBits = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// Pre-calculated tested magic numbers for Rook
static constexpr std::array<Bitboard, 64> PrecalculatedRookMagics = {
    0xa8002c00010080ULL, 0x808000200040080ULL, 0x1000200040100080ULL, 0x8000200040010080ULL,
    0x8000400080020080ULL, 0x8000400080040080ULL, 0x8000400080080080ULL, 0x8000400080100080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL,
    0x20008000400080ULL, 0x40008000200080ULL, 0x80008000200080ULL, 0x100008000200080ULL,
    0x200008000200080ULL, 0x400008000200080ULL, 0x800008000200080ULL, 0x800008000400080ULL
};

// Pre-calculated tested magic numbers for Bishop
static constexpr std::array<Bitboard, 64> PrecalculatedBishopMagics = {
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL,
    0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL, 0x4004084104001ULL
};

Bitboard MagicBitboards::mask_rook_occupancy(Square sq) noexcept {
    Bitboard result = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    for (int nr = r + 1; nr <= 6; ++nr) set_bit(result, make_square(static_cast<File>(f), static_cast<Rank>(nr)));
    for (int nr = r - 1; nr >= 1; --nr) set_bit(result, make_square(static_cast<File>(f), static_cast<Rank>(nr)));
    for (int nf = f + 1; nf <= 6; ++nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(r)));
    for (int nf = f - 1; nf >= 1; --nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(r)));

    return result;
}

Bitboard MagicBitboards::mask_bishop_occupancy(Square sq) noexcept {
    Bitboard result = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    for (int nr = r + 1, nf = f + 1; nr <= 6 && nf <= 6; ++nr, ++nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
    for (int nr = r + 1, nf = f - 1; nr <= 6 && nf >= 1; ++nr, --nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
    for (int nr = r - 1, nf = f + 1; nr >= 1 && nf <= 6; --nr, ++nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));
    for (int nr = r - 1, nf = f - 1; nr >= 1 && nf >= 1; --nr, --nf) set_bit(result, make_square(static_cast<File>(nf), static_cast<Rank>(nr)));

    return result;
}

Bitboard MagicBitboards::compute_rook_attacks_reference(Square sq, Bitboard occ) noexcept {
    Bitboard attacks = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    constexpr int dr[4] = {1, -1, 0, 0};
    constexpr int df[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nf = f + df[i];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            Square target = make_square(static_cast<File>(nf), static_cast<Rank>(nr));
            set_bit(attacks, target);
            if (test_bit(occ, target)) break;
            nr += dr[i];
            nf += df[i];
        }
    }
    return attacks;
}

Bitboard MagicBitboards::compute_bishop_attacks_reference(Square sq, Bitboard occ) noexcept {
    Bitboard attacks = EmptyBB;
    int r = static_cast<int>(rank_of(sq));
    int f = static_cast<int>(file_of(sq));

    constexpr int dr[4] = {1, 1, -1, -1};
    constexpr int df[4] = {1, -1, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nf = f + df[i];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            Square target = make_square(static_cast<File>(nf), static_cast<Rank>(nr));
            set_bit(attacks, target);
            if (test_bit(occ, target)) break;
            nr += dr[i];
            nf += df[i];
        }
    }
    return attacks;
}

Bitboard MagicBitboards::index_to_occupancy(int index, int bits, Bitboard mask) noexcept {
    Bitboard occ = EmptyBB;
    for (int i = 0; i < bits; ++i) {
        Square sq = pop_lsb(mask);
        if (index & (1 << i)) {
            set_bit(occ, sq);
        }
    }
    return occ;
}

static Bitboard random_uint64(std::mt19937_64& rng) {
    return rng();
}

static Bitboard random_sparse_uint64(std::mt19937_64& rng) {
    return random_uint64(rng) & random_uint64(rng) & random_uint64(rng);
}

void MagicBitboards::init() {
    if (initialized_) return;

    std::mt19937_64 rng(1337); // Deterministic seed for reproducible magic search

    // 1. Allocate attack tables
    // Rook table total entries = sum(2^RookBits) = 102,400 entries
    // Bishop table total entries = sum(2^BishopBits) = 5,248 entries
    size_t rook_table_size = 0;
    size_t bishop_table_size = 0;
    for (int i = 0; i < 64; ++i) {
        rook_table_size += (1ULL << RookBits[i]);
        bishop_table_size += (1ULL << BishopBits[i]);
    }

    rook_table_.resize(rook_table_size);
    bishop_table_.resize(bishop_table_size);

    Bitboard* rook_ptr = rook_table_.data();
    Bitboard* bishop_ptr = bishop_table_.data();

    // 2. Initialize Rooks
    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        Bitboard mask = mask_rook_occupancy(s);
        int bits = RookBits[sq];
        int num_occupancies = 1 << bits;

        std::vector<Bitboard> occupancies(num_occupancies);
        std::vector<Bitboard> reference_attacks(num_occupancies);

        for (int i = 0; i < num_occupancies; ++i) {
            occupancies[i] = index_to_occupancy(i, bits, mask);
            reference_attacks[i] = compute_rook_attacks_reference(s, occupancies[i]);
        }

        // Search for valid Magic Number
        Bitboard magic = 0;
        std::vector<Bitboard> used_attacks(num_occupancies);

        for (int iter = 0; iter < 10000000; ++iter) {
            magic = random_sparse_uint64(rng);
            if (popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

            used_attacks.assign(num_occupancies, EmptyBB);
            bool fail = false;

            for (int i = 0; i < num_occupancies; ++i) {
                size_t idx = static_cast<size_t>((occupancies[i] * magic) >> (64 - bits));
                if (used_attacks[idx] == EmptyBB) {
                    used_attacks[idx] = reference_attacks[i];
                } else if (used_attacks[idx] != reference_attacks[i]) {
                    fail = true; // Collision!
                    break;
                }
            }

            if (!fail) break; // Found valid magic multiplier!
        }

        MagicEntry& entry = rook_magics_[sq];
        entry.mask = mask;
        entry.magic = magic;
        entry.shift = static_cast<uint8_t>(64 - bits);
        entry.attacks = rook_ptr;

        for (int i = 0; i < num_occupancies; ++i) {
            size_t idx = static_cast<size_t>((occupancies[i] * magic) >> (64 - bits));
            entry.attacks[idx] = reference_attacks[i];
        }

        rook_ptr += num_occupancies;
    }

    // 3. Initialize Bishops
    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        Bitboard mask = mask_bishop_occupancy(s);
        int bits = BishopBits[sq];
        int num_occupancies = 1 << bits;

        std::vector<Bitboard> occupancies(num_occupancies);
        std::vector<Bitboard> reference_attacks(num_occupancies);

        for (int i = 0; i < num_occupancies; ++i) {
            occupancies[i] = index_to_occupancy(i, bits, mask);
            reference_attacks[i] = compute_bishop_attacks_reference(s, occupancies[i]);
        }

        Bitboard magic = 0;
        std::vector<Bitboard> used_attacks(num_occupancies);

        for (int iter = 0; iter < 10000000; ++iter) {
            magic = random_sparse_uint64(rng);
            if (popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

            used_attacks.assign(num_occupancies, EmptyBB);
            bool fail = false;

            for (int i = 0; i < num_occupancies; ++i) {
                size_t idx = static_cast<size_t>((occupancies[i] * magic) >> (64 - bits));
                if (used_attacks[idx] == EmptyBB) {
                    used_attacks[idx] = reference_attacks[i];
                } else if (used_attacks[idx] != reference_attacks[i]) {
                    fail = true;
                    break;
                }
            }

            if (!fail) break;
        }

        MagicEntry& entry = bishop_magics_[sq];
        entry.mask = mask;
        entry.magic = magic;
        entry.shift = static_cast<uint8_t>(64 - bits);
        entry.attacks = bishop_ptr;

        for (int i = 0; i < num_occupancies; ++i) {
            size_t idx = static_cast<size_t>((occupancies[i] * magic) >> (64 - bits));
            entry.attacks[idx] = reference_attacks[i];
        }

        bishop_ptr += num_occupancies;
    }

    initialized_ = true;
}

} // namespace heavensgate
