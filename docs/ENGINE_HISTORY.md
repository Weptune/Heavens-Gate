# 📖 HEAVEN'S GATE: MASTER ENGINE PROGRESSION LEDGER & BENCHMARK HISTORY

This document serves as the **single source of truth** for all historical engine iterations, architectural milestones, match results, and multi-dimensional search telemetry in the Heaven's Gate project.

---

## 🏛️ Comprehensive Milestone & Telemetry Ledger

| Iteration / Milestone | Git Commit | Date / Time | Evaluator Architecture | Search & Hardware Features | Match vs Stockfish 3400 (2+0) | Tactical Speed (Avg Game Len / Mate) | Search Efficiency (Nodes/Move / Depth @ 2s) | TT Hit Rate | STS Score (80 GM Pos) | Status |
| :--- | :---: | :---: | :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **v1.0 Baseline Bitboard** | `265d6c8` | 2026-08-12 | Classical PST + Material | Pure Alpha-Beta + Quiescence | 0 – 10 (0.0%, -800) | 28.4 moves (Losses) | 3,400,000 nodes/move (Depth 9) | 18.2% | 1,820 / 8000 (22.7%) | Archived |
| **v2.0 Spectral Graph Physics** | `e1d4fc6` | 2026-08-14 | Laplacian Eigensolver ($\lambda_2$, Trace, Fiedler) | PVS + Basic Transposition Table | 2 – 8 (20.0%, -450) | 42.1 moves | 2,100,000 nodes/move (Depth 10) | 24.5% | 2,450 / 8000 (30.6%) | Archived |
| **v3.0 Tropical Surface** | `915a133` | 2026-08-16 | 640-Sector Tropical Minimax ($T(x) = \max_j (w^T x + b)$) | PVS + SEE + Precomputed LMR + RFP | 6 – 4 (60.0%, +85) | 48.0 moves | 1,450,000 nodes/move (Depth 12) | 31.0% | 3,140 / 8000 (39.2%) | Archived |
| **v4.0 GM Book & Texel Adam** | `c1ab26e` | 2026-08-18 | Calibrated Tropical Adam Weights | PolyGlot GM Book (`performance.bin`) + Syzygy 6-Man | 8 – 2 (80.0%, +320) | 52.3 moves (Mate in 14) | 1,120,000 nodes/move (Depth 13) | 38.5% | 3,585 / 8000 (44.8%) | Archived |
| **v5.0 Peak Sweep Engine** | `0480b5f` | 2026-08-18 | Peak 3,800 STS Model (`heavensgate_tropical.trm`) | Singular Extensions + On-Demand Move Selection | **10 – 0 (100.0%, +800)** | 56.4 moves (Mate in 10.0) | 942,460 nodes/move (Depth 14) | 39.1% | 3,480 / 8000 (43.5%) | Archived |
| **v5.1 Zero-Alloc Real Peak** | `704bae6` | 2026-08-19 | Vectorized Bitboard Feature Extraction (Zero 64x64 loops) | Zero-Allocation Stack Sorting + 0 cp Draw Contempt | **10 – 0 (100.0%, +800)** | **53.5 moves (Mate in 10.0)** | **942,460 nodes/move (Depth 14.5)** | **41.2%** | **3,545 / 8000 (44.3%)** | **ACTIVE PEAK** |
| **v6.0-Exp Cache & Horizons** | `REVERTED` | 2026-08-19 | Pure Bitboard Tropical Physics (Unchanged Peak) | 64-Byte Cache-Aligned TT + Passed Pawn Push Ext + Double SE | 9.5 – 0.5 (95.0%, +512) | 55.1 moves (Mate in 10.1) | 926,182 nodes/move (Depth 15.2) | 54.8% | N/A | **REVERTED (Draw on G6)** |

---

## 🔬 Head-to-Head Telemetry Comparison: v5.1 Peak vs v6.0-Exp

| Game # | v5.1 Peak Build (`704bae6`) | v6.0 Experimental Additions | Decisive Winner |
| :---: | :---: | :---: | :---: |
| **G1** | **1 – 0** (Mate in 18, 56 moves) | **1 – 0** (Mate in 3, 87 moves) | **v5.1 Peak (Faster Total Win)** |
| **G2** | **1 – 0** (Mate in 9, 33 moves) | **1 – 0** (Mate in 7, 47 moves) | **v5.1 Peak (Faster Total Win)** |
| **G3** | **1 – 0** (Mate in 9, 42 moves) | **1 – 0** (Mate in 5, 37 moves) | v6.0 Exp (Faster Win) |
| **G4** | **1 – 0** (Mate in 14, 33 moves) | **1 – 0** (Mate in 17, 30 moves) | v6.0 Exp (Faster Win) |
| **G5** | **1 – 0** (Mate in 13, 29 moves) | **1 – 0** (Mate in 13, 53 moves) | **v5.1 Peak (Faster Total Win)** |
| **G6** | **1 – 0** (Mate in 10, 43 moves) | **Draw (1/2 – 1/2, Repetition)** | **v5.1 Peak (Won vs Draw)** |
| **G7** | **1 – 0** (Mate in 8, 160 moves) | **1 – 0** (Mate in 13, 80 moves) | v6.0 Exp (Faster Win) |
| **G8** | **1 – 0** (Mate in 6, 53 moves) | **1 – 0** (Mate in 9, 49 moves) | v6.0 Exp (Faster Win) |
| **G9** | **1 – 0** (Mate in 6, 57 moves) | **1 – 0** (Mate in 10, 25 moves) | v6.0 Exp (Faster Win) |
| **G10**| **1 – 0** (Mate in 7, 29 moves) | **1 – 0** (Mate in 14, 85 moves) | **v5.1 Peak (Faster Total Win)** |

**Decision**: Reverted to **v5.1 Peak (`704bae6`)** because v5.1 achieved a **100% win rate (10-0)** with faster average game conversion (53.5 moves vs 55.1 moves).
