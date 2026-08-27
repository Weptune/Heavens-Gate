#pragma once

#include "../core/types.hpp"
#include <vector>
#include <cstdint>

namespace heavensgate {

enum class TTBound : uint8_t {
    None  = 0,
    Exact = 1, // Exact score
    Lower = 2, // Fail-high cutoff (score >= beta)
    Upper = 3  // Fail-low cutoff (score <= alpha)
};

struct alignas(16) TTEntry {
    uint64_t key{0};
    Move move;
    int16_t score{0};
    uint8_t depth{0};
    TTBound bound{TTBound::None};
    uint8_t generation{0};
    uint8_t pad{0};

    uint64_t data_word() const noexcept {
        uint64_t d = 0;
        // 8 bytes: Move (2B), score (2B), depth (1B), bound (1B), generation (1B), pad (1B)
        const char* src = reinterpret_cast<const char*>(&move);
        char* dst = reinterpret_cast<char*>(&d);
        for (int i = 0; i < 8; ++i) dst[i] = src[i];
        return d;
    }
};

class TranspositionTable {
private:
    std::vector<TTEntry> table_;
    size_t size_{0};
    size_t mask_{0};
    uint8_t generation_{0};

    size_t hits_{0};
    size_t probes_{0};

public:
    TranspositionTable(size_t size_mb = 64);

    void resize(size_t size_mb = 64);
    void clear();
    void new_search() noexcept { generation_++; }
    uint8_t generation() const noexcept { return generation_; }

    TTEntry* probe(uint64_t key) noexcept;
    void store(uint64_t key, Move move, int score, int depth, TTBound bound, int ply) noexcept;

    void prefetch(uint64_t key) const noexcept;

    size_t hits() const noexcept { return hits_; }
    size_t probes() const noexcept { return probes_; }
    double hit_rate() const noexcept { return probes_ > 0 ? (100.0 * hits_) / probes_ : 0.0; }
    size_t capacity() const noexcept { return size_; }
    int hashfull() const noexcept;
};

} // namespace heavensgate
