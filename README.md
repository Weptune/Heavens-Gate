# Heaven's Gate ♟️

> **"How Far Can Mathematics Alone Take a Chess Engine?"**

*Heaven's Gate* is an open-source, mathematically grounded, modern **C++20** chess engine written completely from first principles. Every feature, evaluation component, and search optimization is mathematically justified, empirically measured, and thoroughly documented.

---

## 🌟 Core Philosophy

- **Understanding Over Blind Imitation**: No engine code is copied from Stockfish or third-party engines.
- **Classical Mathematics & Computer Science**: Built without Neural Networks (NNUE/RL/ML) to explore how far pure game theory, boolean algebra, dynamic programming, and search heuristics can go.
- **Empirical Benchmarks**: Every version bump produces quantitative metrics comparing node counts, search time, and Effective Branching Factor (EBF).

---

## 🚀 Engine Milestones & Roadmap

| Version | Milestone | Mathematical & Computer Science Foundations | Status |
| :--- | :--- | :--- | :--- |
| **v0.0** | **Foundations** | Bitboards (`uint64_t`), FEN Parsing, Legal Move Generator, Perft Verification. | ✅ **Verified 100%** |
| **v1.0** | **Minimax Search** | Minimax Theorem, Negamax Formulation, Material Evaluation, JSON Tree Exporter. | ✅ **Verified 100%** |
| **v2.0** | **Alpha-Beta Pruning** | Subtree Elimination, \([\alpha, \beta]\) Inequalities, EBF Drop from 21.3 to 6.7. | ✅ **Verified 100%** |
| **v3.0** | **Move Ordering** | MVV-LVA Captures, Killer Moves, History Heuristic (Approaching \(O(b^{d/2})\)). | 🚧 *In Progress* |
| **v4.0** | **Positional Evaluation** | Piece-Square Tables (PST), Tapered Midgame/Endgame Interpolation, Pawn Structure. | ⏳ *Planned* |
| **v5.0** | **Iterative Deepening** | Dynamic Search Cutoffs, Geometric Series Sum Overhead, Time Management. | ⏳ *Planned* |
| **v6.0** | **Magic Bitboards** | Fast Sliding Ray Attacks, De Bruijn Cycles, Magic Multipliers & Bit Shifts. | ⏳ *Planned* |
| **v7.0** | **Transposition Table** | Dynamic Programming / Memoization, 64-bit Zobrist Hashing (XOR Vector Space). | ⏳ *Planned* |
| **v8.0** | **Quiescence Search** | Horizon Effect Mitigation, Tactical Equilibrium, Stand-Pat Delta Pruning. | ⏳ *Planned* |
| **v9.0** | **Advanced Pruning** | Late Move Reductions (LMR), Null Move Pruning (NMP), Logarithmic Depth Scaling. | ⏳ *Planned* |
| **v10.0** | **UCI Engine** | Universal Chess Interface Compliance & Benchmark Visualizer Pipeline. | ⏳ *Planned* |

---

## 📊 Live Empirical Performance Metrics

### Minimax (v1.0) vs. Alpha-Beta (v2.0) at Depth 4
```text
======================================================
  BENCHMARK COMPARISON: MINIMAX vs ALPHA-BETA (Depth 4)
======================================================

| Metric | Minimax (v1.0) | Alpha-Beta (v2.0) | Improvement |
| :--- | :--- | :--- | :--- |
| Best Move | a2a3 | a2a3 | Identical |
| Eval Score | 0 cp | 0 cp | Identical |
| Total Nodes | 206,603 | 2,011 | 99.0% REDUCTION |
| Time Elapsed | 0.0149 s | 0.0005 s | 32.13x FASTER |
| Branching Factor (EBF) | 21.32 | 6.70 | 14.62 lower |
------------------------------------------------------
```

---

## 🧪 Automated Testing & Perft Verification

The move generator is continuously validated against 6 standard perft test suites up to depth 4:
- Initial Position
- Kiweteam Position (`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`)
- Position 3 (Endgame)
- Position 4 (Promotions & Castling)
- Position 5 (Tactical Pins & Checks)
- Position 6 (Midgame Complexity)

---

## 🛠️ Building & Running Locally

### Prerequisites
- Modern C++20 Compiler (`GCC 11+`, `Clang 13+`, or `MSVC 2022`)
- CMake 3.20+

### Build Commands
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### CLI Usage
```bash
./heavensgate_tests          # Run automated unit test suite
./heavensgate                # Launch interactive engine shell
```

Commands inside interactive shell:
- `alphabeta 5` / `ab 5`: Run Alpha-Beta search at depth 5.
- `compare 4`: Run side-by-side comparison between Minimax and Alpha-Beta.
- `export_tree 3`: Export search tree to `game_tree.json`.
- `perft`: Execute full perft verification suite.
