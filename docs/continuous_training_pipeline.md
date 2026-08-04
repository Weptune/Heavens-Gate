# Standalone Continuous Training & Tournament Pipeline Guide

> *"Complete operational guide for Heaven's Gate continuous self-reinforcement training and automated tournament pipeline."*

---

## 1. Overview

The continuous training pipeline (`run_continuous_training.ps1`) drives Heaven's Gate self-play data generation, Adam gradient descent training, local tournament benchmarking, and automated Git commits.

It runs continuously in a loop, allowing you to train the engine overnight or in the background while working on other tasks.

---

## 2. Pipeline Execution Cycle

Each round follows a 4-step automated loop:

```
  ┌───────────────────────────────────────────────────────────┐
  │ [1/4] Recompiles Trainer & Engine binaries via g++ -O3   │
  └─────────────────────────────┬─────────────────────────────┘
                                │
  ┌─────────────────────────────▼─────────────────────────────┐
  │ [2/4] Simulates 500 Games @ D5 across 12 OpenMP Threads  │
  │       - Samples scored via MasterPositional Ground Truth  │
  │       - Trains 80 Adam Epochs on Tropical Surface        │
  │       - Saves checkpoint to 'heavensgate_tropical.trm'   │
  └─────────────────────────────┬─────────────────────────────┘
                                │
  ┌─────────────────────────────▼─────────────────────────────┐
  │ [3/4] Recompiles Engine with new trained weights          │
  └─────────────────────────────┬─────────────────────────────┘
                                │
  ┌─────────────────────────────▼─────────────────────────────┐
  │ [4/4] Runs 100-Game D5 Tournament vs. Baseline Engine     │
  │       - Saves games & telemetry to 'tournament_results.pgn│
  │       - Auto-commits weights & PGN to git origin/main     │
  └───────────────────────────────────────────────────────────┘
```

---

## 3. How to Run & Monitor

### 3.1 Launch Command
Open PowerShell and run:
```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\abhin\heavensgate\run_continuous_training.ps1"
```

### 3.2 Key Console Outputs to Watch
- **Dataset Generation Progress**:
  `Game  10/500 | Dataset:   1016 | Time: 42.9s`
  Shows dataset sample count and time elapsed.
- **Adam Training RMSE**:
  `[Epoch  80/80] RMSE: 150.53 cp | LR: 0.000904`
  Shows Root Mean Squared Error (RMSE) convergence across epochs.
- **Tournament Final Score**:
  `Score after Game 100: Master 54 - 38 Baseline (8 draws)`
  Shows win/loss/draw record and calculated Elo advantage.

---

## 4. Configuration Parameters (`train_spectral_tropical.cpp`)

To adjust pipeline settings, modify command-line arguments in `run_continuous_training.ps1`:

```powershell
.\train_spectral_tropical.exe [num_games] [depth] [epochs] [learning_rate]
```

- `num_games`: Default `500` games (~43,000 position samples).
- `depth`: Default `5` search depth per self-play move.
- `epochs`: Default `80` Adam gradient descent epochs.
- `learning_rate`: Default `0.002` starting learning rate.
