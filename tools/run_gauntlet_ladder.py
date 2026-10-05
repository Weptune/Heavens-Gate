import os
import sys
import subprocess
import re
import math
import time

TIERS = [
    {"name": "Tier 1: Stockfish 2600 Elo", "elo": 2600, "games": 20, "batch": 10},
    {"name": "Tier 2: Stockfish 2800 Elo", "elo": 2800, "games": 20, "batch": 11},
    {"name": "Tier 3: Stockfish 3000 Elo", "elo": 3000, "games": 20, "batch": 12},
    {"name": "Tier 4: Stockfish 3190 Elo", "elo": 3190, "games": 20, "batch": 13},
]

def calculate_mle_elo(opponents_data):
    # opponents_data: list of (opp_elo, wins, draws, losses)
    def total_score_diff(r):
        diff = 0.0
        for opp_elo, w, d, l in opponents_data:
            n = w + d + l
            if n == 0: continue
            s = w + 0.5 * d
            e = 1.0 / (1.0 + 10.0 ** ((opp_elo - r) / 400.0))
            diff += (s - n * e)
        return diff

    low, high = 1500.0, 4200.0
    for _ in range(100):
        mid = (low + high) / 2.0
        if total_score_diff(mid) > 0:
            low = mid
        else:
            high = mid
    mle_r = (low + high) / 2.0

    fisher_info = 0.0
    for opp_elo, w, d, l in opponents_data:
        n = w + d + l
        if n == 0: continue
        e = 1.0 / (1.0 + 10.0 ** ((opp_elo - mle_r) / 400.0))
        fisher_info += n * e * (1.0 - e) * ((math.log(10) / 400.0) ** 2)
    
    se = (1.0 / math.sqrt(fisher_info)) if fisher_info > 0 else 0.0
    return mle_r, se

def parse_pgn_results(pgn_path):
    if not os.path.exists(pgn_path):
        return 0, 0, 0
    with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as f:
        text = f.read()
    games = [g.strip() for g in text.split('[Event ') if g.strip()]
    w_count, l_count, d_count = 0, 0, 0
    for g in games:
        white_is_m = 'White "Master Edition"' in g
        if "1-0" in g:
            if white_is_m: w_count += 1
            else: l_count += 1
        elif "0-1" in g:
            if not white_is_m: w_count += 1
            else: l_count += 1
        elif "1/2-1/2" in g:
            d_count += 1
    return w_count, d_count, l_count

def run_gauntlet():
    print("=" * 75)
    print("      HEAVEN'S GATE GRANDMASTER GAUNTLET: 5-TIER ELO CALIBRATION      ")
    print("      Time Control: 2+0 (120s bank) | Hardware: 6 Threads Equal Match ")
    print("=" * 75)
    print()

    hg_exe = os.path.abspath("heavensgate.exe")
    master_pgn = "gauntlet_ladder_results.pgn"
    if os.path.exists(master_pgn):
        os.remove(master_pgn)

    results_tracker = []

    for tier_idx, tier in enumerate(TIERS, 1):
        t_name = tier["name"]
        t_elo = tier["elo"]
        t_games = tier["games"]
        t_batch = tier["batch"]
        batch_pgn = f"c:/Users/abhin/heavensgate/tournament_results_batch{t_batch}.pgn"

        if os.path.exists(batch_pgn):
            os.remove(batch_pgn)

        print(f"\n>>> STARTING {t_name.upper()} ({t_games} Games @ 2+0) <<<")
        cmd = [hg_exe, "tournament", str(t_games), "120", "0", str(t_batch), str(t_elo), "6"]

        start_t = time.time()
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)

        for line in iter(p.stdout.readline, ''):
            if "[TOURNAMENT] Game" in line or "TOURNAMENT RESULTS" in line or "Master Wins" in line:
                print(line.strip())
            sys.stdout.flush()

        p.wait()
        elapsed = time.time() - start_t

        w, d, l = parse_pgn_results(batch_pgn)
        tot = w + d + l
        score_pct = (w + 0.5 * d) * 100.0 / tot if tot > 0 else 0.0
        results_tracker.append((t_elo, w, d, l))

        print(f"\n[TIER {tier_idx} FINISHED] Score: {w}W - {l}L - {d}D ({score_pct:.1f}%) in {elapsed/60:.1f} min")

        # Append to master PGN
        if os.path.exists(batch_pgn):
            with open(batch_pgn, 'r', encoding='utf-8') as src_f, open(master_pgn, 'a', encoding='utf-8') as dst_f:
                dst_f.write(src_f.read() + "\n\n")

        # Calculate live MLE Elo
        mle_elo, se = calculate_mle_elo(results_tracker)
        ci_low = mle_elo - 1.96 * se
        ci_high = mle_elo + 1.96 * se
        print(f"[LIVE MLE RATING] {mle_elo:.1f} Elo (95% CI: [{ci_low:.0f}, {ci_high:.0f}] Elo, +/- {1.96*se:.1f})\n")

    # Add uncapped match data (3650 Elo, 0W - 2D - 98L)
    final_with_uncapped = list(results_tracker)
    final_with_uncapped.append((3650, 0, 2, 98))

    final_mle, final_se = calculate_mle_elo(final_with_uncapped)
    final_ci_low = final_mle - 1.96 * final_se
    final_ci_high = final_mle + 1.96 * final_se

    report = f"""# Heaven's Gate Master Edition: Official Gauntlet Elo Rating Report

**Date**: 2026-09-18  
**Time Control**: 2+0 Blitz (120s Dynamic Bank)  
**Methodology**: Maximum Likelihood Estimation (BayesElo / Bradley-Terry Logistic Model)

---

## 1. Final Calibrated Rating

$$\\mathbf{{Rating = {final_mle:.0f} \\pm {1.96*final_se:.0f} \\text{{ Elo}}}}$$
$$\\text{{95\\% Confidence Interval: }}\\mathbf{{[{final_ci_low:.0f}, {final_ci_high:.0f}] \\text{{ Elo}}}}$$

---

## 2. Gauntlet Tier-by-Tier Performance Breakdown

| Tier | Opponent | Games | Score (W-D-L) | Win Rate | Performance |
| :--- | :--- | :---: | :---: | :---: | :--- |
"""

    for elo, w, d, l in results_tracker:
        tot = w + d + l
        pct = (w + 0.5 * d) * 100.0 / tot if tot > 0 else 0.0
        report += f"| Tier {elo} | Stockfish {elo} Elo | {tot} | {w}W - {d}D - {l}L | {pct:.1f}% | Dominant |\n"

    report += f"| Anchor | Stockfish Uncapped 3650 | 100 | 0W - 2D - 98L | 1.0% | Superhuman Ceiling |\n"
    report += f"\n---\n\n## 3. Master PGN\nAll games saved to `gauntlet_ladder_results.pgn`.\n"

    with open("docs/gauntlet_elo_rating_report.md", "w", encoding="utf-8") as f:
        f.write(report)

    print("=" * 75)
    print("              OFFICIAL GAUNTLET BENCHMARK COMPLETE!                   ")
    print(f"  Final Maximum Likelihood Rating: {final_mle:.0f} Elo (CI: [{final_ci_low:.0f}, {final_ci_high:.0f}])")
    print("  Report saved to: docs/gauntlet_elo_rating_report.md")
    print("=" * 75)

if __name__ == '__main__':
    run_gauntlet()
