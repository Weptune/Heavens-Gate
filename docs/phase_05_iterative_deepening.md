# Phase 5: Iterative Deepening & Dynamic Time Management (Version 5.0)

> **YouTube Episode Concept**: *"Iterative Deepening: The Mathematics of Time Allocation"*

---

## 1. The Mathematical Problem

Fixed-depth searches (`depth = 5`) break under real-world time control constraints:
- In sharp tactical positions, a depth-5 search might take 15 seconds.
- In simple positions, a depth-5 search might take 5 milliseconds.

If the engine is allocated 2 seconds per move, a fixed-depth search will either timeout or waste remaining clock time.

---

## 2. Iterative Deepening & Mathematical Proof of Bounded Overhead

Instead of searching directly to depth \(D\), **Iterative Deepening** searches progressively:
\[
\text{depth} = 1, 2, 3, \dots, D
\]

### Proof of Geometric Bounded Overhead
Since the total nodes at depth \(d\) grow exponentially \(N(d) = b^d\):
\[
\sum_{k=1}^{D-1} b^k = \frac{b^D - b}{b - 1} \approx \frac{1}{b - 1} N(D)
\]
For an Effective Branching Factor \(b \approx 6.2\):
\[
\frac{1}{6.2 - 1} \approx 0.192 \implies \mathbf{19.2\% \text{ overhead}}
\]

### Why 19.2% Overhead Gains Massive Net Performance
1. **Optimal PV Move Feeding**: The best move found at iteration \(d-1\) is fed as the initial candidate move for iteration \(d\). This produces immediate \(\beta\)-cutoffs on move 1 of root nodes, reducing total search tree size by far more than 19.2%!
2. **Instant Time Control Cutoff**: If the search timer expires mid-iteration, the engine immediately aborts and returns the complete, verified principal variation from iteration \(d-1\).

---

## 3. Real-Time Benchmark Metrics

Executing `id 6 500` (Iterative Deepening up to Depth 6 with 500 ms target):

```text
Running Iterative Deepening (Max Depth 6, Max Time 500 ms) ...
  Depth 1: Best Move e2e4, Eval 20 cp, 20 nodes
  Depth 2: Best Move e2e4, Eval 25 cp, 85 nodes
  Depth 3: Best Move e2e4, Eval 30 cp, 412 nodes
  Depth 4: Best Move e2e4, Eval 28 cp, 1469 nodes
  Depth 5: Best Move e2e4, Eval 35 cp, 8910 nodes
  Depth 6: Best Move e2e4, Eval 32 cp, 48210 nodes

Completed Depth : 6
Time Elapsed    : 0.1120 s (112 ms)
NPS             : 527,410 NPS
```
