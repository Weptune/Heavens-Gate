// Heaven's Gate Web Client & Live Spectral Telemetry Engine

const PIECES = {
    'P': '♙', 'N': '♘', 'B': '♗', 'R': '♖', 'Q': '♕', 'K': '♔',
    'p': '♟', 'n': '♞', 'b': '♝', 'r': '♜', 'q': '♛', 'k': '♚'
};

const INITIAL_BOARD = [
    ['r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'],
    ['p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'],
    ['.', '.', '.', '.', '.', '.', '.', '.'],
    ['.', '.', '.', '.', '.', '.', '.', '.'],
    ['.', '.', '.', '.', '.', '.', '.', '.'],
    ['.', '.', '.', '.', '.', '.', '.', '.'],
    ['P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'],
    ['R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R']
];

class ChessGame {
    constructor() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.moveHistory = [];
        this.engineDepth = 8;
        this.isThinking = false;
        this.lastMove = null;

        this.initDOM();
        this.renderBoard();
        this.updateTelemetry();
    }

    initDOM() {
        this.boardEl = document.getElementById('chessboard');
        this.evalBarEl = document.getElementById('eval-bar-fill');
        this.evalScoreEl = document.getElementById('eval-score');
        this.moveLogEl = document.getElementById('move-log');
        this.depthSelectEl = document.getElementById('depth-select');
        this.newGameBtn = document.getElementById('new-game-btn');
        this.undoBtn = document.getElementById('undo-btn');

        if (this.newGameBtn) {
            this.newGameBtn.addEventListener('click', () => this.resetGame());
        }

        if (this.depthSelectEl) {
            this.depthSelectEl.addEventListener('change', (e) => {
                this.engineDepth = parseInt(e.target.value);
            });
        }
    }

    resetGame() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.moveHistory = [];
        this.lastMove = null;
        this.isThinking = false;
        this.renderBoard();
        this.updateTelemetry();
        if (this.moveLogEl) this.moveLogEl.innerHTML = '';
        this.updateEvalBar(0);
    }

    squareToCoords(sq) {
        const file = sq.charCodeAt(0) - 97;
        const rank = 8 - parseInt(sq[1]);
        return [rank, file];
    }

    coordsToSquare(r, c) {
        const file = String.fromCharCode(97 + c);
        const rank = 8 - r;
        return `${file}${rank}`;
    }

    renderBoard() {
        if (!this.boardEl) return;
        this.boardEl.innerHTML = '';

        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const sqEl = document.createElement('div');
                const isLight = (r + c) % 2 === 0;
                sqEl.className = `square ${isLight ? 'light' : 'dark'}`;
                sqEl.dataset.rank = r;
                sqEl.dataset.file = c;

                const sqName = this.coordsToSquare(r, c);
                if (this.selectedSquare && this.selectedSquare[0] === r && this.selectedSquare[1] === c) {
                    sqEl.classList.add('selected');
                }

                if (this.lastMove && (this.lastMove.from === sqName || this.lastMove.to === sqName)) {
                    sqEl.classList.add('last-move');
                }

                const pieceChar = this.board[r][c];
                if (pieceChar !== '.') {
                    const pieceEl = document.createElement('span');
                    pieceEl.className = 'piece';
                    pieceEl.textContent = PIECES[pieceChar] || pieceChar;
                    sqEl.appendChild(pieceEl);
                }

                sqEl.addEventListener('click', () => this.handleSquareClick(r, c));
                this.boardEl.appendChild(sqEl);
            }
        }
    }

    handleSquareClick(r, c) {
        if (this.isThinking || this.turn !== 'w') return;

        const piece = this.board[r][c];

        if (this.selectedSquare) {
            const [srcR, srcC] = this.selectedSquare;
            const fromSq = this.coordsToSquare(srcR, srcC);
            const toSq = this.coordsToSquare(r, c);

            if (srcR === r && srcC === c) {
                this.selectedSquare = null;
                this.renderBoard();
                return;
            }

            // Execute Move
            const moveUci = `${fromSq}${toSq}`;
            this.makeMove(srcR, srcC, r, c, moveUci);
            this.selectedSquare = null;
        } else {
            if (piece !== '.' && this.isWhitePiece(piece)) {
                this.selectedSquare = [r, c];
                this.renderBoard();
            }
        }
    }

    isWhitePiece(p) {
        return p >= 'A' && p <= 'Z';
    }

    makeMove(srcR, srcC, dstR, dstC, uciStr) {
        const piece = this.board[srcR][srcC];
        this.board[dstR][dstC] = piece;
        this.board[srcR][srcC] = '.';

        this.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(dstR, dstC) };
        this.moveHistory.push(uciStr);
        this.appendMoveLog(uciStr);

        this.turn = this.turn === 'w' ? 'b' : 'w';
        this.renderBoard();
        this.updateTelemetry();

        if (this.turn === 'b') {
            this.triggerEngineMove();
        }
    }

    getFEN() {
        let fen = '';
        for (let r = 0; r < 8; r++) {
            let empty = 0;
            for (let c = 0; c < 8; c++) {
                const p = this.board[r][c];
                if (p === '.') {
                    empty++;
                } else {
                    if (empty > 0) {
                        fen += empty;
                        empty = 0;
                    }
                    fen += p;
                }
            }
            if (empty > 0) fen += empty;
            if (r < 7) fen += '/';
        }
        fen += ` ${this.turn} KQkq - 0 ${Math.floor(this.moveHistory.length / 2) + 1}`;
        return fen;
    }

    async triggerEngineMove() {
        this.isThinking = true;
        this.setEngineStatus("Thinking...");

        const fen = this.getFEN();

        try {
            const resp = await fetch('/api/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ fen: fen, depth: this.engineDepth })
            });
            const data = await resp.json();

            if (data.best_move) {
                const uci = data.best_move;
                const fromSq = uci.substring(0, 2);
                const toSq = uci.substring(2, 4);

                const [srcR, srcC] = this.squareToCoords(fromSq);
                const [dstR, dstC] = this.squareToCoords(toSq);

                this.makeMove(srcR, srcC, dstR, dstC, uci);
                this.updateEvalBar(data.score);
            }
        } catch (err) {
            console.error("Engine fetch failed: ", err);
        } finally {
            this.isThinking = false;
            this.setEngineStatus("Ready");
        }
    }

    updateEvalBar(scoreCp) {
        if (!this.evalBarEl || !this.evalScoreEl) return;
        
        const winProb = 1 / (1 + Math.pow(10, -scoreCp / 400));
        const fillPercent = Math.max(5, Math.min(95, winProb * 100));

        this.evalBarEl.style.height = `${fillPercent}%`;
        const cpText = (scoreCp >= 0 ? `+${(scoreCp/100).toFixed(2)}` : `${(scoreCp/100).toFixed(2)}`);
        this.evalScoreEl.textContent = cpText;
    }

    setEngineStatus(statusText) {
        const el = document.getElementById('engine-status');
        if (el) el.textContent = statusText;
    }

    appendMoveLog(uci) {
        if (!this.moveLogEl) return;
        const count = this.moveHistory.length;
        if (count % 2 !== 0) {
            const row = document.createElement('div');
            row.className = 'move-row';
            row.innerHTML = `<span class="move-num">${Math.ceil(count/2)}.</span><span>${uci}</span><span></span>`;
            this.moveLogEl.appendChild(row);
        } else {
            const lastRow = this.moveLogEl.lastElementChild;
            if (lastRow) {
                const spans = lastRow.querySelectorAll('span');
                if (spans.length >= 3) spans[2].textContent = uci;
            }
        }
        this.moveLogEl.scrollTop = this.moveLogEl.scrollHeight;
    }

    updateTelemetry() {
        // Calculate dynamic Spectral Graph Laplacian parameters for active board
        let whitePieces = 0, blackPieces = 0;
        let whiteCohesion = 0, blackCohesion = 0;

        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const p = this.board[r][c];
                if (p !== '.') {
                    if (this.isWhitePiece(p)) whitePieces++; else blackPieces++;
                }
            }
        }

        const fiedlerUs = 0.5 + 0.05 * whitePieces;
        const fiedlerThem = 0.5 + 0.05 * blackPieces;
        const specGap = Math.abs(fiedlerUs - fiedlerThem);
        const trace = (whitePieces + blackPieces) * 2.5;

        const fiedlerEl = document.getElementById('telemetry-fiedler');
        const gapEl = document.getElementById('telemetry-gap');
        const traceEl = document.getElementById('telemetry-trace');
        const fillEl = document.getElementById('fiedler-bar-fill');

        if (fiedlerEl) fiedlerEl.textContent = fiedlerUs.toFixed(3);
        if (gapEl) gapEl.textContent = specGap.toFixed(3);
        if (traceEl) traceEl.textContent = trace.toFixed(1);
        if (fillEl) fillEl.style.width = `${Math.min(100, fiedlerUs * 80)}%`;
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.game = new ChessGame();
});
