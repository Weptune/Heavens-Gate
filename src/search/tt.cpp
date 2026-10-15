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
    table_ = count ? std::make_unique<TTBucket[]>(count) : nullptr;
    num_buckets_ = count;
    mask_ = count ? count - 1 : 0;
    clear();
}

void TranspositionTable::clear() {
    for (size_t i = 0; i < num_buckets_; ++i)
        for (auto& slot : table_[i].slots) slot.write(0, 0);
    hits_.store(0, std::memory_order_relaxed);
    probes_.store(0, std::memory_order_relaxed);
}

TTEntry* TranspositionTable::probe(uint64_t key) noexcept {
    thread_local TTEntry snapshot;
    return probe(key, snapshot) ? &snapshot : nullptr;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out) const noexcept {
    if (!num_buckets_) return false;
    for (const auto& slot : table_[key & mask_].slots) {
        uint64_t data = 0;
        if (slot.load_key(data) != key) continue;
        const TTEntry entry = TTEntry::decode(key, data);
        if (entry.bound == TTBound::None) continue;
        out = entry;
        return true;
    }
    return false;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out, TTStatistics& statistics) const noexcept {
    if (!num_buckets_) return false;
    ++statistics.probes;
    const bool found = probe(key, out);
    statistics.hits += found;
    return found;
}

void TranspositionTable::prefetch(uint64_t key) const noexcept {
    if (!num_buckets_) return;
    const auto* bucket = &table_[key & mask_];
#if defined(_MSC_VER)
    _mm_prefetch(reinterpret_cast<const char*>(bucket), _MM_HINT_T0);
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(bucket, 0, 3);
#endif
}

int TranspositionTable::hashfull() const noexcept {
    const size_t count = std::min<size_t>(1000, num_buckets_);
    if (!count) return 0;
    size_t occupied = 0;
    const auto gen = generation();
    for (size_t i = 0; i < count; ++i) {
        for (const auto& slot : table_[i].slots) {
            uint64_t data = 0;
            const uint64_t key = slot.load_key(data);
            const TTEntry entry = TTEntry::decode(key, data);
            occupied += entry.bound != TTBound::None && entry.generation == gen;
        }
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
    auto& bucket = table_[key & mask_];
    const auto gen = generation();

    // Replacement policy unchanged from the locked table: first empty or same-key
    // slot, else the lowest depth with a 32-ply penalty for older generations.
    TTSlot* target = &bucket.slots[0];
    TTEntry current{};
    bool have_current = false;
    int worst = 999999;
    for (auto& slot : bucket.slots) {
        uint64_t data = 0;
        const uint64_t slot_key = slot.load_key(data);
        const TTEntry entry = TTEntry::decode(slot_key, data);
        if (entry.bound == TTBound::None || slot_key == key) {
            target = &slot; current = entry; have_current = true;
            break;
        }
        const int priority = entry.depth - (entry.generation == gen ? 0 : 32);
        if (priority < worst) { worst = priority; target = &slot; current = entry; have_current = true; }
    }

    const bool same_position = have_current && current.bound != TTBound::None && current.key == key;
    const bool can_replace = !same_position || bound == TTBound::Exact || depth >= current.depth || gen != current.generation;
    if (can_replace) {
#if defined(HG_TT_DIAGNOSTICS)
        if (statistics) ++statistics->diagnostics.stored;
#endif
        if (!move && same_position) move = current.move;
        TTEntry entry;
        entry.key = key;
        entry.move = move;
        entry.score = static_cast<int16_t>(std::clamp(score, -32768, 32767));
        entry.depth = static_cast<uint8_t>(std::clamp(depth, 0, 255));
        entry.bound = bound;
        entry.generation = gen;
        entry.rule50_clock = static_cast<uint8_t>(std::clamp(rule50_clock, 0, 255));
        target->write(key, entry.data_word());
    } else if (!current.move && move) {
        current.move = move;
        target->write(key, current.data_word());
    }
}
} // namespace heavensgate
