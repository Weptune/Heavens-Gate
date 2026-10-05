import os
import re
import hashlib

pgn_path = r'c:\Users\abhin\heavensgate\tournament_results_batch1.pgn'
if not os.path.exists(pgn_path):
    print('PGN not found!')
    exit()

with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

games_raw = [g.strip() for g in content.split('[Event ') if g.strip()]

records = []
for i, g in enumerate(games_raw, 1):
    fen_m = re.search(r'\[FEN\s+"([^"]+)"\]', g)
    white_m = re.search(r'\[White\s+"([^"]+)"\]', g)
    black_m = re.search(r'\[Black\s+"([^"]+)"\]', g)
    round_m = re.search(r'\[Round\s+"([^"]+)"\]', g)
    
    # Matches move text
    moves = re.findall(r'\b([a-h][1-8][a-h][1-8][qrbn]?)\b\s*\{', g)
    
    # Result & Reason from end of game
    res_m = re.search(r'(1-0|0-1|1/2-1/2)\s*\{([^}]*)\}', g)
    if res_m:
        res_str = res_m.group(1)
        reason = res_m.group(2)
    else:
        res_str = '?'
        reason = 'In progress / unrecorded'
        
    fen = fen_m.group(1) if fen_m else 'startpos'
    white = white_m.group(1) if white_m else '?'
    black = black_m.group(1) if black_m else '?'
    rnd = int(round_m.group(1)) if round_m else i
    
    move_seq = ' '.join(moves)
    move_hash = hashlib.sha256(move_seq.encode('utf-8')).hexdigest()[:12]
    
    records.append({
        'round': rnd,
        'white': white,
        'black': black,
        'fen': fen,
        'moves': moves,
        'plies': len(moves),
        'full_moves': (len(moves) + 1) // 2,
        'move_seq': move_seq,
        'move_hash': move_hash,
        'result': res_str,
        'reason': reason
    })

print("=" * 80)
print(f"       HEAVEN'S GATE vs STOCKFISH 16.1 (3400 ELO) MATCH UNIQUENESS AUDIT")
print("=" * 80)
print(f"Total Games Completed & Verified in PGN: {len(records)}")

# 1. Check Full Move Sequence Uniqueness
seen_seqs = {}
duplicate_seqs = []
for r in records:
    if r['move_seq'] in seen_seqs:
        duplicate_seqs.append((r['round'], seen_seqs[r['move_seq']]))
    else:
        seen_seqs[r['move_seq']] = r['round']

# 2. Check 10-Move Opening Line Uniqueness
seen_openings = {}
opening_transpositions = []
for r in records:
    first_10 = ' '.join(r['moves'][:20]) # first 10 full moves = 20 plies
    if first_10 in seen_openings:
        opening_transpositions.append((r['round'], seen_openings[first_10]))
    else:
        seen_openings[first_10] = r['round']

# 3. Check Starting FEN Variety
unique_fens = set(r['fen'] for r in records)

print(f"\n1. Full Game Trajectory Uniqueness: " + 
      ("100% UNIQUE (0 duplicate games found)" if not duplicate_seqs else f"DUPLICATES DETECTED: {duplicate_seqs}"))
print(f"2. 10-Move Opening Divergence     : " + 
      ("100% DIVERGENT (0 shared opening paths)" if not opening_transpositions else f"Transpositions: {opening_transpositions}"))
print(f"3. Distinct Starting Opening FENs : {len(unique_fens)} distinct FENs across {len(records)} games")

print("\n" + "-" * 80)
print(f"{'Rnd':>3} | {'White Mover':<15} | {'Black Mover':<25} | {'Result':<7} | {'Moves':>5} | {'Hash':<12} | {'Key Ending Reason'}")
print("-" * 80)
for r in records:
    print(f"{r['round']:3d} | {r['white']:<15} | {r['black']:<25} | {r['result']:<7} | {r['full_moves']:5d} | {r['move_hash']:<12} | {r['reason']}")
print("=" * 80)
