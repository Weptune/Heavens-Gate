#include "polyglot.hpp"
#include "../movegen/movegen.hpp"
#include "../movegen/attack_masks.hpp"
#include <fstream>
#include <random>
#include <algorithm>

namespace heavensgate {

// Official PolyGlot Random Table (781 64-bit integers)
static const uint64_t PolyGlotRandom[781] = {
    #include "polyglot_keys.inc"
};

// Endian swap helper for 16-bit, 32-bit, 64-bit PolyGlot file format (Big-Endian -> Host)
static inline uint16_t swap16(uint16_t val) {
    return (val >> 8) | (val << 8);
}

static inline uint32_t swap32(uint32_t val) {
    return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) | ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
}

static inline uint64_t swap64(uint64_t val) {
    return ((val >> 56) & 0xffULL) |
           ((val >> 40) & 0xff00ULL) |
           ((val >> 24) & 0xff0000ULL) |
           ((val >> 8)  & 0xff000000ULL) |
           ((val << 8)  & 0xff00000000ULL) |
           ((val << 24) & 0xff0000000000ULL) |
           ((val << 40) & 0xff000000000000ULL) |
           ((val << 56) & 0xff00000000000000ULL);
}

uint64_t PolyGlotBook::compute_polyglot_key(const Board& board) {
    uint64_t key = 0;

    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        Piece p = board.piece_at(s);
        if (p == Piece::None) continue;

        Color c = color_of(p);
        PieceType pt = piece_type_of(p);

        int piece_type_num = 0;
        switch (pt) {
            case PieceType::Pawn:   piece_type_num = 1; break;
            case PieceType::Knight: piece_type_num = 2; break;
            case PieceType::Bishop: piece_type_num = 3; break;
            case PieceType::Rook:   piece_type_num = 4; break;
            case PieceType::Queen:  piece_type_num = 5; break;
            case PieceType::King:   piece_type_num = 6; break;
            default: break;
        }

        // PolyGlot piece indexing: Black is even (0, 2, 4...), White is odd (1, 3, 5...)
        int piece_idx = (piece_type_num - 1) * 2 + (c == Color::White ? 1 : 0);
        key ^= PolyGlotRandom[piece_idx * 64 + sq];
    }

    // Castling keys (PolyGlot indices: White O-O 768, White O-O-O 769, Black O-O 770, Black O-O-O 771)
    CastlingRights cr = board.castling_rights();
    if ((cr & WhiteOO)  != 0) key ^= PolyGlotRandom[768];
    if ((cr & WhiteOOO) != 0) key ^= PolyGlotRandom[769];
    if ((cr & BlackOO)  != 0) key ^= PolyGlotRandom[770];
    if ((cr & BlackOOO) != 0) key ^= PolyGlotRandom[771];

    // En Passant key: PolyGlot only hashes EP if an enemy pawn can legally/pseudo-legally capture
    Square ep_sq = board.en_passant_sq();
    if (ep_sq != Square::None) {
        Color us = board.side_to_move();
        Bitboard pawns = board.pieces(make_piece(us, PieceType::Pawn));
        Bitboard ep_attackers = AttackMasks::pawn_attacks(~us, ep_sq) & pawns;
        if (ep_attackers) {
            int file = static_cast<int>(file_of(ep_sq));
            key ^= PolyGlotRandom[772 + file];
        }
    }

    // Side to Move key: PolyGlot hashes 780 if White to move, 0 if Black to move
    if (board.side_to_move() == Color::White) {
        key ^= PolyGlotRandom[780];
    }

    return key;
}

bool PolyGlotBook::load(const std::string& filepath) {
    entries_.clear();
    loaded_ = false;

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t num_entries = file_size / sizeof(PolyGlotEntry);
    entries_.resize(num_entries);

    for (size_t i = 0; i < num_entries; i++) {
        file.read(reinterpret_cast<char*>(&entries_[i]), sizeof(PolyGlotEntry));
        entries_[i].key    = swap64(entries_[i].key);
        entries_[i].move   = swap16(entries_[i].move);
        entries_[i].weight = swap16(entries_[i].weight);
        entries_[i].learn  = swap32(entries_[i].learn);
    }

    // Ensure sorted order for binary search (std::lower_bound)
    std::sort(entries_.begin(), entries_.end(), [](const PolyGlotEntry& a, const PolyGlotEntry& b) {
        return a.key < b.key;
    });

    loaded_ = !entries_.empty();
    return loaded_;
}

Move PolyGlotBook::parse_polyglot_move(const Board& board, uint16_t pg_move) {
    int from_file = (pg_move >> 6) & 7;
    int from_rank = (pg_move >> 9) & 7;
    int to_file   = (pg_move >> 0) & 7;
    int to_rank   = (pg_move >> 3) & 7;
    int promo     = (pg_move >> 12) & 7;

    Square from_sq = static_cast<Square>(from_rank * 8 + from_file);
    Square to_sq   = static_cast<Square>(to_rank * 8 + to_file);

    PieceType promo_pt = PieceType::None;
    switch (promo) {
        case 1: promo_pt = PieceType::Knight; break;
        case 2: promo_pt = PieceType::Bishop; break;
        case 3: promo_pt = PieceType::Rook; break;
        case 4: promo_pt = PieceType::Queen; break;
        default: break;
    }

    MoveList legal_moves;
    MoveGenerator::generate_legal_moves(board, legal_moves);

    for (const auto& m : legal_moves) {
        if (m.from() == from_sq && m.to() == to_sq) {
            if (promo_pt != PieceType::None && m.promotion_piece_type() != promo_pt) continue;
            return m;
        }
    }

    return Move();
}

Move PolyGlotBook::probe(const Board& board) const {
    if (!loaded_ || entries_.empty()) return Move();

    uint64_t key = compute_polyglot_key(board);

    auto compare = [](const PolyGlotEntry& entry, uint64_t target_key) {
        return entry.key < target_key;
    };

    auto it = std::lower_bound(entries_.begin(), entries_.end(), key, compare);
    if (it == entries_.end() || it->key != key) return Move();

    std::vector<const PolyGlotEntry*> matches;
    uint32_t total_weight = 0;

    while (it != entries_.end() && it->key == key) {
        matches.push_back(&(*it));
        total_weight += it->weight;
        ++it;
    }

    if (matches.empty()) return Move();

    static thread_local std::mt19937 rng(1337);
    if (total_weight > 0) {
        std::uniform_int_distribution<uint32_t> dist(1, total_weight);
        uint32_t random_val = dist(rng);
        uint32_t current_sum = 0;

        for (const auto* match : matches) {
            current_sum += match->weight;
            if (random_val <= current_sum) {
                return parse_polyglot_move(board, match->move);
            }
        }
    }

    return parse_polyglot_move(board, matches[0]->move);
}

} // namespace heavensgate
