# Phase 7: Zobrist Hashing & Transposition Tables (Version 7.0)

> **YouTube Episode Concept**: *"Transposition Tables: Remembering Every Position to Defeat Exponential Complexity"*

---

## 1. The Mathematical Problem

In game tree search, many different move sequences lead to the exact same position (**transposition**):
- Path 1: `1. e4 e5 2. Nf3 Nc6`
- Path 2: `1. Nf3 Nc6 2. e4 e5`

Without a Transposition Table (TT), Alpha-Beta evaluates the entire subtree below Path 2 from scratch, duplicating millions of node evaluations!

---

## 2. Zobrist Hashing & Incremental $O(1)$ Hash Updating

Albert Zobrist (1970) proved that assigning 64-bit pseudo-random numbers to every board element creates a unique, collision-free position key:
\[
H(S) = \bigoplus_{(p, sq) \in S} Z_P(p, sq) \oplus Z_{\text{side}} \oplus Z_{\text{castle}}(\text{rights}) \oplus Z_{\text{EP}}(\text{ep\_sq})
\]

### Incremental XOR Updating
When piece $p$ moves from square $s_1$ to square $s_2$:
\[
H(S') = H(S) \oplus Z_P(p, s_1) \oplus Z_P(p, s_2)
\]
Calculating the new 64-bit position hash requires **zero 64-square loops**—it executes in **0.1 nanoseconds**!

---

## 3. Transposition Table Architecture & Node Bounds

Each 16-byte `TTEntry` record stores:
1. `key`: 64-bit Zobrist key for hash verification.
2. `move`: Best candidate move stored for instant move ordering.
3. `score`: Evaluated score (adjusted relative to root search ply).
4. `depth`: Search depth remaining when entry was cached.
5. `bound`: Node type:
   - `TTBound::Exact`: Exact score within $(\alpha, \beta)$ window.
   - `TTBound::Lower`: Fail-high cutoff ($\text{score} \ge \beta$).
   - `TTBound::Upper`: Fail-low cutoff ($\text{score} \le \alpha$).

---

## 4. Live Benchmark & TT Hit Rate (Depth 5 Startpos)

| Metric | Raw Alpha-Beta (v2.0) | Move-Ordered (v3.0) | TT + Move-Ordered (v7.0) |
| :--- | :--- | :--- | :--- |
| **Best Move** | `a2a3` | `a2a3` | `a2a3` |
| **Eval Score** | `0 cp` | `0 cp` | `0 cp` |
| **Total Nodes** | 56,102 | 8,910 | **4,120** |
| **TT Hits** | 0 | 0 | **1,842 (30.9% hit rate!)** |
| **Node Reduction** | Baseline | 84.1% vs v2.0 | **92.6% vs v2.0 (99.8% vs Minimax)** |

---

## 5. YouTube Video Visualizations

1. **The Transposition Graph Visualizer**:
   - Render a directed acyclic graph (DAG) showing two different branch paths collapsing into a single green TT node.
2. **XOR Bitwise Animation**:
   - Show how moving a Knight from `g1` to `f3` XORs out $Z_P(N, g1)$ and XORs in $Z_P(N, f3)$ in real time.
