# Phase 10: UCI Protocol & Real-time Telemetry (Version 10.0)

---

## 1. The Mathematical Problem

A chess engine cannot exist in isolation. To interface with graphical user interfaces (GUIs like ChessBase, Arena, CuteChess, Lichess, and Lichess Bot API), it must implement a standard communication protocol: the **Universal Chess Interface (UCI)**.

Furthermore, analyzing engine behavior requires **real-time telemetry**: per-move evaluation scores, search time, node counts, and Nodes Per Second (NPS).

---

## 2. UCI Command Interface Specification (`src/uci/uci.cpp`)

Heaven's Gate implements the complete standard UCI command set:

| Command | Engine Action | Response / Output |
|:---|:---|:---|
| `uci` | Initializes engine metadata and options. | `id name Heaven's Gate 2.0`<br>`id author DeepMind AGY Team`<br>`uciok` |
| `isready` | Synchronizes GUI and engine state. | `readyok` |
| `ucinewgame` | Clears Transposition Tables and resets evaluation caches. | None |
| `position [startpos \| fen <FEN>] moves <move1> ...` | Sets up board position and plays move history. | Internal board state update |
| `go wtime <ms> btime <ms> winc <ms> binc <ms>` | Launches time-allocated iterative deepening search. | `info depth ... score cp ... nodes ... nps ... pv ...`<br>`bestmove <move>` |
| `stop` | Signals the running search engine to abort immediately. | `bestmove <current_best_move>` |
| `quit` | Terminates the engine process cleanly. | Exit 0 |

---

## 3. Dynamic UCI Time Control Allocator

When receiving clock state (`wtime`, `btime`, `winc`, `binc`), the engine calculates a dynamic move time budget:

$$\text{TargetTimeMs} = \frac{\text{RemainingTime}}{35.0} + 0.8 \times \text{Increment}$$

This ensures the engine distributes clock time safely across 40–60 move games without running into time forfeits.

---

## 4. Real-time Telemetry & PGN UCI Annotations (`src/main.cpp`)

During tournaments and self-play, Heaven's Gate prints per-move telemetry to the console and exports PGN files with standard UCI annotations:

### 4.1 Console Telemetry Output
```text
1. d2d3 (-1cp, 165.1ms, 12502 nodes, 75702 nps) b8a6 (9cp, 8.3ms, 12464 nodes, 1506223 nps)
```

### 4.2 Standard PGN Annotation Format (`tournament_results.pgn`)
```pgn
1. e2e4 { [%eval 15] [%clk 0:03:00] [%nodes 32100] [%nps 710177] } 1... c7c5 { [%eval -12] [%clk 0:03:00] [%nodes 28400] [%nps 1420100] }
```
- `[%eval <cp>]`: Evaluation score in centipawns.
- `[%clk <h:m:s>]`: Remaining clock time.
- `[%nodes <N>]`: Total search nodes evaluated for the move.
- `[%nps <rate>]`: Search speed in Nodes Per Second.
