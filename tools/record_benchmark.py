import sys
import re
import os
from datetime import datetime

def parse_tournament_log(log_path):
    if not os.path.exists(log_path):
        print(f"File {log_path} not found.")
        return None

    with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    hg_nodes = []
    hg_times = []
    sf_nodes = []
    sf_times = []
    mate_plies = []
    game_lengths = []

    for line in content.splitlines():
        if "[G" in line and "Master:" in line:
            n_match = re.search(r'Nodes:\s*(\d+)', line)
            t_match = re.search(r'Time:\s*(\d+)ms', line)
            if n_match: hg_nodes.append(int(n_match.group(1)))
            if t_match: hg_times.append(int(t_match.group(1)))
        elif "[G" in line and "Stockfish" in line:
            n_match = re.search(r'Nodes:\s*(\d+)', line)
            t_match = re.search(r'Time:\s*(\d+)ms', line)
            if n_match: sf_nodes.append(int(n_match.group(1)))
            if t_match: sf_times.append(int(t_match.group(1)))
        elif "Forced Mate in" in line:
            m_match = re.search(r'Forced Mate in (\d+) moves', line)
            if m_match: mate_plies.append(int(m_match.group(1)))
        elif "[TOURNAMENT] Game " in line:
            m_len = re.search(r'\((\d+) moves', line)
            if m_len: game_lengths.append(int(m_len.group(1)))

    wins = content.count("Master 1 - 0") + content.count("Master 2 - 0") + content.count("Master 3 - 0") + content.count("Master 4 - 0") + content.count("Master 5 - 0") + content.count("Master 6 - 0") + content.count("Master 7 - 0") + content.count("Master 8 - 0") + content.count("Master 9 - 0") + content.count("Master 10 - 0")
    total_games = len(game_lengths)

    avg_hg_nodes = sum(hg_nodes) / len(hg_nodes) if hg_nodes else 0
    avg_game_len = sum(game_lengths) / len(game_lengths) if game_lengths else 0
    avg_mate = sum(mate_plies) / len(mate_plies) if mate_plies else 0

    return {
        "games": total_games,
        "avg_nodes": avg_hg_nodes,
        "avg_game_len": avg_game_len,
        "avg_mate": avg_mate
    }

if __name__ == "__main__":
    log_file = sys.argv[1] if len(sys.argv) > 1 else ""
    if log_file:
        res = parse_tournament_log(log_file)
        if res:
            print(f"[TELEMETRY PARSED] Games: {res['games']}, Avg Moves: {res['avg_game_len']:.1f}, Avg Nodes: {res['avg_nodes']:,.0f}, Avg Mate: {res['avg_mate']:.1f}")
