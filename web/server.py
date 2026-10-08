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

PORT = 8000
REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENGINE_PATH = os.path.join(REPO_DIR, "heavensgate.exe")

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
            self._send_command("isready")
            while True:
                line = self.process.stdout.readline()
                if not line or line.strip() == "readyok":
                    break
            print(f"[ENGINE BRIDGE] Successfully launched and synchronized Heaven's Gate UCI Process (PID {self.process.pid}) in {engine_dir}")
        except Exception as e:
            print(f"[ERROR] Failed to start engine process: {e}")

    def _send_command(self, cmd):
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

    def get_move(self, fen, depth=8, movetime=0, wtime=0, btime=0, winc=0, binc=0):
        with self.lock:
            if not self.process or self.process.poll() is not None:
                self.start_engine()

            if not self.process:
                return {"error": "Engine process not available"}

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

class ChessRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        web_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=web_dir, **kwargs)

    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

    def do_POST(self):
        if self.path == "/api/move":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            req = json.loads(post_data.decode('utf-8'))
            
            fen = req.get("fen", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
            depth = req.get("depth", 8)
            movetime = req.get("movetime", 0)
            wtime = req.get("wtime", 0)
            btime = req.get("btime", 0)
            winc = req.get("winc", 0)
            binc = req.get("binc", 0)
            
            res = bridge.get_move(fen, depth=depth, movetime=movetime, wtime=wtime, btime=btime, winc=winc, binc=binc)
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(res).encode('utf-8'))
        elif self.path == "/api/analyze":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            req = json.loads(post_data.decode('utf-8'))
            fen = req.get("fen", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
            depth = req.get("depth", 10)
            movetime = req.get("movetime", 0)
            res = bridge.get_move(fen, depth=depth, movetime=movetime)
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(res).encode('utf-8'))
        else:
            self.send_error(404)

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
