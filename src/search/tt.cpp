#include "tt.hpp"
#include "../evaluation/eval.hpp"
#include <algorithm>

namespace heavensgate {

TranspositionTable::TranspositionTable(size_t size_mb) {
    resize(size_mb);
}

void TranspositionTable::resize(size_t size_mb) {
    size_t num_buckets = (size_mb * 1024 * 1024) / sizeof(TTBucket);
    // Round down to nearest power of 2 for O(1) single-cycle bitwise masking
    size_t pow2_buckets = 1;
    while (pow2_buckets * 2 <= num_buckets) {
        pow2_buckets *= 2;
    }
    num_buckets_ = pow2_buckets;
    mask_ = (pow2_buckets > 0) ? (pow2_buckets - 1) : 0;
    table_.assign(pow2_buckets, TTBucket{});
    clear();
}

void TranspositionTable::clear() {
    std::fill(table_.begin(), table_.end(), TTBucket{});
    hits_ = 0;
    probes_ = 0;
}

TTEntry* TranspositionTable::probe(uint64_t key) noexcept {
    if (num_buckets_ == 0) return nullptr;
    probes_++;

    size_t idx = static_cast<size_t>(key & mask_);
    TTBucket& bucket = table_[idx];

    for (int i = 0; i < 4; ++i) {
        TTEntry& entry = bucket.entries[i];
        if (entry.bound == TTBound::None) continue;
        uint64_t d = entry.data_word();
        if ((entry.key ^ d) == key) {
            hits_++;
            return &entry;
        }
    }
    return nullptr;
}

void TranspositionTable::prefetch(uint64_t key) const noexcept {
    if (num_buckets_ > 0) {
        size_t idx = static_cast<size_t>(key & mask_);
#if defined(_MSC_VER)
        _mm_prefetch(reinterpret_cast<const char*>(&table_[idx]), _MM_HINT_T0);
#elif defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(&table_[idx], 0, 3);
#endif
    }
}

int TranspositionTable::hashfull() const noexcept {
    if (num_buckets_ == 0) return 0;
    size_t sample_size = std::min<size_t>(1000, num_buckets_);
    size_t occupied = 0;
    for (size_t i = 0; i < sample_size; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (table_[i].entries[j].bound != TTBound::None && table_[i].entries[j].generation == generation_) {
                occupied++;
            }
        }
    }
    return static_cast<int>((occupied * 1000) / (sample_size * 4));
}

void TranspositionTable::store(uint64_t key, Move move, int score, int depth, TTBound bound, int ply) noexcept {
    if (num_buckets_ == 0) return;

    // Adjust mate scores relative to root ply
    if (score > ScoreMate - 1000) score += ply;
    else if (score < -ScoreMate + 1000) score -= ply;

    size_t idx = static_cast<size_t>(key & mask_);
    TTBucket& bucket = table_[idx];

    TTEntry* target = nullptr;
    int worst_score = 999999;

    for (int i = 0; i < 4; ++i) {
        TTEntry& e = bucket.entries[i];
        uint64_t old_data = e.data_word();
        uint64_t old_key = e.key ^ old_data;

        // 1. Exact key match -> replace
        if (e.bound != TTBound::None && old_key == key) {
            target = &e;
            break;
        }

        // 2. Empty slot -> take it immediately
        if (e.bound == TTBound::None) {
            target = &e;
            break;
        }

        // 3. Calculate replacement priority: prioritize older generations and shallowest depth
        int e_score = static_cast<int>(e.depth) - (generation_ == e.generation ? 0 : 32);
        if (e_score < worst_score) {
            worst_score = e_score;
            target = &e;
        }
    }

    if (!target) target = &bucket.entries[0];

    uint64_t target_data = target->data_word();
    uint64_t target_key = target->key ^ target_data;
    bool key_matches = (target_key == key);

    TTEntry new_entry;
    new_entry.move = static_cast<bool>(move) ? move : (key_matches ? target->move : Move());
    new_entry.score = static_cast<int16_t>(score);
    new_entry.depth = static_cast<uint8_t>(depth);
    new_entry.bound = bound;
    new_entry.generation = generation_;
    new_entry.pad = 0;
    new_entry.key = key ^ new_entry.data_word();

    *target = new_entry;
}

} // namespace heavensgate
