"""Measurement contracts using synthetic artifacts only; never starts engines."""
import json
import math
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import chess
import analyse_paired_match as analysis
import paired_match as match


class PairedStatisticsTests(unittest.TestCase):
    def test_known_pentanomial_moments(self):
        stats = analysis.pair_statistics([1, 2, 3, 2, 1])
        self.assertEqual(stats["pairs"], 9)
        self.assertEqual(stats["games"], 18)
        self.assertEqual(stats["candidate_score"], .5)
        self.assertAlmostEqual(stats["pair_sample_variance"], .75 / 8)
        self.assertAlmostEqual(stats["pair_standard_error"], math.sqrt(.75 / 72))
        self.assertEqual(stats["logistic_elo_difference_descriptive"], 0)

    def test_pairs_not_individual_games_define_uncertainty(self):
        stats = analysis.pair_statistics([1, 0, 0, 0, 1])
        self.assertEqual(stats["pair_standard_error"], .5)

    def test_saturation_never_returns_finite_cap(self):
        for bins, score in (([100, 0, 0, 0, 0], 0), ([0, 0, 0, 0, 100], 1)):
            stats = analysis.pair_statistics(bins)
            self.assertEqual(stats["candidate_score"], score)
            self.assertIsNone(stats["logistic_elo_difference_descriptive"])
            self.assertIsNone(stats["conditional_intervals"]["normal_score_interval"])
            lower, upper = stats["conditional_intervals"]["hoeffding_score_interval"]
            self.assertLess(lower, upper)
            self.assertIn(None, stats["conditional_intervals"]["hoeffding_elo_interval"])

    def test_zero_variance_and_one_pair_are_not_precise_equality(self):
        for count in (1, 100):
            stats = analysis.pair_statistics([0, 0, count, 0, 0])
            interval = stats["conditional_intervals"]["hoeffding_score_interval"]
            self.assertLess(interval[0], .5)
            self.assertGreater(interval[1], .5)
            self.assertIsNone(stats["conditional_intervals"]["normal_score_interval"])
        self.assertIsNone(analysis.pair_statistics([0, 0, 1, 0, 0])["pair_sample_variance"])

    def test_conditional_normal_interval_for_large_nonzero_variance(self):
        stats = analysis.pair_statistics([100, 200, 300, 200, 100])
        lower, upper = stats["conditional_intervals"]["normal_score_interval"]
        self.assertLess(lower, .5)
        self.assertGreater(upper, .5)
        self.assertAlmostEqual(lower + upper, 1)
        lo, hi = stats["conditional_intervals"]["normal_elo_interval"]
        self.assertAlmostEqual(lo + hi, 0)

    def test_invalid_counts_rejected(self):
        for bins in ([0] * 5, [1, 0], [-1, 0, 0, 0, 2], [True, 0, 0, 0, 0], [1.5, 0, 0, 0, 0]):
            with self.subTest(bins=bins), self.assertRaises(match.InvalidMatch):
                analysis.pair_statistics(bins)

    def fixture(self, directory):
        class Engine:
            options = {name: None for name in ("Threads", "Hash", "OwnBook")}
            def configure(self, options): pass
            def close(self): pass
            def play(self, board, limit, **kwargs):
                move = chess.Move.from_uci(("f2f3", "e7e5", "g2g4", "d8h4")[len(board.move_stack)])
                return SimpleNamespace(move=move, info={"depth": 1, "nodes": 1})
        binary = directory / "fixture.exe"; binary.write_bytes(b"fixture, not executable")
        prefix = directory / "match"
        with patch.object(match.chess.engine.SimpleEngine, "popen_uci", side_effect=lambda *args, **kwargs: Engine()), \
                patch.object(match.time, "perf_counter", side_effect=[n / 1000 for n in range(16)]):
            result = match.run_paired_batch(binary, binary, {}, [chess.STARTING_FEN], prefix,
                                            bank_ms=1000, increment_ms=100)
        Path(str(prefix) + ".summary.json").write_text(json.dumps(result))
        return prefix

    def test_verified_synthetic_match_has_one_pair_not_strength_evidence(self):
        with tempfile.TemporaryDirectory() as temporary:
            report = analysis.analyse(self.fixture(Path(temporary)))
            self.assertEqual(report["verification"], "passed")
            self.assertEqual(report["played_plies_verified"], 8)
            self.assertEqual(report["statistics"]["pentanomial"], [0, 0, 1, 0, 0])
            self.assertFalse(report["predeclared_stopping_rule_verified"])

    def test_fail_closed_on_broken_artifacts(self):
        mutations = {
            "incomplete": ("summary", lambda data: data["games"].pop()),
            "unpaired": ("manifest", lambda data: data.update(paired=False)),
            "flags": ("summary", lambda data: data.update(time_forfeits=1)),
            "protocol": ("summary", lambda data: data.update(protocol_failures=1)),
            "wrong_color": ("summary", lambda data: data["games"][1].update(candidate_white=True)),
            "wrong_result": ("summary", lambda data: data["games"][0].update(result="1-0")),
            "wrong_points": ("summary", lambda data: data["games"][0].update(candidate_points=1)),
            "wrong_hash": ("manifest", lambda data: data.update(candidate_sha256="0" * 64)),
            "duplicate_opening": ("manifest", lambda data: data["fens"].append(data["fens"][0])),
        }
        for name, (suffix, mutate) in mutations.items():
            with self.subTest(case=name), tempfile.TemporaryDirectory() as temporary:
                prefix = self.fixture(Path(temporary))
                path = Path(str(prefix) + f".{suffix}.json")
                data = json.loads(path.read_text()); mutate(data); path.write_text(json.dumps(data))
                with self.assertRaises(match.InvalidMatch): analysis.analyse(prefix)

    def test_illegal_missing_extra_and_late_telemetry_rejected(self):
        for mode in ("illegal", "missing", "extra", "late", "wrong_bank", "wrong_history"):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as temporary:
                prefix = self.fixture(Path(temporary))
                path = Path(str(prefix) + ".moves.jsonl")
                rows = [json.loads(line) for line in path.read_text().splitlines()]
                if mode == "missing": rows.pop()
                elif mode == "extra": rows.append(rows[-1])
                elif mode == "illegal": rows[0]["move"] = "f2f5"
                elif mode == "late": rows[0]["elapsed_ms"] = rows[0]["clock_before_ms"]
                elif mode == "wrong_bank": rows[0]["requested_clocks_ms"]["white"] += 1
                else: rows[0]["fen"] = chess.STARTING_FEN.replace(" w ", " b ")
                path.write_text("\n".join(json.dumps(row) for row in rows) + "\n")
                with self.assertRaises(match.InvalidMatch): analysis.analyse(prefix)

    def test_duplicate_headers_and_footer_results_rejected(self):
        for mode in ("duplicate_header", "wrong_footer", "missing_footer"):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as temporary:
                prefix = self.fixture(Path(temporary))
                path = Path(str(prefix) + ".pgn")
                text = path.read_text()
                if mode == "duplicate_header": text = text.replace('[Result "0-1"]', '[Result "0-1"]\n[Result "0-1"]', 1)
                elif mode == "wrong_footer": text = text.replace("Qh4# 0-1", "Qh4# 1-0", 1)
                else: text = text.replace("Qh4# 0-1", "Qh4#", 1)
                path.write_text(text)
                with self.assertRaises(match.InvalidMatch): analysis.analyse(prefix)


if __name__ == "__main__":
    unittest.main()
