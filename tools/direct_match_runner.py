import subprocess
import time
import os
import sys

TournamentOpenings = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqkbnr/pppp1ppp/8/8/3PP3/8/PPP2PPP/RNBQKBNR b KQkq d3 0 2",
    "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5",
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 4",
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq b3 0 4",
    "rnbqkbnr/pppp1ppp/8/4p3/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 2",
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
    "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",
    "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "r1bqkbnr/pppp1ppp/2n5/3Pp3/4P3/5N2/PPP2PPP/RNBQK2R b KQkq - 0 3",
    "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 3",
    "rnbqkb1r/p2ppppp/5n2/1ppP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq b6 0 4"
]

class UCIEngine:
    def __init__(self, name, exe_path, threads=6):
        self.name = name
        self.exe_path = exe_path
        self.threads = threads
        self.proc = None
        self.start()

    def start(self):
        self.proc = subprocess.Popen(
            [self.exe_path, "uci"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        self.send("uci")
        self.read_until("uciok")
        self.send(f"setoption name Threads value {self.threads}")
        self.send("isready")
        self.read_until("readyok")

    def send(self, cmd):
        if self.proc and self.proc.stdin:
            self.proc.stdin.write(cmd + "\n")
            self.proc.stdin.flush()

    def read_until(self, token, timeout=30):
        start = time.time()
        while time.time() - start < timeout:
            line = self.proc.stdout.readline()
            if not line:
                break
            line = line.strip()
            if token in line:
                return line
        return ""

    def new_game(self):
        self.send("ucinewgame")
        self.send("isready")
        self.read_until("readyok")

    def get_move(self, fen, moves_history, movetime_ms=2000):
        if moves_history:
            cmd = f"position fen {fen} moves {' '.join(moves_history)}"
        else:
            cmd = f"position fen {fen}"
        self.send(cmd)
        self.send("isready")
        self.read_until("readyok")

        self.send(f"go movetime {movetime_ms}")
        start = time.time()
        best_move = None
        last_eval = 0
        last_depth = 0
        last_nodes = 0

        while time.time() - start < (movetime_ms / 1000.0) + 10:
            line = self.proc.stdout.readline()
            if not line:
                break
            line = line.strip()
            if line.startswith("info"):
                parts = line.split()
                if "depth" in parts:
                    try: last_depth = int(parts[parts.index("depth") + 1])
                    except: pass
                if "score" in parts:
                    try:
                        s_idx = parts.index("score")
                        if parts[s_idx+1] == "cp":
                            last_eval = int(parts[s_idx+2])
                        elif parts[s_idx+1] == "mate":
                            m = int(parts[s_idx+2])
                            last_eval = 30000 - m if m > 0 else -30000 - m
                    except: pass
                if "nodes" in parts:
                    try: last_nodes = int(parts[parts.index("nodes") + 1])
                    except: pass
            elif line.startswith("bestmove"):
                parts = line.split()
                if len(parts) >= 2:
                    best_move = parts[1]
                break

        return best_move, last_eval, last_depth, last_nodes

    def quit(self):
        if self.proc:
            try:
                self.send("quit")
                self.proc.terminate()
            except: pass

def run_match(num_games=20, movetime_ms=2000, threads=6):
    exe_p3 = r"c:\Users\abhin\heavensgate\heavensgate_phase3.exe"
    exe_p5 = r"c:\Users\abhin\heavensgate\heavensgate_current.exe"

    print("=" * 85)
    print(f"   DIRECT HEAD-TO-HEAD MATCH: PHASE 3 PEAK vs PHASE 5 CURRENT ({num_games} Games @ {movetime_ms/1000:.1f}s/mv)")
    print("=" * 85)

    eng_p3 = UCIEngine("Phase 3 Peak", exe_p3, threads)
    eng_p5 = UCIEngine("Phase 5 Current", exe_p5, threads)

    p3_score = 0.0
    p5_score = 0.0
    draws = 0

    pgn_path = "c:/Users/abhin/heavensgate/direct_head_to_head_p3_vs_p5.pgn"
    with open(pgn_path, "w", encoding="utf-8") as f:
        f.write("")

    for g in range(1, num_games + 1):
        fen = TournamentOpenings[(g - 1) % len(TournamentOpenings)]
        p3_is_white = (g % 2 != 0)

        white_engine = eng_p3 if p3_is_white else eng_p5
        black_engine = eng_p5 if p3_is_white else eng_p3

        eng_p3.new_game()
        eng_p5.new_game()

        moves_history = []
        result = "*"
        reason = ""

        print(f"\n--- Game {g}/{num_games} ---")
        print(f"White: {white_engine.name} | Black: {black_engine.name}")
        print(f"Opening FEN: {fen}")

        ply = 0
        while ply < 250:
            mover = white_engine if (ply % 2 == 0) else black_engine
            side_str = "W" if (ply % 2 == 0) else "B"
            move_num = (ply // 2) + 1

            mv, ev, depth, nodes = mover.get_move(fen, moves_history, movetime_ms)
            if not mv or mv == "0000" or mv == "(none)":
                result = "0-1" if side_str == "W" else "1-0"
                reason = f"{mover.name} forfeit (no move)"
                break

            moves_history.append(mv)
            print(f"  [M{move_num} {side_str}] {mover.name}: {mv} | Eval: {ev:+d} cp | D:{depth} | Nodes: {nodes}", flush=True)

            # Forced mate detection
            if abs(ev) >= 25000:
                if ev > 0:
                    result = "1-0" if side_str == "W" else "0-1"
                else:
                    result = "0-1" if side_str == "W" else "1-0"
                reason = "Forced Checkmate"
                break

            # Repetition detection
            if len(moves_history) >= 12:
                if moves_history[-1] == moves_history[-5] == moves_history[-9] and moves_history[-2] == moves_history[-6] == moves_history[-10]:
                    result = "1/2-1/2"
                    reason = "Threefold Repetition"
                    break

            ply += 1

        if result == "*":
            result = "1/2-1/2"
            reason = "Move Limit"

        # Tally score
        p3_won = (p3_is_white and result == "1-0") or (not p3_is_white and result == "0-1")
        p5_won = (not p3_is_white and result == "1-0") or (p3_is_white and result == "0-1")

        if p3_won:
            p3_score += 1.0
            winner_str = "Phase 3 Peak WINS"
        elif p5_won:
            p5_score += 1.0
            winner_str = "Phase 5 Current WINS"
        else:
            p3_score += 0.5
            p5_score += 0.5
            draws += 1
            winner_str = "DRAW"

        print(f"[RESULT] Game {g}: {result} ({reason}) -> {winner_str}", flush=True)
        print(f"[STANDINGS] Phase 3: {p3_score:.1f} | Phase 5: {p5_score:.1f} | Draws: {draws}", flush=True)

        with open(pgn_path, "a", encoding="utf-8") as f:
            f.write(f'[Event "Direct Head-to-Head Master Match"]\n')
            f.write(f'[Round "{g}"]\n')
            f.write(f'[White "{white_engine.name}"]\n')
            f.write(f'[Black "{black_engine.name}"]\n')
            f.write(f'[Result "{result}"]\n')
            f.write(f'[FEN "{fen}"]\n')
            f.write(f'{" ".join(moves_history)} {result} {{{reason}}}\n\n')

    eng_p3.quit()
    eng_p5.quit()

    print("\n" + "=" * 85)
    print("                     FINAL HEAD-TO-HEAD MATCH RESULTS")
    print("=" * 85)
    print(f"Phase 3 Peak Score    : {p3_score:.1f} / {num_games} ({100.0 * p3_score / num_games:.1f}%)")
    print(f"Phase 5 Current Score : {p5_score:.1f} / {num_games} ({100.0 * p5_score / num_games:.1f}%)")
    print(f"Draws                 : {draws}")
    print("=" * 85)

if __name__ == '__main__':
    run_match(20, 2000, 6)
