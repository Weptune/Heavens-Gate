#pragma once

#include "../core/types.hpp"
#include "../core/bitwise.hpp"
#include <array>
#include <vector>

namespace heavensgate {

struct MagicEntry {
    Bitboard mask{0};
    Bitboard magic{0};
    uint8_t shift{0};
    Bitboard* attacks{nullptr};
};

class MagicBitboards {
private:
    static std::array<MagicEntry, 64> rook_magics_;
    static std::array<MagicEntry, 64> bishop_magics_;
    
    static std::vector<Bitboard> rook_table_;
    static std::vector<Bitboard> bishop_table_;

    static bool initialized_;

    // Helper functions for magic initialization
    static Bitboard mask_rook_occupancy(Square sq) noexcept;
    static Bitboard mask_bishop_occupancy(Square sq) noexcept;

    static Bitboard compute_rook_attacks_reference(Square sq, Bitboard occ) noexcept;
    static Bitboard compute_bishop_attacks_reference(Square sq, Bitboard occ) noexcept;

    static Bitboard index_to_occupancy(int index, int bits, Bitboard mask) noexcept;

public:
    static void init();

    static Bitboard rook_attacks(Square sq, Bitboard occupied) noexcept {
        const MagicEntry& m = rook_magics_[static_cast<size_t>(sq)];
        Bitboard occ = occupied & m.mask;
        size_t idx = static_cast<size_t>((occ * m.magic) >> m.shift);
        return m.attacks[idx];
    }

    static Bitboard bishop_attacks(Square sq, Bitboard occupied) noexcept {
        const MagicEntry& m = bishop_magics_[static_cast<size_t>(sq)];
        Bitboard occ = occupied & m.mask;
        size_t idx = static_cast<size_t>((occ * m.magic) >> m.shift);
        return m.attacks[idx];
    }

    static Bitboard queen_attacks(Square sq, Bitboard occupied) noexcept {
        return rook_attacks(sq, occupied) | bishop_attacks(sq, occupied);
    }

    static Bitboard get_bishop_attacks(Square sq, Bitboard occupied) noexcept { return bishop_attacks(sq, occupied); }
    static Bitboard get_rook_attacks(Square sq, Bitboard occupied) noexcept { return rook_attacks(sq, occupied); }
};

using Magic = MagicBitboards;

} // namespace heavensgate
