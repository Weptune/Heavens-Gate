#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"
#include <string>
#include <array>
#include <cstdint>
#include <atomic>

namespace heavensgate {

enum class WDLScore {
    Loss = -2,
    BlessedLoss = -1,
    Draw = 0,
    CursedWin = 1,
    Win = 2,
    Unknown = 999
};

struct TBCacheEntry {
    uint64_t key{0};
    WDLScore wdl{WDLScore::Unknown};
    int halfmove_clock{-1};
};

class SyzygyTablebase {
public:
    static constexpr int NO_SCORE = -999999;
    static constexpr int ScoreTBWin = 19000;
    static constexpr int MAX_TB_PIECES = 6; // Probe for 6 or fewer pieces

    static SyzygyTablebase& instance() {
        static SyzygyTablebase inst;
        return inst;
    }

    void init(const std::string& tb_path = "syzygy");
    bool is_enabled() const { return enabled_.load(std::memory_order_relaxed); }
    void set_enabled(bool enable) { enabled_.store(enable, std::memory_order_relaxed); }

    // Exact, rule-aware recognition only. This is NOT a file-backed Syzygy probe.
    // Unproven KPK/KBNK/KBBK/KNNK/fortress/Lucena patterns return NO_SCORE.
    int probe_wdl(const Board& board, int ply);

    // Converts WDL enum to search score bounds
    int wdl_to_score(WDLScore wdl, int ply) const;

private:
    SyzygyTablebase() = default;
    std::atomic<bool> enabled_{true};
    std::atomic<uint64_t> cache_epoch_{0};
    static constexpr size_t CACHE_SIZE = 16384;
    static thread_local std::array<TBCacheEntry, CACHE_SIZE> cache_;
    static thread_local uint64_t local_epoch_;

    // Internal 3-4-5-6 piece WDL evaluator
    WDLScore evaluate_endgame_wdl(const Board& board);

    // Specific endgame solvers for 3, 4, 5, 6 piece positions
    WDLScore solve_krk(const Board& board, Color strong_side);
    WDLScore solve_kqk(const Board& board, Color strong_side);
};

} // namespace heavensgate
