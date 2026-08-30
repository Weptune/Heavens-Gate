#!/usr/bin/env python3
"""
Tactical Benchmark Runner for Heaven's Gate Chess Engine.
Runs the Bratko-Kopec Test (BK-24) and Win-At-Chess (WAC) tactical benchmark suites.
"""

import os
import sys
import time
import subprocess

ENGINE_PATH = os.path.join(os.path.dirname(__file__), "..", "heavensgate.exe")
if not os.path.exists(ENGINE_PATH):
    ENGINE_PATH = "heavensgate.exe"

BK_TEST_SUITE = [
    {"id": "BK.01", "fen": "1k1r4/pp1b1R2/3q1p2/8/1P1B4/6P1/P1P4P/2K2B2 w - - 0 1", "solutions": ["d4f6", "f7f6"], "theme": "Tactical Strike"},
    {"id": "BK.02", "fen": "3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - 0 1", "solutions": ["d4d5", "e4e5"], "theme": "Positional Pawn Break"},
    {"id": "BK.03", "fen": "2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - 0 1", "solutions": ["f6f5"], "theme": "Kingside Breakthrough"},
    {"id": "BK.04", "fen": "rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - 0 1", "solutions": ["e5e6"], "theme": "Tactical Pawn Sac"},
    {"id": "BK.05", "fen": "r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - 0 1", "solutions": ["c3d5", "a2a4"], "theme": "Knight Outpost"},
    {"id": "BK.06", "fen": "2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - 0 1", "solutions": ["g5g6"], "theme": "Pawn Undermining"},
    {"id": "BK.07", "fen": "1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P1R3/2P1B1PP/R2Q2K1 w - - 0 1", "solutions": ["h5f6", "a3b4"], "theme": "Knight Infiltration"},
    {"id": "BK.08", "fen": "4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P1B4P/8 w - - 0 1", "solutions": ["f4f5"], "theme": "Endgame Breakthrough"},
    {"id": "BK.09", "fen": "2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - 0 1", "solutions": ["f4f5"], "theme": "Center Lever"},
    {"id": "BK.10", "fen": "3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - 0 1", "solutions": ["c6e5", "f6d5"], "theme": "Central Outpost"},
    {"id": "BK.11", "fen": "2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - 0 1", "solutions": ["f2f4"], "theme": "Flank Attack"},
    {"id": "BK.12", "fen": "r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - 0 1", "solutions": ["d7f5"], "theme": "Defensive Accuracy"},
    {"id": "BK.13", "fen": "r2q1rk1/4bppp/p2p4/2pPn3/3pP3/1B4QP/PP3PP1/R1B1R1K1 b - - 0 1", "solutions": ["e7h4"], "theme": "Piece Trapping"},
    {"id": "BK.14", "fen": "r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/2N1BN2/1PP1B1PP/R2Q1RK1 b - - 0 1", "solutions": ["g7c3", "c5a4"], "theme": "Positional Exchange"},
    {"id": "BK.15", "fen": "r1b1qr1k/6pp/p2p1b2/8/p3PP2/P2BB1N1/1PP3QP/R5K1 w - - 0 1", "solutions": ["e4e5", "a1e1"], "theme": "Discovered Attack"},
    {"id": "BK.16", "fen": "r1b3k1/4q1pp/2p1pr2/p1pp4/N1P5/1P1P1R2/P4PPP/R2Q2K1 b - - 0 1", "solutions": ["e6e5", "f6f3"], "theme": "Center Control"},
    {"id": "BK.17", "fen": "r1b2rk1/pp2ppbp/3p1np1/q7/2P1P3/2N1B3/PP1QBPPP/R4RK1 b - - 0 1", "solutions": ["c8e6", "f8e8"], "theme": "Development & Pressure"},
    {"id": "BK.18", "fen": "r1b1k2r/pp1p1ppp/2n1p3/8/1bP1n3/2N1PN2/PP1B1PPP/R3KB1R b KQkq - 0 1", "solutions": ["e4d2", "b4c3"], "theme": "Bishop Pair Liquidation"},
    {"id": "BK.19", "fen": "r1b1kb1r/1p1n1ppp/p2p4/3Ppn2/8/2N2N2/PPP1BPPP/R1B1K2R b KQkq - 0 1", "solutions": ["f8e7", "d7c5"], "theme": "Endgame Coordination"},
    {"id": "BK.20", "fen": "8/p2p1pk1/3p2p1/8/8/1P5P/1PP2PPK/8 b - - 0 1", "solutions": ["g7f6"], "theme": "King Activity"},
    {"id": "BK.21", "fen": "8/pkp5/1p6/8/8/P1P5/1K4p1/8 b - - 0 1", "solutions": ["g2g1q", "g2g1r"], "theme": "Pawn Promotion"},
    {"id": "BK.22", "fen": "8/8/pk4p1/4p2p/2P1P2P/3K2P1/8/8 b - - 0 1", "solutions": ["b6c5"], "theme": "Opposition & King Key Squares"},
    {"id": "BK.23", "fen": "8/8/1p6/5pr1/8/4R3/8/k1K5 w - - 0 1", "solutions": ["e3a3"], "theme": "Forced Checkmate (Mate in 1)"},
    {"id": "BK.24", "fen": "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP3/1N3N1P/PP2QPP1/1K1R4 b - - 0 1", "solutions": ["e6b3", "g6e4"], "theme": "Complex Tactical Combination"}
]

WAC_TEST_SUITE = [
    {"id": "WAC.001", "fen": "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1", "solutions": ["g3g6"], "theme": "Queen Sac Mating Net"},
    {"id": "WAC.002", "fen": "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1", "solutions": ["b3b2", "d3d2"], "theme": "Rook Sac Passed Pawn Promotion"},
    {"id": "WAC.003", "fen": "5rk1/1ppb3p/p1pb4/6q1/3P1p1r/2P1R2P/PP1BQ1P1/5RKN w - - 0 1", "solutions": ["e3g3"], "theme": "Pin Exploitation"},
    {"id": "WAC.004", "fen": "r1b1kb1r/7p/1qnp4/3Npp2/4p3/N5P1/PP3P1P/R2QKB1R b KQkq - 0 1", "solutions": ["b6a5", "b6b2"], "theme": "Queen Outpost & Fork"},
    {"id": "WAC.005", "fen": "r1b2rk1/pp1p1pp1/1b1p2B1/n1q3N1/8/8/P4PPP/R2QR1K1 w - - 0 1", "solutions": ["g6f7", "d1h5"], "theme": "Smothered/King Hunt"},
    {"id": "WAC.006", "fen": "r1bqk2r/pppp1ppp/2n5/1B2p3/3Pn3/2P2N2/P1P2PPP/R1BQK2R w KQkq - 0 1", "solutions": ["d1e2", "b5c6"], "theme": "Central Counterattack"},
    {"id": "WAC.007", "fen": "r1b1k2r/ppppqppp/8/4n3/1bP2B2/4P3/PP1NBPPP/R2QK2R b KQkq - 0 1", "solutions": ["e5c4", "e5d3"], "theme": "Pin Overload"},
    {"id": "WAC.008", "fen": "r2q1rk1/1b2bppp/p2p1n2/1p2p3/4P3/1PNB4/P1P1QPPP/R1B2RK1 w - - 0 1", "solutions": ["c3d5", "a2a4"], "theme": "Positional Center Dominance"},
    {"id": "WAC.009", "fen": "r1b2rk1/ppqn1ppp/2pbpn2/8/2PP4/2NB1N2/PP1B1PPP/R2Q1RK1 w - - 0 1", "solutions": ["c4c5", "d1e2"], "theme": "Space Gain & Pawn Tension"},
    {"id": "WAC.010", "fen": "r1bq1rk1/pp3ppp/2n1pn2/2pp4/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQ - 0 1", "solutions": ["c4d5", "e1g1"], "theme": "Pawn Structure Liquidation"},
]

def run_uci_benchmark(positions, suite_name, movetime_ms=1000):
    print(f"\n" + "="*70)
    print(f"  RUNNING BENCHMARK: {suite_name} ({len(positions)} positions @ {movetime_ms}ms/pos)")
    print("="*70)

    try:
        proc = subprocess.Popen(
            [ENGINE_PATH, "uci"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
    except Exception as e:
        print(f"[ERROR] Could not start engine at {ENGINE_PATH}: {e}")
        return 0, len(positions)

    proc.stdin.write("uci\n")
    proc.stdin.flush()
    while True:
        line = proc.stdout.readline()
        if "uciok" in line:
            break

    proc.stdin.write("isready\n")
    proc.stdin.flush()
    while True:
        line = proc.stdout.readline()
        if "readyok" in line:
            break

    solved_count = 0
    total_nodes = 0
    total_time = 0.0

    print(f"{'ID':<8} {'Theme':<25} {'Best Move':<10} {'Solution':<12} {'Result':<8} {'Depth':<6} {'Nodes':<10}")
    print("-" * 80)

    for p in positions:
        pid = p["id"]
        fen = p["fen"]
        sols = p["solutions"]
        theme = p.get("theme", "Tactical")

        proc.stdin.write(f"position fen {fen}\n")
        proc.stdin.write(f"go movetime {movetime_ms}\n")
        proc.stdin.flush()

        best_move = ""
        depth = 0
        nodes = 0

        t0 = time.time()
        while True:
            line = proc.stdout.readline()
            if not line:
                break
            line = line.strip()
            if line.startswith("info"):
                parts = line.split()
                for i, pt in enumerate(parts):
                    if pt == "depth" and i + 1 < len(parts):
                        try: depth = int(parts[i+1])
                        except: pass
                    if pt == "nodes" and i + 1 < len(parts):
                        try: nodes = int(parts[i+1])
                        except: pass
            if line.startswith("bestmove"):
                best_move = line.split()[1]
                break
        elapsed = time.time() - t0
        total_time += elapsed
        total_nodes += nodes

        is_solved = (best_move in sols)
        if is_solved:
            solved_count += 1
            res_str = "[PASS]"
        else:
            res_str = "[FAIL]"

        sol_str = sols[0] + (f" (+{len(sols)-1})" if len(sols) > 1 else "")
        print(f"{pid:<8} {theme[:24]:<25} {best_move:<10} {sol_str:<12} {res_str:<8} {depth:<6} {nodes:<10}")

    proc.stdin.write("quit\n")
    proc.stdin.flush()
    try: proc.wait(timeout=1.0)
    except: proc.terminate()

    pct = (solved_count / len(positions)) * 100.0
    nps = (total_nodes / total_time) / 1e6 if total_time > 0 else 0

    print("-" * 80)
    print(f"Summary for {suite_name}:")
    print(f"  Solved     : {solved_count} / {len(positions)} ({pct:.1f}%)")
    print(f"  Total Nodes: {total_nodes:,} ({nps:.2f}M NPS)")
    print(f"  Total Time : {total_time:.2f} seconds")

    return solved_count, len(positions)

if __name__ == "__main__":
    t_ms = 1000
    if len(sys.argv) > 1:
        try: t_ms = int(sys.argv[1])
        except: pass

    print("======================================================================")
    print("  HEAVEN'S GATE CHESS ENGINE -- TACTICAL BENCHMARK SUITE")
    print("======================================================================")

    bk_solved, bk_total = run_uci_benchmark(BK_TEST_SUITE, "Bratko-Kopec Test (BK-24)", movetime_ms=t_ms)
    wac_solved, wac_total = run_uci_benchmark(WAC_TEST_SUITE, "Win At Chess (WAC Key Set)", movetime_ms=t_ms)

    total_solved = bk_solved + wac_solved
    total_pos = bk_total + wac_total
    overall_pct = (total_solved / total_pos) * 100.0

    bk_pct = bk_solved / bk_total
    estimated_tactical_elo = int(1400 + bk_pct * 1400)

    print("\n" + "="*70)
    print("  OVERALL TACTICAL BENCHMARK RESULTS")
    print("="*70)
    print(f"  Bratko-Kopec (BK-24)  : {bk_solved}/{bk_total} ({bk_pct*100:.1f}%)")
    print(f"  Win At Chess (WAC-10) : {wac_solved}/{wac_total} ({(wac_solved/wac_total)*100:.1f}%)")
    print(f"  Combined Tactical Accuracy : {total_solved}/{total_pos} ({overall_pct:.1f}%)")
    print(f"  Estimated Tactical Rating  : ~{estimated_tactical_elo} Elo")
    print("="*70)
