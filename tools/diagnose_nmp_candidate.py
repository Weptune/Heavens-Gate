"""Serial matched-depth NMP diagnostics. Not a strength test or Game 8 rerun."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import time

import chess
import chess.engine

CASES = [
    ('startpos', chess.STARTING_FEN),
    ('kiwipete', 'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1'),
    ('rook-ending', '8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1'),
    ('midgame', 'r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10'),
    ('berlin', 'r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4'),
    ('pawn-ending', '8/8/4k3/3p4/3P4/4K3/8/8 b - - 0 1'),
]


def fingerprint(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def search(binary, fen, depth, guards=None):
    board = chess.Board(fen)
    with chess.engine.SimpleEngine.popen_uci([str(binary), 'uci'], timeout=60) as engine:
        options = {'Threads': 1, 'Hash': 64, 'OwnBook': False}
        if guards is not None: options['NMPGuards'] = guards
        engine.configure(options)
        started = time.perf_counter()
        info = engine.analyse(board, chess.engine.Limit(depth=depth), info=chess.engine.INFO_ALL)
        elapsed = time.perf_counter() - started
        pv = info.get('pv', [])
        if not pv or pv[0] not in board.legal_moves:
            raise RuntimeError('Missing/illegal root move')
        walk = board.copy()
        for move in pv:
            if move not in walk.legal_moves: raise RuntimeError('Illegal PV continuation')
            walk.push(move)
        if info.get('depth') != depth: raise RuntimeError('Did not complete matched target depth')
        return {'depth': info['depth'], 'score_stm': info['score'].pov(board.turn).score(mate_score=25000),
                'nodes': info['nodes'], 'pv': [move.uci() for move in pv], 'elapsed_seconds': elapsed}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--control', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--depth', type=int, default=8)
    args = parser.parse_args()
    if args.output.exists(): raise FileExistsError('Preserve prior diagnostic')
    args.control, args.candidate = args.control.resolve(), args.candidate.resolve()
    evidence = {'schema': 1, 'started_utc': datetime.now(timezone.utc).isoformat(),
        'control': str(args.control), 'candidate': str(args.candidate),
        'hashes': [fingerprint(args.control), fingerprint(args.candidate)],
        'threads': 1, 'hash_mb': 64, 'book': False, 'depth': args.depth,
        'purpose': 'default-off parity and guarded-candidate diagnostics; not Elo evidence', 'rows': []}
    try:
        for name, fen in CASES:
            for mirrored in (False, True):
                position = chess.Board(fen).mirror() if mirrored else chess.Board(fen)
                actual_fen = position.fen(en_passant='fen')
                baseline = search(args.control, actual_fen, args.depth)
                unchanged = search(args.candidate, actual_fen, args.depth, False)
                candidate = search(args.candidate, actual_fen, args.depth, True)
                parity = all(baseline[key] == unchanged[key] for key in ('depth', 'score_stm', 'nodes', 'pv'))
                evidence['rows'].append({'name': name, 'mirrored': mirrored, 'fen': actual_fen,
                    'control': baseline, 'guards_off': unchanged, 'guards_on': candidate, 'default_off_parity': parity})
                print(name, 'mirror' if mirrored else 'original', 'parity', parity,
                      'nodes', baseline['nodes'], candidate['nodes'], flush=True)
                if not parity: raise RuntimeError('Default-off search landscape changed')
        if evidence['hashes'] != [fingerprint(args.control), fingerprint(args.candidate)]:
            raise RuntimeError('Binary changed during diagnostic')
        evidence['status'] = 'passed'
    except BaseException as error:
        evidence.update(status='failed', error=repr(error))
        raise
    finally:
        with args.output.open('x') as output: json.dump(evidence, output, indent=2)


if __name__ == '__main__':
    main()
