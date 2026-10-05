import http.server
import socketserver
import json
import subprocess
import os
import sys
import threading

PORT = 8000
ENGINE_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "heavensgate.exe")

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
            self._send_command("uci")
            self._send_command("isready")
            print(f"[ENGINE BRIDGE] Successfully launched Heaven's Gate UCI Process (PID {self.process.pid}) in {engine_dir}")
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

            import math
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
            res = bridge.get_move(fen, depth=req.get("depth", 9), movetime=req.get("movetime", 0))
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(res).encode('utf-8'))
        else:
            self.send_error(404)

    def do_GET(self):
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
