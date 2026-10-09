# Rule-50 candidate paired regression screen

## Result and decision

The completed screen scored **19.5/60, 32.5%** for the rule-50/TT-clock
candidate: **11 wins, 17 draws, 32 losses** against its frozen qsearch control.
All 60 games and **8,413 played plies** passed independent legal/result/clock
verification. There were **zero time forfeits, illegal replies or protocol
failures**, no suspend/watchdog event, and empty stderr. The runner exited 0.
Binary, plan and tooling hashes are unchanged. Production remains unchanged;
**do not promote this candidate**.

The working source still contains this experimental strict-clock implementation.
A fresh build of that source is **not** a strength-validated release. The root
production executable was not replaced, and the losing candidate was not silently
rolled back or hidden.

The observed score triggers the predeclared sub-40% descriptive safety alarm.
The primary conservative 95% paired score interval is **7.70%–57.30%**, so
its predeclared formal decision is **inconclusive**, not a confirmed loss of
exactly 127 Elo or proof of non-regression. The secondary normal approximation
is **23.06%–41.94%**, a worrying negative signal under the same unproven opening
independence/representativeness assumptions. It does not replace the primary
rule after seeing results. Pentanomial counts for pair points 0/0.5/1/1.5/2
are **[8, 9, 10, 2, 1]**. The candidate scored 14.5/30 as White and 5/30 as
Black; that split is descriptive, not evidence of a mathematical color defect.

This is a valid relative comparison under **10+0.1, one thread, Hash 64**, not
CCRL calibration or six-thread blitz confirmation. Green correctness/perft
tests and cheap fixed-depth timing did not establish playing-strength safety.
The same run continued across the chat interruption; it never stopped or
restarted. It ran from **15:58:15 to 16:31:54 UTC** on 2026-10-09, approximately
33 minutes 39 seconds. All engines have exited and no scheduled monitor exists.

2026-10-09. The user's `yes` approved a candidate/control paired comparison
after the isolated [rule-50/TT-clock correction](rule50_tt_candidate.md).
This is a fresh experiment, not a restart/resumption/pool of earlier pilots.
The old scheduled monitor stays deleted; no new scheduled task was created.

## Frozen plan before play

Plan: `build/rule50-paired-screen-20261009/plan.json`, SHA-256
`f9ae25c41621c76533976c14af4fdae8a7c4c23c0d6e65c85d7048e598f7cd1c`.
It was written at 15:57:34 UTC before launch. It declares all resource,
validity, failure, stopping and decision rules. No production promotion is allowed.

Candidate is `build/recovery-rule50/heavensgate.exe`, SHA-256
`e586e75297ba4c0772a9cb3fdaa6dac592fb761ef30e91d7dd3de57d298f63fb`.
Control is `build/recovery-qsearch-stalemate/heavensgate.exe`, SHA-256
`0b5de34e941957d7eb051fed74ff465af7cf75f5178ddfb2ddd0598b2a6a1cc6`.
The comparison isolates the rule-50/clock-tag delta on top of the qsearch fix;
it is not a test of both fixes against the original production executable.

Exactly **30 opening pairs / 60 games**, both colors per FEN, serial games,
**10+0.1 seconds**, one thread and Hash 64 per engine, books and ponder off.
Advertised defaults were checked equal in fresh real UCI sessions before play.
NMP/LMR stay enabled and experimental NMPGuards stays false. No strength handicap,
engine score adjudication, evaluation fit or other search change is included.

The external positions come from Stockfish's
[official testing-books repository](https://github.com/official-stockfish/books),
`UHO_4060_v4.epd.zip`, pinned to commit
`65815ccdbc7727cd4f6aee252ba8f67fb740e92f`.
Archive hash `a97424c5b98b42f8c27ff450f0681ad11696148548c975752350e98417ead11d`.
The source contains 241,670 positions. Seed 20261009 shuffles source indices;
the first 30 valid, nonterminal, exact/mirror-unique positions with distinct
pawn-structure proxies are selected. No candidate/control evaluation or game
outcome was used to choose openings. There were no rejected samples before
the first 30 selections. Full selected source-line/FEN provenance is preserved.
Opening-file SHA-256:
`f538c7760f40b81b667f03f148ce04a27d06b54da238b477cc9dbaaa0b9a3444`.

Distinct pawn structures are only a diversity proxy. They do not establish
independent ECO/opening families, representativeness or an absolute rating scale.

## Fixed stopping and interpretation

No score-based early stop, optional extension, cherry-picked retry or automatic
restart. A physical flag, protocol/illegal reply, unfinished move-limit game,
system suspend, watchdog or one-hour wall budget invalidates the experiment and
preserves its complete/partial evidence. Game limit is 1,000 plies; progress
watchdog 90 seconds. An invalid/incomplete run supplies no strength verdict.

Validity requires 60 complete verified games, exact FEN/color pairs, unchanged
hashes/options/resources, legal played moves, actual board/rule endings,
complete bank/increment/history telemetry and no operational failures.

Primary strength rule is the paired 95% bounded Hoeffding score interval,
conditional on independent representative opening pairs:

- Upper bound below 50%: evidence of a large regression under those assumptions.
- Lower bound above 50%: promising positive signal, still requiring independent
  confirmation and no automatic promotion.
- Otherwise: inconclusive; neither gain nor non-regression established.

A score below 40% is a predeclared descriptive safety alarm for investigation,
not a significance test. Paired normal intervals are secondary description only.
The conservative half-width with 30 pairs is about 24.8 percentage points;
this budget cannot establish a small Elo gain. It is a large-damage/operational
screen, **not** a validated SPRT, a CCRL calibration, or a zero-regression proof.
Stockfish's [testing methodology](https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html)
illustrates why definitive development tests normally require a substantially
larger controlled campaign and a declared statistical stopping method.

## Execution and evidence

`build/rule50-paired-screen-20261009/` contains the plan, external archive,
opening selection/provenance, tested launcher, and frozen runner/analyser
dependencies. Launcher fixtures 3/3, existing paired-statistics tests 10/10,
and Windows-supervisor fixtures 5/5 passed before launch.

The scoped Windows job owns only this runner and its two engine children.
A scoped power request inhibits idle sleep; explicit sleep/reboot still
invalidates timing. Console/stderr, launch identity, supervisor events/final
status, integrity record, PGN, move telemetry, manifest and summary are retained.
No builds, benchmarks, corpus scans, tuning or competing matches run while timed
games are active. After exit, the frozen analyser verifies every game and clock
before the plan's decision rule is evaluated. Nothing is promoted automatically.

Final verification is preserved in `verified-analysis.json` and
`completion-verification.json`. The generic analyser intentionally does not
claim a predeclared stopping rule; the completion verifier separately checks
the unchanged plan hash, its timestamp before launch, planned settings and
openings, all 60 games, the single-launch event stream, console/summary agreement,
empty stderr and final integrity. The complete/partial game count is not inferred
from process absence.

Actual endings were 43 checkmates, 3 fifty-move draws, 8 threefold repetitions,
1 stalemate and 5 insufficient-material draws. Both sides had 50 recorded
searches at halfmove clock >=80. Candidate move-100+ behavior has 311 samples:
minimum starting bank 127.747ms, maximum reply time 170.189ms and depth 7–64.
The minimum candidate reply bank before increment over the full run was 1.021ms;
no physical flag occurred, but this thin margin is not a zero-increment safety
guarantee. These samples are increment-supported, single-threaded play and cannot
validate the cancelled six-thread 2+0 clock pilot.

## Diagnostic evidence and next engineering step

Saved telemetry across all played positions reports median depth 11 for the
candidate versus 12 for control; at fullmove 100+ it is 11 versus 14. Those
are **different positions and histories**, not a causal throughput experiment.
Matched initial roots provide a useful countercheck: both versions searched the
same 30 White-to-move FENs from equal 10-second banks. Candidate depth was lower
on 8, equal on 9 and higher on 13, with median depths 13 versus 12. First moves
matched on 25/30; median elapsed ratio was 0.9966 and node ratio 1.0340. The
posthoc `matched-root-telemetry.json` is diagnostic, not a revised acceptance rule.
A uniform depth-collapse or raw-speed explanation is therefore not established.

Code inspection identifies two measurable **hypotheses**, not proven causes:

- Exact-clock score eligibility excludes every mismatch, including ordinary
  low-clock transpositions, affecting cutoffs, evaluation refinement and
  singular score/depth metadata, not just positions near the draw boundary.
- Same-board entries at a different clock replace deeper entries even when the
  new result is shallow quiescence. The metadata test confirms this intentional
  replacement contract; it does not establish that the policy is efficient.

The next engineering investigation should measure compatible-score hits,
hint-only hits and deep-to-shallow context replacements on matched positions,
then isolate a storage/reuse policy that retains the reproduced boundary and
mate-precedence contracts without unnecessary cache loss. Do not permit
incompatible clock scores merely because an arbitrary threshold seems safe:
checks, extensions and qsearch invalidate a naive nominal-depth bound.
Same-clock repetition-history interaction still needs separate treatment.

Preserve the losing candidate and every game. No automatic retry, additional
strength campaign, evaluation retuning, book change or production promotion
follows this result. Correctness and strength must be demonstrated separately;
the next variant must keep the failing boundary regressions green and expose
its exact delta before another approved comparison.
