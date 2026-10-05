import re

pgn_path = 'tournament_results_batch2.pgn'
with open(pgn_path, 'r', encoding='utf-8', errors='ignore') as f:
    content = f.read()

games = [g.strip() for g in content.split('[Event ') if g.strip()]
print(f'Total games in batch 2: {len(games)}')

draws = []
moves_per_game = []
for i, g in enumerate(games, 1):
    round_m = re.search(r'\[Round\s+"([^"]+)"\]', g)
    white_m = re.search(r'\[White\s+"([^"]+)"\]', g)
    black_m = re.search(r'\[Black\s+"([^"]+)"\]', g)
    fen_m = re.search(r'\[FEN\s+"([^"]+)"\]', g)
    moves = re.findall(r'\b([a-h][1-8][a-h][1-8][qrbn]?)\b\s*\{', g)
    moves_per_game.append(len(moves)//2)
    
    if '1/2-1/2' in g:
        reason_m = re.search(r'1/2-1/2\s*\{([^}]*)\}', g)
        draws.append((
            round_m.group(1) if round_m else str(i),
            white_m.group(1) if white_m else '?',
            black_m.group(1) if black_m else '?',
            len(moves)//2,
            reason_m.group(1) if reason_m else '?',
            fen_m.group(1) if fen_m else '?'
        ))

print(f'\nAverage Game Length: {sum(moves_per_game)/len(moves_per_game):.1f} moves')
print(f'Draws ({len(draws)}):')
for d in draws:
    print(f'Round {d[0]}: {d[1]} vs {d[2]} | Moves: {d[3]} | Reason: {d[4]} | FEN: {d[5]}')
