# Heaven's Gate: strength work log (2026-10-15/16)

Baseline: `main` at `b2e7c0c`, measured at about **2360 CCRL Blitz-equivalent**
(400 games vs Blunder 6.1.0-7.6.0; `docs/full_audit_2026-10-15.md`).

Method. One change per commit. Strength changes are game-tested with a chunked,
restart-safe SPRT harness (fastchess; UHO_4060_v4 openings, each played with both
colours; 8+0.08; 1 thread; Hash 16; books off; no adjudication). Changes that must
not alter the search are verified by node identity instead (equal node counts,
scores, best moves and PVs over 80 positions, or the `bench` signature).

## Accepted (in `dev`)

| Commit | Change | Evidence |
|---|---|---|
| `9d9f1e3` | Portable Linux build, `-Werror`, UCI defaults Threads=1 / OwnBook=false | builds on Linux and MinGW; ctest 9/9 |
| `4998e0e` | Delete TTClockContexts; lockless XOR TT | bit-identical; +4% NPS (1 thread) |
| `9a19ad4` | `tools/sprt.py` | |
| `71cd9d7` | No RFP / null move / ProbCut at PV nodes | +6 Elo, 120 games (inconclusive; standard practice) |
| `6107f1a` | **Eval v2**: all terms tunable/linear; new terms; HCE tuner (exact parity); tuned on 704K positions | **+218 Elo**, 160 games, SPRT passed |
| `2d26a77` `bd469eb` `50ede3a` | **Eval v3**: endgame scaling, 7 new terms, cached attack maps, retuned; lazy eval (margin 320) | **+41 Elo vs v2**, 320 games, 95% CI ~[+11, +72] |
| `beefe5f` | PV reset at every node (illegal-PV output) | bit-identical |
| `c4f8e72` | Lazy SMP helpers run real iterative deepening | 2 threads: +0.9 ply at 1 s |
| `3aee9d4` | Precomputed PST lookup | bit-identical; +1.6% NPS |
| `4a26b6f` | Syzygy via Fathom (`SyzygyPath`): WDL in search, DTZ at root | identical without tables; verified on 3/4-piece tables |
| `699be06` | `bench` command | deterministic |
| `b699967` | qsearch skips the eval when in check | bench signature unchanged |
| `53ebae8` | `tools/datagen.cpp` self-play data generator | |

## Rejected or dropped

| Candidate | Result |
|---|---|
| `nolazy` (remove lazy eval on v2) | -35 Elo, 120 games |
| static-eval cache | bit-identical, no speedup |
| deferred quiet scoring | not identical (fresher history); not worth a test slot |

## Built and tested in this session (branches, not yet in `dev`)

| Branch | Change | Offline evidence |
|---|---|---|
| `eval-v4` `55a72e0` | Space, pawn islands, king-to-pawn distance (eg), connected passers, long-diagonal bishops; retuned | val MSE 0.052136 (v3 0.052490); ctest 9/9 |
| `eval-v5` `30a8341` | Material imbalance made tunable (was hard-coded); minor behind pawn, rook on queen file, king threats, restriction; retuned | val MSE 0.051735; untuned build bench-identical to v4; ctest 9/9 |
| `tm` `6175333` | Soft time limit scaled by best-move stability, best-move node share and score drop; removes the compounding fail-low extension | Bench positions at 8s+0.08: mean 992 ms per move on dev (mostly running into the hard limit) vs 459 ms; fixed-depth bench unchanged |
| `improving` `9eb2355` | `improving` compares static evals and skips in-check nodes (was reading a stale sibling value) | bench 6,106,436 nodes (dev 4,592,521); ctest 9/9 |

All four cross-compile with MinGW under `-Werror`.

## Test queue results

| Test | Result | Decision |
|---|---|---|
| `search2` vs dev | -52 Elo, 80 games, 95% CI [-105, -2] | rejected |
| `eval-v4` vs dev | -14 Elo, 240 games, 95% CI [-46, +17] | not merged |
| `eval-v5` vs dev | 0 Elo, 240 games, 95% CI [-31, +31] | not merged |
| `tm` vs dev | **+54 Elo**, 240 games, 95% CI [+26, +83], SPRT [0, 20] passed (LLR 3.01) | **merged into dev** (`6175333`) |
| `improving` vs dev | -61 Elo, 80 games, 95% CI [-114, -12] | rejected: pruning margins are tuned around the old flag; the corrected flag prunes less (bench +33% nodes) |
| `tm-h28` vs tm (increment horizon 35 to 28) | +2 Elo, 160 games, 95% CI [-39, +44] | neutral, not merged |
| `improving2` vs tm (only clear the stale in-check slot) | -4 Elo, 160 games, 95% CI [-48, +39] | neutral, not merged |

Published: `dev` pushed to GitHub as branch `strength-2026-10-16` (head `1797c3d`).
Commit messages were then rewritten to drop the Co-Authored-By trailer, so the
published IDs differ from the IDs quoted above (code identical; `6175333` is now `1797c3d`).

## Anchor rating of the published engine

Same setup as the baseline measurement of `main` (~2360): fastchess, 10+0.1,
noob_3moves openings with colours reversed, 1 thread, 64 MB hash, books off.

| Anchor (CCRL Blitz) | W | D | L | Score | Implied |
|---|---|---|---|---|---|
| Blunder 7.2.0 (2425) | 93 | 5 | 2 | 95.5% | 2956 |
| Blunder 7.4.0 (2532) | 87 | 11 | 2 | 92.5% | 2968 |
| Blunder 7.6.0 (2631) | 73 | 16 | 11 | 81.0% | 2883 |

Pooled maximum likelihood: **2920** (95% about 2860-2990); Blunder 7.6 alone: 2883
(about 2800-2975). About **+560** over `main`. Scores above 90% carry little
information, so ~2900 is the figure to quote. One Heaven's Gate time loss
(61 ms overrun with ~0.12 s per move, 4 engines on 2 cores); follow-up: raise the
default Move Overhead from 25 to 50 ms. An earlier 7.6 run was discarded because
orphaned processes from a stopped match overloaded the CPU (23 of 24 games lost on time).

### Eval v5 retuned on self-play data

v5 (all v4 and v5 terms, tunable imbalance) retuned on 2,585,118 positions: the
quiet-labeled set plus 1,881,363 self-play positions from `tools/datagen.cpp`
(5000 nodes per move, 5 random opening plies, game-result labels). Result against
`main`: **-31 Elo, 160 games, 95% CI [-73, +11], rejected.** The tune pulled the
endgame rook and queen values down sharply (rook 717 to 560, queen 1323 to 970),
which points at the data: fast games fail to convert won endgames, and 5 random
plies (3 of them White's) skew both balance and labels. Round 2 of the generator
(`fa50a72`): 10000 nodes per move, 2 random plies, the first 8 plies skipped, and
labels blended 75% result and 25% sigmoid(search score).

### Eval tuned on Stockfish-labelled positions: +50 Elo

1,691,704 unique positions (self-play rounds 1 and 2 plus 350K from the quiet-labelled
set) were labelled by Stockfish (2026-10-13 build) at 5000 nodes; at 1.2M labels the
set was filtered to quiet positions (best move not a capture, |score| <= 1500),
1,104,620 kept, label = 0.8 * sigmoid(score / 400) + 0.2 * game result. The v5
evaluation (all v4/v5 terms, tunable imbalance) was tuned for 1500 epochs
(val MSE 0.0204). Material came out healthy (rook 676/680, queen 1494/1030), unlike
the self-play tune.

Against `main`: **+50 Elo, 240 games, 95% CI [+17, +83]**, LLR 2.26 under SPRT [0, 20]
(accepted on the confidence interval, as v3lazy was). No time forfeits. Published on
branch `eval-sf-2026-10-17` for review; not merged into `main`.

Anchor rating of this build, same setup as for `main` (settings now applied, Hash 64):

| Anchor | `main` | `eval-sf-2026-10-17` |
|---|---|---|
| Blunder 7.6 (2631), 100 games, 10+0.1 | 81.0% (2883) | 86.0% (2946) |
| Stockfish UCI_Elo 2800, 40 games, 60+0.6 | 66.2% (2917) | 73.8% (2979) |
| Stockfish UCI_Elo 3000, 40 games, 60+0.6 | 25.0% (2809) | 48.8% (2991) |
| Pooled maximum likelihood | 2874 (2817-2933) | **2971 (2909-3035)** |

No time forfeits in 180 games. Same caveats as above: anchors measured here, not on CCRL.

A retune on the full 1,553,619-position set gave no gain over this build
(-17 Elo, 120 games, 95% CI [-65, +29]) and was not published.

### Where the engine loses: Stockfish analysis of 49 lost games

All losses against Blunder 7.6 and Stockfish 2800/3000, analysed by Stockfish at
60k nodes (`tools`-side script `pipe/analyze_losses.py`). A mistake is a move that
lowers Heaven's Gate's win probability by 20 points or more.

- 43 mistakes in 30 games; the other 19 losses were gradual, with no single mistake.
- 28 of 43 in heavy middlegames (phase >= 16), 11 in lighter middlegames, 4 in endgames.
- Only 9 of 43 were missed tactics (Stockfish's move a capture or check); most are quiet
  positional misjudgments at depth 9-16.
- Before the mistake, Heaven's Gate's own eval was on average **+123 cp more optimistic**
  than Stockfish's; 22 of 43 by more than 100 cp, 4 the other way.
- Not a scale effect: on 373 positions from won or drawn games the two evals agree
  on average (HG +151, SF +142).

Conclusion: the evaluation over-rates the engine's chances in complex middlegames
(king safety and piece activity are the likely terms). This is what the
Stockfish-labelled retune targets.

### Cross-check against Stockfish strength levels

Stockfish (source of 2026-10-13, net nn-c9e480e92425 from official-stockfish/networks),
`UCI_LimitStrength=true`, at the documented calibration time control 60+0.6, 1 thread, 64 MB.
`UCI_Elo` accepts 1320-3190 and silently ignores values outside that range (a request
for 3400 leaves the default 1320). Checked behaviourally: agreement with full-strength
Stockfish over 45 positions rises with the setting (1320: 5, 2000: 14, 2800: 17, 3000: 22),
and an out-of-range 3400 behaves like 1320 (8).

| Level | W | D | L | Score | Implied |
|---|---|---|---|---|---|
| UCI_Elo 2800 | 24 | 5 | 11 | 66.2% | 2917 |
| UCI_Elo 3000 | 7 | 6 | 27 | 25.0% | 2809 |

Stockfish pooled: 2867 (95% about 2787-2946). Blunder 7.6 and Stockfish together:
**2874 (about 2817-2933)**. Summary: **about 2870-2900 CCRL Blitz equivalent, about
+520 over `main`.** Stockfish levels are calibrated against Stash on an older version
and weaken by random mistakes, so they are a cross-check, not a precise anchor.

### Fairness audit of all rating matches

- Rated binary is byte-identical to the published build before `aa7200d`.
- Colours exactly balanced (each opening played with both colours); no books, no tablebases, no pondering.
- No crashes, illegal moves or disconnects. Abnormal endings: one Heaven's Gate time loss (vs Blunder 7.6).
- Stockfish's limiter confirmed active in the logs (best move differs from its own PV).
- Heaven's Gate's fastchess settings were skipped because of the menu UCI bug (fixed in
  `aa7200d`), so it ran on its defaults: Threads 1, OwnBook off (as intended), Hash 256 MB
  instead of 64 MB. Blunder used 64 MB. A 64 MB table is about 10% full after a 2 s search,
  so the extra hash is worth at most a couple of Elo.
- The `main` baseline (2360) used a UCI entry point whose settings were applied, so the
  before/after comparison is like for like.
- Some short builds and tests ran on the same 2 cores during matches; symmetric for both
  sides but a possible cause of the single time loss.
- The absolute figure depends on Blunder's CCRL ratings holding at 10+0.1 on this machine
  (CCRL plays 2+1 on its own hardware); roughly 50-100 Elo of extra uncertainty.

Reading of the eval results: v4 and v5 each lowered validation error, but neither
turned that into Elo. Past v3, the 704K-position game-result dataset is the
limit, not the feature set. The next eval step is data: a few million positions
from `tools/datagen.cpp` self-play (or a public resolved set), then retune v5 on that.

## Search work on top of the Stockfish-labelled eval (2026-10-19)

Same harness (8+0.08, UHO_4060_v4, Hash 16, 1 thread, colours reversed). Rule:
a change is merged only when its 95% confidence interval lies above 0.

| Candidate vs `main` | Games | Elo | 95% CI | Decision |
|---|---|---|---|---|
| Remove classic multi-cut pruning | 240 | **+45** | **[+14, +77]** | **merged (`b3cff56`)** |
| Extra LMR ply when the TT move is a capture | 480 | +12 | [-10, +34] | not merged; retested in a combination |
| Correction history that respects bound direction | 240 | +12 | [-18, +41] | not merged; retested in a combination |
| Search parameters from 440 SPSA iterations | 240 | about 0 | | not merged |
| TT move searched before move generation | 120 | -30 | | rejected |
| King-safety additions (queen-ray exposure, minor defenders, shield table) | 120 | -12 | | rejected |
| Mate-distance pruning | 120 | -23 | | rejected |
| Other LMR variant | 120 | -15 | | rejected |
| Null test (`main` vs itself) | 120 | +3 | | harness fair |

Multi-cut searched up to 8 moves at depth - 4 at every non-PV node of depth 8 or
more before the real move loop. Singular extension already supplies that cutoff,
so the extra searches mostly cost time.
