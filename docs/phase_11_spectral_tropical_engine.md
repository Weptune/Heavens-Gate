# Phase 11: The Spectral-Tropical Hybrid Engine

> *"A novel 2D chess evaluation architecture unifying Spectral Graph Theory with Tropical Minimax Geometry into a self-reinforcing mathematical evaluation surface."*

---

## 1. Overview & Architectural Motivation

Traditional chess evaluation functions (Hand-Crafted Evaluation / HCE) use static, scalar linear weights assigned by human intuition (e.g. *knight on outpost = +30 cp*). These models fail to capture non-linear, dynamic piece coordination and spatial dominance.

Heaven's Gate replaces scalar HCE with a **Spectral-Tropical Hybrid Architecture**:
1. **Spectral Graph Theory**: Models piece interactions as a dynamic weighted graph, computing Laplacian eigenvalues ($\lambda_2$ Fiedler value, spectral gap, graph trace) to measure structural piece cohesion.
2. **Tropical Geometry Semiring ($\mathbb{T}$)**: Evaluates positions over a $(\max, +)$ piecewise-linear minimax surface with 32 polyhedral sectors, matching the native algebra of game trees.
3. **Self-Reinforcement Learning**: Optimizes tropical sector weights using Hard-Max Adam gradient descent over self-play games.

---

## 2. Spectral Graph Theory Formulation

The chess board is represented as an adjacency graph $G = (V, E)$ where nodes $V$ are piece positions and edges $E$ reflect attack, defense, and ray protection relationships.

### 2.1 The Laplacian Matrix
$$L = D - A$$
Where:
- $A_{ij}$: Edge weight between piece $i$ and piece $j$ based on move distance, protection, and ray alignment.
- $D_{ii} = \sum_{j} A_{ij}$: Degree matrix representing piece influence.

### 2.2 Key Spectral Features
- **Fiedler Value ($\lambda_2$)**: The second smallest eigenvalue of the Laplacian matrix. Measures algebraic connectivity and piece coordination for White ($\lambda_{2, \text{us}}$) and Black ($\lambda_{2, \text{them}}$).
- **Spectral Gap ($\lambda_N - \lambda_2$)**: Measures global board bottlenecking and control flow.
- **Laplacian Trace ($\text{Tr}(L)$)**: Sum of degree weights, representing total kinetic energy density on the board.

---

## 3. Tropical Semiring Minimax Surface

In tropical algebra, standard addition is replaced by maximum ($\oplus = \max$), and standard multiplication is replaced by addition ($\otimes = +$).

### 3.1 Tropical Evaluation Polynomial
The positional score is computed over $M = 32$ tropical sectors:

$$T(\mathbf{x}) = \bigoplus_{j=1}^{M} \left( w_{j0} \otimes x_1^{w_{j1}} \otimes x_2^{w_{j2}} \otimes \dots \otimes x_K^{w_{jK}} \right) = \max_{j \in \{1..M\}} \left( \mathbf{w}_j^T \mathbf{x} + b_j \right)$$

Where $\mathbf{x} \in \mathbb{R}^{16}$ is the **16-Dimensional Spectral-Tropical Feature Vector**:

| Index | Feature Description | Formula / Scale |
|:---:|:---|:---|
| `x[0]` | Material Difference | Raw Centipawns (Pass-through) |
| `x[1]` | Relative Fiedler Value | $(\lambda_{2, \text{us}} - \lambda_{2, \text{them}}) \times 15.0$ |
| `x[2]` | Subgraph Cohesion | $(\text{Cohesion}_{\text{us}} - \text{Cohesion}_{\text{them}}) \times 5.0$ |
| `x[3]` | Spectral Gap | $(\lambda_N - \lambda_2) \times 2.0$ |
| `x[4]` | Relative PST Difference | Centipawns (PST Modulation) |
| `x[5]` | King Attack Pressure | $(\text{Pressure}_{\text{us}} - \text{Pressure}_{\text{them}}) \times 10.0$ |
| `x[6]` | Ray Battery Alignment | $(\text{Battery}_{\text{us}} - \text{Battery}_{\text{them}}) \times 8.0$ |
| `x[7]` | Pawn Structure Cohesion | $(\text{PawnCohesion}_{\text{us}} - \text{PawnCohesion}_{\text{them}}) \times 12.0$ |
| `x[8]` | Laplacian Trace Energy | $\text{Tr}(L) / 10.0$ |
| `x[9]` | Relative Mobility | $(\text{Mobility}_{\text{us}} - \text{Mobility}_{\text{them}}) \times 3.0$ |
| `x[10]`| Center Control | $(\text{Center}_{\text{us}} - \text{Center}_{\text{them}}) \times 8.0$ |
| `x[11]`| Game Phase | $\text{Phase} \times 50.0$ ($1.0 = \text{Opening}, 0.0 = \text{Endgame}$) |
| `x[12]`| King Shield Energy | $(\text{Shield}_{\text{us}} - \text{Shield}_{\text{them}}) \times 10.0$ |
| `x[13]`| Passed Pawn Advantage | $\text{PassedDiff} \times 30.0$ |
| `x[14]`| **Endgame Passed Pawn Multiplier** | $\text{PassedDiff} \times (1.0 - \text{Phase}) \times 40.0$ |
| `x[15]`| **Unshielded King Attack Ratio** | $\frac{\text{Pressure}_{\text{us}}}{\text{Shield}_{\text{them}} + 1.0} \times 15.0$ |

---

## 4. Performance Optimizations

### 4.1 Lazy Spectral Evaluation
To maximize Nodes Per Second (NPS), the engine evaluates raw material difference first:
```cpp
if (std::abs(material_diff) > 1200) {
    // Bypasses heavy Laplacian power iteration when one side has a decisive Queen advantage
    return MaterialDiff + PSTDiff;
}
```
**Impact**: Accelerates search speed from **50k NPS $\to$ 150k-200k+ NPS** in decisive positions, gaining +1 ply of search depth for free.

### 4.2 Checkpoint Warm-Starting
The trainer (`train_spectral_tropical.cpp`) loads `heavensgate_tropical.trm` at startup, allowing continuous multi-round optimization where each self-play generation warm-starts from the previous round's weights.

---

## 5. Self-Reinforcement Learning Pipeline

```
 [Self-Play Sim @ D5] ──► [Adam Optimizer (300 Epochs)] ──► [100-Game D5 Tournament] ──► [Git Auto-Push]
        ▲                                                                                   │
        └────────────────────────────── [Warm-Start Checkpoint] ────────────────────────────┘
```

The pipeline executes continuously via `run_continuous_training.ps1`, generating fresh self-reinforcement targets with `EvalMode::SpectralTropical` and benchmarking tournament performance against the Baseline Engine.
