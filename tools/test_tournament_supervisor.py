import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from run_supervised_tournament import ProgressWatch, supervise, suspend_gap


class SupervisorTests(unittest.TestCase):
    def test_long_search_not_mistaken_for_stall(self):
        watch = ProgressWatch(0, 60)
        event = {'event': 'search_begin', 'game': 1, 'ply': 22, 'expected_max_ms': 120000, 'fen': 'evidence'}
        watch.ingest(event, 10)
        self.assertFalse(watch.timed_out(250))
        self.assertTrue(watch.timed_out(256))
        self.assertEqual(watch.pending, event)
        watch.ingest({'event': 'search_end', 'game': 1, 'ply': 22}, 255)
        self.assertIsNone(watch.pending)
        self.assertFalse(watch.timed_out(300))

    def test_mismatched_reply_rejected(self):
        watch = ProgressWatch(0)
        watch.ingest({'event': 'search_begin', 'game': 1, 'ply': 2, 'expected_max_ms': 1000}, 1)
        with self.assertRaises(ValueError):
            watch.ingest({'event': 'search_end', 'game': 2, 'ply': 2}, 2)

    def test_suspend_not_wall_clock_change(self):
        self.assertEqual(suspend_gap((10, 8), (11, 9)), 0)
        self.assertEqual(suspend_gap((10, 8), (71, 9)), 60)

    @unittest.skipUnless(os.name == 'nt', 'Windows containment')
    def test_actual_exit_code_and_no_restart(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            status = supervise([sys.executable, '-c', 'import sys; print("fixture"); sys.exit(7)'], root,
                               root / 'test.pgn.events.jsonl', cwd=root, stall_seconds=5)
            self.assertEqual(status['exit_code'], 7, status)
            self.assertEqual(status['status'], 'exited')
            self.assertEqual((root / 'console.log').read_text().strip(), 'fixture')
            events = [json.loads(line) for line in (root / 'supervisor.jsonl').read_text().splitlines()]
            self.assertEqual(sum(event['event'] == 'started' for event in events), 1)
            with self.assertRaises(FileExistsError):
                supervise([sys.executable, '-c', 'pass'], root, root / 'test.pgn.events.jsonl', cwd=root)

    @unittest.skipUnless(os.name == 'nt', 'Windows containment')
    def test_watchdog_kills_only_owned_process_tree(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            # Child and grandchild deliberately hang. Both are contained in this
            # job; a separate unrelated fixture process must survive job closure.
            unrelated = subprocess.Popen([sys.executable, '-c', 'import time; time.sleep(30)'],
                                          creationflags=subprocess.CREATE_NO_WINDOW)
            try:
                script = ('import subprocess,sys,time; p=subprocess.Popen([sys.executable,"-c","import time; time.sleep(30)"]); '
                          'print(p.pid,flush=True); time.sleep(30)')
                status = supervise([sys.executable, '-c', script], root,
                                   root / 'test.pgn.events.jsonl', cwd=root, stall_seconds=1)
                self.assertEqual(status['status'], 'watchdog_timeout', status)
                self.assertIsNotNone(status['exit_code'])
                self.assertIsNone(unrelated.poll())
                import ctypes
                kernel = ctypes.WinDLL('kernel32', use_last_error=True)
                kernel.OpenProcess.argtypes = [ctypes.c_ulong, ctypes.c_int, ctypes.c_ulong]
                kernel.OpenProcess.restype = ctypes.c_void_p
                kernel.WaitForSingleObject.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
                kernel.CloseHandle.argtypes = [ctypes.c_void_p]
                descendant = int((root / 'console.log').read_text().strip())
                handle = kernel.OpenProcess(0x100000, 0, descendant)
                if handle:
                    try: self.assertEqual(kernel.WaitForSingleObject(handle, 1000), 0)
                    finally: kernel.CloseHandle(handle)
            finally:
                unrelated.kill()
                unrelated.wait(timeout=10)


if __name__ == '__main__':
    unittest.main()
