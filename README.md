# 🌌 Heaven's Gate — Spectral-Tropical Hybrid Chess Engine

[![Language](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Architecture](https://img.shields.io/badge/Architecture-Spectral--Tropical%20Hybrid-purple.svg)](#architecture)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**Heaven's Gate** is a high-performance C++20 chess engine built on a novel, 2D mathematical evaluation architecture that unifies **Spectral Graph Theory** (Graph Laplacian eigensolvers) with **Tropical Geometry** (($\max, +$) semiring minimax surfaces).

---

## 🏛️ Key Features & Architecture

- **Spectral Graph Theory Evaluation**: Treats piece configurations as dynamic attack/defense graphs, computing Laplacian eigenvalues ($\lambda_2$ Fiedler value, spectral gap, trace energy) to evaluate algebraic piece coordination.
- **Tropical Minimax Surface**: Evaluates positional terms over 32 polyhedral sectors using $(\max, +)$ tropical algebra — the native mathematical algebra of game trees.
- **16-Dimensional Positional Feature Vector**: Incorporates non-linear cross-terms including Endgame Passed Pawn Multipliers and Unshielded King Attack Ratios.
- **Self-Reinforcement Machine Learning**: Trained via Hard-Max Adam gradient descent over self-play games with cumulative warm-start checkpointing.
- **Lazy Spectral Acceleration**: Bypasses heavy eigensolvers in decisive material positions, achieving **150k–200k+ Nodes Per Second (NPS)**.
- **Advanced Search Engine**: Features Alpha-Beta Pruning, Transposition Tables (TT), Quiescence Search (Q-Search), Null-Move Pruning (NMP), Late Move Reductions (LMR), and Move Ordering (MVV-LVA / Killer Moves / History Heuristics).

---

## 🛠️ Build & Run Instructions

### Prerequisites
- C++20 compliant compiler (`g++` 10+ / `w64devkit` / `clang`)

### Building the Engine
```bash
# Build the main UCI engine
g++ -std=c++20 -O3 -Isrc src/main.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o heavensgate.exe
```

### Running a Grandmaster Tournament
```bash
# Run a 100-game tournament at Depth 5 against the Baseline Engine
.\heavensgate.exe tournament 100 5
```

---

## 🎓 Training the Spectral-Tropical Surface

```bash
# Build the trainer
g++ -std=c++20 -O3 -Isrc tools/train_spectral_tropical.cpp ... -o train_spectral_tropical.exe

# Run 500 self-play games @ Depth 5 with 300 Adam epochs
.\train_spectral_tropical.exe 500 5 300 0.002
```

### Standalone Continuous Training Loop
Run the automated multi-round pipeline directly in PowerShell:
```powershell
powershell -ExecutionPolicy Bypass -File "run_continuous_training.ps1"
```

---

## 📚 Documentation Index

Detailed architectural documentation can be found in the [`docs/`](docs/) directory:
- [Phase 11: Spectral-Tropical Hybrid Engine Architecture](docs/phase_11_spectral_tropical_engine.md)
- [Phase 10: UCI Protocol Implementation](docs/phase_10_uci_protocol.md)
- [Phase 08: Quiescence Search & Tactical Extensions](docs/phase_08_quiescence_search.md)
- [Phase 06: Magic Bitboards & Move Generation](docs/phase_06_magic_bitboards.md)
- [Phase 02: Alpha-Beta Search & Pruning](docs/phase_02_alphabeta.md)
