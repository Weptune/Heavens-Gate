# Heaven's Gate — Technical Documentation Sitemap & Architecture Index

> *"Master Index and Interconnected Technical Documentation for the Heaven's Gate Chess Engine."*

---

## 🏛️ Core Architectural Documents

| Document | Description |
|:---|:---|
| 📐 **[Spectral-Tropical Hybrid Engine](phase_11_spectral_tropical_engine.md)** | Mathematical formulation of Laplacian graph spectra, 16D feature vectors, $(\max, +)$ tropical semiring surface, MasterPositional ground-truth targets, and 80-epoch Adam training. |
| 👑 **[10 Mirrored King-Bucket Architecture](king_bucket_architecture.md)** | Blueprint for Stage 1 parameter scaling ($512 \to 5,440$ parameters), spatial King regions, horizontal symmetry, and $O(1)$ search cost. |
| ♟️ **[Engine Components & Technical Architecture Guide](engine_components_guide.md)** | Complete reference for Bitboards, Magic Bitboards, PVS Alpha-Beta search, 64 MB Transposition Table, Adaptive NMP ($R=2 \to 3$), Quiescence Search, Repetition Penalty, PolyGlot Opening Book, and PGN Telemetry. |
| 🔄 **[Standalone Continuous Training Pipeline](continuous_training_pipeline.md)** | Operational guide for `run_continuous_training.ps1`, self-play dataset generation across 12 OpenMP threads, warm-started Adam optimization, and automated Git commits. |
| 🏆 **[Master Upgrades Roadmap (2000+ Elo Blueprint)](../implementation_plan.md)** | Stage 0 to Stage 4 evolution plan: King-Buckets, Cross-Terms, Zone-Fiedler, Tropical Rational Functions ($T_1 - T_2$), and Spectral Centrality. |

---

## 📚 Engine Development Phase History

| Phase Document | Topic & Theoretical Foundation | Key Concepts Covered |
|:---|:---|:---|
| 00. **[Foundations & MoveGen](phase_00_foundations.md)** | Board State & Bitwise Boolean Algebra | 64-bit Bitboards, LSB/popcount intrinsics, Perft movegen validation. |
| 01. **[Minimax Search](phase_01_minimax.md)** | Adversarial Decision Trees | Zero-Sum Minimax theorem, recursive tree depth traversal. |
| 02. **[Alpha-Beta Pruning](phase_02_alphabeta.md)** | Search Tree Optimization | $[\alpha, \beta]$ cutoff bounds, branch pruning efficiency. |
| 03. **[Move Ordering](phase_03_move_ordering.md)** | Search Heuristics | MVV-LVA (Most Valuable Victim - Least Valuable Aggressor), Killer Moves, History Heuristic. |
| 04. **[Positional Evaluation](phase_04_positional_eval.md)** | Classical Evaluation Function | Piece-Square Tables (PST), Pawn Structure, Passed Pawns, King Safety, Mobility. |
| 05. **[Iterative Deepening](phase_05_iterative_deepening.md)** | Time Management & Search Control | Dynamic time allocation, Principal Variation (PV) tracking across depths. |
| 06. **[Magic Bitboards](phase_06_magic_bitboards.md)** | Fast Slider Move Generation | Occupancy masks, magic multiplication, $O(1)$ Rook/Bishop attack lookups. |
| 07. **[Transposition Table](phase_07_transposition_table.md)** | Zobrist Hashing & Memory Caching | 64-bit PolyGlot Zobrist keys, exact/lower/upper TT bounds, mate score ply scaling. |
| 08. **[Quiescence Search](phase_08_quiescence_search.md)** | Horizon Effect Prevention | Tactical capture/check extension loops at search leaves. |
| 09. **[Advanced Pruning](phase_09_advanced_pruning.md)** | High-Efficiency Branch Reduction | Adaptive Null-Move Pruning ($R=2 \to 3$), Reverse Futility Pruning, Late Move Reductions. |
| 10. **[UCI Protocol](phase_10_uci_protocol.md)** | Universal Chess Interface | Standard GUI integration (ChessBase, Arena, Lichess), time control parsing. |
| 11. **[Spectral-Tropical Engine](phase_11_spectral_tropical_engine.md)** | Novel Mathematical Evaluation | Graph Laplacian eigensolvers ($\lambda_2$), $(\max, +)$ tropical semiring surface, Adam optimizer. |

---

## 🛠️ Operational Quick Reference

### 1. Compile Engine & Trainer
```bash
# Compile standalone UCI engine
g++ -std=c++20 -O3 -Isrc src/main.cpp src/**/*.cpp -o heavensgate.exe

# Compile multi-threaded trainer
g++ -std=c++20 -O3 -fopenmp -Isrc tools/train_spectral_tropical.cpp src/**/*.cpp -o train_spectral_tropical.exe
```

### 2. Run Continuous Training Loop
```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\abhin\heavensgate\run_continuous_training.ps1"
```

### 3. Run Benchmark Perft
```bash
./heavensgate.exe perft 5
```
