# Batch 9 recovery: corrected experiments and private TT accounting

Implemented 2026-10-09. This is an instrumentation/performance recovery, not an Elo claim or an evaluation/search-feature bundle.

## Scope and preserved state

The existing dirty Phase 0/1 work was preserved. Evaluation sources and parameters, move generation, pruning/extension decisions, move-picker learning, and the opening book are unchanged by this recovery. The original root `heavensgate.exe` remains untouched.

Two isolated candidates are available:

| Candidate | Executable | Difference |
|---|---|---|
| A | `build/recovery-harness/heavensgate.exe` | Corrected tournament/UCI reporting and harness |
| B | `build/recovery-tt/heavensgate.exe` | A plus private per-worker TT statistics |

The input snapshot is `build/recovery-baseline/`. Candidate A's sources are frozen in `build/recovery-harness/source/`. Candidate B uses the current workspace sources. No Git reset or rollback of correctness hardening was performed.

## Harness changes

- Reported engine mates no longer adjudicate games. Games must reach played checkmate, stalemate, repetition, rule-50, or insufficient material. Protocol failures and clock flags are explicit forfeits that make the run fail; reaching the move limit is unfinished, not a fabricated draw.
- Stockfish receives the initial FEN plus every played move, preserving repetition history. Bank matches send `go wtime … btime … winc … binc …`, rather than an imposed per-move allocation. Heaven's Gate uses its existing UCI allocation policy through a shared helper; this is not a newly tuned time manager.
- Stockfish scores are associated with the selected `bestmove`'s PV root. Missing/mismatched scores and bounds are not treated as exact evaluations. PGN evaluation comments use White's perspective and pawn units; mate comments retain their mate notation. Missing node counts are not invented.
- Both engines receive the same configured thread and hash limits. A `--paired` option repeats each initial FEN with reversed engine colors. `--book-off` isolates search tests without changing the book itself.
- Every run produces `.manifest.json` and `.moves.jsonl` sidecars. They record binary/book hashes, dirty-source fingerprint (including `.inc` tables and CMake configuration), opponent identity, parameters, timing, full position commands, and protocol status. Existing output files are never silently overwritten.
- Child-process crashes, timeouts, and illegal moves fail explicitly; there is no restart or synthetic move fallback. Output no longer presents an arbitrary `+800 Elo` for a perfect score, or labels a score conversion as a CCRL rating.

The existing coordinate-style PGN movetext was retained. Validation reconstructs its legal moves instead of assuming a standard SAN parser can read it.

## TT change

`TTStatistics` is fixed-size and cache-line aligned, owned by one search worker. Recursive probes increment only that worker's ordinary integer counters. Aggregation and publication occur after the SMP barrier, or at serial search completion.

TT bucket locks, coherent entry snapshots, replacement policy, mate-score normalization, generation handling, and all search decisions are unchanged. Published counts now describe the current search, not a cumulative sequence of moves. Compatibility/raw TT probes are uninstrumented unless passed an explicit statistics object.

The isolated TT production patch touches only `search.cpp`, `search.hpp`, `tt.cpp`, and `tt.hpp`. Tests cover contention-heavy bucket collisions, exact private totals, disabled tables, mate scores, counter reset, terminal returns, and allocation-free serial/SMP search.

## Verification evidence

- Both stages built successfully with the bundled GCC/w64devkit C++20 compiler. The complete C++ suite passed, including `test_move_ordering_reduction`; six Python gate-verifier unit tests passed.
- End-to-end negative CLI matches deliberately crashed the opponent and returned an illegal promotion. Both aborted with exit code 1 and the corresponding recorded failure status, with no fallback move or successful tournament summary. The client fixture also exercises timeouts, score/PV mismatches, and bounds.
- All six reference perft positions matched. Startpos through depth 6 was **119,060,324**; the other reference positions were checked through depth 5.
- Candidate A and B were identical at single-thread depth 9 on nine positions: score, complete PV, and total nodes. This establishes parity for those searches, not deterministic equivalence of timing-dependent SMP runs.
- Candidate B passed the static, perft, strategic/tactical sanity, and real-match smoke gates. One initial run scored STS **4,135/8,000 (51.69%)** at 30 ms/position and BK **16/24** at 1 second/position. These are short sanity tests, not statistical strength validation or proof of equality to historical scores.
- A paired two-game real-clock smoke used 10+0 seconds, two threads each, Hash 16 MiB each, book disabled, and Stockfish 16.1 limited to 2200. Both games reached played checkmate with zero crashes/flags. All **159 plies** were reconstructed; every bank/history command was checked. **32 unavailable selected-move scores** were correctly left unknown. The 2–0 result is not a strength estimate.
- The TT-isolation benchmark used six Batch 9 positions, three repetitions, alternating candidate order, sequential runs, and 0.7-second search limits. Eighteen paired searches at each thread count measured a geometric-mean NPS ratio of **1.0068 at one thread** and **1.0663 at six threads**. Six-thread mean completed depth was **12.67 before / 12.33 after**. Throughput improved in this sample; depth and Elo improvement are not established.

Machine-readable parity/throughput evidence lives in `build/recovery-parity.json` and `build/recovery-benchmark.json`. The benchmark records hashes of the exact pre-reporting-polish binaries, preserved as `harness-isolation.exe` and `tt-isolation.exe`. Removing the rating label affects only the tournament summary; the search and TT-isolation patch are unchanged.

`build/recovery-tt/clock-paired.pgn` and its sidecars preserve the clock test. Verify against its preserved binary:

```powershell
python build/verify_clock_match.py build/recovery-tt/clock-paired.pgn build/recovery-tt/tt-isolation.exe
```

## Next strength experiment — not launched automatically

Freeze both candidates and compare them under the **corrected** harness. The original Batch 8/9 adjudications are not a valid direct control for games now played to actual endings, and the bank allocation interface has changed.

A 60-game pilot gives the first 30 existing opening FENs both engine colors. Run A and B sequentially on an otherwise idle machine with identical settings and unique outputs:

```powershell
.\build\recovery-harness\heavensgate.exe tournament 60 120 0 901 2400 6 build/recovery-harness/pilot-A.pgn --paired --book-off --hash=64
.\build\recovery-tt\heavensgate.exe tournament 60 120 0 902 2400 6 build/recovery-tt/pilot-B.pgn --paired --book-off --hash=64
```

**120 is the bank in seconds**, so these commands mean 2+0 blitz. A value of 1 means a one-second total bank, not one minute. A 60-game run can take hours; this pilot has not been run here.

Use the pilot to catch failures and inspect paired outcomes, then choose a sufficiently powered paired strength test and its stopping/acceptance rule before running it. Stockfish handicap randomness and Lazy SMP timing remain nondeterministic and are declared in the manifest. Do not promote the TT candidate solely on NPS, a tiny win/loss sample, or a handicapped Stockfish score-to-Elo conversion.

Only after validating this recovery should inherited pruning/check-evasion/correction-history risks be isolated one at a time. Evaluation/tuning and opening-book expansion remain separate, deferred projects.
