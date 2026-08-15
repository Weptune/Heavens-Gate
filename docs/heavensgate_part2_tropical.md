---
title: "Heaven's Gate (Part 2): Tropical Geometry & Dual-Surface Rational Functions"
description: "A first-principles mathematical breakdown of Max-Plus semiring algebra, 10 Spatial King Buckets, non-convex Tropical Rational Functions (T1 - T2), Log-Sum-Exp Softmax smoothing, and Adam SGD gradient mechanics."
date: 2026-08-10
tags: ['tropical-geometry', 'semiring', 'chess', 'cpp', 'machine-learning', 'maths']
image: './tropical_surface.png'
pinned: false
---

# Part 2: Tropical Geometry & Dual-Surface Rational Functions

## 2.1 The Failure of Linear Evaluation Functions

In Part 1, we extracted a 25-dimensional feature vector $\mathbf{x} \in \mathbb{R}^{25}$ representing material, piece cohesion, spectral Fiedler values ($\lambda_2$), and Laplacian trace energy.

Now comes the fundamental question of engine architecture: **How do you map a feature vector $\mathbf{x}$ into a single evaluation score in centipawns?**

Traditional Hand-Crafted Evaluation (HCE) uses a simple linear dot product:

$$f(\mathbf{x}) = \mathbf{w}^T \mathbf{x} + b = \sum_{i=1}^{25} w_i x_i + b$$

Linear functions are fast, but mathematically incapable of playing high-level chess because positional features interact non-linearly:
- A pawn shield ($x_{12}$) is priceless when your king is under attack, but worthless if your king has already castled to the opposite flank.
- A rook battery ($x_6$) on an open file is devastating, unless your opponent has a knight firmly anchored on an outpost blockading the file.

Linear models cannot express "if-then" tactical conditional boundaries without hardcoded `if` statements. On the other hand, traditional neural networks (like NNUE) handle non-linearity by passing features through matrix multiplications and non-linear activation functions (like ReLU or Clipped ReLU). But NNUE acts as a black box: it loses all geometric interpretability, and its millions of parameters require massive memory bandwidth.

In Heaven's Gate, we solve this using **Tropical Geometry** and **Tropical Semiring Algebra**.

---

## 2.2 What is Tropical Geometry?

Tropical geometry is a branch of mathematics that replaces standard arithmetic operations with the **Max-Plus Semiring** $(\mathbb{T}, \oplus, \otimes)$:

$$\mathbb{T} = \mathbb{R} \cup \{-\infty\}$$

In the tropical semiring, standard addition ($+$) is replaced by the maximum operator ($\oplus$), and standard multiplication ($\times$) is replaced by addition ($\otimes$):

$$\text{Tropical Addition: } a \oplus b = \max(a, b)$$

$$\text{Tropical Multiplication: } a \otimes b = a + b$$

Under tropical algebra, familiar algebraic expressions take on a completely different geometric meaning. Consider a standard polynomial $P(x) = c_0 + c_1 x + c_2 x^2$. In tropical algebra, this becomes a **Tropical Polynomial**:

$$P_{\text{trop}}(x) = c_0 \oplus (c_1 \otimes x) \oplus (c_2 \otimes x^{\otimes 2}) = \max(c_0, c_1 + x, c_2 + 2x)$$

**A tropical polynomial is a piecewise-linear convex function formed by taking the maximum over a set of linear affine hyperplanes!**

![Tropical Polyhedron Surface Construction](./tropical_surface.png)

### The Deep Connection: Tropical Algebra IS Minimax Algebra

In computer science, the fundamental search algorithm for two-player zero-sum games is **Minimax**. When an engine searches a game tree, MAX nodes take the maximum of child scores ($\max$), and MIN nodes take the minimum ($-\max(-)$).

Tropical algebra is the natural mathematical language of minimax search. Evaluating a position through a tropical polynomial is algebraically equivalent to evaluating a set of competing positional hypotheses and selecting the dominant tactical sector.

---

## 2.3 Phase 2: Spatial Sectors & 10 King Buckets

A single global evaluation surface cannot accurately evaluate the entire game of chess. The positional value of a knight on `e5` is completely different depending on whether the enemy king is castled on `g8` (Kingside), `c8` (Queenside), or trapped in the center `e1`.

To solve this, Heaven's Gate partitions the evaluation space into **10 Spatial King Buckets**:

| Bucket ID | Enemy King Location | Strategic Focus |
| :--- | :--- | :--- |
| **Bucket 0** | White King on Rank 1-2 (f,g,h) | Kingside Shield Protection |
| **Bucket 1** | White King on Rank 1-2 (a,b,c) | Queenside Defense & Expansion |
| **Bucket 2** | White King on Rank 1-2 (d,e) | Central King Safety & Back-Rank |
| **Bucket 3** | White King in Center (Ranks 3-5) | Open-File Attack & Mating Nets |
| **Bucket 4** | White King Advanced (Ranks 6-8) | Endgame King Hunting |
| **Bucket 5-9** | Black King Equivalent Buckets | Symmetric Spatial Partition |

Within each King Bucket $b \in \{0 \dots 9\}$, Master maintains **16 distinct spatial sectors**. Each sector $j$ consists of a weight vector $\mathbf{w}_j \in \mathbb{R}^{25}$ and a bias scalar $b_j \in \mathbb{R}$.

A single tropical evaluation surface $\mathbb{T}(\mathbf{x})$ evaluates a position by selecting the dominant sector hyperplane:

$$\mathbb{T}(\mathbf{x}) = \bigoplus_{j=1}^{16} (\mathbf{w}_j^T \mathbf{x} + b_j) = \max_{j=1}^{16} \left( \sum_{i=1}^{25} w_{j,i} x_i + b_j \right)$$

---

## 2.4 Phase 4: Non-Convex Tropical Rational Functions ($\mathbb{T}_1 - \mathbb{T}_2$)

A single tropical polynomial $\mathbb{T}(\mathbf{x}) = \max_j (\mathbf{w}_j^T \mathbf{x} + b_j)$ is strictly **convex** (an upper envelope of linear hyperplanes). 

While convex functions are powerful, chess evaluation surfaces are non-convex (pins, forks, sacrifices, piece traps). To break convexity without introducing neural network complexity, Heaven's Gate upgrades the evaluator to a **Tropical Rational Function**:

$$f(\mathbf{x}) = \mathbb{T}_1(\mathbf{x}) - \mathbb{T}_2(\mathbf{x})$$

Where:
1. **$\mathbb{T}_1(\mathbf{x})$ (160 Advantage Sectors)**: Evaluates material dominance, attack pressure, piece mobility, passed pawns, and Fiedler graph cohesion across 10 King Buckets.
2. **$\mathbb{T}_2(\mathbf{x})$ (160 Vulnerability Sectors)**: Evaluates king exposure, pawn weaknesses, enemy counter-attacks, and structural bottlenecks.

$$\mathbb{T}_1(\mathbf{x}) = \max_{j=1}^{16} \left( \mathbf{w}_{1,j}^T \mathbf{x} + b_{1,j} \right), \quad \mathbb{T}_2(\mathbf{x}) = \max_{k=1}^{16} \left( \mathbf{w}_{2,k}^T \mathbf{x} + b_{2,k} \right)$$

By subtracting the vulnerability surface $\mathbb{T}_2(\mathbf{x})$ from the advantage surface $\mathbb{T}_1(\mathbf{x})$, $f(\mathbf{x})$ becomes a **general non-convex piecewise-linear surface**. It can model sharp tactical penalties and piece traps while retaining 100% numerical stability on our clean 25D feature vector!

![Dual-Surface Tropical Rational Function](./tropical_rational_surface.png)

---

## 2.5 Differentiable Softmax Smoothing ($\text{LSE}_\tau$) for Adam SGD Training

To allow gradient descent (Adam SGD) to optimize all sectors smoothly during self-play training, we apply **Log-Sum-Exp (LSE)** smoothing with temperature parameter $\tau = 3.0$:

$$\mathbb{T}_1^\tau(\mathbf{x}) = \tau \log \left( \sum_{j=1}^{16} \exp \left( \frac{\mathbf{w}_{1,j}^T \mathbf{x} + b_{1,j}}{\tau} \right) \right)$$

The partial derivatives yield the **Softmax probability distribution**:

$$p_{1,j} = \frac{\partial \mathbb{T}_1^\tau}{\partial b_{1,j}} = \frac{\exp\left(\frac{\mathbf{w}_{1,j}^T \mathbf{x} + b_{1,j}}{\tau}\right)}{\sum_{m=1}^{16} \exp\left(\frac{\mathbf{w}_{1,m}^T \mathbf{x} + b_{1,m}}{\tau}\right)}$$

During training, Adam SGD updates sector weights in proportion to their Softmax contribution ($+e \cdot p_{1,j} \cdot \mathbf{x}$ for $\mathbb{T}_1$, $-e \cdot p_{2,k} \cdot \mathbf{x}$ for $\mathbb{T}_2$).

```cpp
// C++20 Dual-Surface Evaluation Core (src/evaluation/tropical_eval.cpp)
TropicalEvaluator::EvalResult TropicalEvaluator::evaluate_detailed_from_features(
    const std::array<float, NUM_FEATURES>& x, size_t bucket) const 
{
    EvalResult res{};
    size_t base_sec_idx = bucket * SECTORS_PER_SURFACE;

    // 1. Evaluate T1 (Advantage Surface)
    std::array<float, SECTORS_PER_SURFACE> t1_vals;
    float max_t1 = -1e9f;
    size_t win_t1 = 0;

    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        const auto& sec = sectors_t1_[base_sec_idx + j];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) val += sec.w[i] * x[i];
        t1_vals[j] = val;
        if (val > max_t1) { max_t1 = val; win_t1 = j; }
    }

    float sum_exp_t1 = 0.0f;
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) {
        float exp_val = std::exp((t1_vals[j] - max_t1) / SMOOTH_TAU);
        res.softmax_t1[j] = exp_val;
        sum_exp_t1 += exp_val;
    }
    for (size_t j = 0; j < SECTORS_PER_SURFACE; j++) res.softmax_t1[j] /= sum_exp_t1;
    float t1_smooth = max_t1 + SMOOTH_TAU * (std::log(sum_exp_t1) - std::log(static_cast<float>(SECTORS_PER_SURFACE)));

    // 2. Evaluate T2 (Vulnerability Surface)
    std::array<float, SECTORS_PER_SURFACE> t2_vals;
    float max_t2 = -1e9f;
    size_t win_t2 = 0;

    for (size_t k = 0; k < SECTORS_PER_SURFACE; k++) {
        const auto& sec = sectors_t2_[base_sec_idx + k];
        float val = sec.b;
        for (size_t i = 0; i < NUM_FEATURES; i++) val += sec.w[i] * x[i];
        t2_vals[k] = val;
        if (val > max_t2) { max_t2 = val; win_t2 = k; }
    }

    float sum_exp_t2 = 0.0f;
    for (size_t k = 0; k < SECTORS_PER_SURFACE; k++) {
        float exp_val = std::exp((t2_vals[k] - max_t2) / SMOOTH_TAU);
        res.softmax_t2[k] = exp_val;
        sum_exp_t2 += exp_val;
    }
    for (size_t k = 0; k < SECTORS_PER_SURFACE; k++) res.softmax_t2[k] /= sum_exp_t2;
    float t2_smooth = max_t2 + SMOOTH_TAU * (std::log(sum_exp_t2) - std::log(static_cast<float>(SECTORS_PER_SURFACE)));

    // 3. Compute Tropical Rational Difference: f(x) = T1(x) - T2(x)
    float rational_eval_units = t1_smooth - t2_smooth;

    res.score = static_cast<int>(std::round(rational_eval_units * 10.0f));
    res.winning_sector_t1 = base_sec_idx + win_t1;
    res.winning_sector_t2 = base_sec_idx + win_t2;

    return res;
}
```

---

## 2.6 Hard Feature Floor Protection (`feature_floors[25]`)

In early training runs, unconstrained SGD occasionally suffered from feature erosion—driving positional features like piece mobility ($x_9$) down to zero when penalized by blunder losses. 

To solve this, Heaven's Gate enforces **Hard Feature Floors (`feature_floors[25]`)** in `tools/train_spectral_tropical.cpp`. After every Adam SGD step, weights are clamped:

$$w_{j,i} \leftarrow \max\left(\text{feature\_floors}[i], \, w_{j,i}\right)$$

For example, Material is locked to $w_0 \ge 0.85$, Mobility is locked to $w_9 \ge 0.25$, and King Shield is locked to $w_{12} \ge 0.25$. This ensures the model has full freedom to optimize weights across $[0.25, 5.00]$ while guaranteeing that core positional concepts can never be eroded to zero by gradient noise.
