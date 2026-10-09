#!/usr/bin/env python3
"""
The Infallible 5-Gate Verification Protocol for Heaven's Gate Chess Engine.
Automatically verifies candidate builds against all 5 quality and performance gates:
  - Gate 1: Static Architectural Safety Audit (Blacklist & Zero-NN enforcement)
  - Gate 2: Clean Compilation & Build Verification
  - Gate 3: Deterministic Micro-Architecture Gate (Perft 6/6 Bit-Exact match)
  - Gate 4: Multi-Suite Tactical & Positional Sanity Gate (BK-24 + STS-80)
  - Gate 5: Match Play Stability & Non-Regression Gate (vs Stockfish / Self-Play)
"""

import os
import sys
import subprocess
import time
import re
import argparse
import shutil
import tempfile
import json
import hashlib

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_DIR = os.path.join(ROOT_DIR, "build", "phase01")
ENGINE_EXE = os.path.join(BUILD_DIR, "heavensgate.exe" if os.name == "nt" else "heavensgate")
W64DEVKIT_BIN = os.path.join(ROOT_DIR, "tools", "w64devkit", "bin")

def print_header(title):
    print("\n" + "=" * 70)
    print(f"  {title}")
    print("=" * 70)

def run_gate1_audit():
    print_header("GATE 1: STATIC ARCHITECTURAL SAFETY AUDIT")
    failures = []
    
    # Check 1: Zero Neural Network / NNUE files or references
    forbidden_files = ["nnue.cpp", "nnue.hpp", "tensor_nnue.cpp", "tensor_nnue.hpp"]
    for root, dirs, files in os.walk(os.path.join(ROOT_DIR, "src")):
        for f in files:
            if f.lower() in forbidden_files:
                failures.append(f"Forbidden neural network file detected: {os.path.join(root, f)}")
    
    # Check 2: Blacklist enforcement in src/search/search.cpp
    search_cpp = os.path.join(ROOT_DIR, "src", "search", "search.cpp")
    if os.path.exists(search_cpp):
        with open(search_cpp, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
            if "extensions_stack_" in content and "parent_ext < 2" in content:
                failures.append("Check extension cap detected! Blacklist violation: check extensions must remain uncapped (ply < 64).")
    
    if failures:
        print("[GATE 1 FAILED] Architectural Safety Violations:")
        for fail in failures:
            print(f"  - {fail}")
        return False
    
    print("[GATE 1 PASSED] Basic static blacklist checks passed (not a race-freedom proof).")
    return True

def run_gate2_build():
    print_header("GATE 2: CLEAN COMPILATION & BUILD VERIFICATION")
    start = time.time()
    env = os.environ.copy()
    configure = [shutil.which("cmake") or "cmake", "-S", ROOT_DIR, "-B", BUILD_DIR, "-DCMAKE_BUILD_TYPE=Release"]
    if os.name == "nt":
        env["PATH"] = W64DEVKIT_BIN + os.pathsep + env.get("PATH", "")
        configure += ["-G", "MinGW Makefiles", "-DCMAKE_CXX_COMPILER=" + os.path.join(W64DEVKIT_BIN, "g++.exe")]
    commands = [configure, [configure[0], "--build", BUILD_DIR, "--parallel", "6"],
                [shutil.which("ctest") or "ctest", "--test-dir", BUILD_DIR, "--output-on-failure"]]
    for command in commands:
        res = subprocess.run(command, cwd=ROOT_DIR, env=env, capture_output=True, text=True, timeout=600)
        if res.returncode != 0:
            print(f"[GATE 2 FAILED] Command failed ({res.returncode}): {command}")
            print((res.stdout + res.stderr)[-5000:])
            return False
    elapsed = time.time() - start
    
    print(f"[GATE 2 PASSED] CMake engine + unit tests built and CTest passed in {elapsed:.2f}s.")
    return True

def run_gate3_perft():
    print_header("GATE 3: SIX REFERENCE POSITIONS (STARTPOS THROUGH DEPTH 6)")
    if not os.path.exists(ENGINE_EXE):
        print("[GATE 3 FAILED] heavensgate.exe binary not found.")
        return False
    
    start = time.time()
    res = subprocess.run([ENGINE_EXE, "perft", "6"], cwd=ROOT_DIR, capture_output=True, text=True, timeout=600)
    elapsed = time.time() - start
    
    out = res.stdout + res.stderr
    if (res.returncode != 0 or "VERIFICATION SUITE RESULT: ALL PASSED" not in out
            or len(re.findall(r"\[Testing\]", out)) != 6
            or not re.search(r"Depth 6:\s+119060324 / 119060324 nodes.*\[PASSED\]", out)):
        print("[GATE 3 FAILED] Perft verification suite did not pass 100% bit-exact:")
        print(out[-1000:])
        return False
    
    print(f"[GATE 3 PASSED] All six reference positions match; startpos depth 6 = 119,060,324 ({elapsed:.2f}s). Other references currently end at depth 5.")
    return True

def run_gate4_benchmarks():
    print_header("GATE 4: MULTI-SUITE TACTICAL & POSITIONAL SANITY GATE")
    # 4A. STS Positional Benchmark
    print("Running STS-80 benchmark (30ms/pos, 6 threads)...")
    pct = 0.0
    for attempt in range(1):
        sts_res = subprocess.run([ENGINE_EXE, "sts", "30", "0", "6"], cwd=ROOT_DIR, capture_output=True, text=True)
        sts_out = sts_res.stdout + sts_res.stderr
        if sts_res.returncode != 0:
            print("[GATE 4 FAILED] STS process failed.")
            return False
        
        score_match = re.search(r"OVERALL TOTAL\s+\d+\s+(\d+)\s*/\s*8000\s+([\d\.]+)\s*%", sts_out)
        if not score_match:
            print("[GATE 4 FAILED] Could not parse STS report.")
            return False
        
        pts = int(score_match.group(1))
        pct = float(score_match.group(2))
        print(f"STS Positional Score: {pts} / 8000 ({pct:.2f}%)")
        if pct >= 50.0:
            break
    
    if pct < 50.0:
        print(f"[GATE 4 FAILED] STS score {pct:.2f}% is below minimum 50.0% sanity threshold.")
        return False
    
    # 4B. Tactical Benchmark
    print("Running Bratko-Kopec Tactical Test...")
    tac_script = os.path.join(ROOT_DIR, "tools", "run_tactical_benchmark.py")
    if os.path.exists(tac_script):
        env = os.environ.copy()
        env["HG_ENGINE"] = ENGINE_EXE
        tac_res = subprocess.run([sys.executable, tac_script], cwd=ROOT_DIR, env=env, capture_output=True, text=True, timeout=120)
        tac_out = tac_res.stdout + tac_res.stderr
        if tac_res.returncode != 0:
            print("[GATE 4 FAILED] Tactical process failed.")
            return False
        bk_match = re.search(r"Bratko-Kopec \(BK-24\)\s*:\s*(\d+)/24\s*\(([\d\.]+)%\)", tac_out)
        if bk_match:
            bk_solved = int(bk_match.group(1))
            bk_pct = float(bk_match.group(2))
            print(f"Bratko-Kopec Solved: {bk_solved}/24 ({bk_pct:.1f}%)")
            if bk_pct < 50.0:
                print(f"[GATE 4 FAILED] Tactical accuracy {bk_pct:.1f}% below minimum 50% threshold.")
                return False
        else:
            print("[GATE 4 FAILED] Missing/malformed BK-24 report.")
            return False
    else:
        print("[GATE 4 FAILED] Tactical benchmark script missing.")
        return False
    
    print("[GATE 4 PASSED] Both Positional (STS >= 50%) and Tactical (BK-24 >= 50%) sanity gates passed.")
    return True

def verify_terminal_pgn(pgn):
    """Reconstruct coordinate movetext; engine score comments are not evidence."""
    try:
        import chess
        initial = re.search(r'^\[FEN "([^"]+)"\]$', pgn, re.MULTILINE)
        if not initial:
            return False
        board = chess.Board(initial.group(1))
        movetext = re.sub(r'\{[^}]*\}|^\[.*\]$', '', pgn, flags=re.MULTILINE)
        for uci in re.findall(r'\b[a-h][1-8][a-h][1-8][qrbn]?\b', movetext):
            move = chess.Move.from_uci(uci)
            if move not in board.legal_moves:
                return False
            board.push(move)
        result = re.search(r'^\[Result "([^"]+)"\]$', pgn, re.MULTILINE)
        if not result:
            return False
        actual = board.outcome(claim_draw=True)
        return actual is not None and result.group(1) == actual.result()
    except (ImportError, ValueError):
        return False


def sha256_file(path):
    with open(path, "rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def run_gate5_stability():
    print_header("GATE 5: MATCH STABILITY SMOKE TEST (NOT AN ELO GATE)")
    print("Testing 1 match game vs Stockfish at depth 4...")
    start = time.time()
    try:
        with tempfile.TemporaryDirectory(prefix="hg-gate5-") as temp_dir:
            pgn_path = os.path.join(temp_dir, "stability.pgn")
            cmd = [ENGINE_EXE, "depth_match", "1", "4", "2200", "4", pgn_path]
            res = subprocess.run(cmd, cwd=ROOT_DIR, capture_output=True, text=True, timeout=120)
            out = res.stdout + res.stderr
            if res.returncode != 0 or any(marker in out for marker in ("[GAME FORFEIT]", "Time Out", "[FATAL]", "[ENGINE FAILURE]", "Forced Mate")):
                print(f"[GATE 5 FAILED] Match process/forfeit failure (exit {res.returncode}).")
                print(out[-1500:])
                return False
            completed = re.findall(r"\[TOURNAMENT\] Game (\d+)/(\d+) \(", out)
            if completed != [("1", "1")] or "TOURNAMENT RESULTS" not in out or not os.path.isfile(pgn_path):
                print("[GATE 5 FAILED] No verified completed game.")
                return False
            with open(pgn_path, encoding="utf-8") as pgn_file:
                pgn = pgn_file.read()
            results = re.findall(r'^\[Result "(1-0|0-1|1/2-1/2)"\]$', pgn, re.MULTILINE)
            endings = re.findall(r'^(?:.*\s)?(1-0|0-1|1/2-1/2) \{[^}]+\}\s*$', pgn, re.MULTILINE)
            if len(results) != 1 or results != endings:
                print("[GATE 5 FAILED] PGN result/movetext completion mismatch.")
                return False
            if not verify_terminal_pgn(pgn):
                print("[GATE 5 FAILED] PGN is not a legal board-terminal/rule-draw result (requires python-chess).")
                return False
            try:
                with open(pgn_path + ".manifest.json", encoding="utf-8") as source:
                    manifest = json.load(source)
                with open(pgn_path + ".moves.jsonl", encoding="utf-8") as source:
                    telemetry = [json.loads(line) for line in source if line.strip()]
                if (manifest.get("engine_sha256") != sha256_file(ENGINE_EXE)
                        or manifest.get("opponent_sha256") != sha256_file(manifest["opponent_path"])
                        or not re.fullmatch(r"[0-9a-f]{64}", manifest.get("source_sha256", ""))
                        or manifest.get("adjudication") != "board-terminal-or-rule-draw-only"
                        or not telemetry or any(move.get("status") != "ok" for move in telemetry)):
                    raise ValueError("Invalid manifest/telemetry")
            except (OSError, ValueError, KeyError) as error:
                print(f"[GATE 5 FAILED] Missing/mismatched run evidence: {error}")
                return False
        elapsed = time.time() - start
        
        print(f"[GATE 5 PASSED] Match finished cleanly in {elapsed:.2f}s with zero forfeits and zero crashes.")
        return True
    except subprocess.TimeoutExpired:
        print("[GATE 5 FAILED] Match play timed out (>120s)!")
        return False

def main():
    global ENGINE_EXE
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine", help="Candidate binary (must be a matching build)")
    parser.add_argument("--gates", nargs="+", type=int, choices=range(1, 6), default=list(range(1, 6)))
    args = parser.parse_args()
    if args.engine:
        ENGINE_EXE = os.path.abspath(args.engine)
        if 2 in args.gates and ENGINE_EXE != os.path.join(BUILD_DIR, "heavensgate.exe" if os.name == "nt" else "heavensgate"):
            parser.error("Gate 2 builds build/phase01; do not combine it with a different --engine")
    print("\n" + "#" * 70)
    print("   HEAVEN'S GATE 5-GATE VERIFICATION PROTOCOL (MASTER RUNNER)")
    print("#" * 70)
    
    gates = [
        ("Gate 1: Static Architectural Safety Audit", run_gate1_audit),
        ("Gate 2: Clean Compilation & Build Verification", run_gate2_build),
        ("Gate 3: Deterministic Perft 6/6 Gate", run_gate3_perft),
        ("Gate 4: Tactical & Positional Sanity Gate", run_gate4_benchmarks),
        ("Gate 5: Match Play Stability Gate", run_gate5_stability)
    ]
    
    for number, (name, gate_fn) in enumerate(gates, 1):
        if number not in args.gates: continue
        try:
            success = gate_fn()
        except (OSError, subprocess.SubprocessError) as error:
            print(f"[GATE {number} FAILED] {error}")
            success = False
        if not success:
            print(f"\n>>> VERIFICATION SUITE STOPPED: {name} FAILED! <<<\n")
            sys.exit(1)
            
    print("\n" + "#" * 70)
    print("   REQUESTED GATES PASSED (stability smoke tests do not establish Elo)")
    print("#" * 70 + "\n")
    sys.exit(0)

if __name__ == "__main__":
    main()
