#include "test.hpp"
#include "../src/search/tt.hpp"
#include <atomic>

namespace heavensgate::test {
static bool coexist_and_replacement() {
    TranspositionTable table(1);
    if (table.clock_contexts()) return false; // Never enable the candidate by default.
    const size_t capacity = table.capacity();
    table.set_clock_contexts(true);
    const uint64_t key = 123, stride = capacity / 4;
    const Move hint(Square::d2, Square::e2);
    table.store(key, hint, 900, 16, TTBound::Exact, 0, 0);
    table.store(key, Move{}, 0, 1, TTBound::Upper, 0, 99);
    TTStatistics stats;
    TTEntry entry;
    if (!table.probe(key, entry, stats, 0) || !entry.score_matches_rule50(0) || entry.depth != 16 || entry.score != 900) return false;
    if (!table.probe(key, entry, stats, 99) || !entry.score_matches_rule50(99) || entry.depth != 1 || entry.score != 0 || entry.move != hint) return false;
    // A third, absent context can get a hint, but never a compatible score.
    if (!table.probe(key, entry, stats, 80) || entry.score_matches_rule50(80) || entry.depth != 16 || entry.move != hint) return false;
    // Preserve the existing within-context depth replacement policy.
    table.store(key, Move{}, 700, 2, TTBound::Upper, 0, 0);
    if (!table.probe(key, entry, stats, 0) || entry.depth != 16 || entry.score != 900) return false;
    table.store(key + stride, hint, 200, 12, TTBound::Exact, 0, 0);
    table.store(key + 2 * stride, hint, 300, 8, TTBound::Exact, 0, 0);
    // Bucket pressure must select the normal weakest depth/generation slot,
    // not the first same-board/deep-context slot.
    table.store(key, Move{}, 10, 0, TTBound::Upper, 0, 1);
    if (!table.probe(key, entry, stats, 0) || entry.depth != 16 || entry.score != 900) return false;
    if (!table.probe(key, entry, stats, 1) || !entry.score_matches_rule50(1) || entry.depth != 0 || entry.score != 10) return false;
    if (!table.probe(key, entry, stats, 99) || entry.score_matches_rule50(99)) return false;
    // Generation aging still applies to victim choice; capacity does not grow.
    table.new_search();
    table.store(key + 3 * stride, hint, 400, 6, TTBound::Exact, 0, 0);
    if (!table.probe(key, entry, stats, 0) || entry.depth != 16) return false;
    if (table.capacity() != capacity || sizeof(TTEntry) != 16 || sizeof(TTBucket) != 64) return false;
    table.set_clock_contexts(false);
    return !table.probe(key, entry) && !table.clock_contexts() && table.capacity() == capacity;
}

static bool matching_bounds() {
    TranspositionTable table(1);
    table.set_clock_contexts(true);
    for (TTBound bound : {TTBound::Exact, TTBound::Lower, TTBound::Upper}) {
        table.clear();
        for (int clock : {0, 1, 98, 99})
            table.store(123, Move(Square::e2, Square::e4), 100 + clock, clock % 8, bound, 3, clock);
        TTStatistics stats;
        for (int clock : {0, 1, 98, 99}) {
            TTEntry entry;
            if (!table.probe(123, entry, stats, clock) || !entry.score_matches_rule50(clock) ||
                entry.score != 100 + clock || entry.bound != bound || entry.depth != clock % 8) return false;
        }
        TTEntry entry;
        if (!table.probe(123, entry, stats, 100) || entry.score_matches_rule50(100)) return false;
    }
    return true;
}

static bool concurrent_context_snapshots() {
    TranspositionTable table(1);
    table.set_clock_contexts(true);
    std::array<TTStatistics, 4> stats{};
    std::atomic<int> errors{0};
    const auto stride = table.capacity() / 4;
#if defined(_OPENMP)
    #pragma omp parallel for num_threads(4)
#endif
    for (int thread = 0; thread < 4; ++thread) {
        for (int i = 0; i < 20000; ++i) {
            const uint64_t key = 123 + static_cast<uint64_t>((i + thread) % 8) * stride;
            const int clock = (i + thread) % 100;
            table.store(key, Move(Square::e2, Square::e4), clock * 10 + key % 10,
                        clock % 32, TTBound::Exact, 0, clock, &stats[thread]);
            TTEntry entry;
            if (table.probe(key, entry, stats[thread], clock) &&
                (entry.key != key || entry.rule50_clock >= 100 || entry.score != entry.rule50_clock * 10 + key % 10 ||
                 entry.depth != entry.rule50_clock % 32 || entry.bound != TTBound::Exact)) ++errors;
        }
    }
    TTStatistics total;
    for (const auto& worker : stats) total.add(worker);
    return !errors.load() && total.probes == 80000 && total.hits && total.hits <= total.probes;
}

static bool diagnostic_counter_contract() {
#if defined(HG_TT_DIAGNOSTICS)
    TranspositionTable table(1);
    TTStatistics stats;
    table.store(123, Move{}, 900, 16, TTBound::Exact, 0, 0, &stats);
    table.store(123, Move{}, 0, 0, TTBound::Upper, 0, 99, &stats);
    if (stats.diagnostics.store_attempts != 2 || stats.diagnostics.stored != 2 ||
        stats.diagnostics.context_replacements != 1 || stats.diagnostics.shallower_context_replacements != 1 ||
        stats.diagnostics.qsearch_context_replacements_of_deeper != 1) return false;
    TTStatistics sum;
    sum.add(stats); sum.add(stats);
    if (sum.diagnostics.context_replacements != 2) return false;
    sum.reset();
    if (sum.diagnostics.store_attempts || sum.diagnostics.context_replacements) return false;
    stats.reset();
    table.set_clock_contexts(true);
    table.store(123, Move{}, 900, 16, TTBound::Exact, 0, 0, &stats);
    table.store(123, Move{}, 0, 0, TTBound::Upper, 0, 99, &stats);
    return stats.diagnostics.stored == 2 && !stats.diagnostics.context_replacements;
#else
    return true; // Optional counters compile out of normal builds.
#endif
}

static const bool coexist_registered = register_test("TTContexts: distinct clocks coexist, depth victim and mode-reset contract", coexist_and_replacement);
static const bool bounds_registered = register_test("TTContexts: exact-clock Exact/Lower/Upper selection", matching_bounds);
static const bool snapshots_registered = register_test("TTContexts: concurrent full-key/clock/score snapshot coherence", concurrent_context_snapshots);
static const bool diagnostic_registered = register_test("TTContexts: optional per-worker replacement counters, aggregation and reset", diagnostic_counter_contract);
} // namespace heavensgate::test
