import subprocess
import os
import json
import random
import re
import numpy as np
import time

PARAM_DEFS = {
    "lmr_divisor": {"type": "float", "val": 2.20, "c": 0.15, "min": 1.60, "max": 3.20, "uci": "LMR_Divisor"},
    "lmr_hist_bonus": {"type": "int", "val": 500, "c": 60, "min": 150, "max": 1500, "uci": "LMR_HistBonus"},
    "lmr_hist_malus": {"type": "int", "val": 100, "c": 25, "min": 25, "max": 400, "uci": "LMR_HistMalus"},
    "rfp_margin": {"type": "int", "val": 120, "c": 15, "min": 60, "max": 250, "uci": "RFP_Margin"},
    "futility_margin": {"type": "int", "val": 200, "c": 25, "min": 100, "max": 350, "uci": "Futility_Margin"},
    "see_bad_capture_slope": {"type": "int", "val": 100, "c": 15, "min": 40, "max": 200, "uci": "SEE_BadCaptureSlope"},
    "see_quiet_slope": {"type": "int", "val": 40, "c": 8, "min": 15, "max": 100, "uci": "SEE_QuietSlope"},
    "nmp_eval_margin": {"type": "int", "val": 200, "c": 30, "min": 80, "max": 400, "uci": "NMP_EvalMargin"},
}

GXX = r"C:\Users\abhin\heavensgate\tools\w64devkit\bin\g++.exe"
ENGINE_EXE = r"c:\Users\abhin\heavensgate\heavensgate.exe"
ENV = os.environ.copy()
ENV["PATH"] = r"C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + ENV.get("PATH", "")

def update_header_and_compile(params):
    header_code = f"""#pragma once

namespace heavensgate {{

struct SearchParams {{
    float lmr_divisor = {params['lmr_divisor']:.4f}f;
    int lmr_hist_bonus = {int(params['lmr_hist_bonus'])};
    int lmr_hist_malus = {int(params['lmr_hist_malus'])};
    int rfp_margin = {int(params['rfp_margin'])};
    int futility_margin = {int(params['futility_margin'])};
    int see_bad_capture_slope = {int(params['see_bad_capture_slope'])};
    int see_quiet_slope = {int(params['see_quiet_slope'])};
    int nmp_eval_margin = {int(params['nmp_eval_margin'])};

    void reset() noexcept {{
        lmr_divisor = 2.20f;
        lmr_hist_bonus = 500;
        lmr_hist_malus = 100;
        rfp_margin = 120;
        futility_margin = 200;
        see_bad_capture_slope = 100;
        see_quiet_slope = 40;
        nmp_eval_margin = 200;
    }}
}};

extern SearchParams g_search_params;

}} // namespace heavensgate
"""
    with open(r"c:\Users\abhin\heavensgate\src\search\search_params.hpp", "w", encoding="utf-8") as f:
        f.write(header_code)
    
    # Recompile
    cmd = [
        GXX, "-std=c++20", "-O3", "-march=native", "-mavx2", "-mfma", "-fopenmp", "-funroll-loops",
        "-Isrc", "src/main.cpp", "src/board/board.cpp", "src/core/fen.cpp", "src/core/zobrist.cpp",
        "src/core/polyglot.cpp", "src/movegen/magic.cpp", "src/movegen/attack_masks.cpp", "src/movegen/movegen.cpp",
        "src/movegen/perft.cpp", "src/evaluation/pst.cpp", "src/evaluation/eval_features.cpp",
        "src/evaluation/nnue.cpp", "src/evaluation/tensor_eval.cpp", "src/evaluation/tensor_train.cpp",
        "src/evaluation/tensor_quant.cpp", "src/evaluation/tensor_nnue.cpp", "src/evaluation/spectral_graph.cpp",
        "src/evaluation/tropical_eval.cpp", "src/evaluation/eval.cpp", "src/search/move_picker.cpp",
        "src/search/tt.cpp", "src/search/search_params.cpp", "src/search/search.cpp", "src/search/syzygy.cpp",
        "src/visualization/exporter.cpp", "src/benchmark/metrics.cpp", "src/benchmark/sts.cpp", "src/uci/uci.cpp",
        "-o", "heavensgate.exe"
    ]
    res = subprocess.run(cmd, cwd=r"c:\Users\abhin\heavensgate", env=ENV, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return res.returncode == 0

def evaluate_params(params):
    if not update_header_and_compile(params):
        return -9999.0
    
    # Run STS Benchmark
    cmd = [ENGINE_EXE, "sts", "10", "0", "6"]
    proc = subprocess.run(cmd, cwd=r"c:\Users\abhin\heavensgate", env=ENV, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    out = proc.stdout
    
    total_match = re.search(r"OVERALL TOTAL\s+\d+\s+(\d+)\s*/\s*8000", out)
    if not total_match:
        return 0.0
    
    sts_score = float(total_match.group(1))
    return sts_score

def run_spsa(iterations=10):
    print("=" * 75)
    print("  HEAVEN'S GATE SPSA SEARCH PARAMETER OPTIMIZATION")
    print("=" * 75)
    
    curr_params = {k: v["val"] for k, v in PARAM_DEFS.items()}
    best_params = curr_params.copy()
    
    print("\n[SPSA] Evaluating Baseline Parameters...")
    best_score = evaluate_params(curr_params)
    print(f"  Baseline STS Score: {best_score:.0f} / 8000\n")
    
    # SPSA Hyperparameters
    a = 0.40
    A = 5.0
    alpha = 0.602
    gamma = 0.101
    
    keys = list(PARAM_DEFS.keys())
    
    for k in range(1, iterations + 1):
        a_k = a / ((k + A) ** alpha)
        c_k_scale = 1.0 / (k ** gamma)
        
        # 1. Generate Rademacher perturbation vector delta (+1 or -1)
        delta = {key: 1.0 if random.random() > 0.5 else -1.0 for key in keys}
        
        # 2. Compute theta+ and theta-
        theta_plus = {}
        theta_minus = {}
        for key in keys:
            defn = PARAM_DEFS[key]
            pert = defn["c"] * c_k_scale * delta[key]
            
            p_plus = curr_params[key] + pert
            p_minus = curr_params[key] - pert
            
            p_plus = max(defn["min"], min(defn["max"], p_plus))
            p_minus = max(defn["min"], min(defn["max"], p_minus))
            
            if defn["type"] == "int":
                p_plus = round(p_plus)
                p_minus = round(p_minus)
                
            theta_plus[key] = p_plus
            theta_minus[key] = p_minus
            
        print(f"\n--- [Iteration {k}/{iterations}] ---")
        print(f"  Testing theta+...")
        y_plus = evaluate_params(theta_plus)
        print(f"    theta+ Score: {y_plus:.0f}")
        
        print(f"  Testing theta-...")
        y_minus = evaluate_params(theta_minus)
        print(f"    theta- Score: {y_minus:.0f}")
        
        # 3. Estimate gradient
        diff = y_plus - y_minus
        print(f"  Gradient Diff (y+ - y-): {diff:+.1f}")
        
        for key in keys:
            defn = PARAM_DEFS[key]
            c_eff = defn["c"] * c_k_scale * delta[key]
            g_i = diff / (2.0 * c_eff)
            
            # Update
            step = a_k * g_i * (defn["max"] - defn["min"]) * 0.05
            new_val = curr_params[key] + step
            new_val = max(defn["min"], min(defn["max"], new_val))
            if defn["type"] == "int":
                new_val = round(new_val)
            curr_params[key] = new_val
            
        # Check current evaluated score
        curr_score = evaluate_params(curr_params)
        print(f"  Updated Parameters Evaluated Score: {curr_score:.0f} (Best: {best_score:.0f})")
        
        if curr_score > best_score:
            best_score = curr_score
            best_params = curr_params.copy()
            print(f"  >>> NEW BEST SCORE: {best_score:.0f} / 8000! Saving search_params_best.json <<<")
            with open(r"c:\Users\abhin\heavensgate\search_params_best.json", "w") as f:
                json.dump({"score": best_score, "params": best_params}, f, indent=4)
                
    # Restore best parameters
    print("\n" + "=" * 75)
    print(f"SPSA OPTIMIZATION COMPLETE! Final Best Score: {best_score:.0f} / 8000")
    print("=" * 75)
    print(json.dumps(best_params, indent=4))
    update_header_and_compile(best_params)

if __name__ == "__main__":
    run_spsa(iterations=10)
