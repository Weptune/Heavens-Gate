import subprocess

def test_elo(elo):
    p = subprocess.Popen(['tools/stockfish.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    setup = f"""uci
setoption name UCI_Elo value {elo}
setoption name UCI_LimitStrength value true
isready
position startpos
go movetime 1000
quit
"""
    out, _ = p.communicate(setup)
    lines = [l for l in out.splitlines() if 'depth' in l]
    last_line = lines[-1] if lines else "No depth lines"
    print(f"ELO {elo}: {last_line}")

for elo in [1320, 1500, 2000, 2600, 2800, 3000, 3190, 3400]:
    test_elo(elo)
