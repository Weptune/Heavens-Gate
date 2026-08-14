/**
 * HEAVEN'S GATE GRANDMASTER CHESS AI WEB CLIENT
 * Full Interactive Engine Interface, Web Audio Synthesizer, Telemetry & Theme Support
 */

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

// Web Audio API Audio Synthesizer (Zero External Dependencies)
class SoundFX {
    constructor() {
        this.ctx = null;
    }

    init() {
        if (!this.ctx) {
            this.ctx = new (window.AudioContext || window.webkitAudioContext)();
        }
    }

    playMove() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sine';
        osc.frequency.setValueAtTime(440, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(180, this.ctx.currentTime + 0.08);
        gain.gain.setValueAtTime(0.3, this.ctx.currentTime);
        gain.gain.linearRampToValueAtTime(0.01, this.ctx.currentTime + 0.08);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.08);
    }

    playCapture() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(250, this.ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(80, this.ctx.currentTime + 0.12);
        gain.gain.setValueAtTime(0.5, this.ctx.currentTime);
        gain.gain.linearRampToValueAtTime(0.01, this.ctx.currentTime + 0.12);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.12);
    }

    playCheck() {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(587.33, this.ctx.currentTime); // D5
        osc.frequency.setValueAtTime(880, this.ctx.currentTime + 0.08); // A5
        gain.gain.setValueAtTime(0.4, this.ctx.currentTime);
        gain.gain.linearRampToValueAtTime(0.01, this.ctx.currentTime + 0.2);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + 0.2);
    }

    playGameOver() {
        this.init();
        if (!this.ctx) return;
        const notes = [523.25, 659.25, 783.99, 1046.50];
        notes.forEach((freq, idx) => {
            const osc = this.ctx.createOscillator();
            const gain = this.ctx.createGain();
            osc.frequency.setValueAtTime(freq, this.ctx.currentTime + idx * 0.1);
            gain.gain.setValueAtTime(0.3, this.ctx.currentTime + idx * 0.1);
            gain.gain.linearRampToValueAtTime(0.01, this.ctx.currentTime + idx * 0.1 + 0.3);
            osc.connect(gain);
            gain.connect(this.ctx.destination);
            osc.start(this.ctx.currentTime + idx * 0.1);
            osc.stop(this.ctx.currentTime + idx * 0.1 + 0.3);
        });
    }
}

class ChessApp {
    constructor() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.history = []; // FEN & board states for undo
        this.moveHistory = [];
        this.lastMove = null;
        this.engineDepth = 9;
        this.playMode = 'human_white'; // human_white, human_black, ai_vs_ai
        this.isFlipped = false;
        this.isThinking = false;
        this.pendingPromotion = null;
        this.sound = new SoundFX();

        this.initDOM();
        this.renderBoard();
        this.updateEvalBar(0);
    }

    initDOM() {
        this.boardEl = document.getElementById('board');
        this.evalFillEl = document.getElementById('eval-fill');
        this.evalBadgeEl = document.getElementById('eval-badge');
        this.moveHistoryEl = document.getElementById('move-history');
        this.moveCountEl = document.getElementById('move-count');
        this.statusTextEl = document.getElementById('engine-status');
        this.statusDotEl = document.getElementById('status-dot');

        this.teleNodesEl = document.getElementById('tele-nodes');
        this.teleNpsEl = document.getElementById('tele-nps');
        this.teleDepthEl = document.getElementById('tele-depth');
        this.telePvEl = document.getElementById('tele-pv');

        this.promoModalEl = document.getElementById('promotion-modal');
        this.promoChoicesEl = document.getElementById('promotion-choices');
        this.gameoverModalEl = document.getElementById('gameover-modal');

        document.getElementById('new-game-btn')?.addEventListener('click', () => this.resetGame());
        document.getElementById('modal-restart-btn')?.addEventListener('click', () => {
            this.gameoverModalEl?.classList.add('hidden');
            this.resetGame();
        });
        document.getElementById('flip-btn')?.addEventListener('click', () => {
            this.isFlipped = !this.isFlipped;
            this.renderBoard();
        });
        document.getElementById('undo-btn')?.addEventListener('click', () => this.undoMove());
        document.getElementById('hint-btn')?.addEventListener('click', () => this.getHint());

        document.getElementById('depth-select')?.addEventListener('change', (e) => {
            this.engineDepth = parseInt(e.target.value);
            if (this.teleDepthEl) this.teleDepthEl.textContent = `Depth ${this.engineDepth}`;
        });

        document.getElementById('mode-select')?.addEventListener('change', (e) => {
            this.playMode = e.target.value;
            this.isFlipped = (this.playMode === 'human_black');
            this.resetGame();
        });

        document.getElementById('theme-select')?.addEventListener('change', (e) => {
            document.body.className = `theme-${e.target.value}`;
        });
    }

    resetGame() {
        this.board = JSON.parse(JSON.stringify(INITIAL_BOARD));
        this.turn = 'w';
        this.selectedSquare = null;
        this.history = [];
        this.moveHistory = [];
        this.lastMove = null;
        this.isThinking = false;
        this.renderBoard();
        if (this.moveHistoryEl) this.moveHistoryEl.innerHTML = '<div class="empty-history-notice">Moves will appear here as the match progresses.</div>';
        if (this.moveCountEl) this.moveCountEl.textContent = '0 Moves';
        this.updateEvalBar(0);
        this.setStatus("Engine Ready", false);

        if (this.playMode === 'human_black' && this.turn === 'w') {
            this.triggerEngineMove();
        } else if (this.playMode === 'ai_vs_ai') {
            this.triggerEngineMove();
        }
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

    isBlackPiece(p) {
        return p >= 'a' && p <= 'z';
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

                // Rank / File Coordinate Labels
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

                // Render Piece SVG
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
        if (this.isThinking) return;
        if (this.playMode === 'ai_vs_ai') return;
        if (this.playMode === 'human_white' && this.turn !== 'w') return;
        if (this.playMode === 'human_black' && this.turn !== 'b') return;

        const piece = this.board[r][c];
        const isMyPiece = (this.turn === 'w' && this.isWhitePiece(piece)) || (this.turn === 'b' && this.isBlackPiece(piece));

        if (this.selectedSquare) {
            const [srcR, srcC] = this.selectedSquare;
            const fromSq = this.coordsToSquare(srcR, srcC);
            const toSq = this.coordsToSquare(r, c);

            if (srcR === r && srcC === c) {
                this.selectedSquare = null;
                this.renderBoard();
                return;
            }

            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.renderBoard();
                return;
            }

            // Check Pawn Promotion
            const srcPiece = this.board[srcR][srcC];
            if ((srcPiece === 'P' && r === 0) || (srcPiece === 'p' && r === 7)) {
                this.promptPromotion(srcR, srcC, r, c);
                return;
            }

            // Make User Move
            const uciStr = `${fromSq}${toSq}`;
            this.makeMove(srcR, srcC, r, c, uciStr);
            this.selectedSquare = null;
        } else {
            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.renderBoard();
            }
        }
    }

    promptPromotion(srcR, srcC, dstR, dstC) {
        if (!this.promoModalEl || !this.promoChoicesEl) return;
        this.promoChoicesEl.innerHTML = '';

        const promoPieces = (this.turn === 'w') ? ['Q', 'R', 'B', 'N'] : ['q', 'r', 'b', 'n'];

        promoPieces.forEach(p => {
            const choiceEl = document.createElement('div');
            choiceEl.className = 'promo-choice';
            choiceEl.innerHTML = SVG_PIECES[p];
            choiceEl.addEventListener('click', () => {
                this.promoModalEl.classList.add('hidden');
                const fromSq = this.coordsToSquare(srcR, srcC);
                const toSq = this.coordsToSquare(dstR, dstC);
                const promoChar = p.toLowerCase();
                const uciStr = `${fromSq}${toSq}${promoChar}`;
                this.makeMove(srcR, srcC, dstR, dstC, uciStr, p);
                this.selectedSquare = null;
            });
            this.promoChoicesEl.appendChild(choiceEl);
        });

        this.promoModalEl.classList.remove('hidden');
    }

    makeMove(srcR, srcC, dstR, dstC, uciStr, promoPiece = null) {
        // Save history state for undo
        this.history.push({
            board: JSON.parse(JSON.stringify(this.board)),
            turn: this.turn,
            lastMove: this.lastMove
        });

        const srcPiece = this.board[srcR][srcC];
        const isCapture = (this.board[dstR][dstC] !== '.');

        // Execute Move
        this.board[dstR][dstC] = promoPiece ? promoPiece : srcPiece;
        this.board[srcR][srcC] = '.';

        // Castling King move handling
        if (srcPiece === 'K' || srcPiece === 'k') {
            if (srcC === 4 && dstC === 6) { // Kingside
                this.board[srcR][5] = this.board[srcR][7];
                this.board[srcR][7] = '.';
            } else if (srcC === 4 && dstC === 2) { // Queenside
                this.board[srcR][3] = this.board[srcR][0];
                this.board[srcR][0] = '.';
            }
        }

        this.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(dstR, dstC) };
        this.moveHistory.push(uciStr);
        this.appendMoveHistory(uciStr);

        // Sound FX
        if (isCapture) {
            this.sound.playCapture();
        } else {
            this.sound.playMove();
        }

        this.turn = this.turn === 'w' ? 'b' : 'w';
        this.renderBoard();

        // Trigger AI move if applicable
        if (this.playMode === 'ai_vs_ai') {
            setTimeout(() => this.triggerEngineMove(), 300);
        } else if ((this.playMode === 'human_white' && this.turn === 'b') || (this.playMode === 'human_black' && this.turn === 'w')) {
            this.triggerEngineMove();
        }
    }

    undoMove() {
        if (this.history.length === 0) return;

        // In vs AI mode, undo 2 moves (User + Engine)
        const steps = (this.playMode !== 'ai_vs_ai' && this.history.length >= 2) ? 2 : 1;

        for (let i = 0; i < steps; i++) {
            if (this.history.length > 0) {
                const prev = this.history.pop();
                this.board = prev.board;
                this.turn = prev.turn;
                this.lastMove = prev.lastMove;
                this.moveHistory.pop();
            }
        }

        this.renderBoard();
        this.rebuildHistoryUI();
    }

    rebuildHistoryUI() {
        if (!this.moveHistoryEl) return;
        this.moveHistoryEl.innerHTML = '';
        this.moveHistory.forEach(uci => this.appendMoveHistory(uci));
    }

    appendMoveHistory(uciStr) {
        if (!this.moveHistoryEl) return;
        
        const emptyNotice = this.moveHistoryEl.querySelector('.empty-history-notice');
        if (emptyNotice) emptyNotice.remove();

        const moveIdx = this.moveHistory.length;
        const pairIdx = Math.ceil(moveIdx / 2);

        if (moveIdx % 2 !== 0) { // White Move
            const row = document.createElement('div');
            row.className = 'history-row';
            row.id = `hist-row-${pairIdx}`;
            row.innerHTML = `
                <span class="move-num">${pairIdx}.</span>
                <span class="move-white">${uciStr}</span>
                <span class="move-black"></span>
            `;
            this.moveHistoryEl.appendChild(row);
            this.moveHistoryEl.scrollTop = this.moveHistoryEl.scrollHeight;
        } else { // Black Move
            const row = document.getElementById(`hist-row-${pairIdx}`);
            if (row) {
                const blackSpan = row.querySelector('.move-black');
                if (blackSpan) blackSpan.textContent = uciStr;
            }
        }

        if (this.moveCountEl) {
            this.moveCountEl.textContent = `${this.moveHistory.length} Moves`;
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
        if (this.isThinking) return;
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

            // Update Telemetry HUD
            if (data.nodes && this.teleNodesEl) this.teleNodesEl.textContent = data.nodes.toLocaleString();
            if (data.nps && this.teleNpsEl) this.teleNpsEl.textContent = `${(data.nps / 1000).toFixed(1)}k NPS`;
            if (data.pv && this.telePvEl) this.telePvEl.textContent = data.pv;

            if (data.best_move) {
                const uci = data.best_move;
                const fromSq = uci.substring(0, 2);
                const toSq = uci.substring(2, 4);
                const promoChar = uci.length > 4 ? uci[4] : null;

                const [srcR, srcC] = this.squareToCoords(fromSq);
                const [dstR, dstC] = this.squareToCoords(toSq);

                let promoPiece = null;
                if (promoChar) {
                    promoPiece = (this.turn === 'w') ? promoChar.toUpperCase() : promoChar.toLowerCase();
                }

                this.makeMove(srcR, srcC, dstR, dstC, uci, promoPiece);

                // Update Eval Bar
                let evalCp = data.score;
                if (data.is_mate) {
                    evalCp = (data.mate_in > 0) ? 29000 : -29000;
                    this.updateEvalBar(evalCp, true, data.mate_in);
                } else {
                    this.updateEvalBar(evalCp);
                }
            } else {
                // Game Over Checkmate / Stalemate
                this.showGameOver("Game Over", "Match concluded.");
            }
        } catch (err) {
            console.error("Engine API call failed:", err);
        } finally {
            this.isThinking = false;
            this.setStatus("Engine Ready", false);
        }
    }

    async getHint() {
        const fen = this.getFEN();
        this.setStatus("Calculating Hint...", true);
        try {
            const resp = await fetch('/api/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ fen: fen, depth: 7 })
            });
            const data = await resp.json();
            if (data.best_move) {
                const fromSq = data.best_move.substring(0, 2);
                const toSq = data.best_move.substring(2, 4);
                const [srcR, srcC] = this.squareToCoords(fromSq);
                const [dstR, dstC] = this.squareToCoords(toSq);
                this.selectedSquare = [srcR, srcC];
                this.lastMove = { from: fromSq, to: toSq };
                this.renderBoard();
                this.sound.playCheck();
            }
        } catch (e) {
            console.error(e);
        } finally {
            this.setStatus("Engine Ready", false);
        }
    }

    updateEvalBar(cp, isMate = false, mateIn = 0) {
        if (!this.evalFillEl || !this.evalBadgeEl) return;

        if (isMate) {
            const label = mateIn > 0 ? `M${mateIn}` : `-M${Math.abs(mateIn)}`;
            this.evalBadgeEl.textContent = label;
            this.evalFillEl.style.height = mateIn > 0 ? '100%' : '0%';
            return;
        }

        // Sigmoid scaling for smooth eval bar visualization [-10.0 to +10.0 cp]
        const pawns = cp / 100.0;
        const sign = pawns >= 0 ? '+' : '';
        this.evalBadgeEl.textContent = `${sign}${pawns.toFixed(1)}`;

        // Convert pawns to percentage [0% to 100%]
        const pct = 50 + (50 * (2 / (1 + Math.exp(-0.35 * pawns)) - 1));
        const clampedPct = Math.max(2, Math.min(98, pct));
        this.evalFillEl.style.height = `${clampedPct}%`;
    }

    setStatus(msg, isThinking = false) {
        if (this.statusTextEl) this.statusTextEl.textContent = msg;
        if (this.statusDotEl) {
            if (isThinking) {
                this.statusDotEl.classList.add('thinking');
            } else {
                this.statusDotEl.classList.remove('thinking');
            }
        }
    }

    showGameOver(title, sub) {
        if (!this.gameoverModalEl) return;
        document.getElementById('gameover-title').textContent = title;
        document.getElementById('gameover-sub').textContent = sub;
        this.gameoverModalEl.classList.remove('hidden');
        this.sound.playGameOver();
    }
}

// Initialize Application on Page Load
document.addEventListener('DOMContentLoaded', () => {
    window.app = new ChessApp();
});
