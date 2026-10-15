#pragma once

#include "../core/types.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

namespace heavensgate {

enum class TTBound : uint8_t {
    None  = 0,
    Exact = 1, // Exact score
    Lower = 2, // Fail-high cutoff (score >= beta)
    Upper = 3  // Fail-low cutoff (score <= alpha)
};

// Decoded, value-type snapshot of one table slot. Probes copy into this; nothing
// ever holds a pointer into shared storage.
struct TTEntry {
    uint64_t key{0};
    Move move;
    int16_t score{0};
    uint8_t depth{0};
    TTBound bound{TTBound::None};
    uint8_t generation{0};
    // Halfmove clock at store time. Scores are reusable across clocks only far
    // from the 50-move boundary; elsewhere a mismatch leaves a move hint only.
    uint8_t rule50_clock{0};

    bool score_matches_rule50(int clock) const noexcept {
        // Positions far from the boundary (<80) cannot reach a 50-move draw within
        // the search, so their scores are interchangeable. Near it, clocks must match.
        if (clock < 80 && rule50_clock < 80) return true;
        return clock >= 0 && clock < 100 && rule50_clock == clock;
    }

    // Packed layout (little end first): move 16 | score 16 | depth 8 | bound 8 | generation 8 | clock 8.
    uint64_t data_word() const noexcept {
        return uint64_t(move.raw()) | (uint64_t(uint16_t(score)) << 16) | (uint64_t(depth) << 32) |
               (uint64_t(bound) << 40) | (uint64_t(generation) << 48) | (uint64_t(rule50_clock) << 56);
    }
    static TTEntry decode(uint64_t key, uint64_t data) noexcept {
        TTEntry entry;
        entry.key = key;
        entry.move = Move::from_raw(static_cast<uint16_t>(data));
        entry.score = static_cast<int16_t>(static_cast<uint16_t>(data >> 16));
        entry.depth = static_cast<uint8_t>(data >> 32);
        entry.bound = static_cast<TTBound>(static_cast<uint8_t>(data >> 40) & 3);
        entry.generation = static_cast<uint8_t>(data >> 48);
        entry.rule50_clock = static_cast<uint8_t>(data >> 56);
        return entry;
    }
};

// Lockless slot (Hyatt/Mann XOR scheme). key_xor holds key ^ data. A reader that
// observes a torn pair (data from one store, key_xor from another) recomputes a
// key that does not match the probe and treats the slot as a miss. Relaxed
// atomics make concurrent access well-defined without adding fences on x86-64.
struct TTSlot {
    std::atomic<uint64_t> key_xor{0};
    std::atomic<uint64_t> data{0};

    uint64_t load_key(uint64_t& data_out) const noexcept {
        data_out = data.load(std::memory_order_relaxed);
        return key_xor.load(std::memory_order_relaxed) ^ data_out;
    }
    void write(uint64_t key, uint64_t new_data) noexcept {
        data.store(new_data, std::memory_order_relaxed);
        key_xor.store(key ^ new_data, std::memory_order_relaxed);
    }
};

struct alignas(64) TTBucket {
    std::array<TTSlot, 4> slots{};
};
static_assert(sizeof(TTSlot) == 16 && sizeof(TTBucket) == 64, "one bucket must be one cache line");
static_assert(std::atomic<uint64_t>::is_always_lock_free, "TT requires lock-free 64-bit atomics");

#if defined(HG_TT_DIAGNOSTICS)
struct TTDiagnostics {
    uint64_t compatible_hits{0};
    uint64_t rejected_clock_hits{0};
    uint64_t hint_only_hits{0};
    uint64_t score_cutoffs{0};
    uint64_t store_attempts{0};
    uint64_t stored{0};
    // Diagnostic bins only, not thresholds used by search/reuse policy.
    std::array<uint64_t, 10> rejected_clock_deciles{};
    void add(const TTDiagnostics& other) noexcept {
        compatible_hits += other.compatible_hits;
        rejected_clock_hits += other.rejected_clock_hits;
        hint_only_hits += other.hint_only_hits;
        score_cutoffs += other.score_cutoffs;
        store_attempts += other.store_attempts;
        stored += other.stored;
        for (size_t i = 0; i < rejected_clock_deciles.size(); ++i)
            rejected_clock_deciles[i] += other.rejected_clock_deciles[i];
    }
};
#endif

// One writer per search worker. Never shared or atomically incremented at a node.
struct alignas(64) TTStatistics {
    uint64_t probes{0};
    uint64_t hits{0};
#if defined(HG_TT_DIAGNOSTICS)
    TTDiagnostics diagnostics{};
#endif
    void reset() noexcept {
        probes = hits = 0;
#if defined(HG_TT_DIAGNOSTICS)
        diagnostics = {};
#endif
    }
    void add(const TTStatistics& other) noexcept {
        probes += other.probes;
        hits += other.hits;
#if defined(HG_TT_DIAGNOSTICS)
        diagnostics.add(other.diagnostics);
#endif
    }
};

class TranspositionTable {
private:
    std::unique_ptr<TTBucket[]> table_;
    size_t num_buckets_{0};
    size_t mask_{0};
    std::atomic<uint8_t> generation_{0};

    // Published current-search totals, updated only while idle/after the SMP barrier.
    std::atomic<uint64_t> hits_{0};
    std::atomic<uint64_t> probes_{0};

public:
    TranspositionTable(size_t size_mb = 64);

    void resize(size_t size_mb = 64);
    void clear();
    // resize/clear require idle workers; probes/stores/hashfull are concurrent-safe.
    void new_search() noexcept { generation_.fetch_add(1, std::memory_order_relaxed); }
    uint8_t generation() const noexcept { return generation_.load(std::memory_order_relaxed); }

    // Compatibility API returns a thread-local snapshot, never a pointer into the table.
    TTEntry* probe(uint64_t key) noexcept;
    // Raw probes do not alter statistics.
    bool probe(uint64_t key, TTEntry& entry) const noexcept;
    bool probe(uint64_t key, TTEntry& entry, TTStatistics& statistics) const noexcept;
    // Raw/test callers default to clock 0. Search callers must pass the board clock.
    void store(uint64_t key, Move move, int score, int depth, TTBound bound, int ply, int rule50_clock = 0,
               TTStatistics* statistics = nullptr) noexcept;

    void prefetch(uint64_t key) const noexcept;

    void publish_statistics(const TTStatistics& statistics) noexcept {
        probes_.store(statistics.probes, std::memory_order_relaxed);
        hits_.store(statistics.hits, std::memory_order_relaxed);
    }
    uint64_t hits() const noexcept { return hits_.load(std::memory_order_relaxed); }
    uint64_t probes() const noexcept { return probes_.load(std::memory_order_relaxed); }
    double hit_rate() const noexcept { const auto n = probes(); return n ? (100.0 * hits()) / n : 0.0; }
    size_t capacity() const noexcept { return num_buckets_ * 4; }
    int hashfull() const noexcept;
};

} // namespace heavensgate
