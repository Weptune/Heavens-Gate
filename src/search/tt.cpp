#include "tt.hpp"
#include "../evaluation/eval.hpp"
#include <algorithm>

namespace heavensgate {

TranspositionTable::TranspositionTable(size_t size_mb) {
    resize(size_mb);
}

void TranspositionTable::resize(size_t size_mb) {
    size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
    size_ = num_entries;
    table_.assign(num_entries, TTEntry{});
    clear();
}

void TranspositionTable::clear() {
    std::fill(table_.begin(), table_.end(), TTEntry{});
    hits_ = 0;
    probes_ = 0;
}

TTEntry* TranspositionTable::probe(uint64_t key) noexcept {
    if (size_ == 0) return nullptr;
    probes_++;

    size_t idx = static_cast<size_t>(key % size_);
    TTEntry& entry = table_[idx];

    if (entry.key == key && entry.bound != TTBound::None) {
        hits_++;
        return &entry;
    }
    return nullptr;
}

void TranspositionTable::prefetch(uint64_t key) const noexcept {
    if (size_ > 0) {
        size_t idx = static_cast<size_t>(key % size_);
        __builtin_prefetch(&table_[idx], 0, 3);
    }
}

void TranspositionTable::store(uint64_t key, Move move, int score, int depth, TTBound bound, int ply) noexcept {
    if (size_ == 0) return;

    // Adjust mate scores relative to root ply
    if (score > ScoreMate - 1000) score += ply;
    else if (score < -ScoreMate + 1000) score -= ply;

    size_t idx = static_cast<size_t>(key % size_);
    TTEntry& entry = table_[idx];

    // Replacement Policy: Replace if slot is empty, key matches, entry is from older search generation, or new depth >= old depth
    if (entry.bound == TTBound::None || entry.key == key || entry.generation != generation_ || depth >= entry.depth) {
        TTEntry new_entry;
        new_entry.key = key;
        new_entry.move = static_cast<bool>(move) ? move : (entry.key == key ? entry.move : Move());
        new_entry.score = static_cast<int16_t>(score);
        new_entry.depth = static_cast<uint8_t>(depth);
        new_entry.bound = bound;
        new_entry.generation = generation_;
        new_entry.pad = 0;

        entry = new_entry; // 16-byte aligned struct assignment
    }
}

} // namespace heavensgate
