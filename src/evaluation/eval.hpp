#pragma once

#include "../core/types.hpp"
#include "eval_features.hpp"

namespace heavensgate {

class Board; // Forward declaration

constexpr int PawnValue   = 100;
constexpr int KnightValue = 320;
constexpr int BishopValue = 330;
constexpr int RookValue   = 500;
constexpr int QueenValue  = 900;

enum class EvalMode {
    MaterialOnly,
    MasterPositional,
    SpectralTropical
};

class Evaluator {
public:
    static void init();
    static int evaluate(const Board& board);
    static int evaluate_fast(const Board& board, int alpha = -ScoreInfinity, int beta = ScoreInfinity);
    static int evaluate_incremental(const Board& board, int ply, Square from_sq = Square::None, Square to_sq = Square::None);
    static void reset_incremental_cache();
    // Uncached sum of every positional term for one side (no material/PST/tempo).
    // The single definition shared by the search eval and the tuners.
    static ScorePair positional_terms(const Board& board, Color side);
    // Endgame scale (0..64) for the white-relative EG difference.
    static int eg_scale(const Board& board, int eg_white);

    static void set_mode(EvalMode mode) { current_mode_ = mode; }
    static EvalMode mode() { return current_mode_; }

private:
    static int evaluate_side(const Board& board, Color side);
    static ScorePair side_totals(const Board& board, Color side);

    static thread_local EvalMode current_mode_;
};

} // namespace heavensgate
