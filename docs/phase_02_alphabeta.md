# Phase 2: Alpha-Beta Pruning (Version 2.0)

---

## 1. The Mathematical Problem

In Phase 1, we implemented Minimax. While mathematically sound, Minimax has a fatal computational flaw: it insists on evaluating every single leaf node in the game tree ($\Theta(b^d)$).

Consider a scenario at depth 2:
- Player White searches move $A$, resulting in a guaranteed evaluation of **+300 centipawns** (up a piece).
- Player White starts searching move $B$. The first response Black has to move $B$ is a tactical counter that leaves White at **-500 centipawns** (down a rook).

At this exact moment, White knows that move $B$ cannot possibly yield a score higher than **-500**. Since **-500 < +300**, White will **never** choose move $B$.

> **Question**: Why waste compute evaluating the remaining 34 responses Black could play after move $B$?

---

## 2. Mathematical Definition of Alpha-Beta Window $[\alpha, \beta]$

Alpha-Beta pruning maintains a dynamic search window $[\alpha, \beta]$ down the recursion stack:
- $\alpha$: The lower bound—the best score the **maximizing player** can guarantee so far.
- $\beta$: The upper bound—the best score the **minimizing opponent** can guarantee so far.

### Subtree Elimination Theorem
For any state $S$ and child candidate move $m$ returning score $v = -\text{Negamax}(\text{MakeMove}(S, m), d - 1, -\beta, -\alpha)$:

1. **Fail-High Cutoff**:
   
$$
v \ge \beta \implies \text{Prune remaining sibling moves at node } S
$$

   *Intuition*: The score $v$ is so good for the current player that the opponent (at the parent node) would never allow this state to be reached.

2. **Window Tightening**:
   
$$
v > \alpha \implies \alpha \leftarrow v
$$

   *Intuition*: We found a new guaranteed minimum score for the maximizing player.

---

## 3. Computational Complexity Analysis

| Scenario | Move Ordering Quality | Time Complexity | Effective Branching Factor (EBF) |
| :--- | :--- | :--- | :--- |
| **Worst Case** | Worst-to-Best (Ascending) | $O(b^d)$ | $b \approx 35$ |
| **Random Case** | Random Ordering | $O(b^{3d/4})$ | $b \approx 15 - 20$ |
| **Best Case** | Perfect Best-First Ordering | $O(b^{d/2}) = O(\sqrt{N})$ | $b \approx \sqrt{35} \approx 5.9$ |

> **Theorem**: In the best case, Alpha-Beta pruning effectively **doubles the search depth** for the exact same computational budget.

---

## 4. Empirical Benchmark Comparison (Depth 4 Startpos)

| Metric | Minimax (v1.0) | Alpha-Beta (v2.0) | Math Reduction |
| :--- | :--- | :--- | :--- |
| **Best Move** | `e2e4` | `e2e4` | Identical |
| **Evaluation** | `0 cp` | `0 cp` | Identical |
| **Nodes Searched** | 197,281 | 18,920 | **90.4% NODE REDUCTION** |
| **Execution Time** | 7.89 ms | 0.76 ms | **10.4x FASTER** |
| **Effective Branching Factor** | 21.05 | 11.73 | **44.2% lower EBF** |

---
