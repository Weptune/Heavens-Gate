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
            self.process = subprocess.Popen(
                [self.engine_path, "uci"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            self._send_command("uci")
            self._send_command("isready")
            print(f"[ENGINE BRIDGE] Successfully launched Heaven's Gate UCI Process (PID {self.process.pid})")
        except Exception as e:
            print(f"[ERROR] Failed to start engine process: {e}")

    def _send_command(self, cmd):
        if self.process and self.process.stdin:
            self.process.stdin.write(cmd + "\n")
            self.process.stdin.flush()

    def get_move(self, fen, depth=8, movetime=0, wtime=0, btime=0, winc=0, binc=0):
        with self.lock:
            if not self.process or self.process.poll() is not None:
                self.start_engine()

            if not self.process:
                return {"error": "Engine process not available"}

            self._send_command(f"position fen {fen}")
            if wtime > 0 and btime > 0:
                self._send_command(f"go wtime {wtime} btime {btime} winc {winc} binc {binc}")
            elif movetime > 0:
                self._send_command(f"go movetime {movetime}")
            else:
                self._send_command(f"go depth {depth}")

            best_move = None
            eval_score = 0
            is_mate = False
            mate_in = 0
            nodes = 0
            nps = 0
            pv = ""

            while True:
                line = self.process.stdout.readline()
                if not line:
                    break
                line = line.strip()
                if line.startswith("info"):
                    tokens = line.split()
                    for i in range(len(tokens)):
                        if tokens[i] == "score" and i + 2 < len(tokens):
                            if tokens[i+1] == "cp":
                                eval_score = int(tokens[i+2])
                                is_mate = False
                            elif tokens[i+1] == "mate":
                                mate_in = int(tokens[i+2])
                                is_mate = True
                                eval_score = 29000 if mate_in > 0 else -29000
                        elif tokens[i] == "nodes" and i + 1 < len(tokens):
                            nodes = int(tokens[i+1])
                        elif tokens[i] == "nps" and i + 1 < len(tokens):
                            nps = int(tokens[i+1])
                        elif tokens[i] == "pv":
                            pv = " ".join(tokens[i+1:])
                elif line.startswith("bestmove"):
                    parts = line.split()
                    if len(parts) >= 2:
                        best_move = parts[1]
                    break

            return {
                "best_move": best_move,
                "score": eval_score,
                "is_mate": is_mate,
                "mate_in": mate_in,
                "nodes": nodes,
                "nps": nps,
                "pv": pv
            }

bridge = EngineBridge(ENGINE_PATH)

class ChessRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        web_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=web_dir, **kwargs)

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
        else:
            self.send_error(404)

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
