# Heaven's Gate — Standalone Continuous Training & Tournament Pipeline
# Run this script directly in PowerShell to keep training continuously,
# even if Antigravity / IDE is completely closed!

Set-Location -Path "C:\Users\abhin\heavensgate"
$env:PATH = "C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + $env:PATH

$round_num = 1

while ($true) {
    Write-Host "`n========================================================" -ForegroundColor Cyan
    Write-Host "  HEAVEN'S GATE - CONTINUOUS TRAINING ROUND $round_num" -ForegroundColor Green
    Write-Host "========================================================`n" -ForegroundColor Cyan

    # 1. Kill any existing instances
    Stop-Process -Name train_spectral_tropical,heavensgate -Force -ErrorAction SilentlyContinue

    # 2. Recompile Trainer
    Write-Host "[1/4] Recompiling Trainer..." -ForegroundColor Yellow
    g++ -std=c++20 -O3 -fopenmp -Isrc tools/train_spectral_tropical.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o train_spectral_tropical.exe

    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Trainer compilation failed! Retrying in 10s..." -ForegroundColor Red
        Start-Sleep -Seconds 10
        continue
    }

    # 3. Simulate 500 Games at Depth 5 and Train 300 Adam Epochs (Warm-Started)
    Write-Host "[2/4] Simulating 500 Games at Depth 5 and Training 300 Adam Epochs..." -ForegroundColor Yellow
    .\train_spectral_tropical.exe 500 5 300 0.002

    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Training failed! Retrying in 10s..." -ForegroundColor Red
        Start-Sleep -Seconds 10
        continue
    }

    # 4. Recompile Engine
    Write-Host "[3/4] Recompiling Engine..." -ForegroundColor Yellow
    g++ -std=c++20 -O3 -Isrc src/main.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o heavensgate.exe

    # 5. Run 100-Game Depth 5 Grandmaster Tournament
    Write-Host "[4/4] Running 100-Game Depth 5 Grandmaster Tournament..." -ForegroundColor Yellow
    .\heavensgate.exe tournament 100 5

    # 6. Commit and Push to GitHub
    $commit_msg = "Continuous Training Round " + $round_num + " - 500 D5 Games, 300 Adam Epochs, 100 D5 Tournament"
    Write-Host "`n[GitHub] Committing and Pushing Round $round_num results..." -ForegroundColor Cyan
    git add .
    git commit -m $commit_msg
    git push origin main

    Write-Host "`n[SUCCESS] Round $round_num Complete! Starting Next Round in 5s..." -ForegroundColor Green
    Start-Sleep -Seconds 5
    $round_num++
}
