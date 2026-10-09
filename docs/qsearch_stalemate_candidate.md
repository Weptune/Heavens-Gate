# First evidence-backed search correction and measurement tooling

Subsequent work is preserved separately in the
[rule-50/TT-clock candidate report](rule50_tt_candidate.md). This binary and its
evidence remain frozen as that investigation's control; no strength promotion
has occurred. The original results below are unchanged.

2026-10-09. The user approved continuing after the
[skill/data/code audit](engine_skill_audit.md). Games remain stopped and the
scheduled monitor remains deleted. This work did not start a tournament, tune
evaluation, change a book, replace the root executable or promote a candidate.

## Outcome and frozen identity

**A real qsearch defect is reproduced and corrected. Elo improvement is unmeasured.**
The paired-match analyser verifies artifacts and reports paired statistics;
it does not supply a retrospective SPRT decision or an absolute CCRL rating.

Candidate: `build/recovery-qsearch-stalemate/heavensgate.exe`.
Binary SHA-256:
`0b5de34e941957d7eb051fed74ff465af7cf75f5178ddfb2ddd0598b2a6a1cc6`.
Compiled-source SHA-256:
`68bc47aed97a96febc34d360197b394bad0061ba5ffe6728a8d9571d781788c7`.
Control: frozen `build/recovery-search-nmp/heavensgate.exe`, with
**NMPGuards=false**. No NMP changes are bundled in this correction.

## Reproduce before repair

Direct qsearch tests bypass the main search's small-endgame recognizer, which
otherwise can conceal a leaf defect. At this legal stalemate:

```
k7/2Q5/2K5/8/8/8/8/8 b - - 0 1
```

Original qsearch returned −1047cp in a full window, −1500cp from a stand-pat
cutoff, −985cp in a high window, or a seeded cached exact score of +900cp.
The correct terminal value is 0, irrespective of the window or a cached
nonterminal score. The color mirror reproduced the failures. Before-fix tests
covered plies 0/7/63, three windows and cold/warm TT.

The parent `k7/2pQ4/5K2/8/8/8/8/8 w - - 0 1` has one legal tactical capture,
`Qxc7`, which stalemates. Original qsearch raised its 917cp stand-pat to 1058cp
by treating that draw as a material gain. Corrected qsearch leaves stand-pat
at 917cp: the capture continuation is worth 0. **This is a qsearch contract,
not a claim that the parent's true minimax/game-theoretic value is 917cp.**

Preserved `qsearch-before-tests.exe` and `qsearch-before.log` show two failing
groups and one passing checkmate/quiet group. Older binaries were not rebuilt.

## Exact correction, not an evaluation heuristic

`src/search/search.cpp` tests non-check legal mobility before qsearch's
TT/stand-pat cutoffs and static depth fallback. Stalemate returns `ScoreDraw`.
`src/movegen/` supplies allocation-free `has_legal_move`, sharing the legal
filter and pin calculation. An unpinned pawn push or non-king piece move proves
mobility without constructing a full move list. Checked positions, king moves,
pins, pawn captures and EP retain exact filtering, stopping at the first legal move.

No evaluation weight, KingDangerTable, pawn-cache term, check-extension policy,
RFP/ProbCut/LMR/NMP setting, time policy or book changed. The existing history-
capacity safety fallback remains. This is not a complete terminal/rule audit:
checkmate at the hard qsearch depth fallback, general rule-50/history-aware TT
semantics and checking-capture delta pruning remain separate investigations.

## Verification and local cost

- CTest **7/7**: AllTests, QsearchContracts, TexelParity, TexelTrainingContract,
  TuningTools, TournamentSupervisor and PairedStatistics.
- Qsearch groups **4/4**: mirrored stalemate at plies 0/7/63/64, three windows,
  warm/cold TT; capture into stalemate; preserved mate-distance/quiet scores;
  legal-existence equivalence and exact board restoration.
- Legal-existence agrees with full generation and an independent make/check/
  unmake oracle over **6,762** fixture/tree positions, including castling,
  promotion, check, pins, legal EP and EP exposing a rook check.
- All six reference perft positions pass to their available reference depths:
  startpos and position 3 through depth 6, others through depth 5.
  Startpos depth 6 = **119,060,324**. This is not six depth-6 reference tests.
- Existing move-ordering reduction, serial/SMP zero-heap-allocation, symmetry,
  state-restoration, timing and protocol regressions pass.
- Twelve original/mirrored fixed-depth-8 positions, three repetitions each:
  **36/36** preserve exact score, PV and node count against control. These
  ordinary positions do not substitute for the targeted failing leaf test.

The initial full-list mobility check sampled a median seconds/node ratio of
**1.0673** versus control; the exact early-exit implementation sampled **1.0008**.
Optimized per-position ratios ranged approximately 0.866–1.260. These short
wall-clock samples are noisy: they suggest the main overhead was removed but
**do not prove zero slowdown or any Elo gain**. No match was running during builds
or diagnostics.

## Measurement tooling without new games

`tools/analyse_paired_match.py PREFIX --output NEW.json` reads the manifest,
summary, PGN and telemetry produced by `tools/paired_match.py`. It starts no
engines. It checks binary hashes, exact count/FEN/color pairs, legal moves,
board/rule endings, duplicate headers and header/footer result consistency,
clock/increment/history telemetry, and clock/protocol failures. Missing pairs,
extra/missing telemetry, changed binaries and repeated opening positions fail closed.

Pair total points 0/0.5/1/1.5/2 produce five pentanomial counts. Uncertainty uses
one observation `X=(points_white+points_black)/2` per pair, not two independent
games. Descriptive logistic difference is `400*log10(p/(1-p))`; saturated
scores have no finite estimate and are not capped at an invented +800.

Conditional intervals require **independent representative opening pairs and a
stopping rule declared before play**, neither established by these files alone.
A normal approximation is omitted for small/zero-variance samples. A conservative
bounded-pair interval retains nonzero uncertainty for identical pair scores.
Related opening-family dependence and retrospective stopping remain unresolved.
No acceptance decision is made. A sequential campaign needs a validated SPRT,
not repeated inspection of fixed-sample intervals. See
[Stockfish's testing guidance](https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-FAQ.html).

Ten synthetic test methods pass, with multiple negative fixtures; none starts
engines. Read-only replay verified the existing recovery-tuning mate-in-one
smoke: two checkmates, one reversed-color pair, 50% score. It is **inconclusive**:
one pair gives no variance estimate; the bounded score interval is [0,1], not
proof of equal strength.

## Evidence and next gate

`build/recovery-qsearch-stalemate/` preserves before/final test logs and binaries,
build/perft logs, both fixed-depth diagnostics, strict smoke analysis, complete
candidate source snapshot, separate tooling snapshot, `change-manifest.json`
and `artifact-verification.json`.

The comparison also records a **pre-existing supervisor-script change** between
the old NMP snapshot and today's source. It is not this turn's change and is not
linked into the engine. Artifact verification classifies it separately and checks
the compiled source fingerprint, unchanged root/clock-D/C2/NMP binaries and
unchanged tuned evaluation parameters.

This candidate awaits a **separately approved** paired strength campaign, not
production promotion. Declare frozen control, diverse opening families, equal
hardware/options, clocks, book/ponder policy and a fixed-sample or validated
sequential decision rule before launch. A small pilot screens large damage; it
cannot establish a tiny gain. The next safe engineering investigation is rule-50
and TT/history contracts, one reproduced defect at a time, separately from this
candidate. Absolute strength still requires unrestricted rated-opponent pool
calibration under stated conditions; old handicap scores do not supply it.
