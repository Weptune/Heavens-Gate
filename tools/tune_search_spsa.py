"""Game-based SPSA against a frozen HG control; no production header writes.

Each +/- probe plays the SAME paired FENs against the SAME control binary.
Results are candidate settings only and need independent strength validation.
Do not run while a timed pilot is active.
"""
import argparse
import json
import math
from pathlib import Path
import random

import chess
import chess.engine
from extract_quiet_dataset import file_hash
from paired_match import run_paired_batch

PARAM_DEFS = {
    "lmr_divisor": {"type": "float", "c": .20, "min": 1.60, "max": 4.00, "uci": "LMR_Divisor"},
    "lmr_hist_bonus": {"type": "int", "c": 50, "min": 150, "max": 1500, "uci": "LMR_HistBonus"},
    "lmr_hist_malus": {"type": "int", "c": 20, "min": 20, "max": 300, "uci": "LMR_HistMalus"},
    "rfp_margin": {"type": "int", "c": 20, "min": 60, "max": 260, "uci": "RFP_Margin"},
    "futility_margin": {"type": "int", "c": 25, "min": 80, "max": 350, "uci": "Futility_Margin"},
    "see_bad_capture_slope": {"type": "int", "c": 15, "min": 40, "max": 200, "uci": "SEE_BadCaptureSlope"},
    "see_quiet_slope": {"type": "int", "c": 5, "min": 5, "max": 60, "uci": "SEE_QuietSlope"},
    "nmp_eval_margin": {"type": "int", "c": 30, "min": 80, "max": 400, "uci": "NMP_EvalMargin"},
    "singular_margin": {"type": "int", "c": 1, "min": 1, "max": 6, "uci": "Singular_Margin"},
    "aspiration_window_delta": {"type": "int", "c": 5, "min": 5, "max": 100, "uci": "Aspiration_Window_Delta"},
}


def default_parameters(binary):
    with chess.engine.SimpleEngine.popen_uci([str(binary), "uci"]) as engine:
        values = {}
        for name, definition in PARAM_DEFS.items():
            option = engine.options[definition["uci"]]
            value = float(option.default)
            if not definition["min"] <= value <= definition["max"]:
                raise ValueError(f"Binary default outside tuning bounds: {name}")
            values[name] = value
        return values


def uci_parameters(parameters):
    return {definition["uci"]: (f"{parameters[name]:.6f}" if definition["type"] == "float"
                               else int(round(parameters[name]))) for name, definition in PARAM_DEFS.items()}


def probes(parameters, iteration, rng):
    plus, minus = {}, {}
    for name, definition in PARAM_DEFS.items():
        perturbation = definition["c"] * iteration ** -.101 * rng.choice((-1, 1))
        values = [min(definition["max"], max(definition["min"], parameters[name] + sign * perturbation))
                  for sign in (1, -1)]
        if definition["type"] == "int": values = [round(value) for value in values]
        plus[name], minus[name] = values
    return plus, minus


def update_parameters(parameters, plus, minus, y_plus, y_minus, iteration, gain=.01):
    # Work in normalized parameter coordinates. Use ACTUAL signed probe spans,
    # accounting for clipping/rounding rather than inventing the nominal span.
    updated = {}
    for name, definition in PARAM_DEFS.items():
        width = definition["max"] - definition["min"]
        span = (plus[name] - minus[name]) / width
        gradient = (y_plus - y_minus) / span if span else 0.0
        step = gain / (iteration + 5.0) ** .602 * gradient * width
        updated[name] = min(definition["max"], max(definition["min"], parameters[name] + step))
    return updated


def run_spsa(candidate, control, fens, output_dir, *, iterations=10, pairs=4, seed=20261009,
             bank_ms=10000, increment_ms=100, threads=1, hash_mb=64, gain=.01):
    candidate, control, output_dir = Path(candidate).resolve(), Path(control).resolve(), Path(output_dir).resolve()
    if output_dir.exists(): raise FileExistsError("Preserve existing SPSA run")
    if iterations < 1 or pairs < 1 or not math.isfinite(gain) or gain <= 0 or not fens or bank_ms <= 0 or increment_ms < 0 or threads < 1 or hash_mb < 1:
        raise ValueError("Invalid SPSA configuration")
    for fen in fens:
        if not chess.Board(fen).is_valid(): raise ValueError("Invalid tuning opening")
    parameters = default_parameters(candidate)
    candidate_hash, control_hash = file_hash(candidate), file_hash(control)
    output_dir.mkdir(parents=True, exist_ok=False)
    manifest = {"schema": 1, "objective": "paired played-game score against frozen control (not STS)",
        "candidate": str(candidate), "control": str(control), "candidate_sha256": candidate_hash,
        "control_sha256": control_hash, "seed": seed, "iterations": iterations, "pairs_per_probe": pairs,
        "bank_ms": bank_ms, "increment_ms": increment_ms, "threads_each": threads,
        "hash_mb_each": hash_mb, "gain": gain, "initial_parameters": parameters, "fens": fens,
        "promotion": "candidate JSON only; independent paired strength test required"}
    with (output_dir / "manifest.json").open("x") as output: json.dump(manifest, output, indent=2)
    rng = random.Random(seed)
    openings = list(fens)
    rng.shuffle(openings)
    with (output_dir / "iterations.jsonl").open("x") as evidence:
        try:
            for iteration in range(1, iterations + 1):
                if file_hash(candidate) != candidate_hash or file_hash(control) != control_hash:
                    raise RuntimeError("Frozen engine binary changed during SPSA")
                plus, minus = probes(parameters, iteration, rng)
                selected = [openings[((iteration - 1) * pairs + number) % len(openings)] for number in range(pairs)]
                directory = output_dir / f"iteration-{iteration:04d}"
                plus_result = run_paired_batch(candidate, control, uci_parameters(plus), selected, directory / "plus",
                    bank_ms=bank_ms, increment_ms=increment_ms, threads=threads, hash_mb=hash_mb)
                minus_result = run_paired_batch(candidate, control, uci_parameters(minus), selected, directory / "minus",
                    bank_ms=bank_ms, increment_ms=increment_ms, threads=threads, hash_mb=hash_mb)
                if file_hash(candidate) != candidate_hash or file_hash(control) != control_hash:
                    raise RuntimeError("Frozen engine binary changed during SPSA probes")
                updated = update_parameters(parameters, plus, minus, plus_result["score"], minus_result["score"], iteration, gain)
                record = {"iteration": iteration, "before": parameters, "plus": plus, "minus": minus,
                          "plus_result": plus_result, "minus_result": minus_result, "after": updated}
                evidence.write(json.dumps(record) + "\n")
                evidence.flush()
                parameters = updated
                print(f'[SPSA {iteration}/{iterations}] paired scores +={plus_result["score"]:.4f} -={minus_result["score"]:.4f}', flush=True)
        except Exception as error:
            evidence.write(json.dumps({"status": "failed", "diagnostic": str(error)}) + "\n")
            evidence.flush()
            raise
    candidate_settings = {"schema": 1, "parameters": parameters, "uci_options": uci_parameters(parameters),
                          "strength_validation": "pending; not selected by a maximum noisy STS score"}
    with (output_dir / "candidate_search_params.json").open("x") as output:
        json.dump(candidate_settings, output, indent=2)
    return candidate_settings


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--control", type=Path, required=True)
    parser.add_argument("--openings", type=Path, required=True, help="One complete FEN per line; no comments")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--iterations", type=int, default=10)
    parser.add_argument("--pairs", type=int, default=4)
    parser.add_argument("--seed", type=int, default=20261009)
    parser.add_argument("--bank-ms", type=int, default=10000)
    parser.add_argument("--increment-ms", type=int, default=100)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--hash", type=int, default=64)
    parser.add_argument("--gain", type=float, default=.01)
    args = parser.parse_args()
    fens = [line.strip() for line in args.openings.read_text().splitlines() if line.strip()]
    print(json.dumps(run_spsa(args.candidate, args.control, fens, args.output_dir,
        iterations=args.iterations, pairs=args.pairs, seed=args.seed, bank_ms=args.bank_ms,
        increment_ms=args.increment_ms, threads=args.threads, hash_mb=args.hash, gain=args.gain), indent=2))


if __name__ == "__main__":
    main()
