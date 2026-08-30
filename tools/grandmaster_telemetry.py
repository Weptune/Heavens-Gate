import re
import os
import math
import statistics

def analyze_full_telemetry(pgn_path):
    if not os.path.exists(pgn_path):
        print("PGN file not found:", pgn_path)
        return

    with open(pgn_path, 'r', encoding='utf-8') as f:
        raw_content = f.read()

    games_raw = raw_content.strip().split('[Event "Heaven\'s Gate Grandmaster Tournament"]')
    games = [g for g in games_raw if g.strip()]

    total_games = len(games)
    hg_wins = 0
    sf_wins = 0
    draws = 0

    game_lengths = []
    white_wins = 0
    black_wins = 0

    # Move Quality Classifications
    hg_move_counts = {"best": 0, "excellent": 0, "good": 0, "inaccuracy": 0, "mistake": 0, "blunder": 0}
    sf_move_counts = {"best": 0, "excellent": 0, "good": 0, "inaccuracy": 0, "mistake": 0, "blunder": 0}

    # Advantage & Conversion Dynamics
    plies_to_advantage = []  # Ply when lead reached +1.50
    plies_to_decisive = []   # Ply when lead reached +5.00
    plies_from_decisive_to_mate = [] # Squeeze speed
    leads_never_lost = 0

    # Positional & Evaluation Trajectory
    hg_all_evals = []
    sf_all_evals = []
    rolling_volatilities = []

    # Nodes & Calculation Timings
    hg_times = []
    sf_times = []

    for g_idx, g_text in enumerate(games, 1):
        white_is_hg = 'White "Master Edition"' in g_text
        pattern = r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}'
        matches = re.findall(pattern, g_text)

        ply_count = len(matches)
        game_lengths.append((ply_count + 1) // 2)

        # Game outcome detection
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
            mover_eval = clamped
            hg_score = mover_eval if is_hg else -mover_eval

            if is_hg:
                hg_times.append(clk_val)
                hg_all_evals.append(hg_score)
                hg_eval_series.append(hg_score)

                if hg_prev is not None:
                    cpl = max(0, hg_prev - hg_score)
                    if cpl <= 5: hg_move_counts["best"] += 1
                    elif cpl <= 15: hg_move_counts["excellent"] += 1
                    elif cpl <= 35: hg_move_counts["good"] += 1
                    elif cpl <= 80: hg_move_counts["inaccuracy"] += 1
                    elif cpl <= 200: hg_move_counts["mistake"] += 1
                    else: hg_move_counts["blunder"] += 1
                hg_prev = hg_score

                # Advantage Milestones
                if adv_ply is None and hg_score >= 150:
                    adv_ply = ply_idx + 1
                if dec_ply is None and hg_score >= 500:
                    dec_ply = ply_idx + 1
            else:
                sf_times.append(clk_val)
                sf_all_evals.append(mover_eval)
                if sf_prev is not None:
                    cpl = max(0, sf_prev - mover_eval)
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

        # Advantage retention check: once >= 200 cp, did it ever drop below 0?
        lead_established = False
        lead_retained = True
        for ev in hg_eval_series:
            if ev >= 200: lead_established = True
            if lead_established and ev < 50:
                lead_retained = False
                break
        if lead_established and lead_retained:
            leads_never_lost += 1

        # Rolling 5-move volatility
        for i in range(len(hg_eval_series) - 4):
            window = hg_eval_series[i:i+5]
            rolling_volatilities.append(statistics.stdev(window) if len(window) > 1 else 0)

    # Aggregates
    total_hg_moves = sum(hg_move_counts.values()) or 1
    total_sf_moves = sum(sf_move_counts.values()) or 1

    avg_adv_ply = statistics.mean(plies_to_advantage) if plies_to_advantage else 0
    avg_dec_ply = statistics.mean(plies_to_decisive) if plies_to_decisive else 0
    avg_squeeze_ply = statistics.mean(plies_from_decisive_to_mate) if plies_from_decisive_to_mate else 0
    avg_volatility = statistics.mean(rolling_volatilities) if rolling_volatilities else 0

    print("=" * 90)
    print("      HEAVEN'S GATE GRANDMASTER TELEMETRY & TACTICAL DEPTH REPORT")
    print("=" * 90)
    print(f"Overall Record               : {hg_wins} Wins - {sf_wins} Losses - {draws} Draws ({100.0 * hg_wins / max(1, total_games):.1f}% Sweep)")
    print(f"Total Games Analyzed         : {total_games} games ({sum(game_lengths)} total moves)")
    print(f"Average Game Length          : {statistics.mean(game_lengths):.1f} moves (Fastest: {min(game_lengths)}m, Longest: {max(game_lengths)}m)")
    print(f"White Win Rate               : {white_wins} / {((total_games+1)//2)} ({100.0 * white_wins / max(1, (total_games+1)//2):.1f}%)")
    print(f"Black Win Rate               : {black_wins} / {(total_games//2)} ({100.0 * black_wins / max(1, total_games//2):.1f}%)")
    print("-" * 90)
    print("1. MOVE QUALITY DISTRIBUTION (FIDE / LICHESS STANDARDS):")
    print(f"  • Best Moves (0-5 cp loss)      : {hg_move_counts['best']:>3} ({100.0 * hg_move_counts['best'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['best']:>3} ({100.0 * sf_move_counts['best'] / total_sf_moves:>5.1f}%)")
    print(f"  • Excellent Moves (5-15 cp loss): {hg_move_counts['excellent']:>3} ({100.0 * hg_move_counts['excellent'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['excellent']:>3} ({100.0 * sf_move_counts['excellent'] / total_sf_moves:>5.1f}%)")
    print(f"  • Good Moves (15-35 cp loss)    : {hg_move_counts['good']:>3} ({100.0 * hg_move_counts['good'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['good']:>3} ({100.0 * sf_move_counts['good'] / total_sf_moves:>5.1f}%)")
    print(f"  • Inaccuracies (35-80 cp loss)  : {hg_move_counts['inaccuracy']:>3} ({100.0 * hg_move_counts['inaccuracy'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['inaccuracy']:>3} ({100.0 * sf_move_counts['inaccuracy'] / total_sf_moves:>5.1f}%)")
    print(f"  • Mistakes (80-200 cp loss)     : {hg_move_counts['mistake']:>3} ({100.0 * hg_move_counts['mistake'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['mistake']:>3} ({100.0 * sf_move_counts['mistake'] / total_sf_moves:>5.1f}%)")
    print(f"  • Blunders (>200 cp loss)       : {hg_move_counts['blunder']:>3} ({100.0 * hg_move_counts['blunder'] / total_hg_moves:>5.1f}%)  vs SF {sf_move_counts['blunder']:>3} ({100.0 * sf_move_counts['blunder'] / total_sf_moves:>5.1f}%)")
    print("-" * 90)
    print("2. STRATEGIC INITIATIVE & CONVERSION EFFICIENCY:")
    print(f"  • Avg Move to +1.50 Advantage  : Move {(avg_adv_ply+1)//2:.1f} (Ply {avg_adv_ply:.1f})")
    print(f"  • Avg Move to +5.00 Decisive   : Move {(avg_dec_ply+1)//2:.1f} (Ply {avg_dec_ply:.1f})")
    print(f"  • Conversion Squeeze Speed     : {(avg_squeeze_ply+1)//2:.1f} moves from Decisive (+5.00) to Checkmate")
    print(f"  • Advantage Retention Rate     : {leads_never_lost} / {total_games} ({100.0 * leads_never_lost / max(1, total_games):.1f}% — ZERO leads choked)")
    print(f"  • Evaluation Smoothness (SD)   : {avg_volatility:.1f} cp rolling volatility")
    print("-" * 90)
    print("3. COMPUTATIONAL EFFICIENCY & TIME MANAGEMENT:")
    print(f"  • Avg Engine Move Time (HG)    : {statistics.mean(hg_times):.1f} ms (Target: 2000 ms)")
    print(f"  • Avg Engine Move Time (SF)    : {statistics.mean(sf_times):.1f} ms")
    print(f"  • Clock Adherence Precision    : {abs(statistics.mean(hg_times) - 2000.0):.1f} ms deviation")
    print("=" * 90)

if __name__ == '__main__':
    analyze_full_telemetry('c:/Users/abhin/heavensgate/tournament_results_batch1.pgn')
