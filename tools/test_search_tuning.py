"""Paired-game SPSA contract tests; mock matches, never launch engines."""
import json
from pathlib import Path
import random
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import chess
import paired_match as match
from paired_match import InvalidMatch, actual_outcome, outcome_score, paired_cases
import tune_search_spsa as spsa


def defaults():
    return {name: (definition['min'] + definition['max']) / 2 for name, definition in spsa.PARAM_DEFS.items()}


class SearchTuningTests(unittest.TestCase):
    def test_same_fen_is_played_in_both_colors(self):
        fens = [chess.STARTING_FEN, '8/8/8/1R6/K2k4/8/1r6/8 b - - 36 105']
        self.assertEqual(paired_cases(fens), [(fen, color) for fen in fens for color in (True, False)])

    def test_only_actual_board_endings_supply_scores(self):
        board = chess.Board()
        self.assertIsNone(actual_outcome(board))
        for uci in ('f2f3', 'e7e5', 'g2g4', 'd8h4'): board.push_uci(uci)
        outcome = actual_outcome(board)
        self.assertEqual(outcome_score(outcome, False), 1.0)
        self.assertEqual(outcome_score(outcome, True), 0.0)
        self.assertEqual(outcome_score(chess.Outcome(chess.Termination.STALEMATE, None), True), .5)

    def test_probe_update_uses_actual_clipped_and_rounded_span(self):
        parameters = defaults()
        parameters['singular_margin'] = 1.0
        plus, minus = spsa.probes(parameters, 1, random.Random(23))
        update = spsa.update_parameters(parameters, plus, minus, .6, .4, 1)
        for name, definition in spsa.PARAM_DEFS.items():
            self.assertTrue(definition['min'] <= update[name] <= definition['max'])
            if plus[name] != minus[name]:
                expected_direction = 1 if plus[name] > minus[name] else -1
                if definition['min'] < parameters[name] < definition['max']:
                    self.assertGreater((update[name] - parameters[name]) * expected_direction, 0)
        unchanged = spsa.update_parameters(parameters, parameters, parameters, .6, .4, 1)
        self.assertEqual(unchanged, parameters)

    def test_equal_scores_do_not_change_parameters(self):
        parameters = defaults()
        plus, minus = spsa.probes(parameters, 1, random.Random(3))
        self.assertEqual(spsa.update_parameters(parameters, plus, minus, .5, .5, 1), parameters)

    def test_candidate_cannot_override_match_policy(self):
        engine = SimpleNamespace(options={name: None for name in ('Threads', 'Hash', 'OwnBook')})
        for option in ('Threads', 'Hash', 'OwnBook', 'ownbook'):
            with self.subTest(option=option), self.assertRaises(InvalidMatch):
                match.configure_engine(engine, {option: True}, 1, 64)

    def test_binary_change_during_probe_rejects_fitness(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            binary = directory / 'fixture.exe'; binary.write_bytes(b'fixture')
            def changed_binary(*args, **kwargs):
                binary.write_bytes(b'changed fixture')
                return {'score': .5, 'games': [], 'time_forfeits': 0, 'protocol_failures': 0}
            with patch.object(spsa, 'default_parameters', return_value=defaults()), \
                    patch.object(spsa, 'run_paired_batch', side_effect=changed_binary):
                with self.assertRaisesRegex(RuntimeError, 'binary changed'):
                    spsa.run_spsa(binary, binary, [chess.STARTING_FEN], directory / 'run', iterations=1, pairs=1)
            self.assertFalse((directory / 'run/candidate_search_params.json').exists())

    def test_run_uses_common_control_openings_and_candidate_only_export(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            binary, control = directory / 'candidate.exe', directory / 'control.exe'
            binary.write_bytes(b'candidate'); control.write_bytes(b'control')
            result = {'score': .5, 'games': [], 'time_forfeits': 0, 'protocol_failures': 0}
            with patch.object(spsa, 'default_parameters', return_value=defaults()), \
                    patch.object(spsa, 'run_paired_batch', return_value=result) as run:
                output = spsa.run_spsa(binary, control, [chess.STARTING_FEN], directory / 'run', iterations=1, pairs=1)
            self.assertEqual(run.call_count, 2)
            self.assertEqual(run.call_args_list[0].args[1], run.call_args_list[1].args[1])
            self.assertEqual(run.call_args_list[0].args[3], run.call_args_list[1].args[3])
            self.assertTrue((directory / 'run/candidate_search_params.json').exists())
            self.assertIn('pending', output['strength_validation'])
            with self.assertRaises(FileExistsError):
                spsa.run_spsa(binary, control, [chess.STARTING_FEN], directory / 'run')

    def test_failed_probe_preserves_evidence_and_exports_nothing(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            binary = directory / 'fixture.exe'; binary.write_bytes(b'fixture')
            with patch.object(spsa, 'default_parameters', return_value=defaults()), \
                    patch.object(spsa, 'run_paired_batch', side_effect=InvalidMatch('clock loss')):
                with self.assertRaises(InvalidMatch):
                    spsa.run_spsa(binary, binary, [chess.STARTING_FEN], directory / 'run', iterations=1, pairs=1)
            self.assertFalse((directory / 'run/candidate_search_params.json').exists())
            failure = json.loads((directory / 'run/iterations.jsonl').read_text())
            self.assertEqual(failure['status'], 'failed')

    def fake_match(self, directory, *, bad_move=False, crash=False, slow=False):
        class Engine:
            options = {name: None for name in ('Threads', 'Hash', 'OwnBook')}
            def configure(self, options): self.configuration = options
            def close(self): pass
            def play(self, board, limit, **kwargs):
                if crash: raise chess.engine.EngineTerminatedError('fixture crash')
                uci = 'e2e5' if bad_move else ('f2f3', 'e7e5', 'g2g4', 'd8h4')[len(board.move_stack)]
                return SimpleNamespace(move=chess.Move.from_uci(uci), info={'depth': 1, 'nodes': 1})
        binary = directory / 'fixture.exe'; binary.write_bytes(b'fixture')
        stamps = [0.0, 1.2] if slow else [number / 1000 for number in range(16)]
        with patch.object(match.chess.engine.SimpleEngine, 'popen_uci', side_effect=lambda *a, **k: Engine()), \
                patch.object(match.time, 'perf_counter', side_effect=stamps):
            return match.run_paired_batch(binary, binary, {}, [chess.STARTING_FEN], directory / 'match',
                                          bank_ms=1000, increment_ms=2000)

    def test_mock_uci_match_reaches_real_mates_in_both_colors(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            result = self.fake_match(directory)
            self.assertEqual(result['score'], .5)
            self.assertEqual(len(result['games']), 2)
            rows = [json.loads(line) for line in (directory / 'match.moves.jsonl').read_text().splitlines()]
            self.assertEqual(len(rows), 8)
            self.assertTrue(all(row['played'] and row['status'] == 'ok' for row in rows))

    def test_mock_match_rejects_illegal_crashed_and_expired_replies(self):
        for flags, status in (({'bad_move': True}, 'illegal-move'), ({'crash': True}, 'protocol-failure'),
                              ({'slow': True}, 'time-forfeit')):
            with self.subTest(status=status), tempfile.TemporaryDirectory() as temporary:
                directory = Path(temporary)
                with self.assertRaises(InvalidMatch): self.fake_match(directory, **flags)
                row = json.loads((directory / 'match.moves.jsonl').read_text())
                self.assertEqual(row['status'], status)
                self.assertFalse(row['played'])  # Even a 2s increment cannot rescue an expired 1s bank.


if __name__ == '__main__':
    unittest.main()
