"""Fresh-only tournament launcher: durable exit/watchdog evidence, never restart.

Windows job containment affects only this launch and its child engine. A scoped
power request inhibits idle sleep, not explicit sleep, lid actions or shutdown.
Suspension invalidates timing evidence rather than being scored as flag fall.
"""
import argparse
import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time


def utc():
    return datetime.now(timezone.utc).isoformat()


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class WindowsRunScope:
    """Kill-on-close job; no system-wide power or process-list modifications."""
    def __enter__(self):
        if os.name != 'nt':
            raise RuntimeError('Supervised native tournament requires Windows')
        self.kernel = ctypes.WinDLL('kernel32', use_last_error=True)
        k = self.kernel
        k.CreateJobObjectW.argtypes = [ctypes.c_void_p, wintypes.LPCWSTR]
        k.CreateJobObjectW.restype = wintypes.HANDLE
        k.SetInformationJobObject.argtypes = [wintypes.HANDLE, ctypes.c_int, ctypes.c_void_p, wintypes.DWORD]
        k.AssignProcessToJobObject.argtypes = [wintypes.HANDLE, wintypes.HANDLE]
        k.TerminateJobObject.argtypes = [wintypes.HANDLE, wintypes.UINT]
        k.CloseHandle.argtypes = [wintypes.HANDLE]
        k.SetThreadExecutionState.argtypes = [wintypes.DWORD]
        k.SetThreadExecutionState.restype = wintypes.DWORD
        k.GetTickCount64.restype = ctypes.c_ulonglong
        k.QueryUnbiasedInterruptTime.argtypes = [ctypes.POINTER(ctypes.c_ulonglong)]
        class Basic(ctypes.Structure):
            _fields_ = [('process_time', ctypes.c_int64), ('job_time', ctypes.c_int64),
                ('flags', wintypes.DWORD), ('min_ws', ctypes.c_size_t), ('max_ws', ctypes.c_size_t),
                ('processes', wintypes.DWORD), ('affinity', ctypes.c_size_t),
                ('priority', wintypes.DWORD), ('scheduling', wintypes.DWORD)]
        class Io(ctypes.Structure):
            _fields_ = [(name, ctypes.c_ulonglong) for name in ('read_ops', 'write_ops', 'other_ops', 'read_bytes', 'write_bytes', 'other_bytes')]
        class Extended(ctypes.Structure):
            _fields_ = [('basic', Basic), ('io', Io), ('process_memory', ctypes.c_size_t),
                ('job_memory', ctypes.c_size_t), ('peak_process', ctypes.c_size_t), ('peak_job', ctypes.c_size_t)]
        self.job = k.CreateJobObjectW(None, None)
        self.previous_power = 0
        try:
            settings = Extended()
            settings.basic.flags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
            if not self.job or not k.SetInformationJobObject(self.job, 9, ctypes.byref(settings), ctypes.sizeof(settings)):
                raise ctypes.WinError(ctypes.get_last_error())
            self.previous_power = k.SetThreadExecutionState(0x80000001)  # CONTINUOUS | SYSTEM_REQUIRED
            if not self.previous_power:
                raise ctypes.WinError(ctypes.get_last_error())
        except BaseException:
            self.__exit__(None, None, None)
            raise
        return self

    def attach(self, child):
        if not self.kernel.AssignProcessToJobObject(self.job, wintypes.HANDLE(int(child._handle))):
            raise ctypes.WinError(ctypes.get_last_error())

    def sample(self):
        awake = ctypes.c_ulonglong()
        if not self.kernel.QueryUnbiasedInterruptTime(ctypes.byref(awake)):
            raise ctypes.WinError(ctypes.get_last_error())
        return self.kernel.GetTickCount64() / 1000.0, awake.value / 10000000.0

    def terminate(self):
        if not self.kernel.TerminateJobObject(self.job, 124):
            raise ctypes.WinError(ctypes.get_last_error())

    def __exit__(self, *_):
        if self.previous_power:
            self.kernel.SetThreadExecutionState(self.previous_power | 0x80000000)
        if self.job:
            self.kernel.CloseHandle(self.job)


def suspend_gap(before, after):
    return max(0.0, (after[0] - before[0]) - (after[1] - before[1]))


class ProgressWatch:
    def __init__(self, started, stall_seconds=60):
        self.stall_seconds = stall_seconds
        self.deadline = started + stall_seconds
        self.pending = None
        self.last = None

    def ingest(self, event, now):
        self.last = event
        if event['event'] == 'search_begin':
            self.pending = event
            expected = float(event['expected_max_ms']) / 1000.0
            self.deadline = now + max(self.stall_seconds, 2 * expected + 5)
        elif event['event'] == 'search_end':
            if self.pending and any(event.get(key) != self.pending.get(key) for key in ('game', 'ply')):
                raise ValueError('Search-end identity does not match outstanding search')
            self.pending = None
            self.deadline = now + self.stall_seconds
        else:
            self.deadline = max(self.deadline, now + self.stall_seconds)

    def timed_out(self, now):
        return now > self.deadline


def supervise(command, directory, events_path, *, cwd, stall_seconds=60):
    """directory must already be freshly reserved; stdout/stderr are exclusive."""
    directory, events_path = Path(directory), Path(events_path)
    status = {'schema': 1, 'started_utc': utc(), 'command': command, 'restart_policy': 'never',
              'status': 'launching', 'exit_code': None, 'pid': None}
    child = None
    with (directory / 'supervisor.jsonl').open('x', encoding='utf8') as evidence:
        def emit(event, **fields):
            evidence.write(json.dumps({'event': event, 'utc': utc(), **fields}) + '\n')
            evidence.flush()
        emit('launch', command=command)
        try:
            with WindowsRunScope() as scope, (directory / 'console.log').open('xb') as stdout, (directory / 'stderr.log').open('xb') as stderr:
                child = subprocess.Popen(command, cwd=cwd, stdin=subprocess.DEVNULL, stdout=stdout, stderr=stderr,
                    creationflags=subprocess.CREATE_NO_WINDOW)
                try:
                    scope.attach(child)
                except BaseException:
                    child.kill()
                    child.wait(timeout=10)
                    raise
                status['pid'] = child.pid
                emit('started', pid=child.pid, idle_sleep_inhibited=True, contained_in_job=True)
                watch = ProgressWatch(time.monotonic(), stall_seconds)
                previous = scope.sample()
                offset, incomplete = 0, ''
                while child.poll() is None:
                    time.sleep(.25)
                    now = time.monotonic()
                    sample = scope.sample()
                    gap = suspend_gap(previous, sample)
                    previous = sample
                    if events_path.exists():
                        with events_path.open(encoding='utf8') as events:
                            events.seek(offset)
                            chunk = events.read()
                            offset = events.tell()
                        lines = (incomplete + chunk).split('\n')
                        incomplete = lines.pop()
                        for line in lines:
                            if line:
                                watch.ingest(json.loads(line), now)
                    if gap > 2 or watch.timed_out(now):
                        status['status'] = 'system_suspend' if gap > 2 else 'watchdog_timeout'
                        status['outstanding_search'] = watch.pending
                        emit(status['status'], suspended_seconds=gap, outstanding_search=watch.pending)
                        scope.terminate()  # Only this owned job, never unrelated engines.
                        child.wait(timeout=10)
                        break
                status['exit_code'] = child.wait(timeout=10)
                status['last_event'] = watch.last
                status.setdefault('outstanding_search', watch.pending)
                if status['status'] == 'launching':
                    status['status'] = 'exited'
                run_status = Path(str(events_path).removesuffix('.events.jsonl') + '.run.json')
                if run_status.exists():
                    status['engine_run_status'] = json.loads(run_status.read_text())
                emit('exit', exit_code=status['exit_code'], status=status['status'])
        except BaseException as error:
            status.update(status='supervisor_failure', error=repr(error))
            if child is not None:
                if child.poll() is None:
                    child.kill()  # Job scope has already closed/killed its descendants.
                status['exit_code'] = child.wait(timeout=10)
            emit('failure', error=repr(error), exit_code=status['exit_code'])
        finally:
            status['ended_utc'] = utc()
            with (directory / 'supervisor.json').open('x', encoding='utf8') as output:
                json.dump(status, output, indent=2)
    return status


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--engine', type=Path, required=True)
    parser.add_argument('--opponent', type=Path, default=Path('tools/stockfish.exe'))
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--games', type=int, default=60)
    parser.add_argument('--bank-seconds', type=int, default=120)
    parser.add_argument('--increment-seconds', type=int, default=0)
    parser.add_argument('--elo', type=int, default=2400)
    parser.add_argument('--threads', type=int, default=6)
    parser.add_argument('--hash', type=int, default=64)
    parser.add_argument('--stall-seconds', type=float, default=60)
    args = parser.parse_args()
    if args.games < 2 or args.games % 2 or args.bank_seconds <= 0 or args.increment_seconds < 0 or not 1 <= args.threads <= 64 or not 1 <= args.hash <= 16384 or args.stall_seconds < 1:
        parser.error('Require even games, positive bank and watchdog, valid increment/threads/hash')
    directory = args.output_dir.resolve()
    engine, opponent = args.engine.resolve(), args.opponent.resolve()
    hashes = {'engine': {'path': str(engine), 'sha256': digest(engine)},
              'opponent': {'path': str(opponent), 'sha256': digest(opponent)}}
    directory.mkdir(parents=True, exist_ok=False)
    pgn = directory / 'match.pgn'
    command = [str(engine), 'tournament', str(args.games), str(args.bank_seconds), str(args.increment_seconds),
        '0', str(args.elo), str(args.threads), str(pgn), '--paired', '--book-off', f'--hash={args.hash}', f'--sf={opponent}']
    with (directory / 'launch.json').open('x') as output:
        json.dump({'schema': 1, 'started_utc': utc(), 'hashes': hashes, 'command': command,
            'supervisor_script': str(Path(__file__).resolve()), 'supervisor_sha256': digest(__file__),
            'python_executable': sys.executable, 'python_sha256': digest(sys.executable)}, output, indent=2)
    status = supervise(command, directory, Path(str(pgn) + '.events.jsonl'), cwd=Path(__file__).resolve().parent.parent,
                       stall_seconds=args.stall_seconds)
    status['binary_hashes_unchanged'] = digest(engine) == hashes['engine']['sha256'] and digest(opponent) == hashes['opponent']['sha256']
    with (directory / 'integrity.json').open('x') as output:
        json.dump({'hashes_unchanged': status['binary_hashes_unchanged']}, output)
    print(json.dumps(status, indent=2))
    # Completion with flags is deliberately nonzero; read run status to distinguish
    # a complete clock-failed experiment from an early exit or protocol failure.
    return 0 if status['status'] == 'exited' and status['exit_code'] == 0 and status['binary_hashes_unchanged'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
