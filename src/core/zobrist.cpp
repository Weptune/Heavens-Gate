#include "zobrist.hpp"
#include "../board/board.hpp"
#include <random>

namespace heavensgate {

std::array<std::array<Bitboard, 64>, 12> Zobrist::PieceKeys{};
Bitboard Zobrist::SideKey = 0ULL;
std::array<Bitboard, 16> Zobrist::CastlingKeys{};
std::array<Bitboard, 64> Zobrist::EnPassantKeys{};

static bool initialized = false;

void Zobrist::init() {
    if (initialized) return;

    std::mt19937_64 rng(704886); // Deterministic seed for reproducible Zobrist keys

    for (int p = 0; p < 12; ++p) {
        for (int sq = 0; sq < 64; ++sq) {
            PieceKeys[p][sq] = rng();
        }
    }

    SideKey = rng();

    for (int c = 0; c < 16; ++c) {
        CastlingKeys[c] = rng();
    }

    for (int sq = 0; sq < 64; ++sq) {
        EnPassantKeys[sq] = rng();
    }

    initialized = true;
}

Bitboard Zobrist::compute_hash(const Board& board) noexcept {
    init();

    Bitboard hash = 0ULL;

    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        Piece p = board.piece_at(s);
        if (p != Piece::None) {
            hash ^= PieceKeys[static_cast<size_t>(p)][sq];
        }
    }

    if (board.side_to_move() == Color::Black) {
        hash ^= SideKey;
    }

    hash ^= CastlingKeys[static_cast<size_t>(board.castling_rights())];

    if (board.en_passant_sq() != Square::None) {
        hash ^= EnPassantKeys[static_cast<size_t>(board.en_passant_sq())];
    }

    return hash;
}

} // namespace heavensgate
