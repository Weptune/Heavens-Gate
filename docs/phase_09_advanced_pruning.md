# Phase 9: Advanced Search Pruning & Reductions (Version 9.0)

> **YouTube Episode Concept**: *"Advanced Search Pruning: Null Move Pruning and Late Move Reductions"*

---

## 1. The Mathematical Problem

Even with Alpha-Beta pruning, Move Ordering, and Transposition Tables, deep searches ($d \ge 8$) still evaluate millions of redundant nodes.

To search exponentially deeper within millisecond time limits, we introduce two mathematical pruning techniques:

### A. Null Move Pruning (NMP)
- **Zugzwang Assumption**: In chess, doing nothing ("null move") is almost always worse than making any valid move.
- If we pass our turn and search at a reduced depth $R = 2$, and the static evaluation STILL exceeds $\beta$, our position is so overwhelmingly strong that Black cannot possibly prevent a fail-high cutoff!
- **Pruning Condition**:
  \[
  \text{depth} \ge 3 \ \&\& \ !\text{in\_check} \ \&\& \ \text{has\_non\_pawn\_material} \implies \text{search with } (d - 1 - R)
  \]
  If \(\text{null\_score} \ge \beta \implies \text{Return } \beta\).

### B. Late Move Reductions (LMR)
- **Probability Distribution of Best Move**: Because MVV-LVA, Killer Moves, and History Tables put the best candidate move in index 0 or 1 over $90\%$ of the time, moves at index $i \ge 4$ are statistically unlikely to raise $\alpha$.
- Quiet moves at late index positions are searched with a logarithmic depth reduction:
  \[
  R(d, i) = \left\lfloor 1 + \frac{\ln(d) \ln(i + 1)}{2.5} \right\rfloor
  \]
- If the reduced-depth search raises $\alpha$, the engine re-searches the candidate move at full depth $d-1$ to ensure zero tactical oversights.

---

## 2. YouTube Video Visualizations

1. **The Null Move Pass Animation**:
   - Animate White passing their turn, searching a 2-ply shallower subtree, and immediately pruning a 5,000-node branch.
2. **LMR Logarithmic Reduction Chart**:
   - Plot $R(d, i)$ curves over move indices 0 to 30, showing how late quiet moves are aggressively pruned while tactical captures remain unreduced.
