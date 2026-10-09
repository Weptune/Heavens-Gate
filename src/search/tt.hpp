#pragma once

#include "../core/types.hpp"
#include <vector>
#include <cstdint>
#include <atomic>
#include <memory>

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
    // Reuse padding: score bounds depend on the reversible halfmove count.
    // A different count can still supply a move hint, never a score/extension.
    uint8_t rule50_clock{0};
    bool score_matches_rule50(int clock) const noexcept {
        // Opening and middlegame positions far from the 50-move draw boundary (<80)
        // are fully compatible and cannot trigger a 50-move draw within search depth.
        if (clock < 80 && rule50_clock < 80) return true;
        return clock >= 0 && clock < 100 && rule50_clock == clock;
    }

    uint64_t data_word() const noexcept {
        uint64_t d = 0;
        // 8 bytes: Move (2B), score (2B), depth (1B), bound (1B), generation (1B), rule50 (1B)
        const char* src = reinterpret_cast<const char*>(&move);
        char* dst = reinterpret_cast<char*>(&d);
        for (int i = 0; i < 8; ++i) dst[i] = src[i];
        return d;
    }
};

struct alignas(64) TTBucket {
    std::array<TTEntry, 4> entries{};
};
static_assert(sizeof(TTEntry) == 16 && sizeof(TTBucket) == 64);

#if defined(HG_TT_DIAGNOSTICS)
struct TTDiagnostics {
    uint64_t compatible_hits{0};
    uint64_t rejected_clock_hits{0};
    uint64_t hint_only_hits{0};
    uint64_t score_cutoffs{0};
    uint64_t store_attempts{0};
    uint64_t stored{0};
    uint64_t context_replacements{0};
    uint64_t shallower_context_replacements{0};
    uint64_t qsearch_context_replacements_of_deeper{0};
    // Diagnostic bins only, not thresholds used by search/reuse policy.
    std::array<uint64_t, 10> rejected_clock_deciles{};
    void add(const TTDiagnostics& other) noexcept {
        compatible_hits += other.compatible_hits;
        rejected_clock_hits += other.rejected_clock_hits;
        hint_only_hits += other.hint_only_hits;
        score_cutoffs += other.score_cutoffs;
        store_attempts += other.store_attempts;
        stored += other.stored;
        context_replacements += other.context_replacements;
        shallower_context_replacements += other.shallower_context_replacements;
        qsearch_context_replacements_of_deeper += other.qsearch_context_replacements_of_deeper;
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
    std::vector<TTBucket> table_;
    // Contention becomes a cache miss/dropped store, never an unbounded spin.
    std::unique_ptr<std::atomic_flag[]> locks_;
    size_t num_buckets_{0};
    size_t mask_{0};
    std::atomic<uint8_t> generation_{0};
    bool clock_contexts_{false}; // Experimental; configured only while idle.

    bool probe_context(uint64_t key, TTEntry& entry, int rule50_clock) noexcept;

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

    TTEntry* probe(uint64_t key) noexcept;
    // Raw compatibility probes do not alter diagnostic totals.
    bool probe(uint64_t key, TTEntry& entry) noexcept;
    bool probe(uint64_t key, TTEntry& entry, TTStatistics& statistics) noexcept;
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
    bool clock_contexts() const noexcept { return clock_contexts_; }
    void set_clock_contexts(bool enabled) noexcept {
        if (clock_contexts_ != enabled) {
            clock_contexts_ = enabled;
            clear(); // Never mix single-context and multi-context layouts.
        }
    }
    // Prefer a matching score context; otherwise return a board-only move hint.
    bool probe(uint64_t key, TTEntry& entry, TTStatistics& statistics, int rule50_clock) noexcept;
};

} // namespace heavensgate
