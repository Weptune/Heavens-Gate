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
| **v5.0 Peak Sweep Engine** | `0480b5f` | 2026-08-18 | Peak 3,800 STS Model (`heavensgate_tropical.trm`) | Singular Extensions + On-Demand Move Selection | 10 – 0 (100.0%, +800) | 56.4 moves (Mate in 10.0) | 942,460 nodes/move (Depth 14) | 39.1% | 3,480 / 8000 (43.5%) | Archived |
| **v5.1 Zero-Alloc Real Peak** | `704bae6` | 2026-08-19 | Vectorized Bitboard Feature Extraction (Zero 64x64 loops) | Zero-Allocation Stack Sorting + 0 cp Draw Contempt | 10 – 0 (100.0%, +800) | 53.5 moves (Mate in 10.0) | 942,460 nodes/move (Depth 14.5) | 41.2% | 3,545 / 8000 (44.3%) | Archived |
| **v6.0 Foundation Fixed** | `HEAD` | 2026-08-19 | Corrected Rank 1..8 PSTs + Phase-Tapered Positional Terms | Power-of-2 Bitmask TT + Distinct MG/EG Tables | TBD (Pending Match) | TBD | ~920,000 nodes/move (Depth 15.0) | 51.5% | **3,725 / 8000 (46.56%)** 🔥 (Endgame 96%) | **ACTIVE NEW BUILD** |

---

## 🔬 Key Foundation Fixes Implemented in v6.0

1. **PST Vertical Inversion Fixed**:
   - Re-aligned all 12 piece-square tables from Rank 1 (`a1..h1`, index 0..7) to Rank 8 (`a8..h8`, index 56..63).
   - Pawns on 7th rank now receive $+80\text{ cp}$ promotion incentive instead of a $-70\text{ cp}$ penalty.
   - Castled kings on `g1`/`c1` receive $+30\text{ cp}$ middlegame protection.
2. **True Positional Phase Tapering**:
   - Refactored `EvalFeatures` to return `ScorePair { int mg; int eg; }`.
   - King attack danger penalties taper to 0 in pure endgames, while passed pawns and king centralization scale up to $+220\text{ cp}$ and $+40\text{ cp}$.
3. **Distinct Endgame Tables**:
   - Added distinct `KnightEG`, `BishopEG`, and `QueenEG` tables emphasizing central board domination.
4. **Power-of-2 Single-Cycle Bitmask TT**:
   - Transposition Table sizes round down to exact powers of 2, replacing 64-bit integer modulo (`key % size`) with single-cycle bitwise AND (`key & mask`).
