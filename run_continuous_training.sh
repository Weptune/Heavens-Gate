#!/bin/bash
# =============================================================================
# Heaven's Gate — Turnkey Cloud Continuous Training & Tournament Pipeline (Linux)
# =============================================================================

set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_DIR"

echo "========================================================================="
echo "  HEAVEN'S GATE — CLOUD CONTINUOUS TRAINING INITIALIZED"
echo "========================================================================="

while true; do
    # Determine current round number from history JSON
    ROUND_NUM=$(python3 -c "import json, os; h=json.load(open('tournament_detailed_history.json')) if os.path.exists('tournament_detailed_history.json') else []; print(max([r['round'] for r in h] + [0]) + 1)")
    PHASE_NUM=$(( ROUND_NUM <= 8 ? 1 : 2 ))
    PHASE_ROUND_NUM=$(( ROUND_NUM <= 8 ? ROUND_NUM : ROUND_NUM - 8 ))

    echo ""
    echo "========================================================================="
    echo "  HEAVEN'S GATE - PHASE $PHASE_NUM ROUND $PHASE_ROUND_NUM (Overall Round $ROUND_NUM)"
    echo "========================================================================="
    echo ""

    # 1. Compile Trainer with OpenMP & AVX2 SIMD Acceleration
    echo "[1/4] Compiling Trainer (AVX2 SIMD Accelerated)..."
    g++ -std=c++20 -O3 -mavx2 -mfma -fopenmp -Isrc tools/train_spectral_tropical.cpp \
        src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp \
        src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp \
        src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp \
        src/evaluation/tensor_eval.cpp src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp \
        src/evaluation/tensor_nnue.cpp src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp \
        src/evaluation/eval.cpp src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp \
        src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp \
        -o train_spectral_tropical

    # 2. Run Self-Play & SGD Training (250 Games @ Depth 7, 80 Epochs, LR=0.0005)
    echo "[2/4] Simulating 250 Games at Depth 7 and Training 80 Adam Epochs..."
    ./train_spectral_tropical 250 7 80 0.0005

    # 3. Compile Engine Binary (AVX2 SIMD & OpenMP Accelerated)
    echo "[3/4] Compiling Engine (AVX2 SIMD & OpenMP Accelerated)..."
    g++ -std=c++20 -O3 -mavx2 -mfma -fopenmp -Isrc src/main.cpp src/board/board.cpp src/core/fen.cpp src/core/zobrist.cpp src/core/polyglot.cpp \
        src/movegen/magic.cpp src/movegen/attack_masks.cpp src/movegen/movegen.cpp src/movegen/perft.cpp \
        src/evaluation/pst.cpp src/evaluation/eval_features.cpp src/evaluation/nnue.cpp src/evaluation/tensor_eval.cpp \
        src/evaluation/tensor_train.cpp src/evaluation/tensor_quant.cpp src/evaluation/tensor_nnue.cpp \
        src/evaluation/spectral_graph.cpp src/evaluation/tropical_eval.cpp src/evaluation/eval.cpp \
        src/search/move_picker.cpp src/search/tt.cpp src/search/search.cpp \
        src/visualization/exporter.cpp src/benchmark/metrics.cpp src/uci/uci.cpp \
        -o heavensgate

    # 4. Run 100-Game Depth 8 Grandmaster Tournament
    echo "[4/4] Running 100-Game Depth 8 Grandmaster Tournament..."
    ./heavensgate tournament 100 8

    # Archive PGN
    cp tournament_results.pgn "tournament_round_${ROUND_NUM}.pgn"

    # Generate Detailed Round Telemetry JSON & Summary Report
    echo "[Report] Generating Round $ROUND_NUM Telemetry Report..."
    python3 scratch/generate_round_report.py "tournament_round_${ROUND_NUM}.pgn" "$ROUND_NUM"
    python3 scratch/rebuild_history.py
    python3 scratch/build_master_history.py
    python3 scratch/extract_git_weight_history.py

    # 5. Commit and Push to GitHub (Telemetry & Model Weights only)
    COMMIT_MSG="Continuous Training Round ${ROUND_NUM} - 500 D8 Games, 80 Adam Epochs, 100 D8 Tournament"
    echo "[GitHub] Committing and Pushing Round ${ROUND_NUM} results..."
    git add tournament_history_summary.txt tournament_detailed_history.json model_weight_history.json heavensgate_tropical.trm
    git commit -m "$COMMIT_MSG" || true
    git push origin main || true

    echo "[SUCCESS] Round ${ROUND_NUM} Complete! Starting Next Round in 5s..."
    sleep 5
done
