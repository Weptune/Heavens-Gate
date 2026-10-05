import sys
import os
import re
import math

def check_progress(pgn_path=None, target_games=100):
    if pgn_path is None:
        if len(sys.argv) > 1:
            pgn_path = sys.argv[1]
        elif os.path.exists(r'c:\Users\abhin\heavensgate\tournament_results_batch2.pgn'):
            pgn_path = r'c:\Users\abhin\heavensgate\tournament_results_batch2.pgn'
        else:
            pgn_path = r'c:\Users\abhin\heavensgate\tournament_results_batch1.pgn'

    if not os.path.exists(pgn_path):
        print(f"Waiting for PGN file to be generated at {pgn_path}...")
        return

    with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    games_raw = content.strip().split('[Event "Heaven\'s Gate Grandmaster Tournament"]')
    games = [g.strip() for g in games_raw if g.strip()]

    completed = len(games)
    if completed == 0:
        print("Tournament starting... No finished games yet.")
        return

    m_wins = 0
    sf_wins = 0
    draws = 0
    m_white_w, m_white_tot = 0, 0
    m_black_w, m_black_tot = 0, 0
    lengths = []
    recent_history = []

    for g_idx, g in enumerate(games, 1):
        white_is_m = 'White "Master Edition"' in g
        res_m = re.search(r'\[Result\s+"([^"]+)"\]', g)
        res_str = res_m.group(1) if res_m else ""
        
        if not res_str:
            if "1-0" in g: res_str = "1-0"
            elif "0-1" in g: res_str = "0-1"
            elif "1/2-1/2" in g: res_str = "1/2-1/2"

        plies = len(re.findall(r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{', g))
        full_moves = (plies + 1) // 2
        lengths.append(full_moves)

        m_win = (white_is_m and res_str == "1-0") or (not white_is_m and res_str == "0-1")
        sf_win = (white_is_m and res_str == "0-1") or (not white_is_m and res_str == "1-0")

        if white_is_m:
            m_white_tot += 1
            if m_win: m_white_w += 1
        else:
            m_black_tot += 1
            if m_win: m_black_w += 1

        if m_win:
            m_wins += 1
            winner = "Master"
        elif sf_win:
            sf_wins += 1
            winner = "Stockfish"
        else:
            draws += 1
            winner = "Draw"

        reason_m = re.search(r'\{([^}]+)\}\s*$', g.strip())
        reason = reason_m.group(1) if reason_m else "Normal finish"
        recent_history.append((g_idx, "White" if white_is_m else "Black", res_str, winner, full_moves, reason))

    score = m_wins + 0.5 * draws
    score_pct = (score / completed) * 100.0 if completed > 0 else 0.0

    elo_diff = 0.0
    if 0 < score_pct < 100:
        elo_diff = -400.0 * math.log10(100.0 / score_pct - 1.0)
    elif score_pct >= 100:
        elo_diff = 800.0
    elif score_pct <= 0:
        elo_diff = -800.0

    print("=" * 65)
    print(f"  HEAVEN'S GATE vs UNCAPPED STOCKFISH 16.1 -- 2+0 TOURNAMENT")
    print(f"  PGN File: {os.path.basename(pgn_path)}")
    print("=" * 65)
    print(f"Progress        : {completed} / {target_games} Games ({completed * 100.0 / target_games:.1f}%)")
    print(f"Score Record    : Master {m_wins}W - {sf_wins}L - {draws}D")
    print(f"Master Score    : {score_pct:.1f}% ({score}/{completed} pts)")
    print(f"Delta Elo       : {elo_diff:+.0f} Elo")
    if m_white_tot > 0:
        print(f"White Win Rate  : {m_white_w}/{m_white_tot} ({m_white_w * 100.0 / m_white_tot:.1f}%)")
    if m_black_tot > 0:
        print(f"Black Win Rate  : {m_black_w}/{m_black_tot} ({m_black_w * 100.0 / m_black_tot:.1f}%)")
    if lengths:
        print(f"Avg Game Length : {sum(lengths) / len(lengths):.1f} moves (min: {min(lengths)}, max: {max(lengths)})")
    print("-" * 65)
    print("Recent Games:")
    for r in recent_history[-5:]:
        print(f"  Game {r[0]:2d}: Master as {r[1]:5s} -> {r[3]:9s} ({r[2]}) in {r[4]:2d} moves [{r[5]}]")
    print("=" * 65)

if __name__ == '__main__':
    check_progress()
