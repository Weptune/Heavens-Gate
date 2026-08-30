import subprocess
import re
import os
import statistics

cmd = "git show d53ac9f:tournament_results_batch1.pgn"
res = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd="c:/Users/abhin/heavensgate")
if res.returncode != 0:
    print("Failed to get Phase 4 PGN")
    exit(1)

pgn_text = res.stdout

games_raw = pgn_text.strip().split('[Event "Heaven\'s Gate Grandmaster Tournament"]')
games = [g for g in games_raw if g.strip()]

# Phase lists
op_hg_evals, op_sf_evals, op_hg_cpl, op_sf_cpl = [], [], [], []
mid_hg_evals, mid_sf_evals, mid_hg_cpl, mid_sf_cpl = [], [], [], []
end_hg_evals, end_sf_evals, end_hg_cpl, end_sf_cpl = [], [], [], []
op_hg_times, mid_hg_times, end_hg_times = [], [], []
op_sf_times, mid_sf_times, end_sf_times = [], [], []

for g_idx, g_text in enumerate(games, 1):
    white_is_hg = 'White "Master Edition"' in g_text
    pattern = r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}'
    matches = re.findall(pattern, g_text)
    
    ply_idx = 0
    hg_prev_score = None
    sf_prev_score = None
    
    for match in matches:
        num, move_str, eval_str, clk_str = match
        raw_eval = int(eval_str)
        clk_val = float(clk_str)
        clamped = max(-2000, min(2000, raw_eval))
        
        is_white = (ply_idx % 2 == 0)
        is_hg = (is_white and white_is_hg) or (not is_white and not white_is_hg)
        move_num = (ply_idx // 2) + 1
        
        mover_score = clamped
        hg_score = mover_score if is_hg else -mover_score
        
        if move_num <= 10:
            if is_hg:
                op_hg_evals.append(hg_score)
                op_hg_times.append(clk_val)
                if hg_prev_score is not None:
                    op_hg_cpl.append(max(0, min(500, hg_prev_score - hg_score)))
                hg_prev_score = hg_score
            else:
                op_sf_evals.append(mover_score)
                op_sf_times.append(clk_val)
                if sf_prev_score is not None:
                    op_sf_cpl.append(max(0, min(500, sf_prev_score - mover_score)))
                sf_prev_score = mover_score
        elif move_num <= 25:
            if is_hg:
                mid_hg_evals.append(hg_score)
                mid_hg_times.append(clk_val)
                if hg_prev_score is not None:
                    mid_hg_cpl.append(max(0, min(500, hg_prev_score - hg_score)))
                hg_prev_score = hg_score
            else:
                mid_sf_evals.append(mover_score)
                mid_sf_times.append(clk_val)
                if sf_prev_score is not None:
                    mid_sf_cpl.append(max(0, min(500, sf_prev_score - mover_score)))
                sf_prev_score = mover_score
        else:
            if is_hg:
                end_hg_evals.append(hg_score)
                end_hg_times.append(clk_val)
                if hg_prev_score is not None:
                    end_hg_cpl.append(max(0, min(500, hg_prev_score - hg_score)))
                hg_prev_score = hg_score
            else:
                end_sf_evals.append(mover_score)
                end_sf_times.append(clk_val)
                if sf_prev_score is not None:
                    end_sf_cpl.append(max(0, min(500, sf_prev_score - mover_score)))
                sf_prev_score = mover_score
                
        ply_idx += 1

def calc(scores, cpls, times):
    avg_s = statistics.mean(scores) if scores else 0
    avg_c = statistics.mean(cpls) if cpls else 0
    avg_t = statistics.mean(times) if times else 0
    acc = max(40.0, min(99.0, 100.0 - (avg_c * 0.45)))
    return avg_s, avg_c, acc, avg_t

op_s_hg, op_c_hg, op_a_hg, op_t_hg = calc(op_hg_evals, op_hg_cpl, op_hg_times)
op_s_sf, op_c_sf, op_a_sf, op_t_sf = calc(op_sf_evals, op_sf_cpl, op_sf_times)

mid_s_hg, mid_c_hg, mid_a_hg, mid_t_hg = calc(mid_hg_evals, mid_hg_cpl, mid_hg_times)
mid_s_sf, mid_c_sf, mid_a_sf, mid_t_sf = calc(mid_sf_evals, mid_sf_cpl, mid_sf_times)

end_s_hg, end_c_hg, end_a_hg, end_t_hg = calc(end_hg_evals, end_hg_cpl, end_hg_times)
end_s_sf, end_c_sf, end_a_sf, end_t_sf = calc(end_sf_evals, end_sf_cpl, end_sf_times)

print("=" * 90)
print("     PHASE 4 (TT CLUSTERING & SHARED SMP) — PHASE-BY-PHASE MASTERY BREAKDOWN")
print("=" * 90)
print(f"{'Game Phase':<24} {'Metric':<26} {'Heaven\'s Gate (Phase 4)':<24} {'Stockfish 3400'}")
print("-" * 90)
print(f"{'1. OPENING (Moves 1-10)':<24} {'Average Eval Advantage':<26} +{op_s_hg:.1f} cp (+{op_s_hg/100:.2f} P)       {op_s_sf:.1f} cp")
print(f"{'':<24} {'Centipawn Loss (ACPL)':<26} {op_c_hg:.1f} cp                    {op_c_sf:.1f} cp")
print(f"{'':<24} {'Move Accuracy':<26} {op_a_hg:.1f} %                     {op_a_sf:.1f} %")
print(f"{'':<24} {'Avg Calculation Time':<26} {op_t_hg:.1f} ms                  {op_t_sf:.1f} ms")
print("-" * 90)
print(f"{'2. MIDGAME (Moves 11-25)':<24} {'Average Eval Advantage':<26} +{mid_s_hg:.1f} cp (+{mid_s_hg/100:.2f} P)       {mid_s_sf:.1f} cp")
print(f"{'':<24} {'Centipawn Loss (ACPL)':<26} {mid_c_hg:.1f} cp                    {mid_c_sf:.1f} cp")
print(f"{'':<24} {'Move Accuracy':<26} {mid_a_hg:.1f} %                     {mid_a_sf:.1f} %")
print(f"{'':<24} {'Avg Calculation Time':<26} {mid_t_hg:.1f} ms                  {mid_t_sf:.1f} ms")
print("-" * 90)
print(f"{'3. ENDGAME (Moves 26+)':<24} {'Average Eval Advantage':<26} +{end_s_hg:.1f} cp (+{end_s_hg/100:.2f} P)     {end_s_sf:.1f} cp")
print(f"{'':<24} {'Centipawn Loss (ACPL)':<26} {end_c_hg:.1f} cp                    {end_c_sf:.1f} cp")
print(f"{'':<24} {'Move Accuracy':<26} {end_a_hg:.1f} %                     {end_a_sf:.1f} %")
print(f"{'':<24} {'Avg Calculation Time':<26} {end_t_hg:.1f} ms                  {end_t_sf:.1f} ms")
print("=" * 90)
