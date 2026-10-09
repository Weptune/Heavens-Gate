# Tuning pipeline recovery and next isolated search experiment

## Current status — user-requested measurement reset

2026-10-09: scheduled monitoring and games are stopped, with outputs preserved.
No tuning or strength campaign is running or scheduled. The existing tooling
verification remains evidence; do not repeat heavy diagnostics or fit local
data automatically. [The fresh skill audit](engine_skill_audit.md) rechecked
split/source hashes and also found that the legacy 77,455-row file contains only
40,092 exact/color-canonical positions and lacks game/player provenance. Current
measurement priorities supersede the historical execution sequence below.

Updated 2026-10-09. The user approved this work after the Candidate C2 launch.
This is an engineering pipeline, not a strength claim or production promotion.

## Current verification status

Candidate C2 stopped with 16/60 completed games; its failed completion gate and
HG move-141 clock loss remain preserved. See [pilot recovery](pilot_c_recovery.md).
No timed engine process remained while the following post-pilot work ran.

The offline tuner and engine compiled into the new `build/recovery-tuning`.
CTest **4/4 passed** (AllTests, TexelParity, TexelTrainingContract, TuningTools),
all six perft positions passed through depth 4, and the real paired-UCI fixture
reached legal mate in both engine colors. The whole-corpus audit and independent
hash/split-isolation checks completed: train **10,588 samples / 1,454 games**,
validation **1,124 / 175**, test **1,334 / 197**, including color mirrors. Local
engine data is not a diverse grandmaster corpus; no general-purpose fit or
production coefficient export was run. Production evaluation remains unchanged.

The clock-only D candidate and one default-off NMP-eligibility experiment are
separately built and tested. NMP default-off score/PV/node parity passed on twelve
serial matched-depth cases. Neither has been promoted for strength. See
[implementation evidence and prioritized plan](engine_improvement_plan.md) for
exact hashes, frozen controls, corpus rejection reasons, candidate manifests and
the fresh D clock pilot's declared acceptance. Do not build, search, benchmark,
scan the full corpus or tune while any timed pilot is running.

## Implemented tools

`tools/extract_quiet_dataset.py` now requires explicit PGN inputs and a fresh output
directory. It validates the entire played mainline before retaining any sample.
Illegal/unknown moves, inconsistent results, unfinished games, flags, protocol
failures and forced-score adjudications cannot become game-outcome labels. Its
initial policy requires played-board or actually claimable rule endings; even
legitimate resignations are excluded until a trusted provenance policy exists.
SAN and the harness's coordinate notation are both accepted.

Splits are deterministic by whole-game hash (80/10/10). Global exact-position and
color-mirror deduplication removes overlap across splits. Conflicting outcome
labels for the same canonical position are excluded as ambiguous. A retained
sample and its color mirror stay together. Source hashes, game/player identity,
sample metadata, rejection reasons, split counts and file hashes are preserved.
Local engine games are **not** relabeled as grandmaster games.

The default static quiet filter excludes checks, the preceding capture/promotion,
and positions with a legal capture/promotion. It is not a qsearch filter or proof
that the position is tactically resolved. Sparse/biased local data is useful for
pipeline auditing, not sufficient by itself for a general evaluation fit.
`training_ready` means only that all three partitions are nonempty. Review distinct
game counts, label/material/phase coverage and provenance before optimizing.

`tools/run_texel_candidate.py` validates provenance, input hashes and split
isolation, then runs the offline tuner with explicit candidate/report outputs.
It records commands and binary hashes and checks that the production parameter
file did not change. The C++ tuner trains only on the training partition, selects
checkpoints using validation loss, restores the best checkpoint, rounds weights,
and exports only if the rounded model still improves validation MSE. The test
partition is reported, not used to select epochs. Sigmoid K, learning rate,
patience and thread count are explicit. Every output directory must be new.

The prior runtime parity repair remains: fixed residual equals full runtime
White-perspective evaluation minus the modeled terms at baseline weights. Pawn
MG stays anchored at 100. The optimizer's smooth model is not exact integer
runtime evaluation at every future weight vector; candidate parity/symmetry and
actual game validation are still required. No new manually weighted evaluation
feature, KingDangerTable damping or pawn-hash dependency was introduced.

`tools/paired_match.py` plays actual HG-versus-HG games from each FEN with both
engine colors, books off, fixed threads/hash and full move histories. Clock
expiry, crashes, illegal replies and unfinished games invalidate fitness and
preserve the PGN/trace/manifest. Increment is added only after a timely legal move.
Candidate options cannot override the match's threads/hash/book policy.

`tools/tune_search_spsa.py` now uses paired played-game scores against a frozen HG
control rather than a 10-ms STS score. Plus/minus probes use the same FEN/color
pairs and the same control. Updates use signed, actual clipped/rounded probe
spans in normalized coordinates. Binary changes invalidate the run. It exports
candidate UCI settings as JSON; it never rewrites `search_params.hpp` or selects
a release by its highest noisy STS score. Full SPSA has **not** been run.

## Reproduction commands (checks already completed)

Preserve frozen controls and existing build directories. The commands below
document the completed checks, not permission to rebuild frozen controls or
overwrite extraction/smoke outputs. For another build choose entirely fresh paths:

```powershell
python tools/verify_pilot.py build/recovery-clock-c2/pilot-C2-60.pgn
cmake -S . -B build/recovery-tuning -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=C:/Users/abhin/heavensgate/tools/w64devkit/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/Users/abhin/heavensgate/tools/w64devkit/bin/mingw32-make.exe
cmake --build build/recovery-tuning --parallel 6
ctest --test-dir build/recovery-tuning --output-on-failure
.\build\recovery-tuning\heavensgate.exe perft 4
```

CTest includes AllTests (including move ordering and clock regressions),
TexelParity, TexelTrainingContract, and TuningTools when Python is available.
The Python tools require `python-chess`. Their small mocked/fixture regressions
can also run independently without engine processes:

```powershell
python -m unittest discover -s tools -p 'test_*tuning*.py' -v
```

Run the real runner's small mate-in-one fixture in a fresh output location:

```powershell
python tools/paired_match.py --candidate build/recovery-tuning/heavensgate.exe --control build/recovery-clock-c2/heavensgate.exe --openings tests/paired_smoke_fens.txt --output-prefix build/recovery-tuning/paired-smoke --bank-ms 10000 --increment-ms 100 --threads 1 --hash 64 --max-plies 10
```

Verify both paired games reach played checkmate, candidate colors reverse, the
same FEN is used, traces agree with the PGNs and hashes match the supplied binaries.
This fixture is a plumbing check, not an Elo test.

Audit available PGNs without overwriting the legacy `data/quiet_positions.txt`:

```powershell
python tools/extract_quiet_dataset.py pgn_history build/recovery-clock/input-B/pilot-B.pgn build/recovery-clock-c2/pilot-C2-60.pgn --output-dir build/recovery-tuning/dataset-audit
```

Resolve actual source filenames first; pass only existing, finalized PGNs.
Report rejected endings and usable distinct games. Missing/empty holdouts return
code 2 and preserve the audit. Do not weaken validation to force a training run.
Only after data-quality approval, candidate fitting would use:

```powershell
python tools/run_texel_candidate.py --binary build/recovery-tuning/heavensgate_texel_tuner.exe --dataset-dir build/recovery-tuning/dataset-audit --output-dir build/recovery-tuning/texel-candidate --threads 1
```

A lower validation MSE permits export, **not** promotion. Test loss must be
reported even if worse. Do not repeatedly use the test split to choose candidates.
Acquire sufficiently diverse, trustworthy data before general-purpose retuning.

## Next search candidate: null-move eligibility, not global pruning removal

The frozen control's NMP entry condition in `src/search/search.cpp` checks enablement,
depth, check state, excluded move and non-pawn material. It does not explicitly
require a non-PV node, static evaluation at least beta, or a non-null predecessor.
The audit confirmed these omissions; a previous empty move at the
root is not automatically evidence of a null move. The Game 8 ablation does not
prove that `...f5` was wrongly pruned.

These checks are now implemented: explicit previous-null state and the three
eligibility guards are exposed only as experimental `NMPGuards` (default false).
Targeted guards, board restoration/node limits, check/pawn endings, color mirrors,
no-heap serial/SMP tests, AllTests, parity and full reference perft passed. The
matched-depth diagnostic establishes default-off control parity, not strength.
Check extensions, KingDangerTable, RFP, ProbCut, LMR, aspiration, evaluation and
the book remain untouched by this candidate. Paired-game strength validation
remains pending; no automatic promotion or full SPSA campaign was launched.

## Acceptance discipline

The five gates are prerequisites, not a mathematical guarantee of zero regression.
The existing short STS failure stays visible; do not cherry-pick repeat runs to
erase it. STS/BK and Game 8 are diagnostics, not training fitness or Elo estimates.

Predeclare an independent paired-opening strength test before expensive matches:
frozen binary hashes, equal threads/hash, books off, deterministic settings where
possible, fixed time control and a stopping rule. Analyze opening pairs rather
than pretending the two colors are independent samples. Use an appropriate
paired/pentanomial SPRT or fixed-size paired confidence interval; preserve losing
runs and account separately for clock/protocol failures. Final acceptance should
include a longer-time-control check, not just the tuning time control.

No full SPSA campaign or new expensive tournament is automatically launched by
this implementation. Its output remains a candidate until independent evidence
supports promotion. Neither handicap scores nor opening-book gains establish a
3000 CCRL rating. The route toward that target is repeatable measurement followed
by small independently verified search improvements and credible-data tuning.
