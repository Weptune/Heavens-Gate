import re
import os
import statistics

def parse_tournament(pgn_path):
    if not os.path.exists(pgn_path):
        return None

    with open(pgn_path, 'r', encoding='utf-8') as f:
        raw_content = f.read()

    games_raw = raw_content.strip().split('[Event "Heaven\'s Gate Grandmaster Tournament"]')
    games = [g for g in games_raw if g.strip()]

    total_games = len(games)
    if total_games == 0:
        return None

    hg_wins, sf_wins, draws = 0, 0, 0
    game_lengths = []
    hg_move_counts = {"best": 0, "excellent": 0, "good": 0, "inaccuracy": 0, "mistake": 0, "blunder": 0}
    op_cpl, mid_cpl, end_cpl = [], [], []

    for g_text in games:
        white_is_hg = 'White "Master Edition"' in g_text
        pattern = r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}'
        matches = re.findall(pattern, g_text)

        ply_count = len(matches)
        game_lengths.append((ply_count + 1) // 2)

        lines = g_text.strip().split('\n')
        last_line = lines[-1] if lines else ""
        prev_line = lines[-2] if len(lines) > 1 else ""

        if "1-0" in last_line or "1-0" in prev_line:
            if white_is_hg: hg_wins += 1
            else: sf_wins += 1
        elif "0-1" in last_line or "0-1" in prev_line:
            if not white_is_hg: hg_wins += 1
            else: sf_wins += 1
        elif "1/2-1/2" in last_line or "1/2-1/2" in prev_line:
            draws += 1

        hg_prev = None
        for ply_idx, match in enumerate(matches):
            num, move_str, eval_str, clk_str = match
            raw_eval = int(eval_str)
            clamped = max(-2000, min(2000, raw_eval))

            is_white = (ply_idx % 2 == 0)
            is_hg = (is_white and white_is_hg) or (not is_white and not white_is_hg)
            move_num = (ply_idx // 2) + 1
            hg_score = clamped if is_hg else -clamped

            if is_hg:
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

    total_hg = sum(hg_move_counts.values()) or 1
    best_pct = (hg_move_counts["best"] + hg_move_counts["excellent"]) * 100.0 / total_hg
    inacc_pct = hg_move_counts["inaccuracy"] * 100.0 / total_hg
    blunder_pct = hg_move_counts["blunder"] * 100.0 / total_hg

    op_acpl = statistics.mean(op_cpl) if op_cpl else 0
    mid_acpl = statistics.mean(mid_cpl) if mid_cpl else 0
    end_acpl = statistics.mean(end_cpl) if end_cpl else 0

    return {
        "games": total_games,
        "record": f"{hg_wins}W - {sf_wins}L - {draws}D",
        "win_rate": (hg_wins + 0.5 * draws) * 100.0 / max(1, total_games),
        "avg_len": statistics.mean(game_lengths) if game_lengths else 0,
        "op_acpl": op_acpl,
        "op_acc": max(40.0, min(99.0, 100.0 - (op_acpl * 0.45))),
        "mid_acpl": mid_acpl,
        "mid_acc": max(40.0, min(99.0, 100.0 - (mid_acpl * 0.45))),
        "end_acpl": end_acpl,
        "end_acc": max(40.0, min(99.0, 100.0 - (end_acpl * 0.45))),
        "best_pct": best_pct,
        "inacc_pct": inacc_pct,
        "blunder_pct": blunder_pct
    }

def print_live_comparison():
    p3 = parse_tournament("c:/Users/abhin/heavensgate/tournament_results_batch1.pgn")
    p5 = parse_tournament("c:/Users/abhin/heavensgate/tournament_results_batch2.pgn")

    print("=" * 95)
    print("        PARALLEL 100-MATCH GRAND TOURNAMENT: PHASE 3 PEAK vs PHASE 5 CURRENT")
    print("=" * 95)
    print(f"{'Performance Metric':<32} | {'Phase 3 Peak (Batch 1)':<28} | {'Phase 5 Current (Batch 2)':<28}")
    print("-" * 95)

    if not p3 or not p5:
        print(f"Waiting for game data... (P3 games: {p3['games'] if p3 else 0}, P5 games: {p5['games'] if p5 else 0})")
        return

    print(f"{'Completed Games':<32} | {p3['games']:<28} | {p5['games']:<28}")
    print(f"{'Match Score vs Stockfish 3400':<32} | {p3['record']:<28} | {p5['record']:<28}")
    print(f"{'Win Rate %':<32} | {p3['win_rate']:<27.1f}% | {p5['win_rate']:<27.1f}%")
    print(f"{'Average Game Duration':<32} | {p3['avg_len']:<25.1f} mv | {p5['avg_len']:<25.1f} mv")
    print("-" * 95)
    print(f"{'Opening Accuracy (Moves 1-10)':<32} | {p3['op_acc']:<5.1f}% ({p3['op_acpl']:.1f} cp)          | {p5['op_acc']:<5.1f}% ({p5['op_acpl']:.1f} cp)")
    print(f"{'Midgame Accuracy (Moves 11-25)':<32} | {p3['mid_acc']:<5.1f}% ({p3['mid_acpl']:.1f} cp)          | {p5['mid_acc']:<5.1f}% ({p5['mid_acpl']:.1f} cp)")
    print(f"{'Endgame Accuracy (Moves 26+)':<32} | {p3['end_acc']:<5.1f}% ({p3['end_acpl']:.1f} cp)          | {p5['end_acc']:<5.1f}% ({p5['end_acpl']:.1f} cp)")
    print("-" * 95)
    print(f"{'Top Quality Moves (0-15 cp)':<32} | {p3['best_pct']:<27.1f}% | {p5['best_pct']:<27.1f}%")
    print(f"{'Inaccuracy Rate (35-80 cp)':<32} | {p3['inacc_pct']:<27.1f}% | {p5['inacc_pct']:<27.1f}%")
    print(f"{'Blunder Rate (>200 cp)':<32} | {p3['blunder_pct']:<27.1f}% | {p5['blunder_pct']:<27.1f}%")
    print("=" * 95)

if __name__ == '__main__':
    print_live_comparison()
