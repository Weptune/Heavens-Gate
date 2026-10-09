# Recovery results and engine improvement plan

## Current result of the paired rule-50 screen

The approved [60-game paired screen](rule50_paired_screen.md) finished cleanly:
candidate **11 wins, 17 draws, 32 losses, 19.5/60 (32.5%)**, with 8,413 verified
legal plies and zero clock/protocol/system failures. It triggers the predeclared
safety alarm; the conservative statistical decision remains inconclusive.
**Do not promote the strict clock-tag candidate.** Tests passed, but strength
safety did not follow. Preserve both binaries and the full losing run.
The working source still contains the experiment; a fresh build is not a validated
release. The root production executable has not been replaced.

Next priority is an isolated audit of TT score eligibility and replacement cost,
retaining rule-50/mate-precedence regressions, not compensating through evaluation
weights. The code suggests broad cache exclusion and shallow different-clock
replacement as hypotheses; matched opening roots do not support a uniform depth
collapse. No new run or production change is automatic. All engines have exited,
and the scheduled monitor remains deleted. The sections below are historical
engineering stages, not instructions to restart old pilots.

## Latest approved follow-up: rule-50/TT correctness, no games — 2026-10-09

The separate [rule-50 and cached-score candidate](rule50_tt_candidate.md)
reproduces two linked defects with staged red tests, repairs the terminal/cache
contract and passes CTest 8/8, reference perft and mirrored real-UCI smoke.
The previous qsearch candidate is its preserved control, not overwritten.
Exact-clock score eligibility changes search trees; the short cost diagnostic
does not establish Elo or zero regression. Same-clock repetition-history effects
remain unresolved. Production binaries and evaluation/pruning/book parameters
are unchanged. Games remain stopped, the monitor deleted, strength validation
pending and promotion prohibited without the declared validation gate.

## Approved follow-up: one search correction, no games — 2026-10-09

The [qsearch stalemate candidate](qsearch_stalemate_candidate.md) reproduces and
repairs a terminal-scoring defect, with CTest 7/7, reference perft, allocation/
symmetry/state checks and preserved control/candidate evidence. A verified
paired-result analyser is now available. Evaluation/pruning/book parameters and
frozen production binaries are unchanged. Strength validation remains pending;
the cancelled pilot and deleted monitor have not been restarted.

## Current instruction and evidence reset — 2026-10-09

The user cancelled scheduled work and games to reassess skill measurement.
The monitor was deleted and Candidate D was deliberately stopped at 18/60
completed games, with game 19 unfinished. All outputs/binaries are preserved;
`build/recovery-clock-d/pilot-D-60/user-stop.json` records the cancellation.
The fresh [skill/data/code audit](engine_skill_audit.md) supersedes the earlier
execution order below. No clock pilot will be resumed or relaunched automatically.
No source/evaluation changes or new strength games were made during that audit.

2026-10-09. This plan follows the user's request to repair tournament interruptions
and improve Heaven's Gate. Frozen failed experiments remain evidence, not a pool
of favorable results. No neural evaluation, evaluation-weight changes, book
changes, or check-extension cap were introduced.

## What caused the interruptions

There were multiple failures, not one unexplained engine crash:

1. Pilot-B aborted on its first physical HG clock loss under the new fail-fast
   harness. Original C aborted on a Stockfish clock deadline misclassified as a
   protocol failure. C2 already corrected both continuation/classification paths.
2. C2 saved 16/60 games, then stopped recording during Game 17. Windows subsequently
   slept twice and rebooted. The last saved move precedes the first sleep by about
   15 seconds, so the precise original exit/hang mechanism is **not proven**.
   Replaying the outstanding position in fresh processes did not reproduce a hang;
   this is not a replay of the original SMP/TT state.
3. Independently, C2's HG clock reached 0.005ms at move 141 and genuinely flagged.
   The shared recovery policy allowed approximately 10% of a remaining bank on a
   difficult non-opening move (3.5/35). The older tournament policy's ceiling was
   approximately 4% (1.8/45). This operational change was not strength-tested.

Earlier score-based mate adjudication also shortened games artificially. Old
successful batches are not equivalent controls for truthful played endings and
native physical bank clocks. See [full preserved failure evidence](pilot_c_recovery.md).

## Implemented and checked

The current-source control is frozen in `build/recovery-tuning`. CTest 4/4,
perft 4 across six positions, and a real two-color mate-in-one runner smoke passed.
The frozen control's SHA-256 is
`362f60c6eec5bd86277b37b3ed49eb17774299861b17420f0b94975332b4f845`.

Clock/operational candidate D is separate, in `build/recovery-clock-d`, SHA-256
`e098fa77b09e1361ac11cd66fe1fcdd6693bf12762f2f6efc19dca60ed392bde`.
For zero increment and no moves-to-go, with bank B and overhead o:

```
reserve = min(B/2, 35*o)
available = B - reserve
soft = available/45
hard = 1.8*soft
```

Existing opening clamps and finite-bank/fractional caps still apply. Increment
controls retain the previous 35-move horizon/3.5 hard multiplier. No dead clock
is resurrected and increment is never credited after a late reply. Finite
zero-increment banks cannot support a guaranteed positive thinking floor for
arbitrarily long games; this is a conservative risk policy, not such a guarantee.

`src/core/tournament_runtime.hpp` scopes an idle-sleep inhibition request to the
run and restores prior state. It compares elapsed uptime with awake-only uptime
to identify suspension exceeding two seconds. A suspended game is unfinished,
not a fake draw or an engine clock loss. Manual sleep/lid actions and shutdown
can still interrupt the machine. See Microsoft's
[power-request contract](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadexecutionstate)
and [awake-time contract](https://learn.microsoft.com/en-us/windows/win32/api/realtimeapiset/nf-realtimeapiset-queryunbiasedinterrupttime).

Each search now has a flushed begin event **before** it starts, plus a matching
end event. Journaling stays outside search/charged reply intervals. `.run.json`
records normal completion, completion with flags, or an ordinary abort. The
external `tools/run_supervised_tournament.py` records actual child exit codes and
the outstanding search, and uses a scoped Windows job to contain only its own
engine process tree. Its deadline is `max(60s, 2*expected_reply_limit + 5s)`;
timeout kills the owned job and preserves all output, never invents a move or
restarts a run. Reboot cannot execute finalizers, but launch and begin events
remain available. [Windows job semantics](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects).

Candidate D passed CTest **5/5**, perft 4, and the budget test's 200-move adversarial
spending model (60/120-second banks, 2ms overhead, extra 40ms every 20th move).
Supervisor fixtures check nonzero exits, unrelated-process survival, stalled
child/grandchild cleanup, immutable outputs and suspension accounting.

A deliberate real CLI opponent crash returned code 1 and an `aborted` run with
the reason `Engine Failure: process-exited`; a deliberately illegal opponent
reply also returned code 1, reason `Engine Failure: illegal-move`, without a
fallback or successful completion. The real 10+0 two-game supervised
smoke completed both games and correctly returned code 1 for **one Stockfish
clock loss**, with **zero HG flags**. Verification reconstructed **283 legal
plies / 284 attempts**, paired colors/FENs and every bank/history command. Eight
HG searches at move 100+ had a minimum bank **792.957ms** and maximum reply
**17.867ms**. This is promising clock evidence, not proof across the full suite
or an Elo gain. See `build/recovery-clock-d/supervised-smoke/verification.json`.

## First isolated search candidate

The NMP source audit found no explicit non-PV, static-evaluation-at-beta or
consecutive-null guard in the entry condition. The verification-search comment
also overstated its scope: its excluded sentinel disables NMP at that node, not
throughout descendants. This is corrected documentation, not a new verification
algorithm. The classical [Stockfish 11 search source](https://github.com/official-stockfish/Stockfish/blob/sf_11/src/search.cpp)
is a primary reference for conservative NMP eligibility, not evidence that one
engine's margins should be transplanted into another.

`build/recovery-search-nmp` contains one experimental search change, enabled only
by `setoption name NMPGuards value true`. **Default false** preserves control
pruning. Alongside the original depth/check/material/exclusion conditions:

```
eligible = nonPV && !previousWasNull && finiteNonMateBeta && staticEval >= beta
```

`previousWasNull` is explicit, not inferred from an empty root move slot. A null
child clears the corresponding move/piece history slot and restores it afterwards.
Same-ply singular/null verification preserves predecessor state; a real-move
child resets it. No recursive heap storage was added. Check extensions remain
unchanged (`ply < 64`); RFP, ProbCut, LMR, singular margins and aspiration settings
are untouched. Candidate SHA-256:
`5b47133ed2d72b609de13358293b370aec291304865d96d054c84fdae3ff0d10`.

CTest **5/5** passed, including guards, checked/pawn-ending fixtures, exact board
restoration, legal best moves, node caps and allocation-free serial/SMP search.
All six perft references passed (startpos depth 6 **119,060,324**, other positions
through depth 5). Twelve serial depth-8 comparisons, six positions and their
color mirrors, match the clock control exactly in score/PV/nodes when guards are
off. With guards on, node counts change in both directions. This is **not** an
established performance/strength improvement. The exact snapshot change manifest
and `matched-depth.json` are preserved in its build directory. Game 8's previous
48 ablations were not rerun and do not prove that `...f5` was objectively best.

## Evaluation and data audit

The compiled tuner passes exact baseline parity, local unrounded slope tests and
its train/validation/test contract. Production coefficients are unchanged:
`6e53015a967ca8e8ae93d99fdaf279155ff79d038e8337bedf3e89a97d745b97`.

Fresh audit outputs live in `build/recovery-tuning/dataset-audit`. The extractor
verified 5,109 whole-game occurrences, removed 2,530 duplicate games, and rejected
169 games with moves after a terminal board, 110 nonterminal/wrong results, 40
untrusted endings, 32 truncated comments/variations, and other malformed headers
or missing results. It excluded 9 conflicting canonical-position groups.

| Split | Samples including color mirrors | Distinct sampled games |
|---|---:|---:|
| Training | 10,588 | 1,454 |
| Validation | 1,124 | 175 |
| Test | 1,334 | 197 |

Hashes, game-level and exact/color-mirror position isolation passed independent
wrapper validation. There are **6,523 original positions**, not 13,046 independent
positions. Game-hash isolation does not establish independence of related
openings, paired runs or engine families. Original-position phase coverage is
850 early / 2,668 middle / 3,005 late; an exploratory surplus-two-minors-versus-
rook predicate fires in 251 positions. Almost all players are HG/Stockfish or
HG/Baseline Engine. These are **local engine games**, not grandmaster data. A
static quiet filter is also not a proof that a position is tactically resolved.

No general-purpose Texel fit or production export was run: diversified trusted
data and opening/source-family holdouts are still needed. The old hardcoded
material-imbalance coefficients really remain in `eval_features.cpp`; parity
repair makes them fixed residuals, not tuned coefficients. See
[pipeline contract](tuning_pipeline_recovery.md).

## Prioritized route to stronger classical play

Expected Elo gains are unknown until independently measured. They cannot be
added together to promise 3000. The handicapped Stockfish scores do not establish
the claimed 2780–2800 CCRL baseline either. Strong classical search is compatible
with this architecture; Heaven's Gate's actual ceiling is unproven.

1. **Define an honest strength measurement first.** D is cancelled, not awaiting
   automatic completion. Its preserved prefix has 18 played mates, 1,707 legal
   plies, zero observed flags and 14 HG move-100+ searches with at least 4,456ms
   starting bank. This is useful operational evidence, not a completed 60-game
   gate or a rating. Prepare paired/pentanomial analysis and targeted search
   correctness regressions before separately approved candidate-vs-HG strength
   testing. See `build/skill-audit-20261009/pilot-prefix-and-comparison.json`.
2. **Test NMP alone against the clock-only control.** Use independent paired
   openings, equal books/hash/threads and frozen hashes. Start with a small
   operational screening; do not promote on that score. Before a full strength
   run implement paired/pentanomial sequential testing with H0=0 Elo, H1=+5 Elo,
   alpha=beta=0.05 and log-likelihood boundaries ±log(19), or predeclare a
   sufficiently powered fixed-size paired confidence interval. A gate pass is
   not a zero-regression theorem. Require a separate longer-time-control check.
   Search improvements may cost depth/NPS; only played-game strength decides.
3. **Audit qsearch correctness next, not all pruning at once.** Current SEE
   pruning exempts checking captures, but the subsequent delta-pruning predicate
   has no matching check exception. Also inspect qsearch stalemate handling,
   rule-50 dependence of TT cutoffs, and same-ply verification history lifetimes.
   Reproduce a failure with a deterministic regression before making the next
   isolated candidate. Do not damp king danger or cap check extensions to make
   a tactical test pass. Files: `search.cpp`, `tt.*`, targeted tests.
4. **Replace search parameter guessing with paired-game SPSA.** The runner and
   tuner now have correct played-game fitness and immutable candidates. First
   fix source/opening-family isolation and add sequential-test analysis. Tune
   small groups (e.g. LMR divisor/history modulation; aspiration widening) in
   separate campaigns, retaining an independent acceptance match. SPSA gradient
   uses score differences divided by each actual clipped/rounded probe span,
   not a 10ms STS score. Files: `tune_search_spsa.py`, `paired_match.py`, acceptance
   analyzer, `search_params.*`. Full SPSA has not been launched.
5. **Make evaluation globally tunable before adding features.** Move existing
   imbalance literals into parameters with their exact present values, then
   require score-exact parity before tuning. Implement shared feature extraction
   between runtime and tuner so counts/signs/phase taper cannot drift. Cache
   pawn-only features on pawn bitboards; piece attacks/placement stay in piece
   activity, outside the pawn hash. Files: `eval_params.hpp`, `eval_features.cpp`,
   `eval.cpp`, `tune_classical_texel.cpp`, mirrored/parity tests. Do not change
   coefficients in a refactoring commit.
6. **Add zero-initialized, identifiable evaluation features only with data.**
   For the material interaction let M(c)=N(c)+B(c), ΔM=M(c)-M(~c), and
   ΔR=R(~c)-R(c). A sparse feature is
   `x(c)=min(floor(max(ΔM,0)/2),max(ΔR,0))`, gated on equal queen counts.
   Its contribution is `wMG*(x(W)-x(B))` and `wEG*(x(W)-x(B))`, tapered by phase.
   This encodes the interaction, not a manually assumed 50–100cp exchange bonus;
   existing material/mobility still account for linear values and activity.
   Compare against a regularized second-order material matrix rather than
   blindly adding both correlated models. The classical
   [Stockfish material model](https://github.com/official-stockfish/Stockfish/blob/sf_11/src/material.cpp)
   provides a primary reference for a second-order interaction matrix, not
   coefficients to copy. A general matrix has
   `I(c)=Σ_i n_i(c) Σ_j[A_ij*n_j(c)+B_ij*n_j(~c)]`, evaluated as I(W)-I(B).
   Freeze one representation and tune globally, checking sparse feature counts,
   covariance/regularization, mirrored parity and startpos's existing tempo.
   Candidate passer, rook-behind-passer, king-pawn-distance, and opposite-bishop
   scaling are subsequent separate features, not missing-feature shopping lists.
   Opposite-bishop scaling acts on the winning side's endgame advantage only;
   it must not erase king attacks when majors remain. All new weights start at
   zero, and candidate ranges/acceptance are declared before fitting. Trapped
   queen/infiltrator features require SEE/safe-escape evidence, not a penalty for
   being deep in enemy territory. No fit may select weights on the test set.
7. **Opening book is a separate product feature.** Audit legal PolyGlot keys,
   both color coverage and line provenance; add Black Berlin/QGD branches only
   after engine-only strength controls exist. Book-assisted match results are
   not search gains or CCRL engine ratings. Do not edit `performance.bin` during
   a search/time-manager experiment.

The previous short STS gate failure remains visible. We have not claimed all
five strength gates pass or rerun tests until a favorable tactical score appears.
Current checks establish sampled legal state, operational instrumentation and
candidate isolation. Strength validation is pending; the full D clock pilot was
deliberately cancelled and is not scheduled for completion.
