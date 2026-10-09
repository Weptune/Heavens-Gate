"""Verify a completed paired bank pilot without converting clock wins to Elo.

Reads only: python tools/verify_pilot.py path/to/pilot.pgn
"""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import re
import statistics

import chess


def sha256(path):
    with Path(path).open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def rounding_error(value):
    # Frozen match telemetry uses C++ defaultfloat's six significant digits.
    return .5 * 10 ** (math.floor(math.log10(abs(value))) - 5) if value else 0.0


def verify(path):
    manifest = json.loads(Path(str(path) + ".manifest.json").read_text())
    rows = [json.loads(line) for line in Path(str(path) + ".moves.jsonl").read_text().splitlines()]
    blocks = [block for block in re.split(r'(?=^\[Event )', path.read_text(), flags=re.MULTILINE) if block.strip()]
    assert len(blocks) == manifest["games"], "Pilot is incomplete"
    assert manifest["paired_openings"] and len(blocks) % 2 == 0
    assert manifest["bank_ms"] > 0 and manifest["adjudication"] == "board-terminal-or-rule-draw-only"
    assert manifest["engine_sha256"] == sha256(manifest["engine_path"])
    assert manifest["opponent_sha256"] == sha256(manifest["opponent_path"])
    assert {row["game"] for row in rows} <= set(range(1, len(blocks) + 1))
    endings, flags, results = Counter(), Counter(), Counter()
    late_hg, games, attempts, played_total = [], [], 0, 0
    for game, block in enumerate(blocks, 1):
        tags = dict(re.findall(r'^\[(\w+) "([^"]*)"\]$', block, re.MULTILINE))
        assert int(tags["Round"]) == game
        initial = tags["FEN"]
        assert initial == manifest["openings"][((game - 1) // 2) % len(manifest["openings"])]
        hg_white = game % 2 == 1
        assert (tags["White"] == "Master Edition") == hg_white
        assert (tags["Black"] == "Master Edition") != hg_white
        board = chess.Board(initial)
        text = re.sub(r'\{[^}]*\}|^\[.*\]$', '', block, flags=re.MULTILINE)
        moves = re.findall(r'\b[a-h][1-8][a-h][1-8][qrbn]?\b', text)
        ending = re.search(r'(1-0|0-1|1/2-1/2|\*)\s+\{([^}]+)\}\s*$', block)
        assert ending and ending.group(1) == tags["Result"] != "*"
        reason = ending.group(2)
        game_rows = [row for row in rows if row["game"] == game]
        assert len(moves) <= len(game_rows) <= len(moves) + 1
        clocks = {chess.WHITE: float(manifest["bank_ms"]), chess.BLACK: float(manifest["bank_ms"])}
        clock_errors = {chess.WHITE: 0.0, chess.BLACK: 0.0}
        history = []
        flagged = None
        for ply, row in enumerate(game_rows, 1):
            attempts += 1
            assert row["ply"] == ply
            assert row["fen"] == board.fen(en_passant="fen")
            assert row["position"] == "position fen " + initial + (" moves " + " ".join(history) if history else "")
            hg = board.turn == hg_white
            assert (row["engine"] == "HG") == hg
            assert abs(row["clock_before_ms"] - clocks[board.turn]) <= (
                clock_errors[board.turn] + rounding_error(row["clock_before_ms"]) + 1e-6)
            command = row["go"].split()
            assert command[0] == "go" and len(command) == 9
            values = dict(zip(command[1::2], map(int, command[2::2])))
            assert set(values) == {"wtime", "btime", "winc", "binc"}
            assert abs(values["wtime"] - max(1, int(clocks[chess.WHITE]))) <= math.ceil(clock_errors[chess.WHITE]) + 1
            assert abs(values["btime"] - max(1, int(clocks[chess.BLACK]))) <= math.ceil(clock_errors[chess.BLACK]) + 1
            assert values["winc"] == values["binc"] == manifest["increment_ms"]
            assert row["elapsed_ms"] >= 0
            if not row["score_valid"]:
                assert row["score_stm"] is None
            if hg and board.fullmove_number >= 100:
                late_hg.append(row)
            remaining = row["clock_before_ms"] - row["elapsed_ms"]
            if ply > len(moves):
                assert reason == "Time Out" and ply == len(game_rows)
                assert remaining <= 0 and row["status"] in ("ok", "protocol-timeout")
                assert tags["Result"] == ("0-1" if board.turn else "1-0")
                flagged = "HG" if hg else "opponent"
                flags[flagged] += 1
                continue  # An expired-clock reply must never be played.
            assert row["status"] == "ok" and remaining > 0
            assert row["move"] == moves[ply - 1]
            move = chess.Move.from_uci(row["move"])
            assert move in board.legal_moves
            clocks[board.turn] = remaining + manifest["increment_ms"]
            clock_errors[board.turn] = rounding_error(row["clock_before_ms"]) + rounding_error(row["elapsed_ms"])
            board.push(move)
            history.append(row["move"])
            played_total += 1
        if reason == "Time Out":
            assert flagged, "Clock forfeits need an expired-clock telemetry row"
        else:
            assert len(game_rows) == len(moves), "Unexpected unplayed reply"
            actual = board.outcome(claim_draw=True)
            assert actual and actual.result() == tags["Result"], "Result is not board-terminal"
            checks = {"Checkmate": board.is_checkmate, "Stalemate": board.is_stalemate,
                      "Threefold Repetition": lambda: board.is_repetition(3),
                      "50-Move Rule": board.is_fifty_moves,
                      "Insufficient Material": board.is_insufficient_material}
            assert reason in checks and checks[reason](), "Termination reason mismatch"
        endings[reason] += 1
        result = "draw" if tags["Result"] == "1/2-1/2" else (
            "HG_win" if (tags["Result"] == "1-0") == hg_white else "HG_loss")
        results[result] += 1
        games.append({"game": game, "hg_white": hg_white, "initial_fen": initial,
                      "result": tags["Result"], "termination": reason, "flagged": flagged,
                      "played_plies": len(moves), "final_fen": board.fen(en_passant="fen")})
    late = {"searched_moves": len(late_hg)}
    if late_hg:
        elapsed = sorted(row["elapsed_ms"] for row in late_hg)
        late.update(minimum_bank_ms=min(row["clock_before_ms"] for row in late_hg),
                    median_elapsed_ms=statistics.median(elapsed),
                    p95_elapsed_ms=elapsed[min(len(elapsed) - 1, int(.95 * len(elapsed)))],
                    maximum_elapsed_ms=max(elapsed),
                    completed_depth_counts=dict(Counter(row["completed_depth"] for row in late_hg)),
                    thread_counts=dict(Counter(row["threads_used"] for row in late_hg)))
    return {"games_verified": len(games), "paired_fens_and_colors": "passed",
            "attempts_verified": attempts, "legal_played_plies": played_total,
            "clock_and_history_contract": "passed", "terminations": dict(endings),
            "results_including_clock_forfeits": dict(results), "time_forfeits": dict(flags),
            "hg_move_100_plus": late, "games": games,
            "note": "A completed pilot and opponent clock wins do not establish CCRL Elo."}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pgn", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.pgn), indent=2))
