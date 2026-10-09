# Measuring Heaven's Gate: evidence reset and code audit

2026-10-09. This supersedes earlier calibrated-rating and additive-Elo roadmaps.
The user's current instruction is to stop scheduled work and games, then analyse
existing evidence. No new match, build, engine search, tuning run, evaluation
change, book change or candidate promotion was performed for this audit.

**Subsequent approved implementation:** the separate
[qsearch correction and measurement report](qsearch_stalemate_candidate.md)
records a compiled failing regression, isolated repair, CTest 7/7, reference
perft and a verified paired-result analyser. Games remain stopped; no Elo gain
or production promotion is claimed. The next approved
[rule-50/TT-clock investigation](rule50_tt_candidate.md) reproduces and repairs
terminal and clock-context score reuse, with CTest 8/8 and preserved staged
evidence. Full repetition-history-dependent cache semantics remain unresolved.
The subsequently approved [paired strength screen](rule50_paired_screen.md)
completed 60 verified games, with the strict clock-tag candidate scoring only
32.5% versus its frozen qsearch control. It must not be promoted. This remains
relative short-control evidence, not an absolute CCRL rating. All games have now
ended; the monitor stays deleted. The audit below is the pre-repair evidence.

## Outcome and scope

**Heaven's Gate's CCRL rating is presently unknown.** Neither 2780–2800, 2711,
2584 ±104, nor the “3400” result is established by the available experiments.
This does not establish that the engine is weak; it establishes that our old
measurement does not identify its rating or changes in its strength reliably.

The scheduled automation `heaven-s-gate-pilot-c-completion` was deleted using the
app's automation controls. After checking command lines, paths, parentage and
creation times, supervisor PID 9336 was stopped at 14:51:03 UTC / 20:21:03 IST.
Its scoped Windows job stopped its HG child 27808 and Stockfish child 22828.
Subsequent process checks found no HG/Stockfish games running. Nothing was
restarted. `build/recovery-clock-d/pilot-D-60/user-stop.json` records deliberate
cancellation, rather than fabricating a crash or a completed 60-game pilot.

Fresh analysis read all **136 repository PGNs**, including build copies and
negative/smoke fixtures, the legacy training file, the provenance-stamped
recovery dataset and preserved diagnostics. The code review focused on actual
search/evaluation paths, move ordering/SEE, TT, endgame probing, time policy,
tournament/UCI measurement and the tests/tuners supporting strength claims.
This is not a formal proof of every source line or of race freedom. No profiler,
sanitizer or new compiled regression was run; source-level risks below are
distinguished from reproduced historical failures.

Reproducible, exclusively created analysis artifacts are in
`build/skill-audit-20261009/`:

- `pgn-audit.json`: every source hash and per-file counts/replay observations.
- `games.jsonl`: per-game headers, legal prefix, result, issues and game identity.
- `pilot-prefix-and-comparison.json`: cancelled D prefix and matched Batch 8/9.
- `training-and-statistics.json`: both datasets and illustrative sample precision.
- Three `audit_*.py` scripts and `test_audit_contract.py`: no engine launches.

The five audit fixture tests passed: SAN/coordinate mate, nonterminal mate claim,
wrong result/continuation, unfinished/duplicate headers and legal capture into
stalemate. Existing engine-test evidence was inspected, not rerun until favorable.

## 1. Four different measurements, not one “skill number”

| Question | Appropriate evidence | What it cannot establish |
|---|---|---|
| Did this change improve HG? | Frozen candidate vs frozen HG control; matched two-color openings; uncertainty/stopping rule | Absolute CCRL rating |
| How strong is the released engine? | Multi-opponent, unrestricted, versioned gauntlet with an independently anchored pool and matched conditions | An official rating without that organisation's testing |
| What is failing in its chess? | Independent tactical/strategic/endgame suites, fixed-node diagnostics, legal regression fixtures | Elo from a solve percentage |
| Is it operationally safe? | Clock/protocol/legal-state/allocation/concurrency tests and preserved telemetry | Improved chess decisions |

Perft is move-generation evidence. NPS is throughput. Search depth is a selective
search counter. Texel validation loss is a prediction metric. None is a rating.
Likewise, a five-gate pass is a prerequisite, not a theorem of zero regression.
Current Gate 5 itself correctly calls its one-game depth-4 test a stability smoke;
the file's older “Infallible”/non-regression wording overstates that evidence.

Stockfish's strength limiter selects deliberately inferior moves with randomized
bias. A setting of 2400 is a handicap target, not an independently measured 2400
opponent under every thread/time/book configuration. Current official command
documentation describes calibration at 120+1, anchored to CCRL 40/4; that is not
our 120+0 setup, and calibration details are version-dependent. Thus adding a
score-derived difference to the option number is not a CCRL calibration.
[Stockfish handicap mechanism](https://official-stockfish.github.io/docs/stockfish-wiki/Stockfish-FAQ.html#how-do-skill-level-and-uci_elo-work),
[official command documentation](https://github.com/official-stockfish/Stockfish/wiki/UCI-Protocol-and-Stockfish-Commands#uci_elo).

CCRL's published methodology anchors a pool of engines and specifies time/hardware,
hash, pondering, tablebase and opening conditions. A local gauntlet must declare
which list/configuration it approximates; six-thread 2+0 scores do not automatically
transfer to a list's single/other-CPU entries. The accessible 40/15 about page is
an older published snapshot, not confirmation of today's Blitz details; consult
the selected list's current rules before any future launch.
[CCRL testing methodology](https://computerchess.org/about.html).

## 2. What the existing games actually say

### Fresh whole-repository replay

The 136 files contain 5,915 game-block occurrences, 115 distinct file hashes and
3,845 distinct text blocks. **5,215 occurrences** have matching legal board/rule
endings; deduplicating initial position plus played moves gives **2,647 distinct
such games**. This includes fixtures and different engine versions/opponents and
is an inventory, **not a pool eligible for a single rating calculation**.

Observations overlap and are counted per occurrence, including copies:
480 nonterminal declared results; 170 games continuing after an automatic terminal
board; 73 board-result mismatches; 32 truncated comments/variations; 16 duplicate
headers; 15 malformed comments; 3 unfinished/missing results; 1 illegal/unknown
move. Clock losses can legitimately have nonterminal boards, but require separate
telemetry evidence. A nonterminal recorded result is not automatically a bad
chess judgment, and early mate adjudication is not automatically false; its
unplayed claim simply was not independently established by this harness.

### The comparisons that motivated the project

| Evidence | Recorded HG W–D–L | Verified board/rule endings | Consequence |
|---|---:|---:|---|
| Batch 7 | 9–0–1 / 10 | 0 / 10 | All ten stopped on reported mate |
| Batch 8 | 26–2–2 / 30 | 2 / 30 | 28 reported-mate endings |
| Batch 9 | 22–4–4 / 30 | 4 / 30 | 26 reported-mate endings |
| Batches 18, 20, 21 | Each 0–0–10 | Each 0 / 10 | Severe recorded failures, not calibrated −Elo estimates |
| Cancelled D prefix | 16–0–2 / 18 | 18 / 18 | Real played mates; handicap opponent; planned sample unfinished |

The 35–2–3 result underlying 90%/40 is exactly Batch 7+8. The archived PGN headers
identify **Stockfish 16.1**, not the Stockfish 14 named in the original briefing.
Opponent identity and rating basis must come from preserved records, not memory.

Batch 8 and Batch 9 do match on all **30 initial FENs and HG colors**. Their recorded
score difference is −3 points /30, or −10 percentage points. Transitions are:
20 win→win, 3 win→draw, 3 win→loss, 1 draw→win, 1 loss→draw, 1 loss→win, 1 draw→loss.
There is **no matched round with both endings board-verified**. We cannot prove
a particular evaluation feature caused a measured strength loss from this small,
adjudicated, randomized-handicap comparison. Nevertheless its failure positions
are useful diagnostics; they should not be discarded because its rating claim is
invalid. Inspect all transitions, not only the worst losses.

The old frozen tournament source additionally gave SF a per-move allocation and
an instantaneous board rather than today's native bank/full-history protocol.
HG's time policy also changed during recovery. Old PGN `%clk ...ms` comments often
represent spent time, while new clock comments represent remaining bank. Direct
clock or centipawn-column comparisons across formats are therefore misleading.
Old mate adjudication (`abs(score)>=24000`) prevented many difficult endings from
ever being played. Recovery exposed conditions old batches did not exercise.

Archived documents need to be read as claims, not facts: the “3400” document has
seven games, all early mate claims and an arbitrary +800 for a perfect score;
the “100-Game Benchmark” document reports **one** completed game. README's 2711
claim comes from a four-game 37.5% handicap score. The gauntlet's 2584 ±104 estimate
uses handicap targets and an assumed uncapped rating; those are not independent
rating anchors, regardless of the statistical method's label. The tactical tool
still contains `1400 + BK_fraction*1400`, an unsupported rating conversion.

### Cancelled D: keep the useful evidence, reject the rating interpretation

The immutable manifest still requests 60 games. We did **not** change it to 18 to
make the full verifier pass. A separate prefix audit reconstructed **1,707 legal
plies** in the 18 completed games, plus **54 legal plies** of game 19. All completed
games reached checkmate, with zero observed HG and zero opponent flags. Initial
FEN/color pairing, every recorded bank and full position/go history were checked.
The nine opening-pair totals were two pairs scoring 1 point out of 2 and seven
pairs scoring 2 points out of 2.

Fourteen HG searches at full move 100+ used six threads. Their minimum starting
bank was **4,456ms**, maximum reply time **176.485ms**, and completed depths ranged
from 9 to 22. That supports improved operational behavior on these sampled long
games; it does not complete the predeclared reliability gate or establish a gain.
Game 19 ply 55 has a flushed begin without end because of user cancellation.
Stderr is empty; no finished run/supervisor footer was synthesized after killing
the launcher. Acceptance remains **not assessed: deliberately incomplete**.

Pilot-B's HG flag, original C's opponent timeout misclassification, and C2's flag
plus later sleep/reboot remain separate preserved failures. They are not pooled
with D or turned into engine-strength wins/losses for a retrospective rating.
See [failure evidence](pilot_c_recovery.md).

## 3. The statistical contract we were missing

For a candidate against a fixed control, game score is 1/0.5/0 and
`p=(W+0.5D)/N`. A descriptive logistic performance difference is
`ΔE=400*log10(p/(1-p))`. This is conditional on the opponent and test environment.
At p=0 or 1 the finite MLE does not exist; do not invent ±800 or any other cap.

For opening pair j let `Xj=(score_as_white+score_as_black)/2`. Analyse independent
opening blocks, not the two games as independent samples:

```
p_hat = mean(Xj)
v_hat = sum((Xj-p_hat)^2)/(M-1)
SE(p_hat) = sqrt(v_hat/M)
```

For adequately sized fixed samples, form a paired score interval and transform
its endpoints to logistic Elo. Use block bootstrap or a validated pentanomial
likelihood when the simple approximation is unreliable; repeated related openings
require larger family-level blocks. Pair-point counts have five bins:
0, 0.5, 1, 1.5, 2. One point includes both two draws and a win/loss split.
Do not stop at a favorable running fixed-sample interval. For sequential tests,
predeclare hypotheses, error rates, Elo model and stopping rule. For example,
logistic H0=0 versus H1=+5 with α=β=.05 yields nominal LLR boundaries ±ln(19);
the nuisance/draw probabilities need a proper pentanomial implementation, not
just those constants. Normalized Elo is a different scale and must be labelled.
[Fishtest's primary statistical methodology](https://github.com/official-stockfish/fishtest/wiki/Fishtest-mathematics).

Planning illustration, **not confidence intervals for our pilots**: at p≈0.5,
independent opening pairs and pair variance 0.125, a fixed-sample 95% normal
interval gives approximately ±90 Elo with 60 games, ±34 with 400, ±22 with 1000,
±15 with 2000, ±10 with 5000 and ±7 with 10000. Actual width depends on observed
pair distribution. A 60-game clock pilot cannot generally identify a +5/+10
search gain. Sequential tests can save work; they cannot remove uncertainty.

Keep physical-clock outcomes in operational score reporting. For a predeclared
chess-strength promotion gate a flag/crash is a failed or compromised experiment,
not a reason to silently discard the bad game and continue choosing samples.
Report all outcomes separately; do not retrofit a no-flags subset as unbiased.

## 4. Evaluation data is not yet a general-strength training corpus

The legacy `data/quiet_positions.txt` has **77,455** valid FEN/label rows, but only
**40,635 distinct full FENs** and **40,092 exact/color-canonical board positions**.
Thirty-four canonical groups have conflicting outcome labels. Such conflicts can
be legitimate different game outcomes, not necessarily corrupt labels; they do
show that one deterministic outcome per position cannot be assumed. No game,
player, source hash or split provenance is carried by that FEN|label file.
Grandmaster origin and independence are **not established**. Quiet FENs alone do
not validate outcome provenance or a tactically settled evaluation target.

The newer dataset passed independent source/hash/label/split validation again:
10,588 train /1,124 validation /1,334 test samples, including color mirrors.
There are **6,523 original position groups from 1,826 sampled games**, not 13,046
independent observations. All original-sample player names are HG/Stockfish,
HG/Baseline or Phase3/Phase5: local engine games. Exact/mirror and game isolation
are useful, but related openings, paired runs and engine families may cross
splits. No fit was run. General retuning needs diversified trusted sources,
family/source-group holdouts, tactical-settling checks and feature coverage.

The tuner parity repair is real: preserved baseline and perturbation checks pass.
However, exact reconstruction via a fixed residual does not make hardcoded
imbalance terms trainable. Lower held-out Texel loss does not establish playing
strength; an independent played-match acceptance remains necessary. Expose the
existing imbalance coefficients with their **present** values and require exact
score parity before any fit. New feature weights should initially be zero, with
data/support and regularization checked before claiming anything about value.

## 5. What the code audit prioritizes

The engine already has substantial classical search, history/capture learning,
safe mobility, king safety, pawn hashing and tapered evaluation. “Add LMP” or
“add ProbCut” is not a diagnosis: both exist. No missing-feature checklist can
promise a sum of +200 Elo. The following are ranked engineering investigations,
not retrospectively proven causes of individual losses.

### P1: terminal/rule correctness and tactical pruning contracts

1. **Qsearch stalemate path is incomplete** (`search.cpp:138–155`). The non-check
   branch can stand pat or return static evaluation when its capture list is empty,
   without proving a legal quiet move exists. Recursive qsearch bypasses the
   main-search endgame recognizer. A legal fixture is
   `k7/2pQ4/2K5/8/8/8/8/8 w - - 0 1`: `d7c7` captures into stalemate. Independent
   board replay confirms a draw. The corresponding C++ qsearch/window regression
   has **not** been compiled/run in this audit; add that before implementing a fix.
2. **Checking captures can be delta-pruned** (`search.cpp:170–185`). SEE pruning
   explicitly protects checks; the following `stand_pat+victim+200<alpha` predicate
   does not. A low-value checking capture can start a forcing sequence worth far
   more than its victim. Obtain a deterministic mirrored counterexample, then
   test one correction with finite/full windows and TT states.
3. **General rule-50 handling and TT reuse are incomplete** (`search.cpp:287–370`,
   `syzygy.cpp:24–43`, `zobrist.cpp:40–68`). The rule-50 check is in the ≤6-piece
   recognizer, after TT cutoffs; qsearch has no corresponding general check. The
   normal TT key has no halfmove/history state. Identical piece boards can have
   different draw eligibility. Test 7+ pieces, halfmove 99/100, capture/pawn reset,
   mate precedence, TT warm/cold and both colors; do not “fix” this by blindly
   changing Zobrist without a cache/history contract.
4. **Forcing quiet moves are not uniformly protected** (`search.cpp:633–663`).
   Forward futility checks `gives_check`, but LMP's predicate has no checking-move
   exception; quiet/capture SEE pruning also needs pinned/king-legality fixtures.
   SEE currently selects attackers geometrically without explicit pinned-piece
   or illegal-king-recapture filtering (`move_picker.cpp:553`). This can affect
   ordering and pruning; validate against a legal exchange oracle before change.
5. **The 240cp lazy-evaluation margin is not a proved feature bound**
   (`eval.cpp:128–137`). King danger and multiple feature terms can exceed it.
   Its comment says positional terms cannot change a cutoff, but the code does
   not establish that bound. Compare full/windowed evaluation and resulting
   cutoffs in forcing king-attack positions. No KingDangerTable damping is proposed.

Each investigation should first produce an executable failing regression, with
cleared/warm TT variants and color mirrors. Fix terminal truth before tuning an
approximate pruning margin to hide it. Do not bundle all five into one patch.

### P2: prune eligibility and meaningful search tests

Default NMP has no explicit non-PV, staticEval≥beta or consecutive-null guard.
The preserved `NMPGuards=true` experiment addresses those checks, but remains
**default false and unpromoted**. Its CTest, full reference perft and twelve
default-off matched-depth score/PV/node checks are useful correctness/parity
evidence, not a positive match result. RFP and ProbCut also lack explicit non-PV
eligibility in current predicates. Audit node types separately rather than
copying another engine's numbers. [Classical Stockfish 11 search reference](https://github.com/official-stockfish/Stockfish/blob/sf_11/src/search.cpp).

The 48 preserved Game 8 fixed-node ablations are unchanged. Turning off NMP/LMR
changes achieved depth and sometimes the selected move; it does not establish
that `...f5` is objectively best or justify disabling pruning globally.

`test_alphabeta_eval_equivalence` checks only a legal best move, not equality of
scores to an oracle. `test_singular_extension_search` checks only a returned move
and completed depth 7, not extension behavior. `test_move_ordering_reduction` is
a useful one-position node sanity check, not a general proof of ordering quality
or Elo. Furthermore, qsearch still probes/stores TT when normal search's `use_tt`
flag is false; a “TT-off” diagnostic is not fully TT-off. Strengthen these
contracts with a genuine conservative/reference search and targeted fixtures.

The implementation differs from older descriptions: history is halved during
normal **search preparation**, not between every root depth; no explicit
same-target recapture extension is present; move selection scores a full legal
list and selects its maximum, rather than a lazy staged generator. These are
facts to measure, not automatic reasons to change them. Full-list scoring/SEE
may cost work avoided by a staged picker, but profile before optimizing.

### P3: evaluation identifiability, endgames and actual runtime architecture

Material interaction literals 2/3/4/15/−25/12 remain in `eval_features.cpp:640`.
The two-minors-versus-rook interaction is absent. Both are valid model-development
opportunities, not proof that the particular `...Nxf2` game was lost solely from
linear piece values. Search can legitimately accept a small material disadvantage
for compensation. Analyse the full continuation and parameter sensitivity.

Preserve the pawn-cache boundary: pawn geometry alone in the pawn hash; piece
attacks, blockades and safe escape/control features outside it. Preserve color
parity, uncapped check-extension rule (`ply<64`), fixed KingDangerTable and
allocation-free recursive search. Current manifest's MG tempo is **15cp**, not
the original briefing's approximate 18; use the exact frozen build's baseline
for any future startpos-invariance test, not a remembered number.

`SyzygyTablebase::init` ignores its path and reads **no Syzygy files**. The current
recognizer handles terminal/dead-position results and selected immediately
provable draws, declining unproved wins. It is not a complete KPK/KBNK/WDL/DTZ
solver. This safer refusal is preferable to false exact cutoffs, but real
rule-aware endgame capability remains a substantive, separate engineering option.

Actual normal negamax/qsearch call `Evaluator::evaluate_fast`, with default
MasterPositional mode. The full tropical/spectral evaluator is not used by that
search path. Optimizing spectral matrix work would therefore not improve this
default path's search throughput or strength. Before judging any claimed
“model,” identify exactly which evaluator/options the tested binary used.

TT snapshots/private accounting, preallocated workers/traces and state-roundtrip
tests are genuine improvements already supported by preserved evidence. The
sampled six-thread TT accounting speedup did not increase average achieved depth;
do not relabel its NPS gain as an Elo gain.

## 6. Replacement gameplan — no launches authorized by this document

1. **Measurement definition, not another handicap marathon.** Choose one frozen
   reference build and declare engine-only vs book-assisted strength separately.
   Retain the recovery control, clock-only D and default-off NMP binary as separate
   identities. First provide a verified paired/pentanomial result analyser and
   run/config manifest contract. Validate its calculations on known synthetic
   outcomes, zero variance, saturation and incomplete pairs. No match is running.
2. **Deterministic regressions before another feature.** Start with terminal/qsearch
   and rule-50 fixtures above, strengthen the misleading test contracts and test
   each correction separately. Keep control and all failed evidence. No production
   code was changed by the present audit.
3. **Relative-strength test when separately approved.** Candidate vs frozen HG,
   identical CPU/hash/options, pondering/internal books off, full UCI history,
   independent diverse opening lines with reversed colors, idle hardware and
   immutable outputs. A practical primary configuration is one thread, Hash 64,
   60+0.6; a cheaper development screen may use 10+0.1 but is not a final strength
   claim. Predeclare a fixed sample or validated SPRT; do not change conditions or
   stop at a favorable score. Confirm successful changes at longer control and
   the actual six-thread deployment. A failed protocol/clock gate prevents promotion.
4. **External baseline after the measurement is trustworthy.** Use several
   unrestricted independently rated engine versions around the observed strength,
   not many handicap settings of one engine. Fix one published rating list/date/
   CPU configuration, calibrate hardware/conditions, fit pool results and include
   anchored/opponent uncertainty. If almost every game is won/lost, move the pool
   closer; saturation is poor measurement. Report a local estimate with conditions
   until independently tested; an official CCRL result is a separate process.
5. **Then make empirical improvements.** One justified search candidate at a time;
   small-group paired-game SPSA only after an independent acceptance pipeline;
   parameter-only exact-parity evaluation refactoring before credible-data Texel;
   endgame support and opening-book coverage in separate campaigns. No speculative
   per-feature Elo ranges, blanket pruning removal, new nonzero handcrafted weights
   or automatic promotion. Respect the classical/no-neural architecture throughout.

3000 is not architecturally ruled out by classical evaluation. It is **not yet a
measured 200-point gap**, nor is there evidence that a shopping list of heuristics
will close it. The immediate deliverable is an honest baseline and tests that
can distinguish a real gain from changed clocks, favorable handicaps and noise.
The engine is worth investigating; the old rating narrative is not worth repeating.
