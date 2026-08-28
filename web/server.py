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
            # Standard logistic chess win probability formula
            win_chance = 50.0 + 50.0 * (2.0 / (1.0 + math.exp(-0.00368208 * eval_score)) - 1.0)
            win_chance = max(0.0, min(100.0, win_chance))

            return {
                "best_move": best_move,
                "score": eval_score,
                "is_mate": is_mate,
                "mate_in": mate_in,
                "depth": completed_depth,
                "nodes": nodes,
                "nps": nps,
                "time_ms": time_ms,
                "hashfull": hashfull,
                "win_chance": round(win_chance, 1),
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
        if self.path == "/api/puzzles":
            puzzles = [
                {
                    "id": 1,
                    "title": "Kasparov's Immortal Attack",
                    "event": "Kasparov vs Topalov, Wijk aan Zee 1999",
                    "fen": "b2r3r/k4p1p/p2q1np1/NppP4/3p1Q2/P4PPB/1PP4P/1K1RR3 w - - 0 1",
                    "turn": "w",
                    "hint": "Sacrifice the rook to shatter the king's pawn shield!",
                    "solution": "d1d4",
                    "desc": "Find the immortal rook sacrifice that opens the fatal d-file ray."
                },
                {
                    "id": 2,
                    "title": "The Greek Gift Sacrifice",
                    "event": "Classical Tactical Theme",
                    "fen": "r1bq1rk1/ppp2ppp/2n1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQ - 4 7",
                    "turn": "w",
                    "hint": "Bxh7+ crashes through the kingside fortress!",
                    "solution": "d3h7",
                    "desc": "Classic bishop sacrifice on h7 followed by Ng5+ and Qh5."
                },
                {
                    "id": 3,
                    "title": "Mikhail Tal's Knight Sorcery",
                    "event": "Tal vs Larsen, Bled 1965",
                    "fen": "r1b2rk1/pp1n1ppp/2p1pn2/q2p2B1/2PP4/2P1PN2/P1Q1BPPP/R3K2R w KQ - 3 10",
                    "turn": "w",
                    "hint": "Break through the center with e4!",
                    "solution": "e1g1",
                    "desc": "Prepare the central explosion and open tactical diagonal lines."
                },
                {
                    "id": 4,
                    "title": "Opera House Checkmate",
                    "event": "Paul Morphy vs Duke of Brunswick, Paris 1858",
                    "fen": "4kb1r/p2n1ppp/4q3/4p1B1/4P3/1Q6/PPP2PPP/2KR4 w k - 1 1",
                    "turn": "w",
                    "hint": "Queen sacrifice on b8 leads to back-rank mate with Rd8#!",
                    "solution": "b3b8",
                    "desc": "The most famous queen sacrifice in chess history."
                },
                {
                    "id": 5,
                    "title": "Fischer's Game of the Century",
                    "event": "Donald Byrne vs Bobby Fischer, New York 1956",
                    "fen": "r3r1k1/pp3pbp/1qp1b1p1/4B3/2P5/2N2N1P/PP1Q1PP1/R4RK1 b - - 0 16",
                    "turn": "b",
                    "hint": "Offer the queen with Be6 to build a lethal discovered attack windmill!",
                    "solution": "e6c4",
                    "desc": "Fischer's brilliant 13-year-old masterpiece."
                },
                {
                    "id": 6,
                    "title": "Smothered Mate (Philidor's Legacy)",
                    "event": "Classical Tactical Motif",
                    "fen": "6k1/5ppp/8/8/8/8/1Q4PP/6K1 w - - 0 1",
                    "turn": "w",
                    "hint": "Queen check on b8 forces back rank mate!",
                    "solution": "b2b8",
                    "desc": "Deliver the unstoppable back-rank checkmate."
                }
            ]
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(puzzles).encode('utf-8'))
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
