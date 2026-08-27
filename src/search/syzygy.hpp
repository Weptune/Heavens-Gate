#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"
#include <string>
#include <array>
#include <cstdint>

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
    uint64_t key;
    WDLScore wdl;
};

class SyzygyTablebase {
public:
    static constexpr int NO_SCORE = -999999;
    static constexpr int MAX_TB_PIECES = 6; // Probe for 6 or fewer pieces

    static SyzygyTablebase& instance() {
        static SyzygyTablebase inst;
        return inst;
    }

    void init(const std::string& tb_path = "syzygy");
    bool is_enabled() const { return enabled_; }
    void set_enabled(bool enable) { enabled_ = enable; }

    // Probes WDL score for a board position. Returns exact centipawn score or NO_SCORE
    int probe_wdl(const Board& board, int ply);

    // Converts WDL enum to search score bounds
    int wdl_to_score(WDLScore wdl, int ply) const;

private:
    SyzygyTablebase() : enabled_(false), max_pieces_(6) {}

    bool enabled_;
    int max_pieces_;
    static constexpr size_t CACHE_SIZE = 16384;
    std::array<TBCacheEntry, CACHE_SIZE> cache_{};

    // Internal 3-4-5-6 piece WDL evaluator
    WDLScore evaluate_endgame_wdl(const Board& board);

    // Specific endgame solvers for 3, 4, 5, 6 piece positions
    WDLScore solve_kpk(const Board& board, Color strong_side);
    WDLScore solve_krk(const Board& board, Color strong_side);
    WDLScore solve_kqk(const Board& board, Color strong_side);
    WDLScore solve_kbnk(const Board& board, Color strong_side);
    WDLScore solve_kbbk(const Board& board, Color strong_side);
    WDLScore solve_knnk(const Board& board, Color strong_side);
    WDLScore solve_krp_kr(const Board& board, Color strong_side);
    WDLScore solve_wrong_color_bishop_pawn(const Board& board, Color strong_side);
    WDLScore solve_opposite_colored_bishops_1p(const Board& board, Color strong_side);
};

} // namespace heavensgate
