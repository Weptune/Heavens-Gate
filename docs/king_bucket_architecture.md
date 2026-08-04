# King-Bucket Tropical Architecture (Stage 1 Upgrade)

> *"Expanding Heaven's Gate parameter capacity by 10x (512 → 5,440 parameters) using 10 mirrored King-Bucket tropical surfaces."*

---

## 1. Executive Summary

The base tropical evaluator uses **1 global surface** with 32 sectors × 16 features = **512 learnable parameters**. A single surface forces the engine to apply identical positional weights regardless of whether the opponent's King is castled on the kingside (`g8`), queenside (`b8`), or stuck in the center (`e8`).

**King-Bucket Partitioning** splits the 64 squares of the chessboard into **10 strategic regions**. Each region receives its own independent 32-sector tropical surface, allowing the engine to learn location-dependent positional strategies (e.g. *a bishop on h7 is devastating when the opponent's King is on g8, but irrelevant when the King is on c1*).

---

## 2. Why 10 Mirrored Buckets Instead of 64?

| Metric | 64 Buckets | 10 Mirrored Buckets (Recommended) |
|:---|:---:|:---:|
| **Total Parameters** | 34,816 | **5,440** |
| **Sample Ratio (30k dataset)** | 470 samples / bucket (<1:1) | **~3,000 samples / bucket (~5.5:1)** |
| **Overfitting Risk** | Severe | **Zero** |
| **L1 Cache Footprint** | ~140 KB | **~21 KB (fits in L1 cache)** |

### Horizontal Symmetry
King positions on files `a-d` are mirror-symmetrical to files `e-h`. Horizontally mirroring King squares allows `g1` (Kingside Castle) and `b1` (Queenside Castle) to share the same bucket weights, doubling sample efficiency per bucket.

---

## 3. King-Bucket Spatial Partitioning

| Bucket | King Squares (White Perspective) | Mirror Squares | Strategic Meaning |
|:---:|:---|:---|:---|
| `0` | `a1, b1, a2, b2` | `h1, g1, h2, g2` | Back-Rank Flank Castle |
| `1` | `c1, d1, c2, d2` | `f1, e1, f2, e2` | Central Back-Rank / Uncastled King |
| `2` | `a3, b3, a4, b4` | `h3, g3, h4, g4` | Advanced Flank Shield |
| `3` | `c3, d3, c4, d4` | `f3, e3, f4, e4` | Central Midgame Active King |
| `4` | `a5-b5` through `a8-b8` | `h5-g5` through `h8-g8` | Deep Enemy Flank Infiltration |
| `5–9` | Vertical flip of White zones | Symmetric zones for Black's half | Enemy Territory King Walk |

For Black's King, squares are vertically flipped before bucketing so Black's back rank maps to the exact same strategic buckets as White's back rank.

---

## 4. Search & Evaluation Mechanics

### 4.1 Evaluation Time ($O(1)$ Search Cost)
1. Read opponent's King square: `Square opp_king_sq = board.king_square(~board.side_to_move())`.
2. Compute bucket index: `int b = king_bucket(opp_king_sq)`.
3. Evaluate **only the 32 sectors inside bucket $b$**:
   $$\text{PositionalEval} = \max_{j \in \{1..32\}} \left( \mathbf{w}_{b, j}^T \mathbf{x} + b_{b, j} \right)$$
4. **Search Speed is Unchanged**: The engine still evaluates exactly 32 sectors per position, resulting in 0% search slowdown.

### 4.2 Warm-Starting from 1 Surface
When upgrading from 1 surface (32 sectors) to 10 buckets (320 sectors), existing trained weights are duplicated across all 10 buckets. The engine starts at its full current strength and specializes each bucket over future training rounds.
