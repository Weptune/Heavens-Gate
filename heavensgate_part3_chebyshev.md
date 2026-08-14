---
title: "Heaven's Gate (Part 3): 4 Spatial Zones & Chebyshev 2-Hop Graph Convolutions"
description: "A first-principles deep dive into Graph Signal Processing, 4-Zone Spatial Subgraphs, Chebyshev Polynomials of the Graph Laplacian T2(L) = 2L~^2 - I, and the 25D feature vector."
date: 2026-08-10
tags: ['graph-signal-processing', 'chebyshev-polynomials', 'graph-convolutions', 'chess', 'cpp', 'maths']
image: './board_graph.png'
pinned: false
---

# Part 3: 4 Spatial Subgraph Zones & Chebyshev 2-Hop Graph Convolutions

## 3.1 Phase 3: 4-Zone Spatial Fiedler Subgraph Invariants

In Part 1 and Part 2, we extracted global algebraic connectivity $\lambda_2$ for the entire board graph. However, a chess board is spatially heterogeneous: White might have supreme piece coordination on the Kingside while suffering a catastrophic structural collapse on the Queenside.

Global $\lambda_2$ averages these forces together. To solve this, Phase 3 decomposes the board graph into **4 Localized Spatial Subgraphs**:

1. **Kingside Zone ($G_{\text{ks}}$)**: Pieces on files `f`, `g`, `h`. Measures kingside attacking energy and shield defense.
2. **Queenside Zone ($G_{\text{qs}}$)**: Pieces on files `a`, `b`, `c`. Measures queenside pawn pushes and rook file dominance.
3. **Center Zone ($G_{\text{ctr}}$)**: Pieces on central files `d`, `e`. Measures central space control.
4. **Back-Rank Zone ($G_{\text{br}}$)**: Pieces on ranks 1-2 for White, ranks 7-8 for Black. Measures back-rank vulnerability and piece development.

For each zone $Z \in \{\text{ks}, \text{qs}, \text{ctr}, \text{br}\}$, we extract per-side Fiedler values $\lambda_2(G_Z^{\text{Us}})$ and $\lambda_2(G_Z^{\text{Them}})$. This provides 8 additional spatial spectral features, allowing Master to evaluate localized flank attacks with precision.

```cpp
// 4-Zone Spatial Subgraph Extraction (src/evaluation/spectral_graph.cpp)
std::vector<std::pair<Piece, Square>> ks_us, ks_them;
std::vector<std::pair<Piece, Square>> qs_us, qs_them;
std::vector<std::pair<Piece, Square>> ctr_us, ctr_them;
std::vector<std::pair<Piece, Square>> br_us, br_them;

for (const auto& item : us_nodes) {
    int f = static_cast<int>(file_of(item.second));
    int r = static_cast<int>(rank_of(item.second));
    if (f >= 5) ks_us.push_back(item);
    if (f <= 2) qs_us.push_back(item);
    if (f == 3 || f == 4) ctr_us.push_back(item);
    if (r >= us_br_min && r <= us_br_max) br_us.push_back(item);
}

feat.fiedler_ks_us = compute_side_fiedler(ks_us);
feat.fiedler_ks_them = compute_side_fiedler(ks_them);
feat.fiedler_qs_us = compute_side_fiedler(qs_us);
feat.fiedler_qs_them = compute_side_fiedler(qs_them);
feat.fiedler_ctr_us = compute_side_fiedler(ctr_us);
feat.fiedler_ctr_them = compute_side_fiedler(ctr_them);
feat.fiedler_br_us = compute_side_fiedler(br_us);
feat.fiedler_br_them = compute_side_fiedler(br_them);
```

---

## 3.2 Phase 5: Graph Signal Processing & Chebyshev Graph Convolutions

In classical 1-hop adjacency matrices $A$, an edge only represents direct attack or defense. But tactical chess combinations depend on **multi-step indirect coordination**:
- A Queen aligned behind a Bishop on a long diagonal (a 2-step battery).
- A Rook doubled behind another Rook on an open file.
- A Knight guarding an attacking Bishop that directly targets the enemy King square.

In Graph Signal Processing (GSP), spectral graph convolutions are defined using the Graph Fourier Transform. For a graph signal $\mathbf{x} \in \mathbb{R}^N$ and filter $g_\theta$, the convolution is:

$$g_\theta \star \mathbf{x} = U g_\theta(\Lambda) U^T \mathbf{x}$$

Where $L = U \Lambda U^T$ is the Laplacian eigendecomposition.

Computing full matrix eigendecomposition $U$ inside a chess engine is far too expensive. Hammond et al. and Defferrard et al. proved that spectral graph filters can be efficiently approximated using **Chebyshev Polynomials**:

$$g_\theta(\tilde{L}) \approx \sum_{k=0}^K \theta_k T_k(\tilde{L})$$

Where $\tilde{L} = \frac{2}{\lambda_{\max}} L - I$ is the normalized Laplacian scaled to $[-1, 1]$.

### The Chebyshev Recurrence Relation

Chebyshev polynomials $T_k(x)$ are defined recursively:

$$T_0(x) = 1$$
$$T_1(x) = x$$
$$T_2(x) = 2x^2 - 1$$
$$T_3(x) = 4x^3 - 3x$$

The **2-Hop Chebyshev Graph Convolution ($T_2$)** evaluates 2-step indirect paths across the piece network:

$$T_2(\tilde{L}) = 2 \tilde{L}^2 - I$$

```cpp
// Phase 5: Chebyshev 2-Hop Graph Convolutions (src/evaluation/spectral_graph.cpp)
float cheb_us = 0.0f, cheb_them = 0.0f, cheb_king = 0.0f;
Square opp_king_sq = board.king_square(them);

for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        if (i == j) continue;
        float indirect_2hop = 0.0f;
        for (int k = 0; k < N; k++) {
            indirect_2hop += A[i * N + k] * A[k * N + j];
        }
        if (color_of(nodes[i].piece) == us) {
            cheb_us += indirect_2hop;
            if (opp_king_sq != Square::None && nodes[j].sq == opp_king_sq) {
                cheb_king += indirect_2hop;
            }
        } else {
            cheb_them += indirect_2hop;
        }
    }
}

feat.chebyshev_t2_us = cheb_us * 0.01f;
feat.chebyshev_t2_them = cheb_them * 0.01f;
feat.chebyshev_king_threat = cheb_king * 0.1f;
```

---

## 3.3 The Complete 25-Dimensional Feature Map

Combining Phase 1 through Phase 5 yields Heaven's Gate's **25-Dimensional Feature Vector ($\mathbf{x}_0 \dots \mathbf{x}_{24}$)**:

| Feature Index | Feature Code | Mathematical Graph Description |
| :--- | :--- | :--- |
| **x[0]** | `Material` | Scaled material imbalance: `(mat_us - mat_them) / 100.0` |
| **x[1]** | `Fiedler` | Global Fiedler value difference: `(λ₂_us - λ₂_them)` |
| **x[2]** | `Cohesion` | Subgraph piece cohesion: `(cohesion_us - cohesion_them)` |
| **x[3]** | `Gap` | Spectral gap difference: `(gap_us - gap_them)` |
| **x[4]** | `PST` | Piece-Square Table difference: `(pst_us - pst_them) / 100.0` |
| **x[5]** | `KingPress` | King pressure Laplacian attack energy: `(press_us - press_them)` |
| **x[6]** | `Battery` | Ray alignment battery energy: `(bat_us - bat_them)` |
| **x[7]** | `PawnCoh` | Pawn chain Laplacian cohesion difference |
| **x[8]** | `Trace` | Total Board Laplacian trace energy difference |
| **x[9]** | `Mobility` | Attack-degree mobility difference: `(mob_us - mob_them) * 3.0` |
| **x[10]** | `Center` | Center square control difference: `(ctr_us - ctr_them) * 8.0` |
| **x[11]** | `Phase` | Normalized Game Phase: `game_phase * 50.0` |
| **x[12]** | `Shield` | King shield pawn Laplacian energy difference |
| **x[13]** | `Passed` | Passed pawn rank-scaled difference: `(pass_us - pass_them) * 0.8` |
| **x[14]** | `EG_Passed` | Endgame passed pawn acceleration: `x[13] * (1.0 - phase)` |
| **x[15]** | `Attack_Ratio` | King pressure to battery ratio difference |
| **x[16..21]** | `Cross-Terms` | Residual cross-terms (BatXCenter, FiedXPWN, KingXBat, ShldXPWN) |
| **x[22]** | `Cheb_T2_Us` | 2-Hop indirect battery/support graph convolution for Us |
| **x[23]** | `Cheb_T2_Them` | 2-Hop indirect battery/support graph convolution for Opponent |
| **x[24]** | `Cheb_K_Threat` | 2-Hop indirect graph attack paths targeting enemy King 👑 |
