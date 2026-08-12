// High-Resolution Vector SVG Piece Definitions (Lichess/Chess.com standard vector set)

const SVG_PIECES = {
    'P': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M22.5 9c-2.21 0-4 1.79-4 4 0 .89.29 1.71.78 2.38C17.33 16.5 16 18.59 16 21c0 2.03.94 3.84 2.41 5.03-3 1.06-7.41 5.55-7.41 13.47h23c0-7.92-4.41-12.41-7.41-13.47 1.47-1.19 2.41-3 2.41-5.03 0-2.41-1.33-4.5-3.28-5.62.49-.67.78-1.49.78-2.38 0-2.21-1.79-4-4-4z" fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round"/></svg>`,
    'N': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M 22,10 C 32.5,11 38.5,18 38,39 L 15,39 C 15,30 25,32.5 23,18 C 21.5,14.5 12,14 12,14 C 12,14 11,21 17,22.5 C 15.5,22.5 11.5,21 11.5,15 C 11.5,10.5 17.5,9.5 22,10 z" fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round"/><path d="M 24,18 C 24.38,19.92 22.45,21.37 20.53,21 C 18.61,20.62 17.16,18.69 17.54,16.77 C 17.92,14.85 19.85,13.4 21.77,13.78 C 23.69,14.16 25.14,16.09 24.76,18 z" fill="#000"/></svg>`,
    'B': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#fff"><path d="M9 36c1.2-2.5 3.5-3.5 6-3.5s4.8 1 6 3.5H9zM15 32c-2.5 0-3.5-1.5-3.5-3s.5-3.5 2-4.5c1.5-1 3.5-1 5 0s2 3 2 4.5-1 3-3.5 3zM15 23.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"/><circle cx="15" cy="11.5" r="2.5"/></g><path d="M15 9.5v-3M13.5 8h3"/></g></svg>`,
    'R': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#fff" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M9 39h27v-3H9v3zM12 36h21v-4H12v4zM11 32h23l-2-16H13l-2 16zM9 16h27v-4h-4v2h-5v-2h-6v2h-5v-2H9v4z"/><path d="M14 29.5h17M14 16.5h17" stroke-linecap="butt"/></g></svg>`,
    'Q': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M8 36h29v-3H8v3zM11.5 33h22l-1.5-4h-19l-1.5 4zM9 29l4.5-16.5L20 27l2.5-20L25 27l6.5-14.5L36 29H9z"/><circle cx="9" cy="11" r="2"/><circle cx="13.5" cy="11.5" r="2"/><circle cx="22.5" cy="6" r="2"/><circle cx="31.5" cy="11.5" r="2"/><circle cx="36" cy="11" r="2"/></g></svg>`,
    'K': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#fff"><path d="M22.5 11.63c-1.58 0-2.87 1.29-2.87 2.87 0 1.25.8 2.31 1.92 2.68V21.5h-5.5v2h5.5v3.5h-8v2h8V35h-11v4h25v-4h-11v-6h8v-2h-8V23.5h5.5v-2h-5.5v-4.32c1.12-.37 1.92-1.43 1.92-2.68 0-1.58-1.29-2.87-2.87-2.87z"/></g><path d="M22.5 6v4.5M20.25 8.25h4.5"/></g></svg>`,

    'p': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M22.5 9c-2.21 0-4 1.79-4 4 0 .89.29 1.71.78 2.38C17.33 16.5 16 18.59 16 21c0 2.03.94 3.84 2.41 5.03-3 1.06-7.41 5.55-7.41 13.47h23c0-7.92-4.41-12.41-7.41-13.47 1.47-1.19 2.41-3 2.41-5.03 0-2.41-1.33-4.5-3.28-5.62.49-.67.78-1.49.78-2.38 0-2.21-1.79-4-4-4z" fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/></svg>`,
    'n': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M 22,10 C 32.5,11 38.5,18 38,39 L 15,39 C 15,30 25,32.5 23,18 C 21.5,14.5 12,14 12,14 C 12,14 11,21 17,22.5 C 15.5,22.5 11.5,21 11.5,15 C 11.5,10.5 17.5,9.5 22,10 z" fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/><path d="M 24,18 C 24.38,19.92 22.45,21.37 20.53,21 C 18.61,20.62 17.16,18.69 17.54,16.77 C 17.92,14.85 19.85,13.4 21.77,13.78 C 23.69,14.16 25.14,16.09 24.76,18 z" fill="#fff"/></svg>`,
    'b': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#222"><path d="M9 36c1.2-2.5 3.5-3.5 6-3.5s4.8 1 6 3.5H9zM15 32c-2.5 0-3.5-1.5-3.5-3s.5-3.5 2-4.5c1.5-1 3.5-1 5 0s2 3 2 4.5-1 3-3.5 3zM15 23.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"/><circle cx="15" cy="11.5" r="2.5"/></g><path d="M15 9.5v-3M13.5 8h3" stroke="#fff"/></g></svg>`,
    'r': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#222" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M9 39h27v-3H9v3zM12 36h21v-4H12v4zM11 32h23l-2-16H13l-2 16zM9 16h27v-4h-4v2h-5v-2h-6v2h-5v-2H9v4z"/><path d="M14 29.5h17M14 16.5h17" stroke="#fff" stroke-linecap="butt"/></g></svg>`,
    'q': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M8 36h29v-3H8v3zM11.5 33h22l-1.5-4h-19l-1.5 4zM9 29l4.5-16.5L20 27l2.5-20L25 27l6.5-14.5L36 29H9z"/><circle cx="9" cy="11" r="2"/><circle cx="13.5" cy="11.5" r="2"/><circle cx="22.5" cy="6" r="2"/><circle cx="31.5" cy="11.5" r="2"/><circle cx="36" cy="11" r="2"/></g></svg>`,
    'k': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#222"><path d="M22.5 11.63c-1.58 0-2.87 1.29-2.87 2.87 0 1.25.8 2.31 1.92 2.68V21.5h-5.5v2h5.5v3.5h-8v2h8V35h-11v4h25v-4h-11v-6h8v-2h-8V23.5h5.5v-2h-5.5v-4.32c1.12-.37 1.92-1.43 1.92-2.68 0-1.58-1.29-2.87-2.87-2.87z"/></g><path d="M22.5 6v4.5M20.25 8.25h4.5" stroke="#fff"/></g></svg>`
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

class ChessApp {
    constructor() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.moveHistory = [];
        this.lastMove = null;
        this.engineDepth = 9;
        this.isFlipped = false;
        this.isThinking = false;

        this.initDOM();
        this.renderBoard();
        this.updateEvalBar(0);
    }

    initDOM() {
        this.boardEl = document.getElementById('board');
        this.evalFillEl = document.getElementById('eval-fill');
        this.evalValEl = document.getElementById('eval-val');
        this.moveHistoryEl = document.getElementById('move-history');
        this.statusTextEl = document.getElementById('engine-status');
        this.statusDotEl = document.getElementById('status-dot');

        document.getElementById('new-game-btn')?.addEventListener('click', () => this.resetGame());
        document.getElementById('flip-btn')?.addEventListener('click', () => {
            this.isFlipped = !this.isFlipped;
            this.renderBoard();
        });
        document.getElementById('depth-select')?.addEventListener('change', (e) => {
            this.engineDepth = parseInt(e.target.value);
        });
    }

    resetGame() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.moveHistory = [];
        this.lastMove = null;
        this.isThinking = false;
        this.renderBoard();
        if (this.moveHistoryEl) this.moveHistoryEl.innerHTML = '';
        this.updateEvalBar(0);
        this.setStatus("Engine Ready", false);
    }

    coordsToSquare(r, c) {
        const file = String.fromCharCode(97 + c);
        const rank = 8 - r;
        return `${file}${rank}`;
    }

    squareToCoords(sq) {
        const file = sq.charCodeAt(0) - 97;
        const rank = 8 - parseInt(sq[1]);
        return [rank, file];
    }

    isWhitePiece(p) {
        return p >= 'A' && p <= 'Z';
    }

    renderBoard() {
        if (!this.boardEl) return;
        this.boardEl.innerHTML = '';

        for (let rowIdx = 0; rowIdx < 8; rowIdx++) {
            for (let colIdx = 0; colIdx < 8; colIdx++) {
                const r = this.isFlipped ? 7 - rowIdx : rowIdx;
                const c = this.isFlipped ? 7 - colIdx : colIdx;

                const sqEl = document.createElement('div');
                const isLight = (r + c) % 2 === 0;
                sqEl.className = `square ${isLight ? 'light' : 'dark'}`;

                const sqName = this.coordsToSquare(r, c);

                // Highlight Selected Square
                if (this.selectedSquare && this.selectedSquare[0] === r && this.selectedSquare[1] === c) {
                    sqEl.classList.add('selected');
                }

                // Highlight Last Move
                if (this.lastMove && (this.lastMove.from === sqName || this.lastMove.to === sqName)) {
                    sqEl.classList.add('last-move');
                }

                // Rank / File Labels
                if (colIdx === 0) {
                    const rankLabel = document.createElement('span');
                    rankLabel.className = 'coord coord-rank';
                    rankLabel.textContent = 8 - r;
                    sqEl.appendChild(rankLabel);
                }
                if (rowIdx === 7) {
                    const fileLabel = document.createElement('span');
                    fileLabel.className = 'coord coord-file';
                    fileLabel.textContent = String.fromCharCode(97 + c);
                    sqEl.appendChild(fileLabel);
                }

                // Piece Rendering (SVG Vector Set)
                const p = this.board[r][c];
                if (p !== '.') {
                    const wrapper = document.createElement('div');
                    wrapper.style.width = '100%';
                    wrapper.style.height = '100%';
                    wrapper.style.display = 'flex';
                    wrapper.style.alignItems = 'center';
                    wrapper.style.justifyContent = 'center';
                    wrapper.innerHTML = SVG_PIECES[p] || p;
                    sqEl.appendChild(wrapper);
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

            // Make User Move
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

    makeMove(srcR, srcC, dstR, dstC, uciStr) {
        const piece = this.board[srcR][srcC];
        this.board[dstR][dstC] = piece;
        this.board[srcR][srcC] = '.';

        this.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(dstR, dstC) };
        this.moveHistory.push(uciStr);
        this.appendMoveHistory(uciStr);

        this.turn = this.turn === 'w' ? 'b' : 'w';
        this.renderBoard();

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
        this.setStatus("Engine Searching...", true);

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
            console.error("Engine API call failed:", err);
        } finally {
            this.isThinking = false;
            this.setStatus("Engine Ready", false);
        }
    }

    updateEvalBar(scoreCp) {
        if (!this.evalFillEl || !this.evalValEl) return;
        const winProb = 1 / (1 + Math.pow(10, -scoreCp / 400));
        const fillPercent = Math.max(5, Math.min(95, winProb * 100));

        this.evalFillEl.style.height = `${fillPercent}%`;
        const text = scoreCp >= 0 ? `+${(scoreCp/100).toFixed(1)}` : `${(scoreCp/100).toFixed(1)}`;
        this.evalValEl.textContent = text;
    }

    setStatus(msg, active) {
        if (this.statusTextEl) this.statusTextEl.textContent = msg;
        if (this.statusDotEl) {
            this.statusDotEl.style.background = active ? '#00f2fe' : '#22c55e';
        }
    }

    appendMoveHistory(uci) {
        if (!this.moveHistoryEl) return;
        const count = this.moveHistory.length;
        if (count % 2 !== 0) {
            const row = document.createElement('div');
            row.className = 'move-row';
            row.innerHTML = `<span class="move-num">${Math.ceil(count/2)}.</span><span>${uci}</span><span></span>`;
            this.moveHistoryEl.appendChild(row);
        } else {
            const lastRow = this.moveHistoryEl.lastElementChild;
            if (lastRow) {
                const spans = lastRow.querySelectorAll('span');
                if (spans.length >= 3) spans[2].textContent = uci;
            }
        }
        this.moveHistoryEl.scrollTop = this.moveHistoryEl.scrollHeight;
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.app = new ChessApp();
});
