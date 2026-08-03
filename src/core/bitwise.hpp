#pragma once

#include "types.hpp"
#include <bit>
#include <cstdint>
#include <string>
#include <iostream>

namespace heavensgate {

using Bitboard = uint64_t;

constexpr Bitboard EmptyBB = 0ULL;
constexpr Bitboard AllBB   = ~0ULL;

constexpr Bitboard rank_bb(Rank r) noexcept {
    return 0xFFULL << (static_cast<uint8_t>(r) * 8);
}

constexpr Bitboard file_bb(File f) noexcept {
    return 0x0101010101010101ULL << static_cast<uint8_t>(f);
}

constexpr Bitboard square_bb(Square sq) noexcept {
    return 1ULL << static_cast<uint8_t>(sq);
}

inline int popcount(Bitboard b) noexcept {
    return std::popcount(b);
}

inline Square lsb(Bitboard b) noexcept {
    return static_cast<Square>(std::countr_zero(b));
}

inline Square msb(Bitboard b) noexcept {
    return static_cast<Square>(63 - std::countl_zero(b));
}

inline Square pop_lsb(Bitboard& b) noexcept {
    Square s = lsb(b);
    b &= b - 1;
    return s;
}

constexpr bool test_bit(Bitboard b, Square sq) noexcept {
    return (b & square_bb(sq)) != 0;
}

constexpr void set_bit(Bitboard& b, Square sq) noexcept {
    b |= square_bb(sq);
}

constexpr void clear_bit(Bitboard& b, Square sq) noexcept {
    b &= ~square_bb(sq);
}

// Bitboard visualization helper (prints 8x8 grid to stream/string)
inline std::string render_bitboard(Bitboard b) {
    std::string out;
    out.reserve(200);
    out += "  +-----------------+\n";
    for (int r = 7; r >= 0; --r) {
        out += std::to_string(r + 1) + " | ";
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            out += test_bit(b, sq) ? "1 " : ". ";
        }
        out += "|\n";
    }
    out += "  +-----------------+\n";
    out += "    a b c d e f g h\n";
    return out;
}

} // namespace heavensgate
