import http.server
import socketserver
import json
import subprocess
import os
import sys
import threading
import math
import re
import glob
import time

PORT = 8000
REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def find_engine():
    """Pick the engine binary: HG_ENGINE if set, else the CMake build output, else the
    repo-root copy. The repo-root heavensgate.exe is often a stale hand-copied build,
    so the fresh build in build/ wins when both exist."""
    env = os.environ.get("HG_ENGINE")
    if env:
        return os.path.abspath(env)
    names = ["heavensgate.exe", "heavensgate"]
    for d in (os.path.join(REPO_DIR, "build"), REPO_DIR):
        for n in names:
            p = os.path.join(d, n)
            if os.path.isfile(p):
                return p
    return os.path.join(REPO_DIR, "build", "heavensgate.exe")


def warn_if_stale(engine_path):
    """Warn when any engine source file is newer than the binary being served."""
    if not os.path.isfile(engine_path):
        return
    built = os.path.getmtime(engine_path)
    newest = max((os.path.getmtime(f) for f in glob.glob(os.path.join(REPO_DIR, "src", "**", "*.*pp"), recursive=True)), default=0)
    stamp = time.strftime("%Y-%m-%d %H:%M", time.localtime(built))
    print(f"[ENGINE BRIDGE] Engine binary: {engine_path} (built {stamp})")
    if newest > built:
        print("[ENGINE BRIDGE] WARNING: engine sources are newer than this binary; rebuild with "
              "'cmake --build build' so the site plays the current engine.")


ENGINE_PATH = find_engine()

class EngineBridge:
    def __init__(self, engine_path):
        self.engine_path = engine_path
        self.process = None
        self.lock = threading.Lock()
        self.start_engine()

    def start_engine(self):
        if not os.path.exists(self.engine_path):
            print(f"[ERROR] Engine binary not found at: {self.engine_path}")
            return
        
        try:
            engine_abs_path = os.path.abspath(self.engine_path)
            engine_dir = os.path.dirname(engine_abs_path)
            self.process = subprocess.Popen(
                [engine_abs_path, "uci"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                cwd=engine_dir
            )
            # Drain startup output and strictly synchronize UCI protocol
            self._send_command("uci")
            while True:
                line = self.process.stdout.readline()
                if not line or line.strip() == "uciok":
                    break
            # The engine's UCI defaults are now Threads=1 and OwnBook=false (tester
            # convention). Pin the arena's previous settings explicitly.
            self._send_command("setoption name Threads value 6")
            self._send_command("setoption name OwnBook value true")
            self._send_command("isready")
            while True:
                line = self.process.stdout.readline()
                if not line or line.strip() == "readyok":
                    break
            print(f"[ENGINE BRIDGE] Successfully launched and synchronized Heaven's Gate UCI Process (PID {self.process.pid}) in {engine_dir}")
        except Exception as e:
            print(f"[ERROR] Failed to start engine process: {e}")

    def _send_command(self, cmd):
        if "\n" in cmd or "\r" in cmd:
            raise ValueError("engine command must be a single line")
        if self.process and self.process.stdin:
            self.process.stdin.write(cmd + "\n")
            self.process.stdin.flush()

    def stop_engine(self):
        with self.lock:
            if self.process:
                try:
                    self._send_command("quit")
                    self.process.terminate()
                    self.process.wait(timeout=1.0)
                except Exception:
                    pass
                self.process = None

    def get_move(self, fen, depth=8, movetime=0, wtime=0, btime=0, winc=0, binc=0, moves=None):
        with self.lock:
            if not self.process or self.process.poll() is not None:
                self.start_engine()

            if not self.process:
                return {"error": "Engine process not available"}

            # With the move list the engine sees the game history and can avoid (or aim
            # for) repetitions; both boards in the web app start from the initial position.
            if moves:
                self._send_command("position startpos moves " + " ".join(moves))
            else:
                self._send_command(f"position fen {fen}")
            if movetime > 0:
                self._send_command(f"go movetime {movetime}")
            elif wtime > 0 and btime > 0:
                self._send_command(f"go wtime {wtime} btime {btime} winc {winc} binc {binc}")
            else:
                self._send_command(f"go depth {depth}")

            best_move = None
            eval_score = 0
            is_mate = False
            mate_in = 0
            nodes = 0
            nps = 0
            pv = ""
            completed_depth = depth
            time_ms = 0
            hashfull = 0

            while True:
                line = self.process.stdout.readline()
                if not line:
                    break
                line = line.strip()
                if line.startswith("info"):
                    tokens = line.split()
                    for i in range(len(tokens)):
                        if tokens[i] == "depth" and i + 1 < len(tokens):
                            try:
                                completed_depth = int(tokens[i+1])
                            except ValueError:
                                pass
                        elif tokens[i] == "score" and i + 2 < len(tokens):
                            if tokens[i+1] == "cp":
                                eval_score = int(tokens[i+2])
                                is_mate = False
                            elif tokens[i+1] == "mate":
                                mate_in = int(tokens[i+2])
                                is_mate = True
                                eval_score = 29000 if mate_in > 0 else -29000
                        elif tokens[i] == "nodes" and i + 1 < len(tokens):
                            try:
                                nodes = int(tokens[i+1])
                            except ValueError:
                                pass
                        elif tokens[i] == "nps" and i + 1 < len(tokens):
                            try:
                                nps = int(tokens[i+1])
                            except ValueError:
                                pass
                        elif tokens[i] == "time" and i + 1 < len(tokens):
                            try:
                                time_ms = int(tokens[i+1])
                            except ValueError:
                                pass
                        elif tokens[i] == "hashfull" and i + 1 < len(tokens):
                            try:
                                hashfull = int(tokens[i+1])
                            except ValueError:
                                pass
                        elif tokens[i] == "pv":
                            pv = " ".join(tokens[i+1:])
                elif line.startswith("bestmove"):
                    parts = line.split()
                    if len(parts) >= 2:
                        best_move = parts[1]
                    break

            fen_parts = fen.strip().split()
            side_to_move = fen_parts[1] if len(fen_parts) > 1 else 'w'

            # Standard Negamax inversion: UCI score is relative to side_to_move.
            # Convert to White's perspective (+ = White advantage, - = Black advantage)
            if side_to_move == 'b':
                score_white = -eval_score
                mate_in_white = -mate_in if is_mate else 0
            else:
                score_white = eval_score
                mate_in_white = mate_in if is_mate else 0

            # Standard logistic chess win probability formula relative to White
            if is_mate:
                win_chance_white = 100.0 if mate_in_white > 0 else 0.0
            else:
                win_chance_white = 50.0 + 50.0 * (2.0 / (1.0 + math.exp(-0.00368208 * score_white)) - 1.0)
                win_chance_white = max(0.0, min(100.0, win_chance_white))

            if not pv and best_move:
                pv = best_move

            return {
                "best_move": best_move,
                "score": eval_score,
                "score_white": score_white,
                "is_mate": is_mate,
                "mate_in": mate_in,
                "mate_in_white": mate_in_white,
                "side_to_move": side_to_move,
                "depth": completed_depth,
                "nodes": nodes,
                "nps": nps,
                "time_ms": time_ms,
                "hashfull": hashfull,
                "win_chance": round(win_chance_white, 1),
                "pv": pv
            }

import atexit
warn_if_stale(ENGINE_PATH)
bridge = EngineBridge(ENGINE_PATH)
atexit.register(bridge.stop_engine)

def parse_pgn_file(file_path):
    if not os.path.exists(file_path):
        return None
    
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    raw_games = content.split('[Event ')
    games = []
    
    for rg in raw_games:
        if not rg.strip():
            continue
        full = '[Event ' + rg
        
        round_m = re.search(r'\[Round\s+\"(\d+)\"\]', full)
        white_m = re.search(r'\[White\s+\"([^\"]+)\"\]', full)
        black_m = re.search(r'\[Black\s+\"([^\"]+)\"\]', full)
        fen_m = re.search(r'\[FEN\s+\"([^\"]+)\"\]', full)
        date_m = re.search(r'\[Date\s+\"([^\"]+)\"\]', full)
        
        if not round_m:
            continue
            
        round_num = int(round_m.group(1))
        white_name = white_m.group(1) if white_m else "White"
        black_name = black_m.group(1) if black_m else "Black"
        start_fen = fen_m.group(1) if fen_m else "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        date_str = date_m.group(1) if date_m else ""
        
        res_m = re.search(r'(1-0|0-1|1/2-1/2|\*)\s*\{([^}]*)\}', full)
        if not res_m:
            res_m = re.search(r'(1-0|0-1|1/2-1/2|\*)', full.split(']')[-1])
            result = res_m.group(1) if res_m else "*"
            term = ""
        else:
            result = res_m.group(1)
            term = res_m.group(2).strip()
            
        move_regex = re.compile(r'([a-h][1-8][a-h][1-8][qrbn]?)\s*\{\s*\[%eval\s*(-?\d+)\]\s*\[%clk\s*([\d\.]+)ms\]\s*\}')
        moves = []
        for match in move_regex.finditer(full):
            uci = match.group(1)
            ev = int(match.group(2))
            clk = float(match.group(3))
            turn = 'w' if len(moves) % 2 == 0 else 'b'
            moves.append({
                'ply': len(moves) + 1,
                'uci': uci,
                'eval': ev,
                'clk_ms': clk,
                'turn': turn
            })
            
        games.append({
            'round': round_num,
            'white': white_name,
            'black': black_name,
            'date': date_str,
            'fen': start_fen,
            'result': result,
            'termination': term,
            'move_count': len(moves),
            'moves': moves
        })
        
    master_wins = sum(1 for g in games if (('Master' in g['white'] and g['result'] == '1-0') or ('Master' in g['black'] and g['result'] == '0-1')))
    sf_wins = sum(1 for g in games if (('Stockfish' in g['white'] and g['result'] == '1-0') or ('Stockfish' in g['black'] and g['result'] == '0-1')))
    draws = sum(1 for g in games if g['result'] == '1/2-1/2')
    
    return {
        'games': games,
        'stats': {
            'total_games': len(games),
            'master_wins': master_wins,
            'stockfish_wins': sf_wins,
            'draws': draws,
            'score_master': master_wins + 0.5 * draws,
            'score_stockfish': sf_wins + 0.5 * draws,
            'win_rate': round((master_wins + 0.5 * draws) / max(1, len(games)) * 100, 1)
        }
    }

START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
MAX_BODY_BYTES = 64 * 1024
MAX_MOVES = 1000
UCI_MOVE_RE = re.compile(r"[a-h][1-8][a-h][1-8][qrbn]?")
RANK_RE = re.compile(r"[pnbrqkPNBRQK1-8]{1,8}")


class BadRequest(Exception):
    """Client error with an HTTP status code."""
    def __init__(self, message, status=400):
        super().__init__(message)
        self.status = status


def validate_fen(fen):
    """Return a normalised 6-field FEN or raise BadRequest. Only the characters a FEN
    can contain get through, so nothing else can reach the engine's command line."""
    if not isinstance(fen, str) or len(fen) > 100:
        raise BadRequest("fen must be a string of at most 100 characters")
    fields = fen.split()
    if len(fields) == 4:
        fields += ["0", "1"]
    if len(fields) != 6:
        raise BadRequest("fen must have 6 fields")
    board, side, castling, ep, half, full = fields
    ranks = board.split("/")
    if len(ranks) != 8 or not all(RANK_RE.fullmatch(r) for r in ranks):
        raise BadRequest("fen board is malformed")
    for r in ranks:
        if sum(int(ch) if ch.isdigit() else 1 for ch in r) != 8:
            raise BadRequest("every fen rank must hold 8 squares")
    if board.count("K") != 1 or board.count("k") != 1:
        raise BadRequest("fen needs exactly one king per side")
    if side not in ("w", "b"):
        raise BadRequest("fen side to move must be w or b")
    if not re.fullmatch(r"-|K?Q?k?q?", castling) or castling == "":
        raise BadRequest("fen castling field is malformed")
    if not re.fullmatch(r"-|[a-h][36]", ep):
        raise BadRequest("fen en-passant field is malformed")
    if not (half.isdigit() and full.isdigit() and int(half) <= 999 and 1 <= int(full) <= 9999):
        raise BadRequest("fen move counters are out of range")
    return " ".join([board, side, castling, ep, half, full])


def validate_moves(moves):
    if moves is None:
        return []
    if not isinstance(moves, list) or len(moves) > MAX_MOVES:
        raise BadRequest(f"moves must be a list of at most {MAX_MOVES} UCI moves")
    for m in moves:
        if not isinstance(m, str) or not UCI_MOVE_RE.fullmatch(m):
            raise BadRequest("every move must be in UCI form, e.g. e2e4 or e7e8q")
    return moves


def int_field(req, name, default, lo, hi):
    """Integer field clamped to [lo, hi]; strings, floats and booleans are rejected."""
    v = req.get(name, default)
    if isinstance(v, bool) or not isinstance(v, int):
        raise BadRequest(f"{name} must be an integer")
    return max(lo, min(hi, v))


def parse_engine_request(req, default_depth):
    if not isinstance(req, dict):
        raise BadRequest("request body must be a JSON object")
    return {
        "fen": validate_fen(req.get("fen", START_FEN)),
        "moves": validate_moves(req.get("moves")),
        "depth": int_field(req, "depth", default_depth, 1, 20),
        "movetime": int_field(req, "movetime", 0, 0, 30_000),
        "wtime": int_field(req, "wtime", 0, 0, 3 * 3600 * 1000),
        "btime": int_field(req, "btime", 0, 0, 3 * 3600 * 1000),
        "winc": int_field(req, "winc", 0, 0, 60_000),
        "binc": int_field(req, "binc", 0, 0, 60_000),
    }


class ChessRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        web_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=web_dir, **kwargs)

    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

    def send_json(self, status, obj):
        body = json.dumps(obj).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def read_json_body(self):
        length = self.headers.get("Content-Length")
        if length is None or not length.strip().isdigit():
            raise BadRequest("Content-Length header is required", 411)
        length = int(length)
        if length > MAX_BODY_BYTES:
            raise BadRequest("request body is too large", 413)
        try:
            return json.loads(self.rfile.read(length).decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            raise BadRequest("request body is not valid JSON")

    def do_POST(self):
        if self.path not in ("/api/move", "/api/analyze"):
            self.send_error(404)
            return
        try:
            req = self.read_json_body()
            if self.path == "/api/move":
                p = parse_engine_request(req, default_depth=8)
                res = bridge.get_move(p["fen"], depth=p["depth"], movetime=p["movetime"], wtime=p["wtime"],
                                      btime=p["btime"], winc=p["winc"], binc=p["binc"], moves=p["moves"])
            else:
                p = parse_engine_request(req, default_depth=10)
                res = bridge.get_move(p["fen"], depth=p["depth"], movetime=p["movetime"], moves=p["moves"])
        except BadRequest as e:
            self.send_json(e.status, {"error": str(e)})
            return
        self.send_json(200 if "error" not in res else 503, res)

    def do_GET(self):
        from urllib.parse import urlparse, parse_qs
        parsed_url = urlparse(self.path)
        path = parsed_url.path
        query = parse_qs(parsed_url.query)

        if path == "/api/tournaments":
            # List all tournament batches available in REPO_DIR
            pattern = os.path.join(REPO_DIR, "tournament_results_batch*.pgn")
            files = glob.glob(pattern)
            batches = []
            import time
            now = time.time()
            for f in sorted(files, key=lambda x: os.path.getmtime(x), reverse=True):
                fname = os.path.basename(f)
                m = re.search(r'batch(\d+)', fname)
                batch_id = int(m.group(1)) if m else 0
                size = os.path.getsize(f)
                mtime = os.path.getmtime(f)
                is_active = (now - mtime < 60) # modified in last 60 seconds
                
                # Fast parse stats
                parsed = parse_pgn_file(f)
                stats = parsed['stats'] if parsed else {'total_games': 0, 'master_wins': 0, 'stockfish_wins': 0, 'draws': 0, 'score_master': 0, 'score_stockfish': 0, 'win_rate': 0}
                
                batches.append({
                    'batch_id': batch_id,
                    'filename': fname,
                    'size_bytes': size,
                    'last_modified': mtime,
                    'is_active': is_active,
                    'stats': stats
                })
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(batches).encode('utf-8'))

        elif path == "/api/tournament":
            batch_id = query.get('batch', ['8'])[0]
            fname = f"tournament_results_batch{batch_id}.pgn"
            fpath = os.path.join(REPO_DIR, fname)
            parsed = parse_pgn_file(fpath)
            if not parsed:
                self.send_response(404)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'error': f'Batch {batch_id} not found'}).encode('utf-8'))
                return
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(parsed).encode('utf-8'))

        else:
            super().do_GET()

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

if __name__ == "__main__":
    print(f"======================================================")
    print(f"  HEAVEN'S GATE CHESS ENGINE - WEB APPLICATION SERVER")
    print(f"  Server URL: http://localhost:{PORT}")
    print(f"======================================================")
    
    with socketserver.TCPServer(("", PORT), ChessRequestHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down Heaven's Gate web server.")
