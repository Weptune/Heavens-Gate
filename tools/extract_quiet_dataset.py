#!/usr/bin/env python3
"""Fail-closed PGN extraction with game-level splits and immutable provenance.

No default output: existing data/quiet_positions.txt is never overwritten.
Only legal games with played board/rule outcomes are eligible. This does not
assert grandmaster provenance, and the static quiet filter is not a qsearch.
"""
import argparse
from collections import Counter
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re

import chess

RESULTS = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": .5}
UNTRUSTED_ENDING = re.compile(
    r"time\s*out|time\s*forfeit|flag\s*fall|engine\s*failure|forfeit|adjudicat|forced\s*mate|move\s*limit|unfinished|resign", re.I)
HEADER = re.compile(r'^\[(\w+)\s+"((?:[^"\\]|\\.)*)"\]\s*$', re.M)


class RejectedGame(ValueError):
    pass


@dataclass
class VerifiedGame:
    game_id: str
    initial_fen: str
    moves: list[str]
    target: float
    tags: dict[str, str]
    samples: list[str]


def file_hash(path):
    with Path(path).open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def canonical_position(fen, target):
    board = chess.Board(fen)
    normal = " ".join(board.fen(en_passant="legal").split()[:4])
    mirror = " ".join(board.mirror().fen(en_passant="legal").split()[:4])
    return (mirror, 1.0 - target) if mirror < normal else (normal, target)


def split_for_game(game_id, seed):
    bucket = int(hashlib.sha256(f"{seed}:{game_id}".encode()).hexdigest()[:16], 16) % 100
    return "train" if bucket < 80 else "validation" if bucket < 90 else "test"


def mainline_tokens(text):
    """Strip comments, NAGs and RAVs, but never silently skip unknown mainline tokens."""
    result, braces, variations = [], 0, 0
    for line in text.splitlines():
        if line.lstrip().startswith("["):
            continue
        for character in line.split(";", 1)[0]:
            if character == "{":
                if braces: raise RejectedGame("malformed_comments")
                braces = 1
            elif character == "}":
                if not braces: raise RejectedGame("malformed_comments")
                braces = 0
                result.append(" ")
            elif not braces:
                if character == "(": variations += 1
                elif character == ")":
                    if variations == 0: raise RejectedGame("malformed_variation")
                    variations -= 1
                    result.append(" ")
                elif variations == 0: result.append(character)
        result.append(" ")
    if braces or variations: raise RejectedGame("unterminated_comment_or_variation")
    cleaned = re.sub(r"\$\d+", " ", "".join(result))
    cleaned = re.sub(r"\b\d+\.(?:\.\.)?", " ", cleaned)
    return [token.rstrip("!?") for token in cleaned.split()]


def parse_verified_game(block, *, min_ply=16, max_ply=100, stride=4, min_pieces=8, strict_quiet=True):
    if UNTRUSTED_ENDING.search(block): raise RejectedGame("untrusted_termination")
    headers = HEADER.findall(block)
    if sum(line.lstrip().startswith("[") for line in block.splitlines()) != len(headers):
        raise RejectedGame("malformed_header")
    if len({key for key, _ in headers}) != len(headers): raise RejectedGame("duplicate_header")
    tags = dict(headers)
    if tags.get("Variant", "Standard") not in ("Standard", "Chess"): raise RejectedGame("unsupported_variant")
    initial = tags.get("FEN", chess.STARTING_FEN)
    try:
        board = chess.Board(initial)
    except ValueError as error:
        raise RejectedGame("invalid_initial_fen") from error
    if not board.is_valid(): raise RejectedGame("invalid_initial_position")
    initial = board.fen(en_passant="fen")
    tokens = mainline_tokens(block)
    if not tokens or tokens[-1] not in RESULTS: raise RejectedGame("missing_completed_result")
    result = tokens.pop()
    if tags.get("Result", result) != result: raise RejectedGame("result_tag_mismatch")
    if any(token in RESULTS or token == "*" for token in tokens): raise RejectedGame("embedded_result")
    moves, samples = [], []
    for ply, token in enumerate(tokens, 1):
        if board.is_game_over(claim_draw=False): raise RejectedGame("moves_after_terminal_board")
        try:
            move = board.parse_uci(token) if re.fullmatch(r"[a-h][1-8][a-h][1-8][qrbn]?", token) else board.parse_san(token)
        except ValueError as error:
            raise RejectedGame("illegal_or_unknown_move") from error
        if not move or move not in board.legal_moves: raise RejectedGame("illegal_or_unknown_move")
        capture = board.is_capture(move)
        board.push(move)
        moves.append(move.uci())
        if min_ply <= ply <= max_ply and ply % stride == 0 and not board.is_check() and not capture and not move.promotion:
            if len(board.piece_map()) >= min_pieces and (not strict_quiet or not any(
                    board.is_capture(candidate) or candidate.promotion for candidate in board.legal_moves)):
                samples.append(board.fen(en_passant="fen"))
    actual = board.outcome(claim_draw=False)
    if actual is None and (board.is_repetition(3) or board.is_fifty_moves()):
        actual = chess.Outcome(chess.Termination.THREEFOLD_REPETITION if board.is_repetition(3)
                               else chess.Termination.FIFTY_MOVES, None)
    if actual is None or actual.result() != result: raise RejectedGame("nonterminal_or_wrong_result")
    game_id = hashlib.sha256((initial + "\n" + " ".join(moves)).encode()).hexdigest()
    return VerifiedGame(game_id, initial, moves, RESULTS[result], tags, samples)


def build_records(games, seed):
    """First remove whole-game duplicates, then deduplicate across ALL partitions.

    Exact/color-mirrored positions with conflicting game-outcome labels are
    excluded as ambiguous, not misrepresented as invalid chess games.
    """
    positions, seen_games, ambiguous = {}, set(), set()
    counts = Counter()
    for game, provenance in games:
        if game.game_id in seen_games:
            counts["duplicate_games"] += 1
            continue
        seen_games.add(game.game_id)
        split = split_for_game(game.game_id, seed)
        for fen in game.samples:
            key, label = canonical_position(fen, game.target)
            if key in ambiguous:
                counts["ambiguous_samples"] += 1
                continue
            if key in positions:
                counts["duplicate_samples"] += 1
                if positions[key]["canonical_target"] != label:
                    del positions[key]
                    ambiguous.add(key)
                    counts["conflicting_position_groups"] += 1
                continue
            positions[key] = {"fen": fen, "target": game.target, "game_id": game.game_id,
                              "split": split, "position_group": key, "canonical_target": label,
                              "provenance": provenance, "mirrored": False}
    records = []
    for sample in positions.values():
        records.append(sample)
        mirrored = dict(sample, fen=chess.Board(sample["fen"]).mirror().fen(en_passant="fen"),
                        target=1.0 - sample["target"], mirrored=True)
        records.append(mirrored)
    return records, counts


def extract_positions(output_dir, input_paths, *, seed=20261009, **filters):
    output_dir = Path(output_dir).resolve()
    if output_dir.exists(): raise FileExistsError(f"Preserve existing output: {output_dir}")
    games, sources, rejected = [], [], Counter()
    for path in sorted({Path(path).resolve() for path in input_paths}):
        digest = file_hash(path)
        text = path.read_text(encoding="utf-8-sig", errors="strict")
        blocks = [block for block in re.split(r'(?=^\[Event\s)', text, flags=re.M) if block.strip()]
        if not blocks: rejected["empty_source"] += 1
        sources.append({"path": str(path), "sha256": digest, "game_blocks": len(blocks)})
        for number, block in enumerate(blocks, 1):
            try:
                game = parse_verified_game(block, **filters)
            except RejectedGame as error:
                rejected[str(error)] += 1
                continue
            games.append((game, {"source_path": str(path), "source_sha256": digest,
                                 "source_game": number, "white": game.tags.get("White", "unknown"),
                                 "black": game.tags.get("Black", "unknown")}))
    records, duplicate_counts = build_records(games, seed)
    manifest = {"schema": 1, "seed": seed, "split_policy": "game-hash-80-10-10-global-color-mirror-dedup",
                "label_policy": "verified-played-board-or-rule-result-only", "grandmaster_provenance": "not asserted",
                "filters": filters, "sources": sources, "verified_games_before_dedup": len(games),
                "rejections": dict(rejected), "deduplication": dict(duplicate_counts), "partitions": {}}
    output_dir.mkdir(parents=True, exist_ok=False)
    with (output_dir / "samples.jsonl").open("x", encoding="utf-8") as metadata:
        for split in ("train", "validation", "test"):
            subset = [sample for sample in records if sample["split"] == split]
            destination = output_dir / f"{split}.txt"
            with destination.open("x", encoding="utf-8") as output:
                for index, sample in enumerate(subset, 1):
                    output.write(f'{sample["fen"]}|{sample["target"]:.1f}\n')
                    metadata.write(json.dumps(dict(sample, line=index)) + "\n")
            manifest["partitions"][split] = {"path": destination.name, "sha256": file_hash(destination),
                "samples": len(subset), "games": len({sample["game_id"] for sample in subset}),
                "labels": dict(Counter(str(sample["target"]) for sample in subset))}
    manifest["training_ready"] = all(manifest["partitions"][split]["samples"] > 0 for split in manifest["partitions"])
    manifest["samples_sha256"] = file_hash(output_dir / "samples.jsonl")
    with (output_dir / "manifest.json").open("x", encoding="utf-8") as output:
        json.dump(manifest, output, indent=2)
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path, help="PGN files or directories")
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--seed", type=int, default=20261009)
    parser.add_argument("--min-ply", type=int, default=16)
    parser.add_argument("--max-ply", type=int, default=100)
    parser.add_argument("--stride", type=int, default=4)
    parser.add_argument("--min-pieces", type=int, default=8)
    args = parser.parse_args()
    if args.stride < 1 or args.min_ply < 1 or args.max_ply < args.min_ply or args.min_pieces < 0:
        parser.error("Invalid sampling filters")
    paths = [file for path in args.inputs for file in (sorted(path.glob("*.pgn")) if path.is_dir() else [path])]
    if not paths: parser.error("No PGN sources")
    manifest = extract_positions(args.output_dir, paths, seed=args.seed, min_ply=args.min_ply,
        max_ply=args.max_ply, stride=args.stride, min_pieces=args.min_pieces, strict_quiet=True)
    print(json.dumps(manifest, indent=2))
    return 0 if manifest["training_ready"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
