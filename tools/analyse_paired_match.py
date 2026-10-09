"""Verify HG paired-match artifacts and report descriptive pentanomial statistics.

Never launches engines. Conditional intervals are NOT an SPRT or an acceptance
decision: independent opening pairs and a predeclared fixed stopping rule must
be established externally. No inference is made about an absolute CCRL rating.
"""
import argparse
import json
import math
from pathlib import Path
from statistics import NormalDist

import chess
import chess.pgn
from extract_quiet_dataset import file_hash
from paired_match import InvalidMatch, actual_outcome, outcome_score


def require(condition, message):
    if not condition:
        raise InvalidMatch(message)


class StrictGameBuilder(chess.pgn.GameBuilder):
    def begin_game(self):
        super().begin_game()
        self.seen_headers = set()
        self.explicit_result = self.body_result = None

    def visit_header(self, tagname, tagvalue):
        require(tagname not in self.seen_headers, "Duplicate PGN header: " + tagname)
        self.seen_headers.add(tagname)
        if tagname == "Result": self.explicit_result = tagvalue
        super().visit_header(tagname, tagvalue)

    def visit_result(self, result):
        require(result == self.explicit_result, "PGN header/movetext result mismatch")
        self.body_result = result
        super().visit_result(result)

    def end_game(self):
        require(self.body_result is not None, "Missing PGN movetext result")
        super().end_game()


def logistic_elo(score):
    # Saturation has no finite estimate; do not replace infinity by a magic cap.
    return 400 * math.log10(score / (1 - score)) if 0 < score < 1 else None


def pair_statistics(bins, confidence=.95):
    require(len(bins) == 5 and all(type(n) is int and n >= 0 for n in bins), "Invalid pentanomial counts")
    count = sum(bins)
    require(count > 0 and 0 < confidence < 1, "Empty pairs or invalid confidence")
    # A single observation is the average score of BOTH colors, not one game.
    mean = sum(n * i / 4 for i, n in enumerate(bins)) / count
    variance = sum(n * (i / 4 - mean) ** 2 for i, n in enumerate(bins)) / (count - 1) if count > 1 else None
    se = math.sqrt(variance / count) if variance is not None else None
    normal = None
    if count >= 30 and variance > 0:
        margin = NormalDist().inv_cdf((1 + confidence) / 2) * se
        normal = [max(0, mean - margin), min(1, mean + margin)]
    # Bounded independent pairs: conservative, finite-sample Hoeffding interval.
    # It remains nonzero-width even when every observed pair has the same score.
    margin = math.sqrt(math.log(2 / (1 - confidence)) / (2 * count))
    bounded = [max(0, mean - margin), min(1, mean + margin)]
    return {"pairs": count, "games": 2 * count, "pair_points_bins": [0, .5, 1, 1.5, 2],
            "pentanomial": bins, "candidate_score": mean,
            "logistic_elo_difference_descriptive": logistic_elo(mean),
            "pair_sample_variance": variance, "pair_standard_error": se,
            "conditional_intervals": {"confidence": confidence,
                "assumptions": "independent representative opening pairs; fixed stopping rule declared before play; not verified by these artifacts",
                "normal_score_interval": normal,
                "normal_elo_interval": [logistic_elo(p) for p in normal] if normal else None,
                "hoeffding_score_interval": bounded,
                "hoeffding_elo_interval": [logistic_elo(p) for p in bounded],
                "null_endpoint_means": "unbounded; not zero or a finite Elo cap"},
            "decision": "descriptive only; no automatic promotion, SPRT decision or CCRL rating"}


def analyse(prefix):
    paths = {name: Path(str(prefix) + suffix) for name, suffix in
             (("manifest", ".manifest.json"), ("summary", ".summary.json"),
              ("pgn", ".pgn"), ("moves", ".moves.jsonl"))}
    require(all(path.is_file() for path in paths.values()), "Missing/incomplete paired-match artifacts")
    initial_hashes = {name: file_hash(path) for name, path in paths.items()}
    manifest = json.loads(paths["manifest"].read_text())
    summary = json.loads(paths["summary"].read_text())
    require(manifest.get("schema") == 1 and manifest.get("paired") is True and
            manifest.get("book_enabled") is False, "Unsupported or unpaired match policy")
    require(manifest.get("adjudication") == "played-board-or-rule-only; any clock/protocol failure invalidates fitness",
            "Unsupported adjudication")
    require(summary.get("time_forfeits") == 0 and summary.get("protocol_failures") == 0, "Failed clock/protocol gate")
    require(isinstance(manifest.get("candidate_options"), dict) and manifest.get("control_options") == "binary defaults",
            "Missing engine option identity")
    for role in ("candidate", "control"):
        require(file_hash(Path(manifest[role])) == manifest[role + "_sha256"], "Binary hash mismatch: " + role)
    for key in ("bank_ms", "increment_ms", "threads_each", "hash_mb_each"):
        value = manifest[key]
        require(type(value) in (float, int) and math.isfinite(value) and
                (value >= 0 if key == "increment_ms" else value > 0), "Invalid match setting: " + key)
    fens = manifest["fens"]
    require(isinstance(fens, list) and bool(fens), "No opening pairs")
    openings = [chess.Board(fen) for fen in fens]
    require(all(b.is_valid() and actual_outcome(b) is None for b in openings), "Invalid/terminal initial opening")
    canonical = [" ".join(b.fen(en_passant="fen").split()[:4]) for b in openings]
    require(len(set(canonical)) == len(canonical), "Repeated opening positions must not masquerade as independent pairs")
    results = summary["games"]
    require(len(results) == 2 * len(fens), "Incomplete opening pairs")
    games = []
    with paths["pgn"].open() as source:
        while (game := chess.pgn.read_game(source, Visitor=StrictGameBuilder)) is not None:
            require(not game.errors, "Malformed PGN")
            games.append(game)
    require(len(games) == len(results), "PGN count differs from summary")
    rows = [json.loads(line) for line in paths["moves"].read_text().splitlines() if line.strip()]
    cursor, points = 0, []
    for number, (result, game) in enumerate(zip(results, games), 1):
        fen, white = fens[(number - 1) // 2], number % 2 == 1
        require(result["game"] == number and type(result["candidate_white"]) is bool and
                result["candidate_white"] == white and result["fen"] == fen, "Broken FEN/color pair")
        board = game.board()
        require(board.fen(en_passant="fen") == chess.Board(fen).fen(en_passant="fen"), "PGN initial position mismatch")
        require(game.headers.get("Round") == str(number) and
                game.headers.get("White") == ("candidate" if white else "control") and
                game.headers.get("Black") == ("control" if white else "candidate"), "PGN color identity mismatch")
        clocks = {chess.WHITE: float(manifest["bank_ms"]), chess.BLACK: float(manifest["bank_ms"])}
        moves = list(game.mainline_moves())
        require(len(moves) == result["played_plies"], "Played-ply count mismatch")
        for ply, move in enumerate(moves, 1):
            require(cursor < len(rows), "Missing move telemetry")
            row = rows[cursor]; cursor += 1
            require(actual_outcome(board) is None and move in board.legal_moves, "Illegal/post-terminal PGN move")
            require(row["game"] == number and row["ply"] == ply and row["move"] == move.uci() and
                    row["fen"] == board.fen(en_passant="fen"), "Telemetry move/history mismatch")
            require(row["status"] == "ok" and row["played"] is True and
                    row["engine"] == ("candidate" if board.turn == white else "control"), "Invalid engine reply")
            elapsed = row["elapsed_ms"]
            require(type(elapsed) in (int, float) and math.isfinite(elapsed) and 0 <= elapsed < clocks[board.turn], "Physical clock loss")
            require(row["increment_ms"] == manifest["increment_ms"] and
                    math.isclose(row["clock_before_ms"], clocks[board.turn], abs_tol=1e-6, rel_tol=0) and
                    all(math.isclose(row["requested_clocks_ms"][color], clocks[side], abs_tol=1e-6, rel_tol=0)
                        for color, side in (("white", chess.WHITE), ("black", chess.BLACK))), "Clock/history command mismatch")
            clocks[board.turn] += manifest["increment_ms"] - elapsed
            board.push(move)
        outcome = actual_outcome(board)
        require(outcome is not None, "Nonterminal claimed result")
        require(result["result"] == game.headers.get("Result") == outcome.result() and
                result["termination"] == game.headers.get("Termination") == outcome.termination.name, "Board/result mismatch")
        score = outcome_score(outcome, white)
        require(result["candidate_points"] == score, "Candidate points mismatch")
        points.append(score)
    require(cursor == len(rows), "Extra move telemetry")
    require(math.isclose(summary["score"], sum(points) / len(points), abs_tol=1e-12), "Summary score mismatch")
    require(initial_hashes == {name: file_hash(path) for name, path in paths.items()}, "Artifacts changed during read")
    for role in ("candidate", "control"):
        require(file_hash(Path(manifest[role])) == manifest[role + "_sha256"], "Binary changed during read: " + role)
    bins = [0] * 5
    for left, right in zip(points[::2], points[1::2]):
        bins[int(2 * (left + right))] += 1
    return {"schema": 1, "verification": "passed", "prefix": str(Path(prefix).resolve()),
            "artifact_sha256": initial_hashes, "match_identity": manifest,
            "played_plies_verified": len(rows),
            "candidate_wdl": {"wins": points.count(1), "draws": points.count(.5), "losses": points.count(0)},
            "opening_independence": "exact-position duplicates rejected; related opening families are not established as independent",
            "predeclared_stopping_rule_verified": False, "statistics": pair_statistics(bins)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("prefix", type=Path)
    parser.add_argument("--output", type=Path, help="New evidence path, never overwritten")
    args = parser.parse_args()
    if args.output and args.output.exists():
        raise FileExistsError("Preserve earlier analysis")
    report = analyse(args.prefix)
    if args.output:
        with args.output.open("x") as output:
            json.dump(report, output, indent=2, allow_nan=False)
    print(json.dumps(report, indent=2, allow_nan=False))


if __name__ == "__main__":
    main()
