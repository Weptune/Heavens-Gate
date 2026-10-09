"""Validate dataset provenance/split isolation, then fit a non-production candidate.

Do not run during an active timed pilot. The output directory must be new.
"""
import argparse
import json
import math
from pathlib import Path
import subprocess

import chess
from extract_quiet_dataset import canonical_position, file_hash


def validate_dataset(directory):
    directory = Path(directory).resolve()
    manifest = json.loads((directory / "manifest.json").read_text())
    if manifest.get("label_policy") != "verified-played-board-or-rule-result-only" or not manifest.get("training_ready"):
        raise ValueError("Dataset does not have complete, verified outcome partitions")
    if file_hash(directory / "samples.jsonl") != manifest.get("samples_sha256"):
        raise ValueError("Sample provenance changed since extraction")
    metadata = [json.loads(line) for line in (directory / "samples.jsonl").read_text().splitlines()]
    if not manifest.get("sources"):
        raise ValueError("Missing source provenance")
    source_hashes = {source["path"]: source["sha256"] for source in manifest["sources"]}
    for source in manifest["sources"]:
        if file_hash(source["path"]) != source["sha256"]:
            raise ValueError("Source PGN changed since extraction")
    game_splits, position_splits, position_labels, seen_lines = {}, {}, {}, set()
    paths = {}
    for split in ("train", "validation", "test"):
        specification = manifest["partitions"][split]
        path = (directory / specification["path"]).resolve()
        if path.parent != directory or file_hash(path) != specification["sha256"]:
            raise ValueError("Dataset path/hash mismatch")
        rows = path.read_text().splitlines()
        if not rows or len(rows) != specification["samples"]:
            raise ValueError("Empty or incorrectly counted partition")
        paths[split] = path
        partition_metadata = [record for record in metadata if record["split"] == split]
        if len(partition_metadata) != len(rows): raise ValueError("Missing sample provenance")
        for record in partition_metadata:
            identity = (split, record["line"])
            if identity in seen_lines or not 1 <= record["line"] <= len(rows):
                raise ValueError("Duplicated or invalid metadata row")
            seen_lines.add(identity)
            fen, target_text = rows[record["line"] - 1].split("|")
            target = float(target_text)
            board = chess.Board(fen)
            if not board.is_valid() or not math.isfinite(target) or target not in (0.0, .5, 1.0):
                raise ValueError("Invalid position or game-outcome label")
            if fen != record["fen"] or target != record["target"]:
                raise ValueError("Sample/provenance mismatch")
            key, label = canonical_position(fen, target)
            if key != record["position_group"] or label != record["canonical_target"]:
                raise ValueError("Invalid color-mirror grouping")
            game = record["game_id"]
            if game in game_splits and game_splits[game] != split:
                raise ValueError("Game-level train/holdout leakage")
            if key in position_splits and position_splits[key] != split:
                raise ValueError("Exact or color-mirrored train/holdout leakage")
            if key in position_labels and position_labels[key] != label:
                raise ValueError("Conflicting canonical position labels")
            game_splits[game], position_splits[key] = split, split
            position_labels[key] = label
            provenance = record["provenance"]
            if source_hashes.get(provenance["source_path"]) != provenance["source_sha256"]:
                raise ValueError("Unrecognized source provenance")
    if len(seen_lines) != len(metadata): raise ValueError("Unknown partition metadata")
    return paths, manifest


def run_candidate(binary, dataset_dir, output_dir, *, epochs=80, learning_rate=.5, patience=10, threads=1, sigmoid_k=400):
    paths, dataset_manifest = validate_dataset(dataset_dir)
    output_dir = Path(output_dir).resolve()
    if output_dir.exists(): raise FileExistsError("Candidate output already exists")
    binary = Path(binary).resolve()
    binary_hash = file_hash(binary)
    output_dir.mkdir(parents=True, exist_ok=False)
    candidate, report = output_dir / "candidate_eval_params.inc", output_dir / "loss_report.json"
    command = [str(binary), str(epochs), str(learning_rate), str(paths["train"]),
        "--validation", str(paths["validation"]), "--test", str(paths["test"]),
        "--output", str(candidate), "--report", str(report), "--patience", str(patience),
        "--threads", str(threads), "--k", str(sigmoid_k)]
    production = Path(__file__).resolve().parent.parent / "src/evaluation/tuned_eval_params.inc"
    before = file_hash(production)
    evidence = {"schema": 1, "binary": str(binary), "binary_sha256": binary_hash, "command": command,
                "dataset_manifest_sha256": file_hash(Path(dataset_dir) / "manifest.json"),
                "dataset_counts": {key: value["samples"] for key, value in dataset_manifest["partitions"].items()},
                "production_sha256_before": before, "promotion": "never automatic"}
    with (output_dir / "run_manifest.json").open("x") as output:
        json.dump(evidence, output, indent=2)
    result = subprocess.run(command, cwd=production.parent.parent.parent, capture_output=True, text=True)
    with (output_dir / "optimizer.log").open("x") as output:
        output.write(result.stdout + result.stderr)
    if file_hash(production) != before:
        raise RuntimeError("Production parameters unexpectedly changed; preserve outputs and investigate")
    if result.returncode not in (0, 2): raise RuntimeError(f"Tuner failed ({result.returncode}); see optimizer.log")
    summary = json.loads(report.read_text())
    if bool(summary["exported"]) != candidate.exists(): raise RuntimeError("Export evidence mismatch")
    summary.update(production_unchanged=True, paired_game_validation="pending")
    print(json.dumps(summary, indent=2))
    return result.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--dataset-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--epochs", type=int, default=80)
    parser.add_argument("--learning-rate", type=float, default=.5)
    parser.add_argument("--patience", type=int, default=10)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--k", type=float, default=400)
    args = parser.parse_args()
    return run_candidate(args.binary, args.dataset_dir, args.output_dir, epochs=args.epochs,
        learning_rate=args.learning_rate, patience=args.patience, threads=args.threads, sigmoid_k=args.k)


if __name__ == "__main__":
    raise SystemExit(main())
