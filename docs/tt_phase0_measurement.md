# TT clock rejection on `main`: Phase 0 measurement

2026-10-15. Source: `main` at `b2e7c0c`, unmodified except for variant B below.
Evidence bundle: `build/tt-phase0-20261015/` (harness, scripts, suites, raw JSONL, patch, SHA256SUMS).

## Question

`search.cpp` rejects TT scores whenever `score_matches_rule50(clock)` fails, and that check
is **not** gated on `TTClockContexts`. So production (option off) still discards score reuse
when `max(probe_clock, stored_clock) >= 80` and the clocks differ. Does that cost nodes?

## Decision

**No measurable cost. Keep the ungated rejection.** It is cheap soundness, not a leak.
The 32.5% paired-screen regression came from the store side of `TTClockContexts=true`
(several clock variants of one key per cluster), which is already off by default.

## Method

- Two builds, both `-DHG_TT_DIAGNOSTICS=1`, `-O3 -march=native`, GCC 13.3, Linux:
  - **A**: `main` exactly.
  - **B**: `main` with the two `if (!compatible) tt_entry = nullptr;` lines compiled out
    (`HG_PHASE0_NO_CLOCK_REJECT`). Counters still report what A would have rejected.
- `main.cpp`/`run_manifest.hpp` are Windows-only, so a small harness links the engine sources
  directly: 1 thread, 64 MB TT, book off, `MasterPositional` (the UCI mode), Zobrist/MoveGen/
  Evaluator/Syzygy init as in `main.cpp`. Zero warnings under `-Wall -Wextra`.
- **Persistent-TT game replay**, because a fresh TT per search shows *zero* rejections even at
  clock 98: within one tree, transpositions nearly always arrive with equal clocks, and nodes at
  clock 100 return before probing. The real mechanism is cross-move: entries stored on move N at
  clock c, probed on move N+1 at c+2. Each line clears the TT once, then searches every ply at
  depth 13 and plays on.
- Suites, seed 20261015 from `data/quiet_positions.txt`:
  - **main**: 20 middlegame (20-28 men) + 20 endgame (7-12 men), start clock 70, up to 40 plies.
  - **boundary**: 40 positions, 7-16 men, at least 4 pawns, at least 75% of pawns blocked;
    start clock 80, up to 20 plies.
- **Crossed design.** Replaying one engine's line with the other engine is biased: the line's
  author always has a warm TT for the played move. Measured bias: x1.083 (boundary), x1.053
  (main). So each suite runs both ways (A's lines with B replaying, B's lines with A replaying)
  and the per-line log node ratios are averaged, which cancels the bias. 95% CIs are
  bootstrap over the 40 lines (20,000 resamples).

## Results

Determinism: A replaying its own line reproduced **1345/1345** searches bit-identically
(nodes, score, PV).

Rejected hits (A, A-authored lines):

| Suite | Root clock band | Searches | TT hits | Rejected | Share of hits |
|---|---|---|---|---|---|
| main | 0-79 | 1343 | 124,737,580 | 76,728 | 0.06% |
| boundary | 80-89 | 159 | 14,315,126 | 750,746 | 5.24% |
| boundary | 90-99 | 16 | 325,947 | 50,038 | 15.35% |

All rejections land at in-tree clocks 70-99 (deciles 7-9), as the predicate requires.

Node cost of removing the rejection (B/A total nodes, bias-cancelled):

| Suite | B/A | 95% CI |
|---|---|---|
| main | 0.996 | 0.970 to 1.021 |
| boundary | 1.010 | 0.953 to 1.073 |

Both intervals contain 1. Even with 5-15% of hits rejected in the boundary band, reusing them
does not make search cheaper at this resolution.

What B changes is **scores**: at root clock >= 80 on A-authored boundary lines, 51 searches
moved by 50 cp or more, including a mate A found and B lost (`seq` 6, clock 80: A mate score
24983, B 1437). B is reusing scores whose subtrees saw a different 50-move horizon. That is the
unsoundness the rejection exists to prevent, and A pays nothing measurable for preventing it.

## Corrections to the Priority 1 plan

1. **"Decouple the three uses of a hit" is mostly moot.** `tt_move` is assigned before the
   rejection, so the move hint already survives. What the rejection drops is the TT-bound eval
   refinement (search.cpp ~461) and the singular-extension trigger (~566). Both are
   score-dependent, so gating them is correct.
2. **"Identical PV and nodes <= control on every position" is not a valid gate.** Removing
   0.06% of rejections changed the score or move in 567 of 1345 searches (median node ratio
   exactly 1.000, totals swinging several percent either way). Any change to what the TT returns
   reshapes every later tree. The usable pre-game screen is the crossed, persistent-TT paired
   replay above, judged on a confidence interval.
3. **The validity-window redesign (Phase 2) is downgraded.** Its upside is bounded by what B
   recovers, and B recovers nothing measurable. Revisit only if a later change makes the
   boundary band matter (e.g. a 50-move-aware eval scale).

## Revised Priority 1

- **Do:** delete the `TTClockContexts` store path, option and dual layout (Phase 3/4). It is a
  default-off switch carrying a known regression. Keep the probe-side rejection, gated on nothing.
- **Do:** lockless TT entries (key/data XOR check) to drop the separate `locks_` array and the
  second cache line per probe. Judge by NPS, then the crossed replay, then paired games.
- **Note:** `CMakeLists.txt` uses `-Wall -Wextra` without `-Werror`, contrary to the project brief.
