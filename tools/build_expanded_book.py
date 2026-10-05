import chess
import chess.engine
import chess.polyglot
import struct
import shutil
import os
import sys

def encode_polyglot_move(move):
    from_file = move.from_square % 8
    from_rank = move.from_square // 8
    to_file = move.to_square % 8
    to_rank = move.to_square // 8
    promo = 0
    if move.promotion:
        promo_map = {chess.KNIGHT: 1, chess.BISHOP: 2, chess.ROOK: 3, chess.QUEEN: 4}
        promo = promo_map.get(move.promotion, 0)
    return (promo << 12) | (from_rank << 9) | (from_file << 6) | (to_rank << 3) | to_file

TournamentOpenings = [
    ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Startpos"),
    ("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2", "Sicilian Open"),
    ("rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5", "Sicilian Najdorf"),
    ("r1bqkb1r/pp1ppp1p/2n2np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6", "Sicilian Dragon"),
    ("r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", "Ruy Lopez Main Line"),
    ("r1bqkb1r/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", "Ruy Lopez Berlin"),
    ("rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", "French Advance"),
    ("rnbqk1nr/pppp1ppp/4p3/8/3PP3/2b5/PPP2PPP/R1BQKBNR w KQkq - 0 4", "French Winawer"),
    ("rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2", "QGD"),
    ("rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3", "QGA"),
    ("rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 1 6", "KID Classical"),
    ("rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2P2/PP4PP/R1BQKBNR w KQ - 0 6", "KID Samisch"),
    ("rnbqkbnr/pp2pppp/2p5/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", "Caro-Kann Main Line"),
    ("rnbqkbnr/pp2pppp/2p5/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3", "Caro-Kann Advance"),
    ("r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4", "Four Knights"),
    ("r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", "Italian Giuoco Piano"),
    ("r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq b3 0 4", "Italian Evans Gambit"),
    ("rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3", "Petrov"),
    ("rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq d6 0 3", "QGD Exchange"),
    ("rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5", "QGD Tartakower"),
    ("rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 4", "Grunfeld"),
    ("rnbqk2r/pppp1ppp/4pn2/8/2PP4/2P5/P3PPPP/R1BQKBNR w KQkq - 0 4", "Nimzo-Indian"),
    ("rnbqkbnr/pppppp1p/6p1/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", "Modern Defense"),
    ("rnbqkbnr/pppp1ppp/8/4p3/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 2", "English Opening"),
    ("rnbqkbnr/ppp1p1pp/8/3p1p2/2PP4/8/PP2PPPP/RNBQKBNR w KQkq f6 0 3", "Dutch Defense"),
    ("rnbqkbnr/pp1ppppp/8/3P4/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2", "Benoni"),
    ("r1bqk2r/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w kq - 4 8", "Tarrasch"),
    ("rnbqkbnr/pppp1ppp/8/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2", "Vienna Game"),
    ("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", "Scandinavian"),
    ("rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2", "Alekhine"),
    ("rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3", "Slav Defense"),
    ("rnbqk2r/pp2bppp/2p1pn2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQkq - 0 6", "Semi-Slav"),
    ("rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2", "King's Gambit"),
    ("rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", "Pirc Defense"),
    ("rnbqkbnr/ppp1pppp/3p4/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", "Philidor"),
    ("r1bqkbnr/pppp1ppp/2n5/3Pp3/4P3/5N2/PPP2PPP/RNBQK2R b KQkq - 0 3", "Scotch Game"),
    ("rnbqkbnr/pppp1ppp/8/8/3pP3/2P5/PP3PPP/RNBQKBNR b KQkq - 0 3", "Danish Gambit"),
    ("rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq f3 0 1", "Bird's Opening"),
    ("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1", "Reti Opening"),
    ("rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 3", "Catalan"),
    ("rnbqkb1r/p2ppppp/5n2/1ppP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq b6 0 4", "Benko Gambit"),
]

def main():
    print("=" * 65)
    print("  HEAVEN'S GATE GRANDMASTER POLYGLOT BOOK EXPANSION")
    print("=" * 65)

    stockfish_path = os.path.abspath("tools/stockfish.exe")
    if not os.path.exists(stockfish_path):
        print(f"Error: Stockfish not found at {stockfish_path}")
        sys.exit(1)

    # 1. Load existing entries
    existing_entries = {}
    if os.path.exists("performance.bin"):
        with chess.polyglot.open_reader("performance.bin") as reader:
            for e in reader:
                key = e.key
                raw_move = e.raw_move
                weight = e.weight
                learn = e.learn
                existing_entries[(key, raw_move)] = (weight, learn)
        print(f"Loaded {len(existing_entries)} existing entries from performance.bin")
        shutil.copyfile("performance.bin", "performance_backup_758.bin")
        print("Backed up old performance.bin -> performance_backup_758.bin")

    # 2. Spin up Stockfish
    print("\nStarting Stockfish 16.1 engine...")
    engine = chess.engine.SimpleEngine.popen_uci(stockfish_path)
    engine.configure({"Threads": 4, "Hash": 128})

    MAX_PLY_DEPTH = 8 # 8 plies = 4 full moves from each opening position

    new_entries_count = 0

    for idx, (fen, name) in enumerate(TournamentOpenings):
        print(f"\n[{idx+1}/{len(TournamentOpenings)}] Expanding opening: {name}...")
        try:
            root_board = chess.Board(fen)
        except Exception as e:
            print(f"  Warning: Cannot parse FEN: {fen}")
            continue

        # BFS queue: (board, current_ply)
        queue = [(root_board, 0)]
        visited_fens = {root_board.fen()}

        while queue:
            b, ply = queue.pop(0)
            if ply >= MAX_PLY_DEPTH or b.is_game_over():
                continue

            key = chess.polyglot.zobrist_hash(b)

            # Analyze with Stockfish multipv=2
            try:
                # Fast depth 10 analysis (~30-50ms per position)
                analysis = engine.analyse(b, chess.engine.Limit(depth=10), multipv=2)
            except Exception as e:
                break

            moves_to_expand = []
            best_score = None

            for i, item in enumerate(analysis):
                pv = item.get("pv", [])
                if not pv:
                    continue
                move = pv[0]
                score_obj = item.get("score")
                score_cp = score_obj.relative.score(mate_score=10000) if score_obj else 0

                if best_score is None:
                    best_score = score_cp

                # Only include moves within 35 cp of best move
                if score_cp is not None and best_score is not None:
                    if (best_score - score_cp) > 35 and i > 0:
                        continue

                raw_move = encode_polyglot_move(move)
                weight = 100 if i == 0 else 60

                if (key, raw_move) not in existing_entries:
                    existing_entries[(key, raw_move)] = (weight, 0)
                    new_entries_count += 1

                moves_to_expand.append(move)

            # Push top moves to queue for expansion
            for move in moves_to_expand[:2]:
                next_b = b.copy()
                next_b.push(move)
                if next_b.fen() not in visited_fens:
                    visited_fens.add(next_b.fen())
                    queue.append((next_b, ply + 1))

    engine.quit()

    print(f"\nExpansion complete! Total entries: {len(existing_entries)} (+{new_entries_count} new entries)")

    # 3. Sort entries by key (ascending), then weight (descending)
    sorted_entries = sorted(
        existing_entries.items(),
        key=lambda item: (item[0][0], -item[1][0])
    )

    # 4. Serialize to PolyGlot binary format (>QHHI)
    out_path = "performance.bin"
    with open(out_path, "wb") as f:
        for (key, raw_move), (weight, learn) in sorted_entries:
            f.write(struct.pack(">QHHI", key, raw_move, weight, learn))

    print(f"Saved {len(sorted_entries)} entries to {out_path} ({os.path.getsize(out_path)} bytes)")

if __name__ == "__main__":
    main()
