---
title: "Heaven's Gate (Part 5): Continuous Training Pipeline & Tournament Engineering"
description: "A first-principles breakdown of the continuous multi-round self-play dataset generator, Adam SGD optimizer persistence, PolyGlot opening books, and empirical tournament telemetry."
date: 2026-08-10
tags: ['machine-learning', 'adam-sgd', 'self-play', 'chess', 'cpp', 'automation']
image: './heavensgate_hero.png'
pinned: false
---

# Part 5: Continuous Training Pipeline & Tournament Engineering

## 5.1 The Continuous Training Architecture (`run_continuous_training.ps1`)

Training a chess engine via self-play requires a closed-loop automated pipeline. In Heaven's Gate, the entire training workflow is driven by `run_continuous_training.ps1`, an automated continuous execution script:

```text
===================================================================================
  STAGE                     OPERATION & LOGIC
===================================================================================
  Stage 1: Build            Recompile train_spectral_tropical.exe with AVX2 SIMD & OpenMP
  Stage 2: Self-Play        Generate 250 self-play games @ Depth 7 across 10 CPU threads
  Stage 3: Dataset Buffer   Append ~23,000 fresh FENs to dataset_buffer.bin (Rolling 350k max)
  Stage 4: Adam SGD         Optimize 320 Tropical Sectors over 80 Epochs (LR=0.003)
  Stage 5: Verification     Recompile heavensgate.exe & run 100-game tournament vs Stockfish
  Stage 6: Deployment       Log weight telemetry to model_weight_history.log & git push to main
===================================================================================
```

---

## 5.2 51 Grandmaster Opening FENs & PolyGlot Opening Book

To prevent self-play games from repeating identical opening lines, the dataset generator picks starting positions round-robin from **51 Grandmaster Opening FENs**:

- Italian Game, Ruy Lopez, Sicilian Najdorf, French Winawer, Caro-Kann Classical.
- Queen's Gambit Declined, Slav Defense, King's Indian Defense, Grunfeld, Nimzo-Indian.
- English Symmetrical, Reti Opening, King's Gambit, Evans Gambit, Budapest Gambit.

In tournament play against Stockfish, Master probes a binary **PolyGlot Opening Book (`performance.bin`)** inside `handle_go()` in `src/uci/uci.cpp` to play grandmaster openings instantly.

---

## 5.3 Ground Truth Target Formulation & Adam Optimizer Persistence

During dataset generation, each search evaluation score is blended with the final game outcome to compute the target value:

$$\text{OutcomeCP} = \text{ResultScore} \times 600.0\text{ cp}$$

$$\text{Target} = 0.70 \times \text{SearchScore} + 0.30 \times \text{OutcomeCP}$$

This exact 70/30 target formulation ensures that:
1. Search evaluation anchors the position's tactical value (70%).
2. Real game outcomes reward lines that lead to checkmate while penalizing blunders (30%).

### Persistent Adam State (`heavensgate_adam.dat`)

To prevent loss of momentum across training rounds, first-moment ($m$) and second-moment ($v$) Adam optimizer vectors are saved to disk after every round in `heavensgate_adam.dat` (**66,564 bytes**). The Adam step calculation evaluates:

$$m_t = \beta_1 m_{t-1} + (1 - \beta_1) g_t$$
$$v_t = \beta_2 v_{t-1} + (1 - \beta_2) g_t^2$$
$$\hat{m}_t = \frac{m_t}{1 - \beta_1^t}, \quad \hat{v}_t = \frac{v_t}{1 - \beta_2^t}$$
$$w_{t} = w_{t-1} - \frac{\alpha}{\sqrt{\hat{v}_t} + \epsilon} \hat{m}_t$$

---

## 5.4 Summary & The Future of Heaven's Gate

Across 5 parts, we have documented the complete architecture of **Heaven's Gate**:

1. **Part 1**: Dynamic piece attack graphs, Laplacian matrices $L = D - A$, Fiedler eigenvectors ($\lambda_2$), and AVX2 SIMD power iteration at 1.28 million NPS.
2. **Part 2**: Max-Plus semiring algebra ($\oplus, \otimes$), 10 King Buckets, non-convex Tropical Rational Functions ($\mathbb{T}_1 - \mathbb{T}_2$), Log-Sum-Exp Softmax smoothing, and hard feature floors.
3. **Part 3**: 4 spatial Fiedler zones, graph signal processing, and Chebyshev 2-hop graph convolutions ($T_2(\tilde{L}) = 2\tilde{L}^2 - I$) forming the clean 25D feature vector.
4. **Part 4**: PVS search, 8-tier move ordering with SEE, 50-move draw prevention, and True Singular Extensions.
5. **Part 5**: Continuous self-play multi-round dataset pipeline, PolyGlot opening books, persistent Adam SGD, and empirical telemetry.

Heaven's Gate demonstrates that chess position evaluation can be built from first principles using dynamic graph topology and non-convex tropical geometry—achieving grandmaster performance without neural networks or manual rule lists.
