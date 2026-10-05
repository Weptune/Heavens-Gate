import subprocess
import os
import json
import random
import re
import numpy as np
import time

PARAM_DEFS = {
    "lmr_divisor": {"type": "float", "val": 3.20, "c": 0.20, "min": 1.60, "max": 4.00, "uci": "LMR_Divisor"},
    "lmr_hist_bonus": {"type": "int", "val": 425, "c": 50, "min": 150, "max": 1500, "uci": "LMR_HistBonus"},
    "lmr_hist_malus": {"type": "int", "val": 72, "c": 20, "min": 20, "max": 300, "uci": "LMR_HistMalus"},
    "rfp_margin": {"type": "int", "val": 163, "c": 20, "min": 60, "max": 260, "uci": "RFP_Margin"},
    "futility_margin": {"type": "int", "val": 180, "c": 25, "min": 80, "max": 350, "uci": "Futility_Margin"},
    "see_bad_capture_slope": {"type": "int", "val": 124, "c": 15, "min": 40, "max": 200, "uci": "SEE_BadCaptureSlope"},
    "see_quiet_slope": {"type": "int", "val": 15, "c": 5, "min": 5, "max": 60, "uci": "SEE_QuietSlope"},
    "nmp_eval_margin": {"type": "int", "val": 218, "c": 30, "min": 80, "max": 400, "uci": "NMP_EvalMargin"},
    "singular_margin": {"type": "int", "val": 2, "c": 1, "min": 1, "max": 6, "uci": "Singular_Margin"},
    "aspiration_window_delta": {"type": "int", "val": 25, "c": 5, "min": 10, "max": 60, "uci": "Aspiration_Window_Delta"},
}

GXX = r"C:\Users\abhin\heavensgate\tools\w64devkit\bin\g++.exe"
ENGINE_EXE = r"c:\Users\abhin\heavensgate\heavensgate.exe"
ENV = os.environ.copy()
ENV["PATH"] = r"C:\Users\abhin\heavensgate\tools\w64devkit\bin;" + ENV.get("PATH", "")

def evaluate_params(params):
    input_cmds = []
    for k, pdef in PARAM_DEFS.items():
        uci_name = pdef["uci"]
        val = params[k]
        if pdef["type"] == "float":
            input_cmds.append(f"setoption name {uci_name} value {val:.4f}")
        else:
            input_cmds.append(f"setoption name {uci_name} value {int(round(val))}")
    input_cmds.append("sts 10 0 6")
    input_cmds.append("quit")
    stdin_data = "\n".join(input_cmds) + "\n"

    try:
        proc = subprocess.run([ENGINE_EXE], input=stdin_data, cwd=r"c:\Users\abhin\heavensgate", env=ENV, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
        out = proc.stdout
    except Exception as e:
        print(f"[Error] Subprocess execution failed: {e}")
        return 0.0

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
    # Update search_params.hpp with tuned values
    header_path = r"c:\Users\abhin\heavensgate\src\search\search_params.hpp"
    header_content = f"""#pragma once

namespace heavensgate {{

struct SearchParams {{
    float lmr_divisor = {best_params['lmr_divisor']:.4f}f;
    int lmr_hist_bonus = {int(round(best_params['lmr_hist_bonus']))};
    int lmr_hist_malus = {int(round(best_params['lmr_hist_malus']))};
    int rfp_margin = {int(round(best_params['rfp_margin']))};
    int futility_margin = {int(round(best_params['futility_margin']))};
    int see_bad_capture_slope = {int(round(best_params['see_bad_capture_slope']))};
    int see_quiet_slope = {int(round(best_params['see_quiet_slope']))};
    int nmp_eval_margin = {int(round(best_params['nmp_eval_margin']))};
    int singular_margin = {int(round(best_params['singular_margin']))};
    int aspiration_window_delta = {int(round(best_params['aspiration_window_delta']))};

    void reset() noexcept {{
        lmr_divisor = {best_params['lmr_divisor']:.4f}f;
        lmr_hist_bonus = {int(round(best_params['lmr_hist_bonus']))};
        lmr_hist_malus = {int(round(best_params['lmr_hist_malus']))};
        rfp_margin = {int(round(best_params['rfp_margin']))};
        futility_margin = {int(round(best_params['futility_margin']))};
        see_bad_capture_slope = {int(round(best_params['see_bad_capture_slope']))};
        see_quiet_slope = {int(round(best_params['see_quiet_slope']))};
        nmp_eval_margin = {int(round(best_params['nmp_eval_margin']))};
        singular_margin = {int(round(best_params['singular_margin']))};
        aspiration_window_delta = {int(round(best_params['aspiration_window_delta']))};
    }}
}};

extern SearchParams g_search_params;

}} // namespace heavensgate
"""
    with open(header_path, "w", encoding="utf-8") as f:
        f.write(header_content)
    print(f"\n[SPSA] Successfully updated {header_path} with optimized parameters!")

if __name__ == "__main__":
    run_spsa(iterations=10)
