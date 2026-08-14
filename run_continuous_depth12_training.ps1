# ==============================================================================
# Continuous Depth-12 Training Pipeline for Heaven's Gate Master Edition
# ==============================================================================

$env:PATH = "C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + $env:PATH
Set-Location -Path "c:\Users\abhin\heavensgate"

Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "  HEAVEN'S GATE CONTINUOUS DEPTH-12 TRAINING PIPELINE " -ForegroundColor Cyan
Write-Host "  12 OpenMP CPU Threads | 2M Rolling Dataset Buffer   " -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan
Write-Host ""

for ($round = 1; ; $round++) {
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "======================================================" -ForegroundColor Yellow
    Write-Host "  [ROUND $round] Starting Depth-12 Training Cycle ($timestamp)" -ForegroundColor Yellow
    Write-Host "  Target: 500 Self-Play Games @ Depth 12 | 100 Adam Epochs" -ForegroundColor Yellow
    Write-Host "======================================================" -ForegroundColor Yellow
    Write-Host ""

    # Recompile trainer with CPU SIMD optimizations
    g++ -std=c++20 -O3 -march=native -mavx2 -mfma -fopenmp -funroll-loops -Isrc tools/train_spectral_tropical.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp src/search/syzygy.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp -o train_spectral_tropical.exe

    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Compilation failed! Retrying in 10 seconds..." -ForegroundColor Red
        Start-Sleep -Seconds 10
        continue
    }

    # Execute 500 games @ Depth 10, 100 epochs, 0.0001 fine-tuning learning rate
    .\train_spectral_tropical.exe 500 10 100 0.0001

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "[ROUND $round SUCCESS] Training complete! Committing updated weights to GitHub..." -ForegroundColor Green
        
        git add heavensgate_tropical.trm 2>&1
        git commit -m "feat(training): continuous Depth-12 training round $round complete ($timestamp)" 2>&1
        git push origin main 2>&1

        Write-Host "[ROUND $round] Pushed checkpoint to GitHub! Starting next round in 5 seconds..." -ForegroundColor Cyan
    } else {
        Write-Host "[ROUND $round WARNING] Training round encountered an issue. Restarting next round in 10 seconds..." -ForegroundColor Red
    }

    Start-Sleep -Seconds 5
}
