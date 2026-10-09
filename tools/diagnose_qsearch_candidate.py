"""Short serial fixed-depth qsearch candidate diagnostics; never plays games.

Elapsed/node ratios are noisy local throughput samples, not Elo estimates.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
from statistics import median

import chess
from diagnose_nmp_candidate import CASES, fingerprint, search


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--control", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--depth", type=int, default=8)
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.output.exists(): raise FileExistsError("Preserve earlier diagnostic")
    if not 1 <= args.depth <= 10 or not 1 <= args.repeats <= 5: raise ValueError("Bounded diagnostic only")
    args.control, args.candidate = args.control.resolve(), args.candidate.resolve()
    evidence = {"schema": 1, "started_utc": datetime.now(timezone.utc).isoformat(),
                "control": str(args.control), "candidate": str(args.candidate),
                "hashes": [fingerprint(args.control), fingerprint(args.candidate)],
                "threads_each": 1, "hash_mb_each": 64, "book": False, "NMPGuards": False,
                "depth": args.depth, "repeats": args.repeats,
                "purpose": "correctness and local throughput diagnostics; NOT a strength test", "rows": []}
    try:
        for name, fen in CASES:
            for mirrored in (False, True):
                board = chess.Board(fen).mirror() if mirrored else chess.Board(fen)
                row = {"name": name, "mirrored": mirrored, "fen": board.fen(en_passant="fen"), "samples": []}
                for repeat in range(args.repeats):
                    sample = {}
                    order = ("control", "candidate") if repeat % 2 == 0 else ("candidate", "control")
                    for role in order:
                        sample[role] = search(getattr(args, role), row["fen"], args.depth, False)
                    sample["score_pv_nodes_equal"] = all(sample["control"][key] == sample["candidate"][key]
                                                         for key in ("score_stm", "nodes", "pv"))
                    row["samples"].append(sample)
                row["median_elapsed_ratio_candidate_control"] = median(s["candidate"]["elapsed_seconds"] /
                                                                       s["control"]["elapsed_seconds"] for s in row["samples"])
                row["median_seconds_per_node_ratio"] = median((s["candidate"]["elapsed_seconds"] / s["candidate"]["nodes"]) /
                                                               (s["control"]["elapsed_seconds"] / s["control"]["nodes"]) for s in row["samples"])
                evidence["rows"].append(row)
                print(name, "mirror" if mirrored else "original", "equal", all(s["score_pv_nodes_equal"] for s in row["samples"]),
                      "per-node ratio", round(row["median_seconds_per_node_ratio"], 3), flush=True)
        if evidence["hashes"] != [fingerprint(args.control), fingerprint(args.candidate)]:
            raise RuntimeError("Binary changed during diagnostic")
        evidence["median_position_seconds_per_node_ratio"] = median(row["median_seconds_per_node_ratio"] for row in evidence["rows"])
        evidence["status"] = "passed; strength pending"
    except BaseException as error:
        evidence.update(status="failed", error=repr(error))
        raise
    finally:
        with args.output.open("x") as output: json.dump(evidence, output, indent=2)


if __name__ == "__main__":
    main()
