"""Small fixture-only regressions: no engine processes or real dataset scans."""
import json
from pathlib import Path
import tempfile
import unittest
from types import SimpleNamespace
from unittest.mock import patch

import chess
from extract_quiet_dataset import (RejectedGame, VerifiedGame, build_records, canonical_position,
                                  extract_positions, parse_verified_game, split_for_game)
from extract_quiet_dataset import file_hash
import run_texel_candidate as texel
from run_texel_candidate import validate_dataset

FOOLS_MATE = '[Event "Fixture"]\n[White "Local"]\n[Black "Baseline"]\n[Result "0-1"]\n\n1. f3 e5 2. g4 Qh4# 0-1 {Checkmate}\n'
FILTERS = dict(min_ply=1, max_ply=20, stride=1, min_pieces=0, strict_quiet=False)


def complete_fixture(directory):
    blocks = [FOOLS_MATE.replace('f3', first).replace(' e5 ', f' {second} ')
              for first in ('f3', 'f4') for second in ('e5', 'e6')]
    ids = [parse_verified_game(block, **FILTERS).game_id for block in blocks]
    seed = next(seed for seed in range(1000) if len({split_for_game(game, seed) for game in ids}) == 3)
    source = directory / 'fixtures.pgn'
    source.write_text('\n'.join(blocks))
    dataset = directory / 'dataset'
    extract_positions(dataset, [source], seed=seed, **FILTERS)
    return dataset, source


class ExtractionTests(unittest.TestCase):
    def test_san_and_coordinate_games_agree(self):
        san = parse_verified_game(FOOLS_MATE, **FILTERS)
        coordinate = parse_verified_game(FOOLS_MATE.replace('f3 e5 2. g4 Qh4#', 'f2f3 e7e5 2. g2g4 d8h4'), **FILTERS)
        self.assertEqual(san.game_id, coordinate.game_id)
        self.assertEqual(san.target, 0.0)
        self.assertTrue(san.samples)

    def test_no_partial_game_can_supply_samples(self):
        for block in (FOOLS_MATE.replace('Qh4#', 'a1a8'), FOOLS_MATE.replace('Qh4#', 'garbage'),
                      FOOLS_MATE.replace('2. g4 Qh4#', ''), FOOLS_MATE.replace('"0-1"', '"1-0"'),
                      FOOLS_MATE.replace('Checkmate', 'Time Out'), FOOLS_MATE.replace('Checkmate', 'Engine Failure'),
                      FOOLS_MATE.replace('Checkmate', 'Forced Mate in 4'),
                      FOOLS_MATE.replace('[Result', '[FEN "invalid"]\n[Result')):
            with self.subTest(block=block), self.assertRaises(RejectedGame):
                parse_verified_game(block, **FILTERS)

    def test_comments_and_variations_do_not_add_mainline_moves(self):
        decorated = FOOLS_MATE.replace('f3', 'f3 {a1a8 is not played} (1. e4 e5) $1', 1)
        self.assertEqual(parse_verified_game(decorated, **FILTERS).moves,
                         parse_verified_game(FOOLS_MATE, **FILTERS).moves)
        with self.assertRaises(RejectedGame):
            parse_verified_game(decorated.replace('}', ''), **FILTERS)

    def test_duplicate_games_and_mirrors_stay_in_one_partition(self):
        game = parse_verified_game(FOOLS_MATE, **FILTERS)
        duplicate = VerifiedGame('different-source-game', game.initial_fen, game.moves, 1.0, {},
                                 [chess.Board(fen).mirror().fen() for fen in game.samples])
        records, counts = build_records([(game, {}), (game, {}), (duplicate, {})], 42)
        self.assertEqual(counts['duplicate_games'], 1)
        groups = {}
        for record in records:
            groups.setdefault(record['position_group'], set()).add(record['split'])
        self.assertTrue(all(len(partitions) == 1 for partitions in groups.values()))
        self.assertEqual(len(records), 2 * len(game.samples))

    def test_conflicting_labels_are_excluded(self):
        game = parse_verified_game(FOOLS_MATE, **FILTERS)
        conflict = VerifiedGame('conflict', game.initial_fen, game.moves, 1.0, {}, game.samples)
        records, counts = build_records([(game, {}), (conflict, {})], 42)
        self.assertEqual(records, [])
        self.assertEqual(counts['conflicting_position_groups'], len(game.samples))

    def test_immutable_outputs_and_insufficient_holdouts(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)
            source = path / 'fixture.pgn'
            source.write_text(FOOLS_MATE)
            manifest = extract_positions(path / 'dataset', [source], **FILTERS)
            self.assertFalse(manifest['training_ready'])
            self.assertEqual(manifest['grandmaster_provenance'], 'not asserted')
            with self.assertRaises(FileExistsError): extract_positions(path / 'dataset', [source], **FILTERS)
            with self.assertRaises(ValueError): validate_dataset(path / 'dataset')

    def test_split_is_game_based_and_repeatable(self):
        first = [split_for_game(str(number), 123) for number in range(100)]
        self.assertEqual(first, [split_for_game(str(number), 123) for number in range(100)])
        self.assertEqual(set(first), {'train', 'validation', 'test'})

    def test_complete_partitions_have_checked_provenance_and_no_leakage(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary)
            dataset, source = complete_fixture(path)
            paths, manifest = validate_dataset(dataset)
            self.assertEqual(set(paths), {'train', 'validation', 'test'})
            self.assertTrue(manifest['training_ready'])
            source.write_text(source.read_text() + '\n')
            with self.assertRaises(ValueError): validate_dataset(dataset)

    def test_recomputed_hashes_do_not_hide_train_holdout_leakage(self):
        with tempfile.TemporaryDirectory() as temporary:
            dataset, _ = complete_fixture(Path(temporary))
            metadata_path, manifest_path = dataset / 'samples.jsonl', dataset / 'manifest.json'
            metadata = [json.loads(line) for line in metadata_path.read_text().splitlines()]
            train = next(record for record in metadata if record['split'] == 'train')
            index = next(index for index, record in enumerate(metadata) if record['split'] == 'validation')
            replacement = dict(train, split='validation', line=metadata[index]['line'])
            metadata[index] = replacement
            validation_path = dataset / 'validation.txt'
            lines = validation_path.read_text().splitlines()
            lines[replacement['line'] - 1] = f"{replacement['fen']}|{replacement['target']}"
            validation_path.write_text('\n'.join(lines) + '\n')
            metadata_path.write_text(''.join(json.dumps(record) + '\n' for record in metadata))
            manifest = json.loads(manifest_path.read_text())
            manifest['samples_sha256'] = file_hash(metadata_path)
            manifest['partitions']['validation']['sha256'] = file_hash(validation_path)
            manifest_path.write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'leakage'):
                validate_dataset(dataset)

    def test_wrapper_preserves_failure_and_requires_export_evidence(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            dataset, _ = complete_fixture(directory)
            binary = directory / 'fixture.exe'; binary.write_bytes(b'not an executable')
            for returncode, exported in ((1, False), (0, True), (2, False)):
                output_dir = directory / f'run-{returncode}'
                def fake_optimizer(command, **kwargs):
                    report = Path(command[command.index('--report') + 1])
                    report.write_text(json.dumps({'exported': exported}))
                    return SimpleNamespace(returncode=returncode, stdout='fixture log', stderr='')
                with patch.object(texel.subprocess, 'run', side_effect=fake_optimizer), patch('builtins.print'):
                    if returncode == 2:
                        self.assertEqual(texel.run_candidate(binary, dataset, output_dir, epochs=0), 2)
                    else:
                        with self.assertRaises(RuntimeError): texel.run_candidate(binary, dataset, output_dir, epochs=0)
                self.assertEqual((output_dir / 'optimizer.log').read_text(), 'fixture log')
                self.assertTrue((output_dir / 'run_manifest.json').exists())
                self.assertFalse((output_dir / 'candidate_eval_params.inc').exists())


if __name__ == '__main__':
    unittest.main()
