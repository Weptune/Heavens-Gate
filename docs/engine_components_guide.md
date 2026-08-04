# Heaven's Gate Engine Components & Technical Architecture Guide

> *"Comprehensive technical reference for all core sub-systems in Heaven's Gate."*

---

## 1. Core Board Representation & Move Generation

### 1.1 Bitboard Architecture (`src/board/`)
- **64-bit Bitboards (`uint64_t`)**: Every piece type and color is stored as a 64-bit bitboard where bit $k$ corresponds to square $k$ ($0 = a1, 63 = h8$).
- **Occupancy Bitboards**: `white_pieces`, `black_pieces`, `all_pieces`.
- **Fast LSB Extraction**: Uses compiler intrinsics (`__builtin_ctzll` / `_BitScanForward64`) for $O(1)$ piece bit scanning.

### 1.2 Magic Bitboards (`src/movegen/magic.cpp`)
- **Sliding Piece Move Generator**: Rook and Bishop attacks are pre-computed using 64-bit **Magic Bitboards**.
- **Occupancy Masking**: Uses magic multiplication `(occupancy & mask) * magic >> shift` to index pre-computed attack tables in $O(1)$ time.

---

## 2. Search Sub-System (`src/search/`)

### 2.1 Principal Variation Search (PVS / NegaMax Alpha-Beta)
- **Principal Variation Search**: Searches the first move (likely best move) with a full window $(\alpha, \beta)$. Subsequent moves are searched with a zero-window $(-\alpha - 1, -\alpha)$ to prove fail-low cutoffs faster.

### 2.2 Transposition Table (`src/search/tt.cpp`)
- **Zobrist Key Indexing**: 64-bit PolyGlot Zobrist hash keys index a 64 MB Transposition Table (`TTEntry`).
- **TT Bounds**: Stores exact scores (`TTBound::Exact`), lower bounds (`TTBound::Lower`), and upper bounds (`TTBound::Upper`).
- **Ply Adjustments**: Mate scores are adjusted relative to distance-from-root (`ply`) to prevent illegal mate pathing.

### 2.3 Adaptive Null-Move Pruning (NMP)
- **Concept**: If giving the opponent a free move still produces a score above $\beta$, the position is so strong that searching deeper is unnecessary.
- **Adaptive Reduction ($R$)**:
  $$R = \begin{cases} 3 & \text{if depth} \ge 6 \\ 2 & \text{if depth} < 6 \end{cases}$$

### 2.4 Quiescence Search & Bounded Check Extensions
- **Quiescence Search (QS)**: Resolves tactical capture sequences at leaf nodes to avoid the horizon effect.
- **Bounded Check Extension**: Bounded to `ply < 4 && depth > 1` to extend root check lines without triggering exponential tree depth explosions.

### 2.5 Winning Repetition Penalty
- **Draw Repetition Penalty**: If a move leads to a 3-fold repetition while holding a material/positional lead ($> +150\text{ cp}$), the engine returns $-100\text{ cp}$ to reject premature draw repetition.

---

## 3. Opening Book & Telemetry (`src/core/polyglot.cpp`, `src/main.cpp`)

### 3.1 PolyGlot Grandmaster Opening Book
- **Binary Book Probing**: Probes PolyGlot `.bin` opening books via Zobrist hashing, playing instant 0.0001 ms Grandmaster opening moves.

### 3.2 Real-time Telemetry & PGN Annotations
- Console logs real-time evaluation, time elapsed (ms), nodes searched, and Nodes Per Second (NPS).
- PGN exports (`tournament_results.pgn`) automatically record UCI annotations:
  `1. e2e4 (15cp, 45.2ms, 32100 nodes, 710177 nps)`
