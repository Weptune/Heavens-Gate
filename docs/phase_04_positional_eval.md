# Phase 4: Positional Evaluation & MasterPositional Mechanics (Version 4.0)

---

## 1. Mathematical Problem

Up to Phase 3, our engine was a purely material-counting calculator. If two legal moves resulted in equal material balance, the engine regarded them as mathematically identical.

For example, a move advancing a central pawn to `e4` was rated identically to advancing an edge pawn to `h3`. A knight sitting on the active central square `d4` was rated identically to a knight trapped on the corner square `a1`.

To solve this, we engineer a **Positional Linear Evaluation Function**:

$$
\text{Eval}(S) = \sum_p \text{Material}(p) + \text{PST}(p, \text{sq}, \text{phase}) + \text{PositionalFeatures}(S, \text{side})
$$

---

## 2. Piece-Square Tables (PST) & Tapered Phase Interpolation

### Piece-Square Tables
Each piece type has two 64-element positional weight matrices:
1. **Midgame (MG)**: Prioritizes center control, pawn shields, piece development, and king safety in castled corners.
2. **Endgame (EG)**: Prioritizes passed pawn advancement and centralizing the king to support pawn promotion.

### Game Phase Calculation $\phi \in [0, 24]$
Game phase is calculated dynamically based on remaining non-pawn pieces:

$$
\phi = \text{popcount}(N) \times 1 + \text{popcount}(B) \times 1 + \text{popcount}(R) \times 2 + \text{popcount}(Q) \times 4
$$

### Linear Tapered Interpolation
To avoid sudden discontinuous evaluation jumps as pieces are traded, the final evaluation linearly interpolates between Midgame and Endgame PST scores:

$$
\text{Eval}_{\text{PST}}(S) = \frac{\text{Score}_{\text{MG}} \times \phi + \text{Score}_{\text{EG}} \times (24 - \phi)}{24}
$$

---

## 3. MasterPositional Feature Suite (`src/evaluation/eval_features.cpp`)

Heaven's Gate evaluates five handcrafted positional dimensions:

### 3.1 Pawn Structure Evaluation (`evaluate_pawn_structure`)
- **Doubled Pawns**: $-15\text{ cp}$ penalty for two pawns of the same color on the same file.
- **Isolated Pawns**: $-20\text{ cp}$ penalty for pawns with no friendly pawns on adjacent files.
- **Pawn Chains**: $+10\text{ cp}$ bonus for diagonal pawn protection structures.

### 3.2 Passed Pawn Evaluation (`evaluate_passed_pawns`)
- **Passed Pawns**: Pawns with no enemy pawns on the same or adjacent files ahead.
- **Rank Scaling**: Bonus scales quadratically with rank advancement ($+20\text{ cp}$ on 4th rank $\to +150\text{ cp}$ on 7th rank).
- **Connected Passed Pawns**: $+40\text{ cp}$ bonus for side-by-side passed pawns.

### 3.3 King Safety Evaluation (`evaluate_king_safety`)
- **Pawn Shield**: $+30\text{ cp}$ bonus per pawn shielding the King on files $f, g, h$ (or $a, b, c$).
- **Open File Penalty**: $-40\text{ cp}$ penalty for an open file adjacent to the King.
- **Attacking Pressure**: $-15\text{ cp}$ penalty per enemy piece attacking the King's 8-square neighborhood.

### 3.4 Piece Activity & Mobility (`evaluate_piece_activity`, `evaluate_mobility`)
- **Rook on Open File**: $+25\text{ cp}$ bonus.
- **Rook on 7th Rank**: $+40\text{ cp}$ bonus.
- **Bishop Pair**: $+30\text{ cp}$ bonus for holding both light-squared and dark-squared Bishops.
- **Mobility**: $+4\text{ cp}$ per legal move available to Knights, Bishops, Rooks, and Queens.

---

## 4. Empirical Benchmark Comparison

| Engine Mode | Strategic Depth | Evaluation Scope | Benchmark Rating |
|:---|:---|:---|:---:|
| **Material Only** | Zero positional understanding | Raw piece material counting | ~1200 Elo |
| **Material + PST** | Static square preferences | Square-coordinate lookups | ~1500 Elo |
| **MasterPositional** | Dynamic positional heuristics | Pawn structure + King safety + Passed pawns + Mobility | **~2000 Elo** |

