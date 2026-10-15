"""HCE tuner contract (tools/tune_hce.cpp). CTest passes the tuner binary.

- parity: the linear model reproduces the engine eval exactly, including under
  random parameter perturbations (checked by the binary itself);
- training: a short fit on labelled data writes both source files, keeps the
  PawnMG anchor, and does not increase validation error.
"""
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

BINARY = None
ROOT = Path(__file__).resolve().parent.parent


class HceTunerContract(unittest.TestCase):
    def run_tuner(self, *arguments, timeout=300):
        return subprocess.run([str(BINARY), *map(str, arguments)], cwd=ROOT, capture_output=True, text=True, timeout=timeout)

    def test_parity(self):
        result = self.run_tuner('--parity', ROOT / 'tests/texel_parity_positions.txt')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertLess(float(re.search(r'max \|model - engine\| = ([0-9.e+-]+)', result.stdout).group(1)), 1e-6)

    def test_short_training_run(self):
        with tempfile.TemporaryDirectory() as temporary:
            result = self.run_tuner('--tune', ROOT / 'data/quiet_positions.txt', '--limit', 3000,
                                    '--epochs', 200, '--val', 0.2, '--out', temporary)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            out = Path(temporary)
            params = (out / 'tuned_eval_params.inc').read_text()
            self.assertIn('p.pawn_mg = 100;', params)
            self.assertEqual((out / 'pst_tables.inc').read_text().count('PieceSquareTables::'), 12)
            start = float(re.search(r'val MSE ([0-9.e-]+)', result.stdout).group(1))
            best = float(re.search(r'best val MSE ([0-9.e-]+)', result.stdout).group(1))
            self.assertLessEqual(best, start)

    def test_rejects_unknown_arguments(self):
        self.assertEqual(self.run_tuner('--bogus').returncode, 2)


if __name__ == '__main__':
    BINARY = Path(sys.argv.pop(1)).resolve()
    unittest.main()
