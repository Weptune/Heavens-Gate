# Heaven's Gate — Standalone Continuous Training & Tournament Pipeline
# Run this script directly in PowerShell to keep training continuously,
# even if Antigravity / IDE is completely closed!

Set-Location -Path "C:\Users\abhin\heavensgate"
$env:PATH = "C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + $env:PATH
$env:OMP_NUM_THREADS = "10"

$max_history_round = python -c "import json; h=json.load(open('tournament_detailed_history.json')); print(max(r['round'] for r in h))"
if ($max_history_round) {
    $round_num = [int]$max_history_round + 1
} else {
    $round_num = 1
}

while ($true) {
    $phase_num = 2
    $phase_round_num = $round_num - 8
    $phase_title = "Phase 2: 22 Features (Pure Convex Single-Surface T1)"

    Write-Host "`n========================================================" -ForegroundColor Cyan
    Write-Host "  HEAVEN'S GATE - $phase_title ROUND $phase_round_num (Overall Round $round_num)" -ForegroundColor Green
    Write-Host "========================================================`n" -ForegroundColor Cyan

    # 1. Kill any existing instances
    Stop-Process -Name train_spectral_tropical,heavensgate -Force -ErrorAction SilentlyContinue

    # 2. Recompile Trainer
    Write-Host "[1/4] Recompiling Trainer (AVX2 SIMD Accelerated)..." -ForegroundColor Yellow
    g++ -std=c++20 -O3 -mavx2 -mfma -fopenmp -Isrc tools/train_spectral_tropical.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o train_spectral_tropical.exe

    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Trainer compilation failed! Retrying in 10s..." -ForegroundColor Red
        Start-Sleep -Seconds 10
        continue
    }

    # 3. Simulate 250 Games at Depth 7 and Train 80 Adam Epochs (LR=0.003)
    Write-Host "[2/4] Simulating 250 Games at Depth 7 and Training 80 Adam Epochs (LR=0.003)..." -ForegroundColor Yellow
    .\train_spectral_tropical.exe 250 7 80 0.003

    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Training failed! Retrying in 10s..." -ForegroundColor Red
        Start-Sleep -Seconds 10
        continue
    }

    # 4. Recompile Engine (AVX2 SIMD & OpenMP Accelerated)
    Write-Host "[3/4] Recompiling Engine (AVX2 SIMD & OpenMP Accelerated)..." -ForegroundColor Yellow
    g++ -std=c++20 -O3 -mavx2 -mfma -fopenmp -Isrc src/main.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o heavensgate.exe

    # 5. Run 100-Game 1000ms Time Control Grandmaster Tournament (Native C++ OpenMP Parallelized)
    Write-Host "[4/4] Running 100-Game 1000ms Time Control Grandmaster Tournament (10 CPU Cores Dynamic)..." -ForegroundColor Yellow
    if (Test-Path -Path "tournament_results.pgn") { Remove-Item -Path "tournament_results.pgn" -Force }
    .\heavensgate.exe tournament 100 1000

    # Archive PGN per round into pgn_history/ folder
    if (!(Test-Path -Path "pgn_history")) { New-Item -ItemType Directory -Path "pgn_history" }
    $pgn_dst = "pgn_history/tournament_round_" + $round_num + ".pgn"
    Copy-Item -Path "tournament_results.pgn" -Destination $pgn_dst -Force

    # Generate Detailed Round Telemetry JSON & Summary Report
    Write-Host "`n[Report] Generating Round $round_num Telemetry Report..." -ForegroundColor Green
    python scratch/generate_round_report.py $pgn_dst $round_num
    python scratch/rebuild_history.py
    python scratch/build_master_history.py
    python scratch/extract_git_weight_history.py

    # 6. Commit and Push to GitHub (Telemetry & Model Weights only)
    $commit_msg = "Continuous Training Round " + $round_num + " - 250 D7 Games, 80 Adam Epochs, 100 D8 Tournament"
    Write-Host "`n[GitHub] Committing and Pushing Round $round_num results..." -ForegroundColor Cyan
    git add tournament_history_summary.txt tournament_detailed_history.json model_weight_history.json heavensgate_tropical.trm
    git commit -m "$commit_msg"
    git push origin main

    Write-Host "`n[SUCCESS] Round $round_num Complete! Starting Next Round in 5s..." -ForegroundColor Green
    Start-Sleep -Seconds 5
    $round_num++
}
