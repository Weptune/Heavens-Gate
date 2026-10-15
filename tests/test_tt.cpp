#include "test.hpp"
#include "../src/search/tt.hpp"
#include <atomic>

namespace heavensgate::test {

// Replacement policy: first empty or same-key slot; otherwise the lowest depth,
// with older generations penalised by 32 plies. Same-key results replace only when
// Exact, at least as deep, or from a newer generation.
static bool replacement_policy() {
    TranspositionTable table(1);
    const uint64_t key = 123, stride = table.capacity() / 4; // Same bucket, distinct keys.
    const Move hint(Square::d2, Square::e2);
    TTEntry entry;
    table.store(key, hint, 900, 16, TTBound::Exact, 0, 0);
    table.store(key, Move{}, 700, 2, TTBound::Upper, 0, 0);          // Shallower non-exact: kept out.
    if (!table.probe(key, entry) || entry.depth != 16 || entry.score != 900 || entry.move != hint) return false;
    table.store(key, Move{}, 650, 2, TTBound::Exact, 0, 0);          // Exact replaces, keeps the move.
    if (!table.probe(key, entry) || entry.depth != 2 || entry.score != 650 || entry.move != hint) return false;
    table.store(key, hint, 900, 16, TTBound::Exact, 0, 0);
    table.store(key + stride, hint, 200, 12, TTBound::Exact, 0, 0);
    table.store(key + 2 * stride, hint, 300, 8, TTBound::Exact, 0, 0);
    table.store(key + 3 * stride, hint, 400, 4, TTBound::Exact, 0, 0);
    table.store(key + 4 * stride, hint, 500, 6, TTBound::Exact, 0, 0); // Evicts the depth-4 slot.
    if (table.probe(key + 3 * stride, entry) || !table.probe(key + 4 * stride, entry) || entry.score != 500) return false;
    if (!table.probe(key, entry) || entry.depth != 16) return false;
    table.new_search();
    table.store(key + 5 * stride, hint, 600, 1, TTBound::Exact, 0, 0); // Old generation loses 32 plies: evicts depth 6.
    if (table.probe(key + 4 * stride, entry) || !table.probe(key + 5 * stride, entry) || entry.generation != table.generation()) return false;
    // Same key, new generation, shallower: replaces (stale results do not pin slots).
    table.store(key, Move{}, 5, 1, TTBound::Upper, 0, 0);
    return table.probe(key, entry) && entry.depth == 1 && entry.score == 5 && entry.move == hint;
}

static bool packed_layout() {
    for (int16_t score : {int16_t(-32768), int16_t(-1), int16_t(0), int16_t(31999), int16_t(32767)}) {
        TTEntry entry;
        entry.key = 0xfedcba9876543210ULL; entry.move = Move(Square::g1, Square::f3, MoveType::Capture);
        entry.score = score; entry.depth = 255; entry.bound = TTBound::Upper; entry.generation = 254; entry.rule50_clock = 99;
        const TTEntry back = TTEntry::decode(entry.key, entry.data_word());
        if (back.key != entry.key || back.move != entry.move || back.score != score || back.depth != 255 ||
            back.bound != TTBound::Upper || back.generation != 254 || back.rule50_clock != 99 ||
            (entry.data_word() >> 56) != 99) return false;
    }
    TranspositionTable table(1);
    TTEntry entry;
    // An all-zero slot is empty even for key 0; a stored key 0 is found.
    if (table.probe(0, entry)) return false;
    table.store(0, Move{}, 1, 1, TTBound::Lower, 0, 0);
    return table.probe(0, entry) && entry.bound == TTBound::Lower &&
           sizeof(TTSlot) == 16 && sizeof(TTBucket) == 64 && alignof(TTBucket) == 64;
}

// Lockless contract: under heavy same-bucket contention every hit must be an entry
// that some writer actually stored for that key (no torn key/data combination).
static bool concurrent_coherence() {
    TranspositionTable table(1);
    const uint64_t stride = table.capacity() / 4;
    std::atomic<int> errors{0};
    std::array<TTStatistics, 8> stats{};
#if defined(_OPENMP)
    #pragma omp parallel for num_threads(8)
#endif
    for (int thread = 0; thread < 8; ++thread) {
        for (int i = 0; i < 50000; ++i) {
            const uint64_t key = 0x9e3779b97f4a7c15ULL + static_cast<uint64_t>((i * 7 + thread) % 16) * stride;
            const int depth = static_cast<int>((key >> 3) % 40), score = static_cast<int>(key % 2000) - 1000;
            table.store(key, Move(Square::e2, Square::e4), score, depth, TTBound::Exact, 0, static_cast<int>(key % 100));
            TTEntry entry;
            const uint64_t probe_key = 0x9e3779b97f4a7c15ULL + static_cast<uint64_t>((i * 3 + thread) % 16) * stride;
            if (table.probe(probe_key, entry, stats[thread]) &&
                (entry.key != probe_key || entry.depth != (probe_key >> 3) % 40 ||
                 entry.score != static_cast<int>(probe_key % 2000) - 1000 || entry.bound != TTBound::Exact ||
                 entry.rule50_clock != probe_key % 100 || entry.move != Move(Square::e2, Square::e4))) ++errors;
            if (i % 1000 == 0 && table.hashfull() > 1000) ++errors;
        }
    }
    TTStatistics total;
    for (const auto& worker : stats) total.add(worker);
    return !errors.load() && total.probes == 400000 && total.hits && total.hits <= total.probes;
}

static bool diagnostic_counters() {
#if defined(HG_TT_DIAGNOSTICS)
    TranspositionTable table(1);
    TTStatistics stats;
    table.store(123, Move{}, 900, 16, TTBound::Exact, 0, 0, &stats);
    table.store(123, Move{}, 0, 0, TTBound::Upper, 0, 99, &stats); // Shallower non-exact: attempted, not stored.
    if (stats.diagnostics.store_attempts != 2 || stats.diagnostics.stored != 1) return false;
    TTStatistics sum;
    sum.add(stats); sum.add(stats);
    if (sum.diagnostics.store_attempts != 4 || sum.diagnostics.stored != 2) return false;
    sum.reset();
    return !sum.diagnostics.store_attempts && !sum.diagnostics.stored;
#else
    return true; // Optional counters compile out of normal builds.
#endif
}

static const bool replacement_registered = register_test("TT: replacement policy, generation aging and move retention", replacement_policy);
static const bool layout_registered = register_test("TT: packed 64-bit data word and one-cache-line buckets", packed_layout);
static const bool coherence_registered = register_test("TT: lockless XOR snapshots stay coherent under 8-thread contention", concurrent_coherence);
static const bool diagnostic_registered = register_test("TT: optional per-worker store counters, aggregation and reset", diagnostic_counters);
} // namespace heavensgate::test
