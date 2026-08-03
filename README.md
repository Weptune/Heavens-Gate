# Heaven's Gate ♟️

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=for-the-badge&logo=c%2B%2B" alt="C++20" />
  <img src="https://img.shields.io/badge/Build-CMake%20%7C%20GCC%2016.1-brightgreen.svg?style=for-the-badge" alt="Build" />
  <img src="https://img.shields.io/badge/Unit%20Tests-18%2F18%20Passing-success.svg?style=for-the-badge" alt="Tests" />
  <img src="https://img.shields.io/badge/Protocol-UCI%20Compliant-orange.svg?style=for-the-badge" alt="UCI Protocol" />
  <img src="https://img.shields.io/badge/License-MIT-red.svg?style=for-the-badge" alt="License" />
</p>

> **"How Far Can Mathematics Alone Take a Chess Engine?"**

**Heaven's Gate** is an open-source, mathematically grounded, modern **C++20** chess engine built completely from first principles. Every algorithm, data structure, evaluation feature, and search heuristic is implemented from scratch, mathematically justified, empirically measured, and thoroughly documented.

---

## 🌟 Core Philosophy

- **Zero Copied Code**: Built 100% from scratch without copying Stockfish or open-source engine codebases.
- **Pure Classical Mathematics**: Developed without neural networks (NNUE), machine learning, or reinforcement learning. Explores how far game theory, boolean algebra, dynamic programming, and search heuristics can be pushed.
- **Empirical Measurement**: Every architectural phase features quantitative metrics comparing node counts, execution times, effective branching factors (EBF), and transposition table hit rates.
- **First-Class Educational Visualizations**: Includes a JSON search tree exporter (`game_tree.json`) to visualize tree structures, search cutoffs, and decision branches.

---

## 🚀 Engine Milestones & Complete 10-Phase Architecture

| Version | Phase Milestone | Mathematical & Computer Science Foundations | Status |
| :---: | :--- | :--- | :---: |
| **v0.0** | **Foundations** | 64-bit Bitboards (`uint64_t`), FEN Parser, Legal Move Generator, Perft Verification. | ✅ **100% Complete** |
| **v1.0** | **Minimax Search** | Minimax Theorem, Negamax Formulation, Centipawn Material Eval, JSON Tree Exporter. | ✅ **100% Complete** |
| **v2.0** | **Alpha-Beta Pruning** | Subtree Elimination, \([\alpha, \beta]\) Window Inequalities, 99.0% Node Reduction. | ✅ **100% Complete** |
| **v3.0** | **Move Ordering** | MVV-LVA Captures, 2-Slot Killer Moves, 64x64 History Table (Approaching \(O(b^{d/2})\)). | ✅ **100% Complete** |
| **v4.0** | **Positional Evaluation** | Piece-Square Tables (PST), Tapered Midgame/Endgame Interpolation (\(\phi \in [0, 24]\)). | ✅ **100% Complete** |
| **v5.0** | **Iterative Deepening** | Progressive Deepening (\(d=1,2,3...\)), Millisecond Time Control, Bounded Overhead. | ✅ **100% Complete** |
| **v6.0** | **Magic Bitboards** | \(O(1)\) Sliding Piece Attacks (Rooks, Bishops, Queens) via Sparse Multipliers & Shift Hashing. | ✅ **100% Complete** |
| **v7.0** | **Transposition Table** | 64-bit Zobrist Hash XOR Keys, 64 MB Direct-Mapped Cache, Bound Stores & PV Recovery. | ✅ **100% Complete** |
| **v8.0** | **Quiescence Search** | Horizon Effect Elimination, Stand-Pat Evaluation Threshold, Delta Pruning. | ✅ **100% Complete** |
| **v9.0** | **Advanced Pruning** | Null Move Pruning (NMP \(R=2\)), Late Move Reductions (LMR \(R \propto \ln d \ln i\)). | ✅ **100% Complete** |
| **v10.0**| **UCI Engine Standard** | Universal Chess Interface Compliance (`uci`, `isready`, `position`, `go`, `quit`). | ✅ **100% Complete** |

---

## 📊 Live Benchmark & Search Space Reduction

Empirical search space metrics at **Depth 4** from initial starting position:

```text
========================================================================================================
  BENCHMARK: EVOLUTIONARY SEARCH REDUCTION (Initial Position, Depth 4)
========================================================================================================

| Engine Version | Algorithm / Techniques Enabled | Total Nodes Searched | Reduction vs Minimax |
| :--- | :--- | :--- | :--- |
| **v1.0 (Minimax)** | Raw Negamax Minimax Search | 206,603 nodes | Baseline (1.0x) |
| **v2.0 (Alpha-Beta)** | Unordered Alpha-Beta Pruning | 2,056 nodes | 99.0% Reduction |
| **v3.0 (Move Ordering)**| MVV-LVA + Killer + History Heuristic | 1,469 nodes | 99.3% Reduction |
| **v7.0 (Transposition)**| 64MB Transposition Table Cache | 1,052 nodes | 99.5% Reduction |
| **v9.0 (Advanced Pruning)**| Null Move Pruning + Late Move Reductions | 412 nodes | **99.8% REDUCTION** |
--------------------------------------------------------------------------------------------------------
```

---

## 🛠️ Building & Running Locally

### Prerequisites
- Modern C++20 Compiler (`GCC 11+`, `Clang 13+`, or `MSVC 2022`)
- CMake 3.20+

### Build Commands

```bash
# Clone the repository
git clone https://github.com/Weptune/Heavens-Gate.git
cd Heavens-Gate

# Create build directory and compile
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## 🎮 Running the Engine

### 1. Run Automated Unit Test Suite (18/18 Tests)
```bash
./heavensgate_tests
```

### 2. Interactive CLI Mode
```bash
./heavensgate
```
Available CLI commands:
- `id <depth> [time_ms]` : Run Iterative Deepening + NMP + LMR search.
- `ab <depth>`           : Run Move-Ordered Alpha-Beta search.
- `compare <depth>`      : Run side-by-side benchmark comparing all search engines.
- `export_tree <depth>`  : Export search tree to `game_tree.json`.
- `perft`                : Run Perft move-generator verification suite.
- `display` / `d`        : Display ASCII chessboard, side to move, and 64-bit Zobrist key.
- `fen <str>`            : Parse position from FEN string.
- `uci`                  : Switch process to standard UCI mode.

### 3. Universal Chess Interface (UCI Mode)
To connect Heaven's Gate to graphical chess GUIs (Cute Chess, Arena, Bankstatement, LiChess bot):
```bash
./heavensgate uci
```
Supports standard UCI protocol commands: `uci`, `isready`, `ucinewgame`, `position startpos moves ...`, `position fen <str> moves ...`, `go depth <d> wtime <ms> btime <ms>`, and `quit`.

---

## 📚 Mathematical Documentation & Essays

Educational architectural write-ups for every algorithm built:

- 📖 [Phase 03: Move Ordering Heuristics](docs/phase_03_move_ordering.md)
- 📖 [Phase 04: Positional Evaluation & PST](docs/phase_04_positional_eval.md)
- 📖 [Phase 05: Iterative Deepening & Time Controls](docs/phase_05_iterative_deepening.md)
- 📖 [Phase 06: Magic Bitboards & O(1) Sliding Attacks](docs/phase_06_magic_bitboards.md)
- 📖 [Phase 07: Zobrist Hashing & Transposition Tables](docs/phase_07_transposition_table.md)
- 📖 [Phase 08: Quiescence Search & Horizon Effect Elimination](docs/phase_08_quiescence_search.md)
- 📖 [Phase 09: Advanced Search Pruning (NMP & LMR)](docs/phase_09_advanced_pruning.md)
- 📖 [Phase 10: Universal Chess Interface (UCI) Protocol](docs/phase_10_uci_protocol.md)

---

## 📄 License

This project is open-source and licensed under the **MIT License**.
