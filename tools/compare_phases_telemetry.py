import subprocess
import re
import os
import statistics

commits = [
    ("Phase 1 (Fixes)", "776a501", False),
    ("Phase 2 (Ordering)", "e33e0ba", False),
    ("Phase 3 (Pruning)", "750fae2", False),
    ("Phase 4 (TT Cluster)", "d53ac9f", False),
    ("Phase 5 (Master)", "current", True)
]

def analyze_pgn_text(name, pgn_text):
    games_raw = pgn_text.strip().split('[Event "Heaven\'s Gate Grandmaster Tournament"]')
    games = [g for g in games_raw if g.strip()]

    total_games = len(games)
    hg_wins = 0
    sf_wins = 0
    draws = 0

    game_lengths = []
    white_wins = 0
    black_wins = 0

    hg_move_counts = {"best": 0, "excellent": 0, "good": 0, "inaccuracy": 0, "mistake": 0, "blunder": 0}
    sf_move_counts = {"best": 0, "excellent": 0, "good": 0, "inaccuracy": 0, "mistake": 0, "blunder": 0}

    plies_to_advantage = []
    plies_to_decisive = []
    plies_from_decisive_to_mate = []
    leads_never_lost = 0
    rolling_volatilities = []
    hg_times = []

    # Phase-specific buckets
    op_cpl, mid_cpl, end_cpl = [], [], []

    for g_idx, g_text in enumerate(games, 1):
        white_is_hg = 'White "Master Edition"' in g_text
        pattern = r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}'
        matches = re.findall(pattern, g_text)

        ply_count = len(matches)
        game_lengths.append((ply_count + 1) // 2)

        if "1-0" in g_text.split("\n")[-1] or "1-0" in g_text.split("\n")[-2]:
            if white_is_hg: hg_wins += 1; white_wins += 1
            else: sf_wins += 1; white_wins += 1
        elif "0-1" in g_text.split("\n")[-1] or "0-1" in g_text.split("\n")[-2]:
            if not white_is_hg: hg_wins += 1; black_wins += 1
            else: sf_wins += 1; black_wins += 1
        else:
            draws += 1

        hg_eval_series = []
        adv_ply = None
        dec_ply = None
        hg_prev = None
        sf_prev = None

        for ply_idx, match in enumerate(matches):
            num, move_str, eval_str, clk_str = match
            raw_eval = int(eval_str)
            clk_val = float(clk_str)
            clamped = max(-2000, min(2000, raw_eval))

            is_white = (ply_idx % 2 == 0)
            is_hg = (is_white and white_is_hg) or (not is_white and not white_is_hg)
            move_num = (ply_idx // 2) + 1
            mover_eval = clamped
            hg_score = mover_eval if is_hg else -mover_eval

            if is_hg:
                hg_times.append(clk_val)
                hg_eval_series.append(hg_score)
                if hg_prev is not None:
                    cpl = max(0, min(500, hg_prev - hg_score))
                    if move_num <= 10: op_cpl.append(cpl)
                    elif move_num <= 25: mid_cpl.append(cpl)
                    else: end_cpl.append(cpl)

                    if cpl <= 5: hg_move_counts["best"] += 1
                    elif cpl <= 15: hg_move_counts["excellent"] += 1
                    elif cpl <= 35: hg_move_counts["good"] += 1
                    elif cpl <= 80: hg_move_counts["inaccuracy"] += 1
                    elif cpl <= 200: hg_move_counts["mistake"] += 1
                    else: hg_move_counts["blunder"] += 1
                hg_prev = hg_score

                if adv_ply is None and hg_score >= 150: adv_ply = ply_idx + 1
                if dec_ply is None and hg_score >= 500: dec_ply = ply_idx + 1
            else:
                if sf_prev is not None:
                    cpl = max(0, min(500, sf_prev - mover_eval))
                    if cpl <= 5: sf_move_counts["best"] += 1
                    elif cpl <= 15: sf_move_counts["excellent"] += 1
                    elif cpl <= 35: sf_move_counts["good"] += 1
                    elif cpl <= 80: sf_move_counts["inaccuracy"] += 1
                    elif cpl <= 200: sf_move_counts["mistake"] += 1
                    else: sf_move_counts["blunder"] += 1
                sf_prev = mover_eval

        if adv_ply: plies_to_advantage.append(adv_ply)
        if dec_ply: 
            plies_to_decisive.append(dec_ply)
            plies_from_decisive_to_mate.append(ply_count - dec_ply)

        lead_established = False
        lead_retained = True
        for ev in hg_eval_series:
            if ev >= 200: lead_established = True
            if lead_established and ev < 50:
                lead_retained = False
                break
        if lead_established and lead_retained: leads_never_lost += 1

        for i in range(len(hg_eval_series) - 4):
            window = hg_eval_series[i:i+5]
            rolling_volatilities.append(statistics.stdev(window) if len(window) > 1 else 0)

    total_hg = sum(hg_move_counts.values()) or 1
    best_pct = (hg_move_counts["best"] + hg_move_counts["excellent"]) * 100.0 / total_hg
    inacc_pct = hg_move_counts["inaccuracy"] * 100.0 / total_hg
    mistake_pct = hg_move_counts["mistake"] * 100.0 / total_hg
    blunder_pct = hg_move_counts["blunder"] * 100.0 / total_hg

    avg_len = statistics.mean(game_lengths) if game_lengths else 0
    avg_adv = (statistics.mean(plies_to_advantage)+1)/2 if plies_to_advantage else 0
    avg_dec = (statistics.mean(plies_to_decisive)+1)/2 if plies_to_decisive else 0
    avg_sqz = (statistics.mean(plies_from_decisive_to_mate)+1)/2 if plies_from_decisive_to_mate else 0
    vol = statistics.mean(rolling_volatilities) if rolling_volatilities else 0
    avg_time = statistics.mean(hg_times) if hg_times else 0

    op_acpl = statistics.mean(op_cpl) if op_cpl else 0
    mid_acpl = statistics.mean(mid_cpl) if mid_cpl else 0
    end_acpl = statistics.mean(end_cpl) if end_cpl else 0

    return {
        "name": name,
        "games": total_games,
        "score": f"{hg_wins}-{sf_wins}",
        "avg_len": avg_len,
        "op_acpl": op_acpl,
        "mid_acpl": mid_acpl,
        "end_acpl": end_acpl,
        "op_acc": max(40.0, min(99.0, 100.0 - (op_acpl * 0.45))),
        "mid_acc": max(40.0, min(99.0, 100.0 - (mid_acpl * 0.45))),
        "end_acc": max(40.0, min(99.0, 100.0 - (end_acpl * 0.45))),
        "best_pct": best_pct,
        "inacc_pct": inacc_pct,
        "mistake_pct": mistake_pct,
        "blunder_pct": blunder_pct,
        "avg_adv_move": avg_adv,
        "avg_dec_move": avg_dec,
        "squeeze_speed": avg_sqz,
        "lead_retention": 100.0 * leads_never_lost / max(1, total_games),
        "volatility": vol,
        "avg_time": avg_time
    }

results = []
for name, commit, is_current in commits:
    if is_current:
        with open("c:/Users/abhin/heavensgate/tournament_results_batch1.pgn", "r", encoding="utf-8") as f:
            pgn_content = f.read()
            data = analyze_pgn_text(name, pgn_content)
            results.append(data)
    else:
        cmd = f"git show {commit}:tournament_results_batch1.pgn"
        res = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd="c:/Users/abhin/heavensgate")
        if res.returncode == 0:
            data = analyze_pgn_text(name, res.stdout)
            results.append(data)

print("=" * 135)
print("                       GRANDMASTER TELEMETRY EVOLUTION (PHASES 1 TO 5)")
print("=" * 135)
print(f"{'Performance Metric':<28} | {'Phase 1':<16} | {'Phase 2':<16} | {'Phase 3':<16} | {'Phase 4':<16} | {'Phase 5 (Master)':<16}")
print("-" * 135)
print(f"{'Match vs SF 3400':<28} | {results[0]['score']:<16} | {results[1]['score']:<16} | {results[2]['score']:<16} | {results[3]['score']:<16} | {results[4]['score']:<16}")
print(f"{'Average Game Duration':<28} | {results[0]['avg_len']:<13.1f} mv | {results[1]['avg_len']:<13.1f} mv | {results[2]['avg_len']:<13.1f} mv | {results[3]['avg_len']:<13.1f} mv | {results[4]['avg_len']:<13.1f} mv")
print("-" * 135)
print(f"{'Opening ACPL (Moves 1-10)':<28} | {results[0]['op_acpl']:<13.1f} cp | {results[1]['op_acpl']:<13.1f} cp | {results[2]['op_acpl']:<13.1f} cp | {results[3]['op_acpl']:<13.1f} cp | {results[4]['op_acpl']:<13.1f} cp")
print(f"{'Opening Accuracy %':<28} | {results[0]['op_acc']:<15.1f}% | {results[1]['op_acc']:<15.1f}% | {results[2]['op_acc']:<15.1f}% | {results[3]['op_acc']:<15.1f}% | {results[4]['op_acc']:<15.1f}%")
print(f"{'Midgame ACPL (Moves 11-25)':<28} | {results[0]['mid_acpl']:<13.1f} cp | {results[1]['mid_acpl']:<13.1f} cp | {results[2]['mid_acpl']:<13.1f} cp | {results[3]['mid_acpl']:<13.1f} cp | {results[4]['mid_acpl']:<13.1f} cp")
print(f"{'Midgame Accuracy %':<28} | {results[0]['mid_acc']:<15.1f}% | {results[1]['mid_acc']:<15.1f}% | {results[2]['mid_acc']:<15.1f}% | {results[3]['mid_acc']:<15.1f}% | {results[4]['mid_acc']:<15.1f}%")
print(f"{'Endgame ACPL (Moves 26+)':<28} | {results[0]['end_acpl']:<13.1f} cp | {results[1]['end_acpl']:<13.1f} cp | {results[2]['end_acpl']:<13.1f} cp | {results[3]['end_acpl']:<13.1f} cp | {results[4]['end_acpl']:<13.1f} cp")
print(f"{'Endgame Accuracy %':<28} | {results[0]['end_acc']:<15.1f}% | {results[1]['end_acc']:<15.1f}% | {results[2]['end_acc']:<15.1f}% | {results[3]['end_acc']:<15.1f}% | {results[4]['end_acc']:<15.1f}%")
print("-" * 135)
print(f"{'Top Quality Moves (0-15 cp)':<28} | {results[0]['best_pct']:<15.1f}% | {results[1]['best_pct']:<15.1f}% | {results[2]['best_pct']:<15.1f}% | {results[3]['best_pct']:<15.1f}% | {results[4]['best_pct']:<15.1f}%")
print(f"{'Inaccuracy Rate (35-80 cp)':<28} | {results[0]['inacc_pct']:<15.1f}% | {results[1]['inacc_pct']:<15.1f}% | {results[2]['inacc_pct']:<15.1f}% | {results[3]['inacc_pct']:<15.1f}% | {results[4]['inacc_pct']:<15.1f}%")
print(f"{'Mistake Rate (80-200 cp)':<28} | {results[0]['mistake_pct']:<15.1f}% | {results[1]['mistake_pct']:<15.1f}% | {results[2]['mistake_pct']:<15.1f}% | {results[3]['mistake_pct']:<15.1f}% | {results[4]['mistake_pct']:<15.1f}%")
print(f"{'Blunder Rate (>200 cp)':<28} | {results[0]['blunder_pct']:<15.1f}% | {results[1]['blunder_pct']:<15.1f}% | {results[2]['blunder_pct']:<15.1f}% | {results[3]['blunder_pct']:<15.1f}% | {results[4]['blunder_pct']:<15.1f}%")
print("-" * 135)
print(f"{'Advantage Milestone (+1.50)':<28} | Move {results[0]['avg_adv_move']:<11.1f} | Move {results[1]['avg_adv_move']:<11.1f} | Move {results[2]['avg_adv_move']:<11.1f} | Move {results[3]['avg_adv_move']:<11.1f} | Move {results[4]['avg_adv_move']:<11.1f}")
print(f"{'Decisive Milestone (+5.00)':<28} | Move {results[0]['avg_dec_move']:<11.1f} | Move {results[1]['avg_dec_move']:<11.1f} | Move {results[2]['avg_dec_move']:<11.1f} | Move {results[3]['avg_dec_move']:<11.1f} | Move {results[4]['avg_dec_move']:<11.1f}")
print(f"{'Squeeze Speed (+5.00 to Mate)':<28} | {results[0]['squeeze_speed']:<13.1f} mv | {results[1]['squeeze_speed']:<13.1f} mv | {results[2]['squeeze_speed']:<13.1f} mv | {results[3]['squeeze_speed']:<13.1f} mv | {results[4]['squeeze_speed']:<13.1f} mv")
print("=" * 135)
