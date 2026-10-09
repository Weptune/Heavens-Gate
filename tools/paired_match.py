"""Small fail-closed paired HG-vs-HG UCI runner for search tuning.

Actual games, full position histories, physical banks, and no score adjudication.
Never use concurrently with a timed validation pilot.
"""
from contextlib import ExitStack
import argparse
import json
import math
from pathlib import Path
import time

import chess
import chess.engine
import chess.pgn
from extract_quiet_dataset import file_hash


class InvalidMatch(RuntimeError):
    pass


def actual_outcome(board):
    result = board.outcome(claim_draw=False)
    if result is None and board.is_repetition(3):
        result = chess.Outcome(chess.Termination.THREEFOLD_REPETITION, None)
    if result is None and board.is_fifty_moves():
        result = chess.Outcome(chess.Termination.FIFTY_MOVES, None)
    return result


def outcome_score(outcome, candidate_white):
    return .5 if outcome.winner is None else float(outcome.winner == candidate_white)


def paired_cases(fens):
    return [(fen, color) for fen in fens for color in (chess.WHITE, chess.BLACK)]


def configure_engine(engine, options, threads, hash_mb):
    if any(name.casefold() in {"threads", "hash", "ownbook"} for name in options):
        raise InvalidMatch("Candidate settings cannot override match threads/hash/book policy")
    required = {"Threads": threads, "Hash": hash_mb, "OwnBook": False, **options}
    for name in required:
        if name not in engine.options: raise InvalidMatch(f"Engine lacks required option {name}")
    engine.configure(required)


def run_paired_batch(candidate, control, candidate_options, fens, output_prefix, *, bank_ms=10000,
                     increment_ms=100, threads=1, hash_mb=64, max_plies=400):
    candidate, control = Path(candidate).resolve(), Path(control).resolve()
    if not math.isfinite(bank_ms) or not math.isfinite(increment_ms) or bank_ms <= 0 or increment_ms < 0 or threads < 1 or hash_mb < 1 or max_plies < 1 or not fens:
        raise ValueError("Invalid paired-match configuration")
    for fen in fens:
        if not chess.Board(fen).is_valid(): raise ValueError("Invalid opening FEN")
    prefix = Path(output_prefix)
    outputs = {"pgn": Path(str(prefix) + ".pgn"), "moves": Path(str(prefix) + ".moves.jsonl"),
               "manifest": Path(str(prefix) + ".manifest.json")}
    if any(path.exists() for path in outputs.values()): raise FileExistsError("Preserve existing match outputs")
    prefix.parent.mkdir(parents=True, exist_ok=True)
    manifest = {"schema": 1, "candidate": str(candidate), "control": str(control),
        "candidate_sha256": file_hash(candidate), "control_sha256": file_hash(control),
        "candidate_options": candidate_options, "control_options": "binary defaults",
        "bank_ms": bank_ms, "increment_ms": increment_ms, "threads_each": threads,
        "hash_mb_each": hash_mb, "book_enabled": False, "paired": True, "fens": fens,
        "adjudication": "played-board-or-rule-only; any clock/protocol failure invalidates fitness"}
    with outputs["manifest"].open("x") as output:
        json.dump(manifest, output, indent=2)
    scores, games = [], []
    with outputs["pgn"].open("x") as pgn, outputs["moves"].open("x") as moves, ExitStack() as stack:
        engines = []
        for binary in (candidate, control):
            engine = chess.engine.SimpleEngine.popen_uci([str(binary), "uci"], timeout=30)
            stack.callback(engine.close)
            engines.append(engine)
        configure_engine(engines[0], candidate_options, threads, hash_mb)
        configure_engine(engines[1], {}, threads, hash_mb)
        for game_number, (fen, candidate_white) in enumerate(paired_cases(fens), 1):
            board = chess.Board(fen)
            clocks = {chess.WHITE: float(bank_ms), chess.BLACK: float(bank_ms)}
            game_token = object()
            failure = None
            for ply in range(1, max_plies + 1):
                if actual_outcome(board): break
                candidate_turn = board.turn == candidate_white
                engine = engines[0 if candidate_turn else 1]
                before = clocks[board.turn]
                row = {"game": game_number, "ply": ply, "fen": board.fen(en_passant="fen"),
                       "engine": "candidate" if candidate_turn else "control", "clock_before_ms": before,
                       "requested_clocks_ms": {"white": clocks[chess.WHITE], "black": clocks[chess.BLACK]},
                       "increment_ms": increment_ms, "move": None, "status": "ok", "played": False}
                started = time.perf_counter()
                try:
                    engine.timeout = max(.001, before / 1000.0)
                    reply = engine.play(board, chess.engine.Limit(white_clock=clocks[chess.WHITE] / 1000.0,
                        black_clock=clocks[chess.BLACK] / 1000.0, white_inc=increment_ms / 1000.0,
                        black_inc=increment_ms / 1000.0), game=game_token, info=chess.engine.INFO_BASIC)
                    row["move"] = reply.move.uci() if reply.move else None
                    row["nodes"], row["depth"] = reply.info.get("nodes"), reply.info.get("depth")
                except (chess.engine.EngineError, TimeoutError, OSError) as error:
                    row["status"], row["diagnostic"] = "protocol-failure", str(error)
                    failure = row["diagnostic"]
                row["elapsed_ms"] = (time.perf_counter() - started) * 1000.0
                remaining = before - row["elapsed_ms"]
                if remaining <= 0:
                    row["status"] = "time-forfeit"
                    failure = "Physical clock expired"
                elif failure is None:
                    if not reply.move or reply.move not in board.legal_moves:
                        row["status"] = "illegal-move"
                        failure = "Illegal or empty move"
                    else:
                        clocks[board.turn] = remaining + increment_ms
                        board.push(reply.move)
                        row["played"] = True
                moves.write(json.dumps(row) + "\n")
                moves.flush()
                if failure: break
            outcome = actual_outcome(board)
            if not failure and outcome is None: failure = "Unfinished: move limit"
            game = chess.pgn.Game.from_board(board)
            game.headers.update({"Event": "HG paired tuning (not a CCRL rating)", "Round": str(game_number),
                "White": "candidate" if candidate_white else "control",
                "Black": "control" if candidate_white else "candidate",
                "Result": outcome.result() if outcome and not failure else "*",
                "Termination": failure or outcome.termination.name})
            pgn.write(str(game) + "\n\n")
            pgn.flush()
            if failure: raise InvalidMatch(f"Game {game_number}: {failure}; all outputs preserved")
            score = outcome_score(outcome, candidate_white)
            scores.append(score)
            games.append({"game": game_number, "fen": fen, "candidate_white": candidate_white,
                          "result": outcome.result(), "candidate_points": score,
                          "played_plies": len(board.move_stack), "termination": outcome.termination.name})
    return {"score": sum(scores) / len(scores), "games": games, "time_forfeits": 0, "protocol_failures": 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--control", type=Path, required=True)
    parser.add_argument("--openings", type=Path, required=True, help="One complete FEN per line")
    parser.add_argument("--settings", type=Path, help="Candidate JSON containing uci_options, or an option object")
    parser.add_argument("--output-prefix", type=Path, required=True)
    parser.add_argument("--bank-ms", type=int, default=10000)
    parser.add_argument("--increment-ms", type=int, default=100)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--hash", type=int, default=64)
    parser.add_argument("--max-plies", type=int, default=400)
    args = parser.parse_args()
    summary = Path(str(args.output_prefix) + ".summary.json")
    if summary.exists(): raise FileExistsError("Preserve existing match summary")
    options = json.loads(args.settings.read_text()) if args.settings else {}
    if not isinstance(options, dict): raise ValueError("Candidate settings must be an option object")
    options = options.get("uci_options", options)
    if not isinstance(options, dict): raise ValueError("Candidate settings must be an option object")
    fens = [line.strip() for line in args.openings.read_text().splitlines() if line.strip()]
    result = run_paired_batch(args.candidate, args.control, options, fens, args.output_prefix,
        bank_ms=args.bank_ms, increment_ms=args.increment_ms, threads=args.threads,
        hash_mb=args.hash, max_plies=args.max_plies)
    with summary.open("x") as output: json.dump(result, output, indent=2)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
