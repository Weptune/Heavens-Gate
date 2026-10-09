#pragma once
#include "../core/types.hpp"
namespace heavensgate {
// Candidate only: the caller retains depth/check/material/excluded-move guards.
// Explicit predecessor state distinguishes a null search from an empty root
// move/history slot. Evaluation is the corrected static eval, not a TT bound.
inline bool guarded_null_move_allowed(bool non_pv, int static_eval, int beta, bool previous_was_null) noexcept {
    return non_pv && !previous_was_null && beta > -ScoreMate + 1000 && beta < ScoreMate - 1000 && static_eval >= beta;
}
} // namespace heavensgate
