"""SPRT gate for Heaven's Gate changes: one candidate vs one frozen base.

Every strength claim goes through this (or an equivalent OpenBench run):
paired openings (each played with both colours), 1 thread per engine,
books off, no score adjudication, and a predeclared SPRT stopping rule.

Typical use (build both binaries first, from two commits):

  python tools/sprt.py --base build-base/heavensgate.exe \
      --candidate build/heavensgate.exe --name lockless-tt \
      --book books/UHO_4060_v4.epd --gainer

  --gainer       SPRT elo0=0 elo1=5   (the change should gain strength)
  --nonreg       SPRT elo0=-5 elo1=0  (simplifications, cleanups, speed-neutral)
  default        STC 8+0.08; confirm gainers at LTC with --tc 40+0.4

Results, PGN and a manifest (binary SHA-256s, book hash, exact command) go to
build/sprt/<name>/. fastchess: https://github.com/Disservin/fastchess
(Windows and Linux release binaries; pass --fastchess PATH if it is not on PATH).
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--base", required=True, help="frozen control binary")
    parser.add_argument("--candidate", required=True, help="binary under test")
    parser.add_argument("--name", required=True, help="test name; output in build/sprt/<name>/")
    parser.add_argument("--book", required=True, help="EPD opening book (e.g. UHO_4060_v4.epd)")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--gainer", action="store_true", help="SPRT [0, 5] (default)")
    mode.add_argument("--nonreg", action="store_true", help="SPRT [-5, 0]")
    parser.add_argument("--elo0", type=float, help="override lower bound (normalized Elo)")
    parser.add_argument("--elo1", type=float, help="override upper bound (normalized Elo)")
    parser.add_argument("--tc", default="8+0.08", help="time control, default 8+0.08 (STC)")
    parser.add_argument("--hash", type=int, default=16, help="Hash MB per engine, default 16")
    parser.add_argument("--concurrency", type=int, default=max(1, (os.cpu_count() or 2) - 1))
    parser.add_argument("--max-rounds", type=int, default=40000, help="hard cap on opening pairs")
    parser.add_argument("--seed", type=int, default=None, help="opening shuffle seed (default: time)")
    parser.add_argument("--fastchess", default=shutil.which("fastchess") or "fastchess")
    parser.add_argument("--engine-args", default="uci", help="arguments that put the binary in UCI mode")
    parser.add_argument("--dry-run", action="store_true", help="print the command and manifest only")
    args = parser.parse_args()

    elo0, elo1 = (-5.0, 0.0) if args.nonreg else (0.0, 5.0)
    if args.elo0 is not None: elo0 = args.elo0
    if args.elo1 is not None: elo1 = args.elo1
    if not elo0 < elo1:
        sys.exit("elo0 must be below elo1")
    seed = args.seed if args.seed is not None else int(time.time())

    out = Path("build") / "sprt" / args.name
    out.mkdir(parents=True, exist_ok=True)
    base, candidate, book = (Path(p).resolve() for p in (args.base, args.candidate, args.book))
    for path in (base, candidate, book):
        if not path.is_file():
            sys.exit(f"missing file: {path}")
    if sha256(base) == sha256(candidate):
        print("warning: base and candidate are byte-identical binaries", file=sys.stderr)

    common = ["proto=uci", f"tc={args.tc}", "option.Threads=1", f"option.Hash={args.hash}",
              "option.OwnBook=false"]
    engine = lambda name, path: ["-engine", f"cmd={path}", f"name={name}", f"args={args.engine_args}",
                                 f"dir={path.parent}"]
    command = [args.fastchess,
               *engine("candidate", candidate), *engine("base", base),
               "-each", *common,
               "-openings", f"file={book}", "format=epd", "order=random",
               "-srand", str(seed),
               "-rounds", str(args.max_rounds), "-repeat",
               "-concurrency", str(args.concurrency),
               "-sprt", f"elo0={elo0}", f"elo1={elo1}", "alpha=0.05", "beta=0.05", "model=normalized",
               "-recover",
               "-pgnout", f"file={out / 'games.pgn'}", "notation=san",
               "-log", f"file={out / 'fastchess.log'}", "level=warn"]
    manifest = {
        "name": args.name, "started_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "base": str(base), "base_sha256": sha256(base),
        "candidate": str(candidate), "candidate_sha256": sha256(candidate),
        "book": str(book), "book_sha256": sha256(book), "seed": seed,
        "tc": args.tc, "hash_mb": args.hash, "threads_each": 1, "own_book": False,
        "sprt": {"elo0": elo0, "elo1": elo1, "alpha": 0.05, "beta": 0.05, "model": "normalized"},
        "concurrency": args.concurrency, "max_rounds": args.max_rounds, "command": command,
    }
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(" ".join(command))
    if args.dry_run:
        return 0

    with open(out / "output.txt", "w") as log:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
        for line in process.stdout:
            log.write(line); log.flush()
            if line.startswith(("Elo:", "LLR:", "Games:", "Ptnml", "SPRT", "H0", "H1")) or "was accepted" in line:
                print(line.rstrip(), flush=True)
        code = process.wait()
    print(f"\nresults: {out / 'output.txt'}   games: {out / 'games.pgn'}")
    return code


if __name__ == "__main__":
    sys.exit(main())
