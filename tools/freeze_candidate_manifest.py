"""Describe exact snapshot changes without confusing them with unrelated dirty work."""
import argparse
import hashlib
import json
from pathlib import Path


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def files(directory):
    return {path.relative_to(directory).as_posix(): path for path in directory.rglob('*') if path.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--control-dir', type=Path, required=True)
    parser.add_argument('--candidate-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--candidate-options', default='{}')
    args = parser.parse_args()
    if args.output.exists(): raise FileExistsError('Preserve earlier change manifest')
    control, candidate = args.control_dir.resolve(), args.candidate_dir.resolve()
    before, after = files(control / 'source'), files(candidate / 'source')
    changes = []
    for name in sorted(before.keys() | after.keys()):
        old, new = sha(before[name]) if name in before else None, sha(after[name]) if name in after else None
        if old != new: changes.append({'path': name, 'control_sha256': old, 'candidate_sha256': new})
    evidence = {'schema': 1, 'control_directory': str(control), 'candidate_directory': str(candidate),
        'control_binary_sha256': sha(control / 'heavensgate.exe'),
        'candidate_binary_sha256': sha(candidate / 'heavensgate.exe'),
        'candidate_options': json.loads(args.candidate_options), 'changes': changes,
        'strength_validation': 'pending; manifests and diagnostics are not Elo evidence',
        'promotion': 'never automatic'}
    with args.output.open('x') as output: json.dump(evidence, output, indent=2)
    print(json.dumps(evidence, indent=2))


if __name__ == '__main__':
    main()
