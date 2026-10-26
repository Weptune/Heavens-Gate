# Heaven's Gate: full engine audit

2026-10-15. Source: `main` at `b2e7c0c` (445 commits, 2026-08-03 to 2026-10-10), read in full for
search, move ordering, TT, time management, SMP and evaluation; built and tested on Linux
(GCC 13.3, `-O3 -march=native`); strength measured against engines with published CCRL Blitz ratings.
Evidence bundle: `build/audit-20261015/`.

## 1. Bottom line

- **Measured strength: about 2361 CCRL Blitz-equivalent** (95% range 2322-2399), from
  400 games against four Blunder releases with published CCRL Blitz ratings (2155 to 2631).
  That is roughly 640 Elo below 3000, and about 440 below the "~2800" earlier documents claimed.
- **Is 3000 viable? Yes for a handcrafted engine, but not soon, and not through the spectral-tropical work.**
  Handcrafted engines reach far past 3000: Stash 37.0 (no neural net) is 3417 on CCRL Blitz and
  Ethereal 12.x (handcrafted) about 3390-3420. Stash's own README went from "rated around 2160"
  (May 2020) to "around 2880" (Nov 2020), so +700 is possible in months, but only with a
  test-every-change pipeline running thousands of games per patch. Without that infrastructure the
  history of this repo shows what happens: many features, unmeasured, net strength ~2360.
- **The engine that plays is a conventional alpha-beta engine with a modest classical eval.** The
  Spectral-Tropical evaluator is not reachable from the playing search in any mode, and its weights
  file is not in the repository. Every Elo point Heaven's Gate has today comes from the classical
  code path.

## 2. How strength was measured

fastchess, Heaven's Gate (Linux build of `b2e7c0c`, `Threads=1 Hash=64 OwnBook=false`) vs
Blunder built from tagged source, 10+0.1, 1 thread each, 2 concurrent games on 2 cores,
`noob_3moves.epd` (balanced; official-stockfish/books @65815cc), random order seed 20261015,
every opening played with both colours. Zero time losses, zero crashes; every decisive game
ended in mate on the board.

| Anchor | CCRL Blitz | HG W-D-L | Score | Implied HG |
|---|---|---|---|---|
| Blunder 6.1.0 | 2155 | 63-9-28 | 67.5% | 2282 |
| Blunder 7.2.0 | 2425 | 31-12-57 | 37.0% | 2333 |
| Blunder 7.4.0 | 2532 | 23-16-61 | 31.0% | 2393 |
| Blunder 7.6.0 | 2631 | 19-15-66 | 26.5% | 2454 |
| **Pooled max likelihood** | | | | **2361** (2322-2399) |

Caveats: the implied rating rises with anchor strength (2282 to 2454), meaning these Blunder
versions are closer together at 10+0.1 than on CCRL; the pooled figure sits in the middle and the
true CCRL number could plausibly be anywhere from about 2280 to 2450. Anchor ratings are Blunder's README figures for CCRL Blitz (2'+1"), our time control is
much faster, the range treats games as independent, and one engine family as the pool. It is a
calibrated estimate, not a CCRL rating. It is, however, the first number in the repo that is
anchored to an external rating list rather than to a handicapped Stockfish.

How the games are lost (pgn comment analysis, all 400 games):

- Heaven's Gate searches **as deep as its opponents** (median depth 11-12 vs 7-12) and uses slightly
  more time per move (0.24-0.25 s vs 0.19-0.21 s). Speed is not the bottleneck.
- In **91 of 212 losses** HG's own eval was level or better (>= -1.00) and then fell by 2+ pawns
  within its next two moves: it walked into something it did not see.
- In **71 of 212 losses** the opponent already reported HG at -2.00 or worse while HG still
  reported >= 0.00: evaluation blind spots.
- fastchess flagged **22 corrupt PVs from HG** (illegal moves, or PV continuing past a threefold
  or 50-move draw). The move played is unaffected; analysis output is not trustworthy.

## 3. What is done well

- **Move generation is correct and fast.** Perft passes the full CPW suite bit-exact on Linux
  (startpos depth 6 = 119,060,324; Kiwipete-class positions to depth 5), 57-110 M leaves/s.
- **Clean engineering core.** C++20, no heap traffic on the search path, thread-local workers,
  bitboards with magic sliders, incremental material/PST, a pawn hash (32K, per thread).
  Single-thread search is fully deterministic (1345/1345 replays identical in Phase 0).
- **A broad modern search feature set already exists:** PVS, aspiration windows, IIR, null move with
  verification, reverse futility, futility, LMP, history-scaled LMR, SEE pruning, singular extension
  with multicut and negative extension, ProbCut, killers, countermoves, main/capture history,
  continuation history at plies 1, 2, 4 and 6, and three correction-history tables.
- **Rule correctness.** 50-move, stalemate-in-qsearch, mate-distance scoring and repetition are
  covered by contract tests that pass.
- **Measurement culture (since October).** Paired statistics, run manifests, frozen controls, and an
  audit that retracted inflated claims. This is the rarest and most valuable asset in the repo.
  The remaining gap is scale, not attitude.

## 4. Defects found (ranked by likely strength impact)

Each needs its own SPRT; the ranking is engineering judgement, not measured Elo.

1. **Pruning at PV nodes.** Reverse futility (`search.cpp:476`), null move (`:491`, guards default
   off) and ProbCut (`:535`) run at PV nodes. Standard engines restrict these to non-PV nodes; pruning
   on the PV degrades exactly the lines the engine plays. Consistent with the "sudden collapse" losses.
2. **King danger is an order of magnitude too large and lazy eval cannot see it.**
   `KingDangerTable[i] = 3*i*i` (`eval_features.cpp:72`) with attacker weights Q8/R5/B3/N3 and no
   minimum-attacker gate: one queen near the king costs 192 cp, queen+rook 507 cp, the cap is 2,883 cp.
   Meanwhile lazy evaluation (`eval.cpp:129`) returns material+PST when it is 240 cp outside the
   window, so the same position scores differently depending on alpha/beta. Static evals that
   depend on the window then feed RFP, futility, NMP, ProbCut and correction history.
3. **Lazy SMP helpers are nearly useless.** Helpers search every root move with a full
   `(-inf, +inf)` window and never raise alpha (`search.cpp:1344`), so they do far more work per depth
   than the main thread. Measured: 2 threads gave 1.93x NPS and no extra depth (13.1 vs 13.2).
   The default `Threads` is **6** (`uci.cpp:121`), so every multi-thread game and the earlier
   six-thread pilots paid for this.
4. **Correction history updates ignore bound direction.** Updated after fail-highs and fail-lows
   alike (`:872`, `:893`), including when a lower bound says nothing about overestimation; plain
   additive with clamping rather than a weighted average; and `raw_static_eval != 0` is used as a
   "has eval" sentinel, so every exactly-zero eval is skipped.
5. **Evaluation is thin and mostly untuned.** All six PSTs are hand-written (not PeSTO, despite the
   brief) and have never been tuned. The Texel tuner fits 63 scalars on 40,092 distinct positions
   with K fixed at 400. One tuned parameter pair (`pawn_attack_threat_*`) is not used anywhere in the
   eval, so the tuner fits a dead term. Missing: pawn attacks on pieces, hanging/loose pieces, safe
   checks, king-ring attack counts, passed-pawn blockers/free path/rank-scaled king distance,
   non-linear mobility, space, and any endgame scaling (opposite bishops, KNN, wrong-bishop rook
   pawn, pawnless drawishness). Syzygy reads no files.
6. **Expensive or risky search heuristics.** Multi-cut at depth >= 8 searches up to 8 moves at
   depth-4 on every non-PV node; double singular extensions have no per-path cap; fail-hard returns
   mixed with fail-soft TT scores; static eval is not cached in the TT; `eval_stack_` is stale when in
   check (improving flag reads a sibling's eval); quiet-history malus is capped at 72 vs bonus 425.
7. **PV table corruption.** Early returns (repetition, 50-move) skip `init_ply`, so a parent copies a
   stale child PV. Source of the 22 illegal-PV warnings.
8. **Test and build claims do not hold at HEAD.**
   - 3 of 26 portable tests fail deterministically (`Rule50: compatible bounds...`,
     `TTContexts: distinct clocks coexist...`, `TTContexts: exact-clock ... selection`): the
     `clock < 80` relaxation in `tt.hpp` was added in the same commit (`4ee9298`) as tests expecting
     exact-clock selection. Logic, not platform: these fail on Windows too. The "9/9 green" claim is
     stale; please confirm with `ctest` locally.
   - No `-Werror`/`/WX`, and the tree has a live `-Wunused-variable` warning (`tt.cpp:152`).
   - `main.cpp`, `run_manifest.hpp` and two test files need `windows.h`, so the engine cannot be
     built on Linux, which rules out OpenBench workers, CI and most cloud testing.
9. **Tuning budget.** `tune_search_spsa.py` defaults to 10 iterations x 4 pairs = 80 games. SPSA on
   search parameters typically needs tens of thousands of games to move anything reliably.

## 5. The Spectral-Tropical layer

- The alpha-beta search calls only `Evaluator::evaluate_fast` (9 call sites). `evaluate()`, the only
  entry to the tropical/spectral evaluator, is called solely by the teaching minimax
  (`search.cpp:258`). Setting `SpectralTropical` mode changes nothing in real play.
- `heavensgate_tropical.trm` is not in the repository, so even the reachable path would run untrained.
- A tension worth naming: a tropical rational function `max(...) - max(...)` trained by gradient
  descent is exactly the function class of a ReLU network (Zhang, Naitzat & Lim, 2018). The
  "no neural networks" rule and the tropical evaluator are mathematically the same kind of object.
  That is not a reason to drop either, but the rule should say what it is protecting.
- Recommendation: describe it honestly as research/visualisation, or turn single quantities (e.g.
  algebraic connectivity of the defence graph) into cheap eval terms and let SPRT decide. Full
  eigensolves per node will cost more depth than they repay.

## 6. Process: what the history says

- 423 of 445 commits landed in August; many subjects claim Elo from 10-game matches against
  `UCI_Elo`-handicapped Stockfish ("10-0 vs SF 3400", "+512 Elo vs SF 2400"). The October audit
  correctly retracted these. Today's anchored figure (~2361) is consistent with that retraction.
- Features were added in batches (e.g. "Tier 1 & Tier 2 upgrades") rather than one change per test,
  so nobody can say which ones help. Six explicit reverts, including a full-source restoration.
- Roughly a third of commits touch web UI, animations and video. Fine as a project, but none of it
  moves strength.

## 7. Roadmap to 3000

The single most important addition is not chess code: it is **a testing pipeline that can run
~5,000-20,000 games per patch**. Every engine that climbed fast (Stash, Ethereal, Berserk, Weiss)
did it with OpenBench/fishtest-style SPRT on every change. Concretely: make the engine build on
Linux, adopt fastchess, run SPRT `[0, 5]` at 8+0.08 then confirm at 40+0.4, one change per test.

Then, in order of expected value per effort (ranges are typical of what such changes gave other
engines at this level, offered as priors to test, not promises):

| Phase | Work | Rough prior |
|---|---|---|
| A. Fix | Non-PV guards on RFP/NMP/ProbCut; remove lazy eval or make it window-independent; rescale king danger with a 2-attacker gate; corrhist bound rules; fix SMP helpers (own ID loop + aspiration); PV table; drop or SPRT multi-cut; cap double extensions | +100 to +250 combined |
| B. Tune | Large labelled dataset (millions of positions, e.g. self-play at fixed depth or a public resolved set), fit K, tune **all** terms including PSTs; delete dead parameters | +100 to +200 |
| C. Evaluate | Add the missing terms from section 4.5, each through the tuner and then SPRT; endgame scaling and a KPK bitbase | +100 to +200 |
| D. Search | TT static eval, fail-soft, cutnode-aware LMR, history pruning of quiets, qsearch TT eval, SPSA with real budgets | +50 to +150 |
| E. Endgames | Syzygy probing via Fathom (MIT, no neural content) | +10 to +30 |

Phases A-C plausibly reach 2650-2800 with disciplined testing. 2800-3000 is where gains shrink to
+2 to +5 per patch and the testing budget decides the timeline: realistically many months to a
year or more of steady SPRT work, depending on hardware.

## 8. Immediate next steps

1. Fix the three failing tests (decide the intended `score_matches_rule50` contract), add `-Werror`,
   and make a portable build (`run_manifest.hpp` behind `#ifdef _WIN32`).
2. Set the UCI default to `Threads=1` and `OwnBook=false` until SMP is fixed.
3. Stand up the SPRT pipeline (this audit's `match.sh` + fastchess is a starting point).
4. Then Priority 1 Step 1/2 (delete `TTClockContexts`, lockless TT) as the first SPRT-gated patches,
   followed by Phase A items one at a time.

## Sources

- [CCRL Blitz rating list](https://computerchess.org.uk/404/rating_list_all.html) (Stash 37.0 3417, Ethereal 12.x)
- Blunder README ratings table, github.com/algerbrex/blunder (CCRL Blitz per release)
- Stash README "rated around" history, github.com/mhouppin/stash-bot (tags v15.2 to v29.0)
- [Stash on the Chess Programming Wiki](https://chessprogramming.org/Stash)
- Zhang, Naitzat, Lim, "Tropical Geometry of Deep Neural Networks", ICML 2018
