"""Bounded serial TT clock/context diagnostics. No games, tuning or Elo estimate."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import time

import chess
import chess.engine
import chess.pgn
from diagnose_nmp_candidate import CASES


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def positions(pgn_path):
    result = []
    for name, fen in CASES:
        for mirrored in (False, True):
            board = chess.Board(fen)
            if mirrored: board = board.mirror()
            result.append((name + ("-mirror" if mirrored else ""), board))
    if pgn_path:
        games = []
        with Path(pgn_path).open() as source:
            while (game := chess.pgn.read_game(source)) is not None:
                if game.errors: raise ValueError("Malformed preserved PGN")
                games.append(game)
        if len(games) != 60: raise ValueError("Expected preserved complete 60-game screen")
        # Predetermined diagnostic sample, never selected by search results.
        for pair in (1, 10, 20):
            result.append((f"pilot-pair-{pair}-initial", games[2 * (pair - 1)].board()))
        for number in (10, 30, 50):
            game = games[number - 1]
            moves = list(game.mainline_moves())
            count = min(60, len(moves) - 2)
            if count < 1: raise ValueError("Too short for preserved-history sample")
            board = game.board()
            for move in moves[:count]: board.push(move)
            if board.is_game_over(claim_draw=True): raise ValueError("Diagnostic sample is terminal/claimable")
            result.append((f"pilot-game-{number}-after-{count}-plies", board))
    return result


def run(binary, board, depth, contexts=None):
    with chess.engine.SimpleEngine.popen_uci([str(binary), "uci"], timeout=60) as engine:
        options = {"Threads": 1, "Hash": 64, "OwnBook": False, "NMPGuards": False}
        if contexts is not None:
            if "TTClockContexts" in engine.options: options["TTClockContexts"] = contexts
            elif contexts: raise ValueError("Candidate lacks experimental context option")
        engine.configure(options)
        began = time.perf_counter()
        info = engine.analyse(board, chess.engine.Limit(depth=depth), info=chess.engine.INFO_ALL)
        elapsed = time.perf_counter() - began
        if info.get("depth") != depth: raise RuntimeError("Incomplete matched depth")
        pv = info.get("pv", [])
        if not pv: raise RuntimeError("Missing PV")
        walk = board.copy()
        for move in pv:
            if move not in walk.legal_moves: raise RuntimeError("Illegal PV")
            walk.push(move)
        data = {"depth": info["depth"], "score_stm": info["score"].pov(board.turn).score(mate_score=25000),
                "nodes": info["nodes"], "pv": [move.uci() for move in pv], "elapsed_seconds": elapsed}
        text = info.get("string", "")
        if text.startswith("tt_audit "):
            counters = json.loads(text[len("tt_audit "):])
            if counters["hits"] != counters["compatible_hits"] + counters["rejected_clock_hits"]:
                raise RuntimeError("Counter partition is inconsistent")
            if sum(counters["rejected_clock_deciles"]) != counters["rejected_clock_hits"]:
                raise RuntimeError("Counter clock histogram is inconsistent")
            if not 0 <= counters["score_cutoffs"] <= counters["compatible_hits"] <= counters["hits"] <= counters["probes"] <= data["nodes"]:
                raise RuntimeError("Invalid counter ordering")
            if not 0 <= counters["qsearch_context_replacements_of_deeper"] <= counters["shallower_context_replacements"] <= counters["context_replacements"] <= counters["stored"] <= counters["store_attempts"]:
                raise RuntimeError("Invalid replacement counters")
            data["tt_audit"] = counters
        return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--reference", type=Path)
    parser.add_argument("--pilot-pgn", type=Path)
    parser.add_argument("--depth", type=int, default=10)
    parser.add_argument("--contexts", action="store_true")
    parser.add_argument("--require-parity", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists(): raise FileExistsError("Preserve earlier diagnostic")
    if not 1 <= args.depth <= 12: raise ValueError("Bounded depths only")
    args.binary = args.binary.resolve()
    if args.reference: args.reference = args.reference.resolve()
    evidence = {"schema": 1, "utc": datetime.now(timezone.utc).isoformat(),
                "binary": str(args.binary), "binary_sha256": digest(args.binary),
                "reference": str(args.reference) if args.reference else None,
                "reference_sha256": digest(args.reference) if args.reference else None,
                "pilot_pgn_sha256": digest(args.pilot_pgn) if args.pilot_pgn else None,
                "depth": args.depth, "threads": 1, "hash_mb": 64, "book": False,
                "contexts": args.contexts, "purpose": "matched-position cache diagnostics, NOT strength or throughput validation", "rows": []}
    try:
        for name, board in positions(args.pilot_pgn):
            row = {"name": name, "fen": board.fen(en_passant="fen"),
                   "initial_fen": board.root().fen(en_passant="fen"), "history": [move.uci() for move in board.move_stack]}
            if args.reference: row["reference"] = run(args.reference, board, args.depth, False)
            row["measured"] = run(args.binary, board, args.depth, args.contexts)
            if "tt_audit" not in row["measured"]: raise RuntimeError("Missing diagnostic output")
            if args.reference:
                row["score_pv_nodes_equal"] = all(row["reference"][key] == row["measured"][key] for key in ("score_stm", "pv", "nodes"))
                if args.require_parity and not row["score_pv_nodes_equal"]: raise RuntimeError("Instrumentation/default-off parity failure")
            evidence["rows"].append(row)
            count = row["measured"]["tt_audit"]
            print(name, "nodes", row["measured"]["nodes"], "mismatches", count["rejected_clock_hits"],
                  "shallower context evictions", count["shallower_context_replacements"],
                  "parity", row.get("score_pv_nodes_equal"), flush=True)
        if digest(args.binary) != evidence["binary_sha256"]: raise RuntimeError("Binary changed")
        if args.reference and digest(args.reference) != evidence["reference_sha256"]: raise RuntimeError("Reference changed")
        aggregate = {}
        for row in evidence["rows"]:
            for key, value in row["measured"]["tt_audit"].items():
                if isinstance(value, list):
                    aggregate.setdefault(key, [0] * len(value))
                    aggregate[key] = [left + right for left, right in zip(aggregate[key], value)]
                else: aggregate[key] = aggregate.get(key, 0) + value
        evidence.update(status="passed; strength unmeasured", aggregate=aggregate)
    except BaseException as error:
        evidence.update(status="failed", error=repr(error))
        raise
    finally:
        with args.output.open("x") as output: json.dump(evidence, output, indent=2)


if __name__ == "__main__":
    main()
