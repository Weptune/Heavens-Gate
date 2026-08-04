#pragma once

#include "types.hpp"
#include "../board/board.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace heavensgate {

struct PolyGlotEntry {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};

class PolyGlotBook {
public:
    PolyGlotBook() = default;

    // Load binary PolyGlot book file (.bin)
    bool load(const std::string& filepath);

    // Probe position key and return a master move (weighted random selection)
    Move probe(const Board& board) const;

    // Compute 64-bit PolyGlot Zobrist key for board
    static uint64_t compute_polyglot_key(const Board& board);

    bool is_loaded() const { return loaded_; }

private:
    std::vector<PolyGlotEntry> entries_;
    bool loaded_ = false;

    static Move parse_polyglot_move(const Board& board, uint16_t pg_move);
};

} // namespace heavensgate
