# Phase 1: Minimax Search & Material Counting (Version 1.0)

---

## 1. Mathematical Problem

In Phase 0, our chess engine generated legal moves but chose between them using random numbers. It had no concept of value, loss, or looking into the future.

To give the engine strategic intelligence, we formulate chess as a **Two-Player Zero-Sum Game of Perfect Information**.

Let $\mathcal{S}$ be the set of legal game states, and let $f: \mathcal{S} \rightarrow \mathbb{R}$ be an evaluation function mapping state $S$ to a real score relative to the active player. Zero-sum dynamics imply:

$$
f_{\text{Black}}(S) = -f_{\text{White}}(S)
$$

The goal of the player to move at state $S_t$ is to pick move $m^* \in \text{Moves}(S_t)$ that maximizes their worst-case payoff under the assumption that the opponent will play optimally to minimize it.

---

## 2. Minimax Theorem & Negamax Formulation

### Von Neumann's Minimax Theorem
For depth $d$, the value of position $S$ is defined recursively:

$$
V(S, d) = \begin{cases}
f(S) \& \text{if } d = 0 \text{ or } \text{IsTerminal}(S) \\
\max_{m \in \text{Moves}(S)} \Big\{ -V(\text{MakeMove}(S, m), d - 1) \Big\} \& \text{if } d > 0
\end{cases}
$$

By mathematically exploiting zero-sum symmetry, the **Negamax** algorithm replaces separate `max_node` and `min_node` functions with a single symmetric recursive function.

---

## 3. Computational Complexity & The Exponential Explosion

Minimax performs an exhaustive depth-first search of the entire game tree.

If the average branching factor is $b \approx 35$ and search depth is $d$:
- **Time Complexity**: $\Theta(b^d)$
- **Space Complexity**: $\Theta(d)$ stack frames

### Search Tree Node Growth Curve
| Depth $d$ | Total Nodes Searched $N = b^d$ | Time @ 25M NPS |
| :--- | :--- | :--- |
| **Depth 1** | 20 | $0.0008\text{ ms}$ |
| **Depth 2** | 400 | $0.016\text{ ms}$ |
| **Depth 3** | 8,902 | $0.35\text{ ms}$ |
| **Depth 4** | 197,281 | $7.89\text{ ms}$ |
| **Depth 5** | 4,865,609 | $194\text{ ms}$ |
| **Depth 6** | 119,060,324 | $4.76\text{ seconds}$ |
| **Depth 7** | ~2.9 Billion | $1.9\text{ minutes}$ |
| **Depth 8** | ~71 Billion | $47\text{ minutes}$ |

> **Key Mathematical Insight**: Without pruning, searching just **1 extra move ahead** costs **35 times more computing time**. Minimax alone hits a hard computational wall at depth 5-6.

---
