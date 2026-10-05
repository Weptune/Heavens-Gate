import os
import re
import chess

pgn_path = r'c:\Users\abhin\heavensgate\tournament_results_batch1.pgn'
with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

games = [g.strip() for g in content.split('[Event ') if g.strip()]

g24_text = None
for g in games:
    if '[Round "24"]' in g:
        g24_text = g
        break

if not g24_text:
    print("Game 24 not found!")
    exit(1)

print("=" * 80)
print("              DEEP AUTOPSY REPORT: ROUND 24")
print("        Stockfish 16.1 (White) vs Heaven's Gate Master (Black)")
print("=" * 80)

# Extract Header Info
fen_m = re.search(r'\[FEN\s+"([^"]+)"\]', g24_text)
starting_fen = fen_m.group(1) if fen_m else chess.STARTING_FEN
print(f"Starting FEN: {starting_fen}")

# Extract move entries: move, eval, clk
pattern = r'(\d+\.)?\s*([a-h1-8NBRQKx\+#=]+)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}'
matches = re.findall(pattern, g24_text)

board = chess.Board(starting_fen)

print(f"\nTotal Plies: {len(matches)} ({len(matches)//2} full moves)")
print("-" * 80)
print(f"{'Ply':>3} | {'Move':<7} | {'Turn':<5} | {'Engine':<12} | {'Eval (cp)':>10} | {'Time (ms)':>10} | {'FEN / Notes'}")
print("-" * 80)

eval_history = []
blunders = []

prev_hg_eval = None
prev_sf_eval = None

for ply_idx, m in enumerate(matches, 1):
    num_str, uci_move, eval_str, clk_str = m
    ev = int(eval_str)
    clk = float(clk_str)
    turn_color = "White" if board.turn == chess.WHITE else "Black"
    engine_name = "Stockfish" if board.turn == chess.WHITE else "Master"
    
    # Calculate centipawn drop
    # In PGN, eval is side-to-move's perspective
    # If Master is Black: positive eval for Master means Black is winning.
    eval_history.append((ply_idx, engine_name, uci_move, ev, clk))
    
    flag = ""
    if engine_name == "Master":
        if prev_hg_eval is not None:
            cpl = prev_hg_eval - ev
            if cpl > 150:
                flag = f"!! BLUNDER (-{cpl} cp drop)"
                blunders.append((ply_idx, uci_move, prev_hg_eval, ev, cpl, board.fen()))
            elif cpl > 70:
                flag = f"! MISTAKE (-{cpl} cp drop)"
        prev_hg_eval = ev
    else:
        if prev_sf_eval is not None:
            cpl = prev_sf_eval - ev
            if cpl > 150:
                flag = f"?? SF BLUNDER (-{cpl} cp drop)"
        prev_sf_eval = ev

    print(f"{ply_idx:3d} | {uci_move:<7} | {turn_color:<5} | {engine_name:<12} | {ev:+10d} | {clk:9.1f} | {flag}")
    
    try:
        move_obj = chess.Move.from_uci(uci_move)
        board.push(move_obj)
    except Exception as e:
        print(f"Illegal or parse error move {uci_move}: {e}")
        break

print("=" * 80)
print(f"CRITICAL SWING ANALYSIS & BLUNDERS DETECTED: {len(blunders)}")
print("=" * 80)
for b in blunders:
    ply, mv, pre, post, cpl, fen = b
    print(f"• Ply {ply} (Move {(ply+1)//2} Black): Master played '{mv}'")
    print(f"  Eval dropped from {pre:+d} cp to {post:+d} cp (CPL drop: {cpl} cp)")
    print(f"  Position before move: {fen}\n")
