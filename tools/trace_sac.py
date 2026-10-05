import chess

b = chess.Board('rnbqkbnr/pppp1ppp/8/4p3/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 2')
game_moves = 'g8f6 e2e3 f8c5 d2d4 e5d4 g1f3 d4e3 c1e3 c5e3 d1e2 e8g8 f2e3 b8c6 e2d2 f6e4 d2d3 f8e8 a2a3 e4c5 d3c3 c5a4 c3d2 a4b2 d2b2 e8e3 f1e2 d8e7 e1f2 c6e5 h1e1 e7c5 f3d4 e5c6 f2e3'.split()

cur_b = b.copy()
for i, m_str in enumerate(game_moves):
    m = chess.Move.from_uci(m_str)
    san = cur_b.san(m)
    cur_b.push(m)
    move_num = (i // 2) + 1
    turn = "Black" if i % 2 == 0 else "White" # note: black played move 0
    if i >= 16: # from move 9 onwards
        print(f"Ply {i+1:2d} ({turn}): {san:7s} | FEN: {cur_b.fen()}")
