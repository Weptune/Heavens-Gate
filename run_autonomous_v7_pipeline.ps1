# =============================================================================
# Heaven's Gate - Autonomous Multi-Round Deep Training & Benchmark Pipeline
# =============================================================================

param (
    [int]$Rounds = 50,
    [int]$Games = 150,
    [int]$Depth = 12,
    [int]$Epochs = 80,
    [float]$LR = 0.0008,
    [int]$SfElo = 3400,
    [int]$TournamentGames = 10
)

$env:PATH = "C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + $env:PATH
Set-Location "c:\Users\abhin\heavensgate"

Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  HEAVEN'S GATE - AUTONOMOUS DEEP LEARNING AND BENCHMARK PIPELINE     " -ForegroundColor Cyan
Write-Host "  SPSA-Optimized Search | Bitmask TT | Bounded Tropical Adam SGD      " -ForegroundColor Cyan
Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "Settings: Rounds=$Rounds | Games/Round=$Games | Depth=$Depth | Epochs=$Epochs | SF Elo=$SfElo" -ForegroundColor Yellow
Write-Host ""

# Ensure continuous trainer is compiled
Write-Host "[Pipeline] Building train_continuous_v7.exe..." -ForegroundColor Yellow
g++ -std=c++20 -O3 -march=native -mavx2 -mfma -fopenmp -funroll-loops -Isrc tools/train_continuous_v7.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search_params.cpp src/search/search.cpp src/search/syzygy.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/benchmark/sts.cpp src/uci/uci.cpp -o train_continuous_v7.exe 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to build train_continuous_v7.exe" -ForegroundColor Red
    exit 1
}

# Ensure engine is compiled
Write-Host "[Pipeline] Building heavensgate.exe..." -ForegroundColor Yellow
g++ -std=c++20 -O3 -march=native -mavx2 -mfma -fopenmp -funroll-loops -Isrc src/main.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search_params.cpp src/search/search.cpp src/search/syzygy.cpp src/visualization/exporter.cpp src/benchmark/metrics.cpp src/benchmark/sts.cpp src/uci/uci.cpp -o heavensgate.exe 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Failed to build heavensgate.exe" -ForegroundColor Red
    exit 1
}

$LogFile = "tournament_v7_history.log"

for ($r = 1; $r -le $Rounds; $r++) {
    $TimeStamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host ""
    Write-Host "======================================================================" -ForegroundColor Green
    Write-Host "  STARTING ROUND $r OF $Rounds [$TimeStamp]" -ForegroundColor Green
    Write-Host "======================================================================" -ForegroundColor Green

    # 1. Run Self-Play and Training
    Write-Host "[Round $r] Running Depth-$Depth Self-Play Simulation ($Games games) and Adam Optimization..." -ForegroundColor Yellow
    .\train_continuous_v7.exe $Games $Depth $Epochs $LR
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[WARNING] Training step encountered error code $LASTEXITCODE. Continuing..." -ForegroundColor Yellow
    }

    # 2. Archive checkpoint
    $CheckpointName = "heavensgate_v7_round$r.trm"
    Copy-Item "heavensgate_tropical.trm" $CheckpointName -Force
    Write-Host "[Round $r] Saved checkpoint to $CheckpointName" -ForegroundColor Cyan

    # 3. Run Benchmark Tournament vs Stockfish
    Write-Host "[Round $r] Running $TournamentGames-Game Tournament vs Stockfish $SfElo Elo..." -ForegroundColor Yellow
    .\heavensgate.exe tournament $TournamentGames 120 0 1 $SfElo 6

    # 4. Log Round Summary
    $SummaryLine = "Round $r [$TimeStamp] | Model: $CheckpointName | SF Elo: $SfElo"
    Add-Content -Path $LogFile -Value $SummaryLine
    Write-Host "[Round $r Complete] Logged to $LogFile" -ForegroundColor Green
}

Write-Host ""
Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  ALL $Rounds ROUNDS COMPLETED SUCCESSFULLY!                          " -ForegroundColor Cyan
Write-Host "======================================================================" -ForegroundColor Cyan
