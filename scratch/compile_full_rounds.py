import os
import re
import json

def compile_full_rounds():
    pgn_dir = "pgn_history"
    if not os.path.exists(pgn_dir):
        print(f"[ERROR] Directory {pgn_dir} does not exist.")
        return

    # Find all pgn files in numerical order of round number
    files = []
    for f in os.listdir(pgn_dir):
        m = re.match(r'tournament_round_(\d+)\.pgn', f)
        if m:
            files.append((int(m.group(1)), os.path.join(pgn_dir, f)))

    files.sort(key=lambda x: x[0])

    # Extract all complete games from all PGN files
    all_games = []
    for r_num, filepath in files:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as pf:
            text = pf.read().strip()
            if not text:
                continue
            
            # Split games by [Event
            parts = text.split('[Event ')
            for p in parts:
                p_str = p.strip()
                if not p_str:
                    continue
                # Reconstruct full game string
                full_game = '[Event ' + p_str
                # Verify game has a result
                lines = [line.strip() for line in p_str.split('\n') if line.strip()]
                if not lines:
                    continue
                last_line = lines[-1]
                if '1-0' in last_line or '0-1' in last_line or '1/2-1/2' in last_line:
                    all_games.append(full_game)

    print(f"[COMPILING] Total valid complete games collected across all rounds: {len(all_games)}")

    # Chunk into 100-game complete rounds
    compiled_rounds = []
    chunk_size = 100
    for i in range(0, len(all_games), chunk_size):
        chunk = all_games[i:i+chunk_size]
        # Only keep full 100-game rounds (or the active running chunk if >= 50 games)
        if len(chunk) == 100:
            compiled_rounds.append(chunk)
        elif len(chunk) >= 50:
            print(f"[NOTICE] Trailing partial chunk of {len(chunk)} games kept as last round.")
            compiled_rounds.append(chunk)
        else:
            print(f"[NOTICE] Trailing small chunk of {len(chunk)} games merged or skipped.")

    print(f"[COMPILING] Formed {len(compiled_rounds)} clean full-sized rounds!")

    # Write out compiled PGN files and regenerate telemetry reports
    from generate_round_report import generate_round_report

    # Clear existing history summary and master json before clean rebuild
    if os.path.exists("tournament_history_summary.txt"):
        os.remove("tournament_history_summary.txt")
    if os.path.exists("tournament_detailed_history.json"):
        os.remove("tournament_detailed_history.json")

    for idx, c_games in enumerate(compiled_rounds, 1):
        c_pgn_path = os.path.join(pgn_dir, f"compiled_round_{idx}.pgn")
        with open(c_pgn_path, 'w', encoding='utf-8') as cf:
            cf.write('\n\n'.join(c_games) + '\n\n')

        # Run telemetry report generator for this compiled 100-game round
        generate_round_report(c_pgn_path, idx)

    print(f"[SUCCESS] Cleaned and compiled datasheet into {len(compiled_rounds)} 100-game rounds!")

if __name__ == "__main__":
    compile_full_rounds()
