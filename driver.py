import subprocess, sys, time
import chess

UCI = "d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8 f1e2 c8g4 c1e3 f6d7 e1g1 g4f3 e2f3 b8c6 f3g4 e7e5 d4d5 c6d4 g4h3 c7c5 a1b1 f7f5 f2f3 a7a6 e4f5 g6f5 a2a4 a8b8 g1h1 d8f6 b2b4 b7b6 c3e2 c5b4 b1b4 d7c5 e2d4 e5d4 e3f2 a6a5 b4b2 f6h6 b2b1 h6g6 f1g1 g6h6 g1e1 h6g5 f2d4 g7d4 d1d4 b8e8 d4d1 e8e3 e1f1 e3d3 d1e2 d3e3 e2c2 g5e7 b1b6 e3e1 b6b1 e1e2 c2c3 c5a4 c3a5 a4c5 a5c3 e7g7 c3g7 g8g7 g2g3 c5d3 b1b7 g7g8 f1a1 d3f2 h1g2 f2d3 g2g1 d3e5 a1a3 e2d2 b7e7 d2c2 c4c5 c2c5 a3a7 f8f6 f3f4 e5f7 h3f1 g8g7 f1e2 g7g6 g1f2 c5c2 f2e1 c2c1 e1d2 c1h1 h2h4 h1b1 e7e6 b1b2 d2d1 f6e6 d5e6 f7h8 e2c4 b2b1 d1e2 b1b2 e2f3 b2b8 e6e7 g6f6 c4b5 h8g6 e7e8q b8e8 b5e8 g6e7 f3e3 e7d5 e3d4 d5e7 h4h5 f6e6 a7c7 e6f6 c7b7 e7c8 d4d5 c8e7 d5d6 e7c8 d6d7 h7h6 d7c8 f6e6 b7d7 e6f6 d7b7 f6e6 b7b6 e6d5 c8b7 d5e4 e8c6 e4e3 b6b2 e3d4 b2e2 d4c4 c6g2 c4d4 e2e5 d4c4 e5e8 c4d3 e8e7 d3d2 g2c6 d2c3 e7e3 c3d2 e3e8 d2d3 c6b5 d3c3 b7c8 c3b2 e8e3 b2a2 e3e2 a2b3 b5c6 b3b4 c8d7 b4c3 e2e3 c3c4 e3e5 c4d4 d7c7 d4c4 c6b5 c4c3 e5e6 c3d4 e6e8 d4c5 b5f1 c5d5 f1a6 d5c5 e8e3 c5d4 e3e5 d4c3 c7b6 c3d4 a6c8 d4c3 b6c5 c3c2 e5e2 c2b1 c5b4 b1c1 c8e6 c1d1 e6c4 d1c1 c4b5 c1b1 b4c4 b1c1 b5a4 c1b1 a4c6 b1c1 c4d5 c1d1 e2a2 d1c1 d5e6 c1b1 a2h2 b1c1 e6f7 c1d1 c6f3 d1e1 f3c6 e1f1 f7e6 f1g1 h2d2".split()

WTIME = "30000"  # max ~15.7s, soft stop ~1.1s per move

def send(p, s):
    p.stdin.write(s + "\n"); p.stdin.flush()

def go_and_get_best(p):
    send(p, "go wtime " + WTIME)
    last_info = ""
    while True:
        line = p.stdout.readline()
        if not line: return None, last_info
        line = line.strip()
        if line.startswith("info depth"):
            last_info = line
        if line.startswith("bestmove"):
            return line.split()[1], last_info

p = subprocess.Popen(["./4k_full"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
send(p, "uci")
# drain until uciok
while True:
    l = p.stdout.readline()
    if l.strip()=="uciok": break
send(p, "ucinewgame")
send(p, "isready")
while True:
    l=p.stdout.readline()
    if l.strip()=="readyok": break

START_WHITE_MOVE = 73   # start polluting TT here
END_WHITE_MOVE   = 99
board = chess.Board()

for i, u in enumerate(UCI):
    movenum = i//2 + 1
    white_to_move = (i % 2 == 0)
    # set position to current ply, then if it's a white move in our window, search BEFORE applying
    if white_to_move and START_WHITE_MOVE <= movenum <= END_WHITE_MOVE:
        send(p, "position startpos moves " + " ".join(UCI[:i]))
        send(p, "isready")
        while True:
            l=p.stdout.readline()
            if l.strip()=="readyok": break
        best, info = go_and_get_best(p)
        game_move = u
        san = board.san(chess.Move.from_uci(game_move))
        flag = "  <-- matches game" if best==game_move else "  *** DEVIATES from game ***"
        # extract score
        score = ""
        if "score" in info:
            parts=info.split()
            si=parts.index("score")
            score=" ".join(parts[si:si+2])
        print(f"move {movenum:>3} W: engine={best:<6} game={game_move:<6}({san:<5}) [{score}]{flag}")
    board.push(chess.Move.from_uci(u))

send(p, "quit")
p.wait()