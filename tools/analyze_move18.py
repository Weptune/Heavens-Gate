import chess
import chess.engine

sf = chess.engine.SimpleEngine.popen_uci('tools/stockfish.exe')
b = chess.Board('r1b3k1/pppp1ppp/2n5/2q5/2PN4/P3K3/1Q2B1PP/RN2R3 b - - 0 19')

print("Stockfish Top 5 Moves in Critical Position (Move 18 Black to move):")
infos = sf.analyse(b, chess.engine.Limit(depth=18), multipv=5)
for i, info in enumerate(infos, 1):
    mv = info['pv'][0]
    score = info['score'].relative.score(mate_score=30000)
    
    # Format line cleanly
    line_board = b.copy()
    sans = []
    for m in info['pv'][:6]:
        if m in line_board.legal_moves:
            sans.append(line_board.san(m))
            line_board.push(m)
        else:
            sans.append(m.uci())
    pv_str = ' '.join(sans)
    print(f"#{i} {b.san(mv):8s} ({mv.uci()}) | Score: {score:+d} cp | Line: {pv_str}")

print("\nStockfish Evaluation of Master's choice 'b5':")
b_b5 = b.copy()
b_b5.push(chess.Move.from_uci('b7b5'))
info_b5 = sf.analyse(b_b5, chess.engine.Limit(depth=18))
score_b5 = info_b5['score'].relative.score(mate_score=30000) # relative to White
print(f"After 18... b5, Stockfish evaluation for White: {score_b5:+d} cp")

sf.quit()
