"""Compiled-tuner CLI regression. Run only AFTER the timed pilot ends.

CTest supplies the freshly built tuner executable as the sole argument.
"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BINARY = None
ROOT = Path(__file__).resolve().parent.parent
FIXTURE = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|0.5\n'


class TexelContractTests(unittest.TestCase):
    def run_tuner(self, arguments):
        return subprocess.run([str(BINARY), *map(str, arguments)], cwd=ROOT,
                              capture_output=True, text=True, timeout=30)

    def test_training_requires_heldout_paths_and_explicit_outputs(self):
        result = self.run_tuner([0, .5, ROOT / 'tests/texel_parity_positions.txt'])
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('Training requires', result.stderr)

    def test_zero_epoch_model_exports_report_but_no_parameters(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            for split in ('train', 'validation', 'test'): (directory / f'{split}.txt').write_text(FIXTURE)
            candidate, report = directory / 'candidate.inc', directory / 'loss.json'
            arguments = [0, .5, directory / 'train.txt', '--validation', directory / 'validation.txt',
                         '--test', directory / 'test.txt', '--output', candidate, '--report', report]
            result = self.run_tuner(arguments)
            self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
            self.assertFalse(candidate.exists())
            summary = json.loads(report.read_text())
            self.assertFalse(summary['validation_improved'])
            self.assertFalse(summary['exported'])
            self.assertEqual(summary['epochs_completed'], 0)
            self.assertEqual(self.run_tuner(arguments).returncode, 1)  # Never overwrite evidence.

    def test_malformed_targets_fail_instead_of_being_skipped(self):
        with tempfile.TemporaryDirectory() as temporary:
            fixture = Path(temporary) / 'malformed.txt'
            for target in ('nan', '2', '0.5garbage'):
                fixture.write_text(FIXTURE.split('|')[0] + '|' + target + '\n')
                result = self.run_tuner([0, .5, fixture, '--parity-only'])
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)


if __name__ == '__main__':
    BINARY = Path(sys.argv.pop(1)).resolve()
    unittest.main()
