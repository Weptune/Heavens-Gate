# Phase 4: Positional Evaluation & Piece-Square Tables (Version 4.0)

> **YouTube Episode Concept**: *"Feature Engineering: Teaching Mathematics the Concept of Space"*

---

## 1. Mathematical Problem

Up to Phase 3, our engine was a purely material-counting calculator. If two legal moves resulted in equal material balance, the engine regarded them as mathematically identical.

For example, a move advancing a central pawn to `e4` was rated identically to advancing an edge pawn to `h3`. A knight sitting on the active central square `d4` was rated identically to a knight trapped on the corner square `a1`.

To solve this, we engineer a **Positional Linear Evaluation Function**:
\[
\text{Eval}(S) = \sum_p \text{Material}(p) + \text{PST}(p, \text{sq}, \text{phase}) + \text{Mobility}(p)
\]

---

## 2. Piece-Square Tables (PST) & Tapered Phase Interpolation

### Piece-Square Tables
Each piece type has two 64-element positional weight matrices:
1. **Midgame (MG)**: Prioritizes center control, pawn shields, piece development, and king safety in castled corners.
2. **Endgame (EG)**: Prioritizes passed pawn advancement and centralizing the king to support pawn promotion.

### Game Phase Calculation \(\phi \in [0, 24]\)
Game phase is calculated dynamically based on remaining non-pawn pieces:
\[
\phi = \text{popcount}(N_{\text{White}} + N_{\text{Black}}) \times 1 + \text{popcount}(B_{\text{White}} + B_{\text{Black}}) \times 1 + \text{popcount}(R_{\text{White}} + R_{\text{Black}}) \times 2 + \text{popcount}(Q_{\text{White}} + Q_{\text{Black}}) \times 4
\]

### Linear Tapered Interpolation
To avoid sudden discontinuous evaluation jumps as pieces are traded, the final evaluation linearly interpolates between Midgame and Endgame PST scores:
\[
\text{Eval}(S) = \frac{\text{Score}_{\text{MG}} \times \phi + \text{Score}_{\text{EG}} \times (24 - \phi)}{24}
\]

---

## 3. YouTube Visualizations

1. **Positional Heatmaps**:
   - Render glowing 8x8 heatmaps over the chessboard showing positional values for Knights on center squares (+20 cp) vs rim squares (-50 cp).
2. **Phase Transition Slider**:
   - Demonstrate the evaluation of the King smoothly transitioning from corner safety (+30 cp on g1 in midgame) to center activity (+40 cp on e4 in endgame).
