# Phase 3: Move Ordering Heuristics (Version 3.0)

> **YouTube Episode Concept**: *"Move Ordering: How Heuristics Guide Mathematics to Optimal Efficiency"*

---

## 1. Mathematical Problem

In Phase 2, we introduced Alpha-Beta pruning. However, Alpha-Beta efficiency is highly sensitive to node visitation order:
- If moves are evaluated from **worst to best**: \(O(b^d)\) — zero subtrees are pruned (identical to raw Minimax).
- If moves are evaluated from **best to worst**: \(O(b^{d/2}) = O(\sqrt{N})\) — optimal tree pruning.

Without move ordering, Alpha-Beta searches moves in whatever arbitrary order `generate_legal_moves` returns them. 

To approach theoretical optimal complexity \(O(b^{d/2})\), we must rank moves using cheap, fast heuristics before searching deep into subtrees.

---

## 2. The Move Ordering Hierarchy

Moves are evaluated and sorted using a 4-tier score hierarchy:

### Tier 1: MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
Captures are sorted by the net value difference between the captured piece (victim) and the attacking piece:
\[
\text{Score}(\text{Capture}(A \rightarrow V)) = 1,000,000 + 10 \times \text{Value}(V) - \text{Value}(A)
\]
- Example: Pawn capturing Queen (\(10 \times 900 - 100 = 8900\)) is searched **before** Queen capturing Pawn (\(10 \times 100 - 900 = 100\)).

### Tier 2: Killer Move Heuristic
Non-capture moves that recently caused a \(\beta\)-cutoff at the exact same depth level in sibling branches are recorded in a fixed-size table:
\[
\text{Score}(\text{Killer}_1) = 900,000, \quad \text{Score}(\text{Killer}_2) = 800,000
\]
If a quiet move was strong enough to cut off a branch elsewhere at depth \(d\), it is highly likely to cut off branches at depth \(d\) in neighboring positions.

### Tier 3: History Heuristic
Moves \((s_{\text{from}}, s_{\text{to}})\) that historically produced \(\beta\)-cutoffs anywhere in the search tree accumulate weight proportional to depth squared:
\[
\text{HistoryTable}[\text{Color}][s_{\text{from}}][s_{\text{to}}] += \text{depth}^2
\]
Quiet non-killer moves are sorted by their accumulated history score.

---

## 3. Empirical Benchmark Comparison (Depth 4 Startpos)

| Metric | Minimax (v1.0) | Raw Alpha-Beta (v2.0) | Move-Ordered (v3.0) | Improvement |
| :--- | :--- | :--- | :--- | :--- |
| **Best Move** | `a2a3` | `a2a3` | `a2a3` | Identical |
| **Eval Score** | `0 cp` | `0 cp` | `0 cp` | Identical |
| **Total Nodes** | 206,603 | 2,011 | 1,480 | **26.4% reduction vs v2.0 (99.3% vs v1.0)** |
| **Effective Branching Factor (EBF)** | 21.32 | 6.70 | 6.20 | **Lower EBF** |
| **Cutoff Rate** | 0% | ~45% | **~68%** | **Significantly earlier cutoffs** |

---

## 4. Visualizations for YouTube

1. **Pruning Explosion Comparison**:
   - Show how MVV-LVA puts winning captures at the very front of the search queue, causing \(\beta\)-cutoffs on move 1 or 2 of almost every node.
2. **History Heuristic Heatmap**:
   - Render an 8x8 heatmap of the chessboard showing which squares develop the highest history cutoff counts over time.

---

## 5. YouTube Narrative Script Concept

> **Narrator**: 
> *"Alpha-Beta pruning was a game-changer, but it had a dirty secret: its efficiency depends entirely on luck.*
> *If you check the worst moves first, Alpha-Beta prunes NOTHING.*
> *If you check the best move first, Alpha-Beta prunes ALMOST EVERYTHING.*
> *Today, we introduce three mathematical heuristics: MVV-LVA, Killer Moves, and History Tables.*
> *Watch how sorting our moves before searching cuts down another 26% of our tree..."*
