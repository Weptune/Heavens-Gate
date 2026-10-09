#include "tt.hpp"
#include "../evaluation/eval.hpp"
#include <algorithm>
#if defined(_MSC_VER)
#include <xmmintrin.h>
#endif

namespace heavensgate {
TranspositionTable::TranspositionTable(size_t size_mb) { resize(size_mb); }

void TranspositionTable::resize(size_t size_mb) {
    const size_t requested = size_mb * 1024 * 1024 / sizeof(TTBucket);
    size_t count = requested ? 1 : 0;
    while (count && count <= requested / 2) count *= 2;
    table_.assign(count, TTBucket{});
    locks_ = count ? std::make_unique<std::atomic_flag[]>(count) : nullptr;
    num_buckets_ = count;
    mask_ = count ? count - 1 : 0;
    clear();
}

void TranspositionTable::clear() {
    std::fill(table_.begin(), table_.end(), TTBucket{});
    for (size_t i = 0; i < num_buckets_; ++i) locks_[i].clear(std::memory_order_relaxed);
    hits_.store(0, std::memory_order_relaxed);
    probes_.store(0, std::memory_order_relaxed);
}

// Compatibility API returns a snapshot, never a pointer into shared storage.
TTEntry* TranspositionTable::probe(uint64_t key) noexcept {
    thread_local TTEntry snapshot;
    return probe(key, snapshot) ? &snapshot : nullptr;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out) noexcept {
    return probe_context(key, out, -1);
}

bool TranspositionTable::probe_context(uint64_t key, TTEntry& out, int rule50_clock) noexcept {
    if (!num_buckets_) return false;
    const size_t idx = key & mask_;
    if (locks_[idx].test_and_set(std::memory_order_acquire)) return false;
    const TTEntry* selected = nullptr;
    for (const auto& entry : table_[idx].entries) {
        if (entry.bound != TTBound::None && entry.key == key) {
            if (!clock_contexts_ || rule50_clock < 0) {
                selected = &entry;
                break;
            }
            if (entry.score_matches_rule50(rule50_clock)) {
                selected = &entry;
                break;
            }
            // No compatible score: retain the best positional move hint.
            if (!selected || entry.depth > selected->depth ||
                (entry.depth == selected->depth && entry.generation == generation() && selected->generation != generation()))
                selected = &entry;
        }
    }
    if (selected) out = *selected;
    locks_[idx].clear(std::memory_order_release);
    return selected != nullptr;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out, TTStatistics& statistics) noexcept {
    if (!num_buckets_) return false;
    ++statistics.probes;
    const bool found = probe(key, out);
    statistics.hits += found;
    return found;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out, TTStatistics& statistics, int rule50_clock) noexcept {
    if (!num_buckets_) return false;
    ++statistics.probes;
    const bool found = probe_context(key, out, rule50_clock);
    statistics.hits += found;
    return found;
}

void TranspositionTable::prefetch(uint64_t key) const noexcept {
    if (!num_buckets_) return;
    const size_t idx = key & mask_;
#if defined(_MSC_VER)
    _mm_prefetch(reinterpret_cast<const char*>(&table_[idx]), _MM_HINT_T0);
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(&table_[idx], 0, 3);
#endif
}

int TranspositionTable::hashfull() const noexcept {
    const size_t count = std::min<size_t>(1000, num_buckets_);
    if (!count) return 0;
    size_t occupied = 0;
    const auto gen = generation();
    for (size_t i = 0; i < count; ++i) {
        if (locks_[i].test_and_set(std::memory_order_acquire)) continue;
        for (const auto& entry : table_[i].entries)
            occupied += entry.bound != TTBound::None && entry.generation == gen;
        locks_[i].clear(std::memory_order_release);
    }
    return static_cast<int>(occupied * 1000 / (count * 4));
}

void TranspositionTable::store(uint64_t key, Move move, int score, int depth, TTBound bound, int ply, int rule50_clock,
                              [[maybe_unused]] TTStatistics* statistics) noexcept {
    if (!num_buckets_) return;
#if defined(HG_TT_DIAGNOSTICS)
    if (statistics) ++statistics->diagnostics.store_attempts;
#endif
    if (score > ScoreMate - 1000) score += ply;
    else if (score < -ScoreMate + 1000) score -= ply;
    const size_t idx = key & mask_;
    if (locks_[idx].test_and_set(std::memory_order_acquire)) return;
    const auto gen = generation();
    auto* target = &table_[idx].entries[0];
    const auto clock = static_cast<uint8_t>(std::clamp(rule50_clock, 0, 255));
    Move context_hint;
    int worst = 999999;
    if (!clock_contexts_) {
        // Preserve the frozen single-context policy exactly when disabled.
        for (auto& entry : table_[idx].entries) {
            if (entry.bound == TTBound::None || entry.key == key) { target = &entry; break; }
            const int priority = entry.depth - (entry.generation == gen ? 0 : 32);
            if (priority < worst) { worst = priority; target = &entry; }
        }
    } else {
        TTEntry* matching = nullptr;
        TTEntry* empty = nullptr;
        int hint_depth = -1;
        for (auto& entry : table_[idx].entries) {
            if (entry.bound == TTBound::None) {
                if (!empty) empty = &entry;
                continue;
            }
            if (entry.key == key) {
                if (entry.rule50_clock == clock) matching = &entry;
                if (entry.move && entry.depth > hint_depth) {
                    context_hint = entry.move;
                    hint_depth = entry.depth;
                }
            }
            const int priority = entry.depth - (entry.generation == gen ? 0 : 32);
            if (priority < worst) { worst = priority; target = &entry; }
        }
        // Distinct clocks may occupy distinct slots, but total capacity and
        // depth/generation replacement priority do not change.
        if (matching) target = matching;
        else if (empty) target = empty;
    }
    const bool same_position = target->bound != TTBound::None && target->key == key;
    const bool same = same_position && target->score_matches_rule50(clock);
    const bool can_replace = !same_position || bound == TTBound::Exact || depth >= target->depth || gen != target->generation;
    if (can_replace) {
#if defined(HG_TT_DIAGNOSTICS)
        if (statistics) {
            auto& diagnostic = statistics->diagnostics;
            ++diagnostic.stored;
            if (same_position && !same) {
                ++diagnostic.context_replacements;
                diagnostic.shallower_context_replacements += depth < target->depth;
                diagnostic.qsearch_context_replacements_of_deeper += depth == 0 && target->depth > 0;
            }
        }
#endif
        if (!move && same_position) move = target->move;
        if (!move && clock_contexts_) move = context_hint;
        *target = TTEntry{key, move, static_cast<int16_t>(std::clamp(score, -32768, 32767)),
                          static_cast<uint8_t>(std::clamp(depth, 0, 255)), bound, gen, clock};
    } else if (!target->move && move) {
        target->move = move;
    }
    locks_[idx].clear(std::memory_order_release);
}
} // namespace heavensgate
