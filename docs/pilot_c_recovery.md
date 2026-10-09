# Candidate C recovery, tuner parity, and Game 8 diagnostic

## User-requested stop and fresh audit — 2026-10-09

At 14:51:03 UTC / 20:21:03 IST the scheduled monitor was deleted and D's verified
supervisor/owned engine tree was stopped at the user's explicit request to
reassess engine skill. This is not a random tournament abort or engine crash.
The original manifest remains 60 games; no original output or frozen binary was
overwritten. The separate `pilot-D-60/user-stop.json` records intentional
cancellation. No restart or new match is scheduled.

A separate prefix audit checked 18 completed PGNs (all played mates, HG 16 wins
/2 losses, zero flags on either side), 1,707 legal played plies and their complete
bank/history commands, plus 54 legal plies of unfinished game 19. The outstanding
HG search is game 19 ply 55. Fourteen HG searches at move 100+ used six threads,
starting bank at least 4,456ms, reply time at most 176.485ms, depths 9–22.
These are sampled clock facts, not completion of the predeclared pilot or Elo.

Evidence: `build/skill-audit-20261009/pilot-prefix-and-comparison.json` and
[the comprehensive skill/data/code audit](engine_skill_audit.md). Earlier C/B/C2
failures below remain separate and preserved. The new audit supersedes automatic
execution/completion instructions in historical sections.

Updated 2026-10-09. This is clock/harness validation and a search diagnostic, not an Elo promotion.

## Latest status: C2 interrupted, zero-forfeit gate failed

The subsequent recovery preserves this failure. Current-source tooling is now
compiled/verified, and a separate clock/operational Candidate D is prepared for
a fresh supervised pilot; it does not resume or pool C2. See
[implementation evidence and engine improvement plan](engine_improvement_plan.md)
and the predeclared `build/recovery-clock-d/pilot-D-plan.json`. The D launch and
current process state are recorded separately in `build/recovery-clock-d/pilot-D-60`.

At the 2026-10-09 18:19 IST check, process 19668 and both engine processes were
absent. Only **16/60 completed PGNs** exist; telemetry ends at Game 17, ply 87
(Stockfish's move 45...Bb8), at **13:40:30.790 IST**. There is no tournament footer,
the stderr file is empty, and every saved reply has protocol status `ok`.
The frozen C2 binary still matches its declared SHA-256. No restart, rebuild,
output replacement or pooling was performed. The existing heartbeat is paused.

Windows Power-Troubleshooter records sleep at **13:40:45.266 IST**, wake at
13:59:16.675, another sleep at 14:00:33.812, and wake at 18:04:45.202. Kernel-General
then records shutdown at 18:06:43.509 and boot at 18:06:51.500. These interruptions
compromise the timed experiment. The logs do not establish an engine crash or
the exact mechanism that terminated the match; do not equate an absent process
with a diagnosed chess-engine defect.

Separately, **HG already forfeited Game 12 as Black at move 141** before those
interruptions. Among the 16 recorded endings there is one HG clock loss and no
opponent clock losses. The final attempted reply is unplayed:

- FEN: `8/7R/8/8/4k3/8/8/3K4 b - - 63 141`.
- Attempt: `e4e3`, status `ok`, depth 0, 2 nodes, one thread.
- Physical bank: **0.005 ms**; hard/soft budget: **0.0025 ms**.
- Outer elapsed time: **0.014 ms**, exceeding the physical bank.
- The preceding HG move started with 0.020 ms and consumed 0.015 ms.

This is a genuine physical-clock loss, not a protocol-timeout classification
failure. The zero-forfeit gate therefore has not passed, regardless of the later
system interruption. Low-overhead emergency search cannot rescue an already
depleted microsecond bank. Inspect allocation/reserve consumption throughout this
game before designing another isolated clock candidate; never conceal the failure
by crediting a minimum fictitious bank, playing an expired reply or adjudicating
the losing endgame. No 60-game completion or CCRL strength claim is valid.

Original interrupted-output hashes (SHA-256), for preservation checks:

| Output | SHA-256 |
|---|---|
| PGN | `d805467b8f82a4aeb94ea5f263ff8b124ec8f077c4cb35d5cf814905f133021e` |
| Move trace | `ffd5d0d57a842223e94c4f534405f9a6dbc4a62791b9f2a681766ff3d79278b6` |
| Manifest | `a32808a6b38a3172cb71fd78bbee8619ea8d0e015064a686ff5dc91cf5deb0ae` |
| Console | `40aeb1cf649e72301c5471e8bd0e2eae2d3fbd5c433ade2d55a15761c707f888` |
| Stderr (empty) | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |

The sections below retain the original launch/verification record. Their
monitoring/post-completion instructions are historical: C2 is no longer running.

## Preserved pilots and binaries

Pilot-B's original 14 games remain in `build/recovery-clock/input-B/`. They are not pooled with later runs.

The original Candidate C run, `build/recovery-clock/pilot-C-60.pgn`, stopped after two games. Its binary remains frozen at SHA-256 `14d6ded479a178bc55d5ef446224cfaf42da150d81c8a67ce9c9971c496197b5`.

Game 1 ended in played checkmate. Game 2 reached move 128, but Stockfish's next attempted reply timed out with a physical bank of 1.686 ms. Its final telemetry row records a protocol timeout and 36.063 ms outer elapsed time. The old harness classified that clock expiry as an engine failure and aborted the pilot. The reported Game 2 win is not a played-board win or evidence of strength.

Heaven's Gate completed its move-100+ searches in that game without flagging, down to approximately 21 ms remaining. This is limited evidence, not completion of a 60-game clock validation.

Candidate C2 changes the harness to classify opponent deadline expiry at its physical bank as a time forfeit, and reinitialize the opponent before the next game. Crashes, malformed/illegal replies, and other protocol failures still abort. HG and opponent clock losses are counted separately, and any time forfeit still makes the full run return failure rather than silently pass a stability gate.

The separate C2 binary is `build/recovery-clock-c2/heavensgate.exe`, SHA-256 `b247d1a1a49848626167a8a4e4afc4a96a294c6e934624d86a1721a6c2bbfcf7`. Its compiled-source fingerprint is `7fcc1bec4b57a763f4c5a3449c8892452e19fee1766f4c084b2a551a30c8975a`.

The new, independent 60-game pilot started at **2026-10-09 07:07:40 UTC**, process 19668:

```powershell
.\build\recovery-clock-c2\heavensgate.exe tournament 60 120 0 0 2400 6 build/recovery-clock-c2/pilot-C2-60.pgn --paired --book-off --hash=64
```

This means a 120-second total bank, zero increment, 30 paired initial FENs with both engine colors, Hash 64 MiB each, and six threads each against Stockfish 16.1 limited to 2400. The manifest declares Stockfish handicap randomness and nondeterministic Lazy SMP. Logs are `pilot-C2-60.console.log` and `pilot-C2-60.stderr.log` in the same directory.

The heartbeat now monitors **C2**, quietly during ordinary progress. Do not rebuild this directory, restart the match, overwrite outputs, or run competing benchmarks while it is active. After completion:

```powershell
python tools/verify_pilot.py build/recovery-clock-c2/pilot-C2-60.pgn
```

The verifier checks binary hashes, complete count, FEN/color pairing, legal played moves, full history and bank commands, board-terminal results or evidenced clock losses, and move-100+ HG clock/depth/thread behavior. Clock comparisons account for the frozen telemetry's six-significant-digit serialization. Clock wins remain separated from board-terminal endings.

## Verification before the timed C2 pilot

- C2's CTest passed AllTests (including move-ordering reduction) and TexelParity. Perft gate passed all references, including startpos depth 6 = 119,060,324. The real-opponent fixed-depth smoke passed.
- End-to-end deliberately crashed and illegal-reply opponents still aborted with exit code 1 and no successful tournament headline. Evidence: `build/hg-negative-matches-jswwcha2/`.
- A deliberately timing-out opponent completed both paired fixture games, with **HG flags 0 / opponent flags 2**, then returned failure. Evidence: `build/recovery-clock-c2/timeout-continuation.pgn` and sidecars. This proves clock-loss continuation, not engine strength.
- Nine single-thread depth-9 comparisons against frozen B matched score, PV and node count. Forty low-clock UCI round trips measured median 2.215 ms / p95 2.459 ms for C2 versus 6.167 / 8.104 ms for B. This sample is not an OS scheduling guarantee. Evidence: `build/recovery-clock-c2/comparison.json`.
- **Not all five gates are green:** the original C short STS run scored 3,475/8,000, below the 50% gate. Additional short runs were noisy and retained, not selected to erase the failed gate. C2 is experimental, not promoted on these checks or NPS.

## Texel parity repair

In `tools/tune_classical_texel.cpp`, the old fixed term represented only PSTs and omitted runtime king danger/storms, trapped pieces and material interactions. Pawn shield and uncastled-king features were also extracted when queens were absent, unlike the production evaluator.

For reference parameter vector w0, cache:

`base_constant = runtime_full_eval_white(w0) - modeled_tunable_score_white(w0)`.

This includes Pawn MG's locked 100 cp anchor, full non-modeled positional terms, and the reference integer-taper residual. The baseline therefore exactly matches the full-window `Evaluator::evaluate_fast`, with correct White/Black tempo orientation. Parameter perturbations reconstruct the board so its incremental material uses the new values, and clear the pawn cache so old weights cannot leak into the test.

`--parity-only` exits before Adam or parameter export. It checks all 75 exposed weights at in-range +/-1 and +/-8 cp perturbations against both the integer runtime score and an independently assembled unrounded production-feature oracle. The latter distinguishes incorrect feature slopes from the two per-side integer divisions; quantization can change the smooth-model residual by less than 2 cp.

Evidence from `build/recovery-game8/`:

- CTest passed AllTests and TexelParity in the separate diagnostic build.
- Seven adversarial fixtures: maximum baseline error 0 cp.
- Deterministic 128-position dataset sample plus color mirrors: **256 positions**, maximum baseline error **0 cp**, unrounded slope error **0.000001272 cp**, integer/smooth discrepancy at most **1.625 cp**.
- Dataset SHA-256: `f28b113ae5f6f47aa79a7db8272990e2842cef8d258a1a1a33aee1a82a6b1799`.
- Sample and final passing log: `texel-dataset-sample.txt`, `texel-dataset-sample-verified.log`. The earlier overly strict one-cp quantization check's log is also preserved.

No tuning optimization was run and no evaluation coefficients were changed. Exact parity at the reference vector is not a claim that a smooth model reproduces integer rounding at every future parameter vector. Held-out loss and paired game validation remain mandatory before promoting new weights.

## Game 8, ply 22: NMP/LMR ablation

`build/game8_fixed_nodes.py` replays the complete frozen Pilot-B move history, not just its final FEN. It tests the root, played `...Rc8`, and alternative `...f5`, independently with a fresh game/TT, one thread, Hash 64 and books off, at 100k, 300k, 1M and 3M nodes for all four NMP/LMR combinations: **48 searches**.

The initial diagnostic exposed node counting after budget exhaustion during recursive re-search. The current-source fix checks the node cap before counting a new node. Added regression cases exercise both child positions and caps of 1, 2, 25k and 100k. Explicit node limits force serial search; normal time-controlled searches use no node cap and retain their existing default pruning. This diagnostic-only fix was built in `build/recovery-game8`, **not** into either frozen C/C2 binary.

Diagnostic binary SHA-256: `5416cd6579c00e78e41669e96285b3c6bed4418dfa032bfc14680147be2dcccb`. Evidence: `build/recovery-game8/ply22-fixed-nodes.json`. Every search respected its node ceiling.

At 3M nodes, scores below are from Black's perspective. Child searches have their own full 3M-node budget, so their scores are not root MultiPV scores or equal-depth comparisons.

| NMP | LMR | Root move / depth / score | After Rc8: score / depth | After f5: score / depth |
|---|---|---|---|---|
| On | On | Rc8 / 13 / +6 | 0 / 16 | -35 / 15 |
| Off | On | Rc8 / 11 / -7 | -20 / 13 | -63 / 13 |
| On | Off | Rc8 / 10 / +26 | +8 / 11 | -40 / 11 |
| Off | Off | f5 / 9 / +32 | -7 / 10 | -9 / 10 |

Removing LMR selects f5 at some smaller budgets, and removing both selects f5 at 3M, but also reaches a substantially shallower root depth. All four separate child comparisons at 3M favor Rc8 over f5. This shows pruning/order/horizon sensitivity; it does **not** establish that f5 is the objectively superior move or that one heuristic incorrectly pruned it. Disabling NMP/LMR globally is not justified by this position.

The next search investigation should trace the critical f5 responses, reductions and re-searches at matched completed depths, reproduce across additional tactical positions, and test one isolated correctness repair at a time. Do that after the timed pilot, not concurrently with it. Strength acceptance requires a predefined, adequately powered paired test against frozen controls; neither this diagnostic nor a 2400-handicapped opponent establishes CCRL Elo or a 3000 rating.
