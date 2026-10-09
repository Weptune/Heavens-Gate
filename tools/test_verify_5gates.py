"""Fail-closed protocol regression tests; no real engine processes are launched."""
import contextlib
import io
from pathlib import Path
import subprocess
import unittest
import json
from unittest.mock import patch
import verify_5gates as gates


class GateEvidenceTests(unittest.TestCase):
    def quiet_call(self, function):
        with contextlib.redirect_stdout(io.StringIO()):
            return function()

    def test_perft_requests_depth_six_and_checks_exit(self):
        evidence = "[Testing] reference\n" * 6
        evidence += "Depth 6: 119060324 / 119060324 nodes [PASSED]\nVERIFICATION SUITE RESULT: ALL PASSED\n"
        for code, stdout, expected in [(0, evidence, True), (1, evidence, False),
                                       (0, "VERIFICATION SUITE RESULT: ALL PASSED", False)]:
            with self.subTest(code=code, expected=expected), patch.object(gates.os.path, "exists", return_value=True), \
                    patch.object(gates.subprocess, "run", return_value=subprocess.CompletedProcess([], code, stdout, "")) as run:
                self.assertEqual(self.quiet_call(gates.run_gate3_perft), expected)
                self.assertEqual(run.call_args.args[0][1:], ["perft", "6"])

    def test_match_requires_process_and_completed_pgn_evidence(self):
        completed = "[TOURNAMENT] Game 1/1 (30 moves, Checkmate)\nTOURNAMENT RESULTS\n"
        valid_pgn = '[FEN "7k/8/5KQ1/8/8/8/8/8 w - - 0 1"]\n[Result "1-0"]\n\n1. g6g7 1-0 {Checkmate}\n'
        cases = [(0, completed, valid_pgn, True), (1, completed, valid_pgn, False),
                 (0, "", valid_pgn, False), (0, completed, None, False),
                 (0, completed, '[Result "*"]\n', False),
                 (0, completed, '[Result "0-1"]\n\n1-0 {Checkmate}\n', False),
                 (0, completed + "[GAME FORFEIT]", valid_pgn, False),
                 (0, completed + "Time Out", valid_pgn, False)]
        for code, stdout, pgn, expected in cases:
            def fake_run(command, **kwargs):
                if pgn is not None:
                    Path(command[-1]).write_text(pgn, encoding="utf-8")
                    Path(command[-1] + ".manifest.json").write_text(json.dumps({"engine_sha256": "a"*64,
                        "opponent_sha256": "a"*64, "opponent_path": "fixture.exe", "source_sha256": "b"*64,
                        "adjudication": "board-terminal-or-rule-draw-only"}), encoding="utf-8")
                    Path(command[-1] + ".moves.jsonl").write_text('{"status":"ok"}\n', encoding="utf-8")
                return subprocess.CompletedProcess(command, code, stdout, "")
            with self.subTest(code=code, expected=expected), patch.object(gates.subprocess, "run", side_effect=fake_run), \
                    patch.object(gates, "sha256_file", return_value="a"*64):
                self.assertEqual(self.quiet_call(gates.run_gate5_stability), expected)

    def test_match_timeout_fails(self):
        with patch.object(gates.subprocess, "run", side_effect=subprocess.TimeoutExpired("match", 120)):
            self.assertFalse(self.quiet_call(gates.run_gate5_stability))

    def test_reported_mate_in_nonterminal_position_is_not_evidence(self):
        self.assertFalse(gates.verify_terminal_pgn('[FEN "' +
            'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"]\n' +
            '[Result "1-0"]\n\n1. e2e4 {[%eval #6]} 1-0 {Forced Mate in 6 moves}\n'))

    def test_legal_moves_are_required_for_terminal_result(self):
        self.assertFalse(gates.verify_terminal_pgn('[FEN "7k/8/5KQ1/8/8/8/8/8 w - - 0 1"]\n' +
            '[Result "1-0"]\n\n1. g6h8 1-0 {Checkmate}\n'))

    def test_missing_tactical_report_fails_and_uses_candidate_binary(self):
        responses = [subprocess.CompletedProcess([], 0, "OVERALL TOTAL 800 4400 / 8000 55.00%", ""),
                     subprocess.CompletedProcess([], 0, "Missing BK report", "")]
        with patch.object(gates.subprocess, "run", side_effect=responses) as run:
            self.assertFalse(self.quiet_call(gates.run_gate4_benchmarks))
            self.assertEqual(run.call_args.kwargs["env"]["HG_ENGINE"], gates.ENGINE_EXE)


if __name__ == "__main__":
    unittest.main()
