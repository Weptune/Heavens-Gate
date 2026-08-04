# Phase 8: Quiescence Search & Horizon Effect Elimination (Version 8.0)

---

## 1. The Mathematical Problem: The Horizon Effect

In fixed-depth searches (`depth = 5`), the evaluation function `Evaluator::evaluate(board)` is invoked abruptly at the search boundary ($d = 0$).

If a position reaches $d = 0$ in the middle of a tactical sequence (e.g., White captures Black's Knight with `1. Qxd7`), a fixed-depth evaluation rates White as $+300$ centipawns ahead. However, on the very next ply ($d = -1$), Black recaptures White's Queen (`1... Qxd7`), leaving White at $-600$ centipawns!

Because the search stopped at $d = 0$, the engine was completely blind to Black's immediate counter-strike. This phenomenon is known as the **Horizon Effect**.

---

## 2. Mathematical Formalism of Quiescence Search ($Q$-Search)

When regular search reaches `depth <= 0`, it transitions into a **Quiescence Search**:

### A. Stand-Pat Threshold
Before searching any captures, the engine evaluates the static position:

$$
\text{stand\\_pat} = \text{Evaluator::evaluate}(S)
$$

- If $\text{stand\\_pat} \ge \beta$: Return $\beta$ (Fail-high cutoff! Side to move can simply opt not to make any further captures).
- If $\text{stand\\_pat} > \alpha$: $\alpha = \text{stand\\_pat}$.

### B. Delta Pruning
If a capture cannot possibly raise $\alpha$ even assuming the captured piece is a Queen ($+900$ cp plus a 200 cp safety buffer):

$$
\text{stand\\_pat} + \text{VictimValue} + 200 < \alpha \implies \text{Prune Capture!}
$$

### C. Tactical Recaptures Only
$Q$-Search generates **only capture moves** (`MoveType::Capture`, `MoveType::EnPassant`, `PromoCapture`), ensuring the search tree terminates quickly as the board settles into a quiet, non-tactical state.

---

## 3. Empirical Test Result

In test position `r1bqkb1r/pppp1ppp/8/3n4/4P3/5Q2/PPP2PPP/RNB1KB1R w KQkq - 0 1`:
- **Fixed-Depth Search (Without Q-Search)**: White played `1. Qxd5?`, blundering a Queen for a Knight due to horizon blindness!
- **Quiescence Search (With Q-Search)**: White correctly calculated the recapture `1... Qxd5`, rejected `1. Qxd5` as a blunder, and played positional `1. e5` instead!

---

## 4. YouTube Video Visualizations

1. **The Cliff-Edge Evaluation Graph**:
   - Plot an evaluation curve over depth plies. Show how fixed-depth search misreads a cliff drop at $d=0$, while Quiescence Search extends past the cliff to find the true static equilibrium.
2. **Delta Pruning Cutoff Visualizer**:
   - Animate a pawn capturing a pawn being pruned in Q-Search because even +100 cp cannot overcome a 500 cp deficit.
