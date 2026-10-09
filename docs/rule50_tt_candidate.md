# Rule-50 and cached-score correctness candidate

Subsequent [paired strength screen](rule50_paired_screen.md): the candidate
scored **19.5/60 (32.5%)**, 11 wins/17 draws/32 losses, in a clean verified
10+0.1 single-thread run. The conservative primary inference is inconclusive,
but the predeclared safety alarm triggered. **Do not promote.** The correctness
results below remain valid; they did not establish strength safety. Both frozen
binaries and all losing games are preserved. TT policy costs need investigation.

2026-10-09. Approved deterministic engineering follow-up to the
[qsearch stalemate correction](qsearch_stalemate_candidate.md). Games remain
stopped and the scheduled monitor remains deleted. No tournament, SPSA/Texel
fit, book change, production binary replacement or promotion was performed.

## Outcome and identity

Two linked search defects are reproduced and corrected: rule-50 terminal
handling, and reuse of score bounds at a different reversible-move clock.
This improves a proven correctness contract; its playing-strength effect is
unmeasured. This is not a complete TT/history correctness proof.

Control: frozen `build/recovery-qsearch-stalemate/heavensgate.exe`, SHA-256
`0b5de34e941957d7eb051fed74ff465af7cf75f5178ddfb2ddd0598b2a6a1cc6`.
Candidate: separate `build/recovery-rule50/heavensgate.exe`, SHA-256
`e586e75297ba4c0772a9cb3fdaa6dac592fb761ef30e91d7dd3de57d298f63fb`.
Compiled-source SHA-256:
`57d0419e02000c2a394c911c407deef216736aa5b74029f67e11ea59c30bd8fe`.

The candidate includes the previous stalemate fix as its unchanged baseline.
Its exact eight-file delta is in `build/recovery-rule50/change-manifest.json`;
that comparison does not attribute unrelated dirty-worktree changes to this work.
NMP/LMR remain enabled; experimental `NMPGuards` remains false.

## Reproduce, isolate, repair

The principal fixture has seven pieces, avoiding the small-endgame recognizer:

```
7k/8/8/8/8/8/3Q4/KRNBN3 w - - 99 1
```

It has only quiet legal moves and no mate-in-one. At clock 99 every continuation
reaches clock 100. Under the engine/runner's existing take-available-claims
policy, the value is therefore 0. The color mirror reproduces the defect.
At clock 100 the original qsearch returned +2470cp, depth-2 negamax +2564cp,
or a warm TT's seeded +900cp. At clock 99 root/negamax searches returned
+2673cp. A claimable root with pawn-reset moves also returned +71cp instead
of the current policy's draw. A mate at clock 100 could be hidden by a warm
TT or qsearch's hard-depth fallback.

The first five regression groups compiled against unchanged engine code:
**one passed, four failed**. After terminal handling alone, **four passed,
one failed**. That remaining failure used actual cached search results:

- Clock-0 winning score +2673cp was reused at clock 99, where the result is 0.
- Clock-99 draw 0 was reused at clock 0, whose fresh search scored +2673cp.
- Clock-99 full-search draw also contaminated clock-0 qsearch (+2470cp fresh).

This staged reproduction distinguishes the terminal defect from the cache
context defect. Before/intermediate binaries and both failing logs are preserved.
Final tests include an additional TT metadata/replacement group: **six pass**.

## Correction and invariants

In `src/search/search.cpp`, qsearch and negamax now check clock >=100 before
TT bounds, stand-pat, ordinary pruning and the qsearch depth fallback. Checked
positions use exact legal-existence testing: already-checkmated positions retain
mate-distance scores; checked positions with legal replies use the draw policy.
Alpha-beta and iterative-deepening roots apply that same policy before their
internal book/search paths. Claimable roots return a legal fallback move with
completed depth 0, not a fabricated completed search. Mate/stalemate roots return
no move.

In `src/search/tt.hpp/.cpp`, the entry's spare byte records the exact halfmove
clock. A score is eligible only when `stored_clock == board_clock < 100`.
Incompatible entries may still order a move, but cannot supply score cutoffs,
static-evaluation refinement or singular score/depth metadata. All four search
stores pass the actual clock. A new clock context replaces the old same-position
entry regardless of its deeper depth; an absent new move can retain its legal
position-based hint. Only one cached clock context per board key is retained.

Repetition Zobrist identity, TT indexing/prefetch, locks, private statistics and
generation policy are unchanged. Entries remain **16 bytes**, clusters **64 bytes**.
TT hit statistics still count matching board keys, including hint-only matches,
not just score-compatible hits. No new search-path allocation is introduced.
Evaluation weights, pawn hashing, KingDangerTable, uncapped `ply < 64` check
extensions, RFP/ProbCut/LMR/NMP parameters, time policy and books are untouched.

This deliberately retains the runner's existing 50-move-claim convention, not
a new assertion that FIDE automatically ends every game at 50 moves. The
[classical Stockfish 11 draw check](https://github.com/official-stockfish/Stockfish/blob/sf_11/src/position.cpp)
likewise distinguishes an already-checkmated position from its rule-50 search
draw handling. No external implementation was copied into the engine.

## Verification and cost

- CTest **8/8**: AllTests, QsearchContracts, Rule50Contracts, TexelParity,
  TexelTrainingContract, TuningTools, TournamentSupervisor, PairedStatistics.
- Six rule-50 groups cover mirrored boundary windows/cold and warm TT,
  mate precedence, checked legal replies, quiet crossings, root claims,
  mate on halfmove 100, real clock-context contamination, compatible
  Exact/Lower/Upper bounds and incompatible-bound rejection/replacement.
- Pawn moves, captures, EP and promotions reset to zero. Make/unmake restores
  the board. Synthetic null moves do not advance the clock or manufacture
  repetition. A reversible knight cycle changes the clock while retaining
  board identity and repetition recognition.
- Existing concurrent TT snapshot stress also verifies the new clock byte.
  Existing move-ordering, serial/SMP zero-heap, evaluation symmetry/cache,
  state restoration, timing and protocol regressions pass.
- All six reference perft suites pass to their available depths: startpos and
  position 3 through depth 6; the other four through depth 5. Startpos depth 6
  is **119,060,324**. This is not six depth-6 reference tests.
- Real UCI smoke: **10 mirrored/original depth-1 cases** pass, with books off,
  one thread, Hash 64. The first smoke's assertion assumed terminal replies
  would be `None`; the client represents `bestmove 0000` as `Move.null()`.
  Correcting the test required no engine change. Its failed output is preserved.

Twelve ordinary original/mirrored positions at depth 8, three repetitions each,
are deterministic within each binary. Across binaries, 8/12 scores and 6/12 PVs
match; **none** preserves score/PV/node counts together. Tighter score eligibility
changes selective-search decisions even away from the 50-move boundary. This
is not parity evidence and does not identify which ordinary result is better.

Sampled median candidate/control node ratio is **1.0066** (range 0.6650–1.1822),
elapsed ratio **0.9606**, seconds/node ratio **0.9798** (range 0.8183–1.2694).
These short serial wall-clock samples are noisy and compare different trees.
They show no clear aggregate cost in this small sample, **not** a speed gain,
zero-regression guarantee or Elo gain. No competing match was running.

## Remaining limits and next strength gate

Exact clock tags do **not** solve graph-history interaction: identical boards
at identical clocks can have different repetition histories and future draws.
Other unmodified limits include history/stack-exhaustion fallbacks, checkmate
below clock 100 at qsearch's hard-depth fallback, and diagnostic minimax.
The UCI layer's separate book shortcut can bypass the search-root guard with
`OwnBook=true`. Claimable-root early returns emit `bestmove`, but no UCI score
info; the smoke checks that reply, not a nonexistent reported score. Approved
measurement remains books off. These limitations are explicit, not hidden by
the green test count.

Evidence in `build/recovery-rule50/` includes the staged failing binaries/logs,
final build/test/perft logs, fixed-depth diagnostic, both smoke outputs, source
and tooling snapshots, change manifest and `artifact-verification.json`.
Artifact verification checks compiled-source identity, snapshot/live agreement,
deterministic repeated diagnostics, exact file deltas and unchanged root,
qsearch, NMP, clock-D/C2 binaries and tuned evaluation parameters.

No automatic promotion is justified. The next strength decision needs a
**separately approved, predeclared candidate/control paired campaign** with
diverse independent opening families, equal resources/options, fixed hashes,
books/ponder off, a fixed-sample or validated sequential rule, and complete
legal/clock/result evidence. Compare this candidate to its frozen qsearch
control first so the new change is identifiable; do not bundle NMP guards,
evaluation fitting or book updates. A small pilot can screen major damage,
not prove a tiny gain. Absolute CCRL-style calibration remains separate and
requires an unrestricted rated-opponent pool under stated conditions.
