# Phase 0: Foundations & Move Generation (Version 0.0)

---

## 1. The Mathematical Problem

Chess has an estimated state space size of $\sim 10^{43}$ legal board positions and a game-tree complexity of $\sim 10^{120}$ (the Shannon number). 

Before a machine can evaluate or plan a single move, it must answer two mathematical questions:
1. **State Representation**: How do we encode a complete board state $S \in \mathcal{S}$ with minimal memory footprint and zero information loss?
2. **State Transition Mapping**: Given a state $S_t$, what is the exact legal transition function $\mathcal{T}(S_t) \rightarrow \{S_{t+1}^{(1)}, S_{t+1}^{(2)}, \dots, S_{t+1}^{(k)}\}$?

---

## 2. Bitwise Boolean Algebra & Bitboards

A chessboard consists of 64 squares:

$$
\mathcal{B} = \{a1, b1, \dots, h8\} \cong \{0, 1, 2, \dots, 63\}
$$

Instead of storing an array of 64 object pointers, modern engine design represents piece presence using **Bitboards**—64-bit unsigned integers (`uint64_t`):

$$
B \in \{0, 1\}^{64}
$$

Each bit index $i \in [0, 63]$ represents the presence ($1$) or absence ($0$) of a specific piece type on square $i$.

### Set Operations as Bitwise Operators
- **Union** (Pieces of White OR Black): $B_{\text{Occupied}} = B_{\text{White}} \cup B_{\text{Black}} \equiv B_{\text{White}} \mid B_{\text{Black}}$
- **Intersection** (White Pawns on 4th rank): $B_{\text{Match}} = B_{\text{White Pawns}} \cap B_{\text{Rank 4}} \equiv B_{\text{White Pawns}} \& B_{\text{Rank 4}}$
- **Difference** (Empty Squares): $B_{\text{Empty}} = \neg B_{\text{Occupied}} \equiv \sim B_{\text{Occupied}}$

### Bit Manipulation Primitives
- **Population Count** (`popcount(B)`): Computes $\sum_{i=0}^{63} b_i$, returning the exact piece count in 1 CPU instruction.
- **Least Significant Bit** (`lsb(B)`): Finds the lowest set bit index $\min \{i \mid b_i = 1\} \equiv `countr_zero(B)`$.
- **Bit Clearing** ($B \leftarrow B \& (B - 1)$): Resets the lowest set bit in $O(1)$ time.

---

## 3. Move Generation Mechanics & Perft Verification

A move generator must correctly account for complex game rules:
- **Pins**: A piece pinned to its king along an attack ray cannot move out of the ray.
- **Checks & Double Checks**: When in check, only moves that block or capture the checking piece (or move the king) are legal.
- **Special States**: Castling rights, en passant target squares, pawn double-pushes, and promotions.

### Perft (Performance Test) Formula
Perft measures the total number of leaf nodes at search depth $d$:

$$
\text{Perft}(d) = \sum_{m \in \text{LegalMoves}(S)} \text{Perft}(d - 1, \text{MakeMove}(S, m))
$$

with base case $\text{Perft}(0) = 1$.

If a single move generation bug exists (e.g. illegal castling through check, missing en passant pin), the node counts will deviate exponentially as depth increases.

---

## 4. Benchmark Metrics - Version 0.0

| Position Suite | Depth 1 | Depth 2 | Depth 3 | Depth 4 | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Initial Position | 20 | 400 | 8,902 | 197,281 | Verified PASS |
| Kiweteam Position | 48 | 2,039 | 97,862 | 4,085,603 | Verified PASS |
| Position 3 (Endgame) | 14 | 191 | 2,812 | 43,238 | Verified PASS |
| Position 4 (Castling & Promos) | 6 | 264 | 9,467 | 422,333 | Verified PASS |
| Position 5 (Tactical Pins) | 44 | 1,486 | 62,379 | 2,103,487 | Verified PASS |
| Position 6 (Midgame) | 46 | 2,079 | 89,890 | 3,894,594 | Verified PASS |

---
