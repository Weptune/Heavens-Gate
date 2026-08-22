/**
 * HEAVEN'S GATE CHESS ENGINE - WEB CLIENT (MASTER EDITION)
 * Active Precision Clock Timers + Manual Start Match Trigger + Smart Opening Time Management
 */

const SVG_PIECES = {
    'P': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M22.5 9c-2.21 0-4 1.79-4 4 0 .89.29 1.71.78 2.38C17.33 16.5 16 18.59 16 21c0 2.03.94 3.84 2.41 5.03-3 1.06-7.41 5.55-7.41 13.47h23c0-7.92-4.41-12.41-7.41-13.47 1.47-1.19 2.41-3 2.41-5.03 0-2.41-1.33-4.5-3.28-5.62.49-.67.78-1.49.78-2.38 0-2.21-1.79-4-4-4z" fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round"/></svg>`,
    'N': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M 22,10 C 32.5,11 38.5,18 38,39 L 15,39 C 15,30 25,32.5 23,18 C 21.5,14.5 12,14 12,14 C 12,14 11,21 17,22.5 C 15.5,22.5 11.5,21 11.5,15 C 11.5,10.5 17.5,9.5 22,10 z" fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round"/><path d="M 24,18 C 24.38,19.92 22.45,21.37 20.53,21 C 18.61,20.62 17.16,18.69 17.54,16.77 C 17.92,14.85 19.85,13.4 21.77,13.78 C 23.69,14.16 25.14,16.09 24.76,18 z" fill="#000"/></svg>`,
    'B': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#fff"><path d="M9 36c1.2-2.5 3.5-3.5 6-3.5s4.8 1 6 3.5H9zM15 32c-2.5 0-3.5-1.5-3.5-3s.5-3.5 2-4.5c1.5-1 3.5-1 5 0s2 3 2 4.5-1 3-3.5 3zM15 23.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"/><circle cx="15" cy="11.5" r="2.5"/></g><path d="M15 9.5v-3M13.5 8h3"/></g></svg>`,
    'R': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#fff" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M9 39h27v-3H9v3zM12 36h21v-4H12v4zM11 32h23l-2-16H13l-2 16zM9 16h27v-4h-4v2h-5v-2h-6v2h-5v-2H9v4z"/><path d="M14 29.5h17M14 16.5h17" stroke="#fff" stroke-linecap="butt"/></g></svg>`,
    'Q': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#fff" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M8 36h29v-3H8v3zM11.5 33h22l-1.5-4h-19l-1.5 4zM9 29l4.5-16.5L20 27l2.5-20L25 27l6.5-14.5L36 29H9z"/><circle cx="9" cy="11" r="2"/><circle cx="13.5" cy="11.5" r="2"/><circle cx="22.5" cy="6" r="2"/><circle cx="31.5" cy="11.5" r="2"/><circle cx="36" cy="11" r="2"/></g></svg>`,
    'K': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#fff"><path d="M22.5 11.63c-1.58 0-2.87 1.29-2.87 2.87 0 1.25.8 2.31 1.92 2.68V21.5h-5.5v2h5.5v3.5h-8v2h8V35h-11v4h25v-4h-11v-6h8v-2h-8V23.5h5.5v-2h-5.5v-4.32c1.12-.37 1.92-1.43 1.92-2.68 0-1.58-1.29-2.87-2.87-2.87z"/></g><path d="M22.5 6v4.5M20.25 8.25h4.5"/></g></svg>`,

    'p': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M22.5 9c-2.21 0-4 1.79-4 4 0 .89.29 1.71.78 2.38C17.33 16.5 16 18.59 16 21c0 2.03.94 3.84 2.41 5.03-3 1.06-7.41 5.55-7.41 13.47h23c0-7.92-4.41-12.41-7.41-13.47 1.47-1.19 2.41-3 2.41-5.03 0-2.41-1.33-4.5-3.28-5.62.49-.67.78-1.49.78-2.38 0-2.21-1.79-4-4-4z" fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/></svg>`,
    'n': `<svg viewBox="0 0 45 45" class="piece-svg"><path d="M 22,10 C 32.5,11 38.5,18 38,39 L 15,39 C 15,30 25,32.5 23,18 C 21.5,14.5 12,14 12,14 C 12,14 11,21 17,22.5 C 15.5,22.5 11.5,21 11.5,15 C 11.5,10.5 17.5,9.5 22,10 z" fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round"/><path d="M 24,18 C 24.38,19.92 22.45,21.37 20.53,21 C 18.61,20.62 17.16,18.69 17.54,16.77 C 17.92,14.85 19.85,13.4 21.77,13.78 C 23.69,14.16 25.14,16.09 24.76,18 z" fill="#fff"/></svg>`,
    'b': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#222"><path d="M9 36c1.2-2.5 3.5-3.5 6-3.5s4.8 1 6 3.5H9zM15 32c-2.5 0-3.5-1.5-3.5-3s.5-3.5 2-4.5c1.5-1 3.5-1 5 0s2 3 2 4.5-1 3-3.5 3zM15 23.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7z"/><circle cx="15" cy="11.5" r="2.5"/></g><path d="M15 9.5v-3M13.5 8h3" stroke="#fff"/></g></svg>`,
    'r': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#222" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M9 39h27v-3H9v3zM12 36h21v-4H12v4zM11 32h23l-2-16H13l-2 16zM9 16h27v-4h-4v2h-5v-2h-6v2h-5v-2H9v4z"/><path d="M14 29.5h17M14 16.5h17" stroke="#fff" stroke-linecap="butt"/></g></svg>`,
    'q': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="#222" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M8 36h29v-3H8v3zM11.5 33h22l-1.5-4h-19l-1.5 4zM9 29l4.5-16.5L20 27l2.5-20L25 27l6.5-14.5L36 29H9z"/><circle cx="9" cy="11" r="2"/><circle cx="13.5" cy="11.5" r="2"/><circle cx="22.5" cy="6" r="2"/><circle cx="31.5" cy="11.5" r="2"/><circle cx="36" cy="11" r="2"/></g></svg>`,
    'k': `<svg viewBox="0 0 45 45" class="piece-svg"><g fill="none" fill-rule="evenodd" stroke="#fff" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><g fill="#222"><path d="M22.5 11.63c-1.58 0-2.87 1.29-2.87 2.87 0 1.25.8 2.31 1.92 2.68V21.5h-5.5v2h5.5v3.5h-8v2h8V35h-11v4h25v-4h-11v-6h8v-2h-8V23.5h5.5v-2h-5.5v-4.32c1.12-.37 1.92-1.43 1.92-2.68 0-1.58-1.29-2.87-2.87-2.87z"/></g><path d="M22.5 6v4.5M20.25 8.25h4.5" stroke="#fff"/></g></svg>`
};

const OPENING_BOOK = {
    "e2e4 e7e5 g1f3 b8c6 f1c4": "Italian Game",
    "e2e4 e7e5 g1f3 b8c6 f1b5": "Ruy Lopez",
    "e2e4 c7c5": "Sicilian Defense",
    "e2e4 e7e6": "French Defense",
    "e2e4 c7c6": "Caro-Kann Defense",
    "d2d4 d7d5 c2c4": "Queen's Gambit",
    "d2d4 g8f6 c2c4 g7g6": "King's Indian Defense",
    "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4": "Nimzo-Indian Defense",
    "c2c4": "English Opening",
    "g1f3": "Reti Opening"
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

class SoundFX {
    constructor() { this.ctx = null; }
    init() { if (!this.ctx) this.ctx = new (window.AudioContext || window.webkitAudioContext)(); }
    playTone(freq, duration, type='sine', gainVal=0.3) {
        this.init();
        if (!this.ctx) return;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = type;
        osc.frequency.setValueAtTime(freq, this.ctx.currentTime);
        gain.gain.setValueAtTime(gainVal, this.ctx.currentTime);
        gain.gain.linearRampToValueAtTime(0.01, this.ctx.currentTime + duration);
        osc.connect(gain);
        gain.connect(this.ctx.destination);
        osc.start();
        osc.stop(this.ctx.currentTime + duration);
    }
    playMove() { this.playTone(380, 0.08, 'sine', 0.2); }
    playCapture() { this.playTone(180, 0.12, 'triangle', 0.35); }
    playCheck() { this.playTone(880, 0.16, 'sawtooth', 0.25); }
    playGameOver() { this.playTone(523, 0.3, 'sine', 0.35); }
}

class ChessRulesEngine {
    static isWhite(p) { return p >= 'A' && p <= 'Z'; }
    static isBlack(p) { return p >= 'a' && p <= 'z'; }
    static sameColor(p1, p2) {
        if (p1 === '.' || p2 === '.') return false;
        return (this.isWhite(p1) && this.isWhite(p2)) || (this.isBlack(p1) && this.isBlack(p2));
    }
    static cloneBoard(board) { return board.map(row => [...row]); }

    static findKing(turn, board) {
        const target = turn === 'w' ? 'K' : 'k';
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                if (board[r][c] === target) return [r, c];
            }
        }
        return null;
    }

    static isSquareAttacked(r, c, byTurn, board) {
        const isAttacker = byTurn === 'w' ? this.isWhite : this.isBlack;
        const pawnDir = byTurn === 'w' ? 1 : -1;
        
        for (const dc of [-1, 1]) {
            const pr = r + pawnDir, pc = c + dc;
            if (pr >= 0 && pr < 8 && pc >= 0 && pc < 8) {
                const p = board[pr][pc];
                if ((byTurn === 'w' && p === 'P') || (byTurn === 'b' && p === 'p')) return true;
            }
        }
        const knightD = [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]];
        for (const [dr, dc] of knightD) {
            const nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                const p = board[nr][nc];
                if ((byTurn === 'w' && p === 'N') || (byTurn === 'b' && p === 'n')) return true;
            }
        }
        const directions = [
            [-1, 0, 'R'], [1, 0, 'R'], [0, -1, 'R'], [0, 1, 'R'],
            [-1, -1, 'B'], [-1, 1, 'B'], [1, -1, 'B'], [1, 1, 'B']
        ];
        for (const [dr, dc, type] of directions) {
            let currR = r + dr, currC = c + dc;
            while (currR >= 0 && currR < 8 && currC >= 0 && currC < 8) {
                const p = board[currR][currC];
                if (p !== '.') {
                    if (isAttacker.call(this, p)) {
                        const up = p.toUpperCase();
                        if (up === 'Q' || up === type) return true;
                    }
                    break;
                }
                currR += dr;
                currC += dc;
            }
        }
        for (let dr = -1; dr <= 1; dr++) {
            for (let dc = -1; dc <= 1; dc++) {
                if (dr === 0 && dc === 0) continue;
                const kr = r + dr, kc = c + dc;
                if (kr >= 0 && kr < 8 && kc >= 0 && kc < 8) {
                    const p = board[kr][kc];
                    if ((byTurn === 'w' && p === 'K') || (byTurn === 'b' && p === 'k')) return true;
                }
            }
        }
        return false;
    }

    static isInCheck(turn, board) {
        const kingPos = this.findKing(turn, board);
        if (!kingPos) return false;
        const enemyTurn = turn === 'w' ? 'b' : 'w';
        return this.isSquareAttacked(kingPos[0], kingPos[1], enemyTurn, board);
    }

    static getLegalMoves(r, c, board, turn, castlingRights, epTarget) {
        const piece = board[r][c];
        if (piece === '.') return [];
        if ((turn === 'w' && !this.isWhite(piece)) || (turn === 'b' && !this.isBlack(piece))) return [];

        const pseudoMoves = [];
        const isWhite = this.isWhite(piece);
        const up = piece.toUpperCase();

        if (up === 'P') {
            const fwd = isWhite ? -1 : 1;
            const startR = isWhite ? 6 : 1;
            if (r + fwd >= 0 && r + fwd < 8 && board[r + fwd][c] === '.') {
                pseudoMoves.push([r + fwd, c]);
                if (r === startR && board[r + 2 * fwd][c] === '.') {
                    pseudoMoves.push([r + 2 * fwd, c]);
                }
            }
            for (const dc of [-1, 1]) {
                const tr = r + fwd, tc = c + dc;
                if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8) {
                    const target = board[tr][tc];
                    if (target !== '.' && !this.sameColor(piece, target)) {
                        pseudoMoves.push([tr, tc]);
                    }
                    if (epTarget && epTarget[0] === tr && epTarget[1] === tc) {
                        pseudoMoves.push([tr, tc]);
                    }
                }
            }
        } else if (up === 'N') {
            const knightD = [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]];
            for (const [dr, dc] of knightD) {
                const tr = r + dr, tc = c + dc;
                if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && !this.sameColor(piece, board[tr][tc])) {
                    pseudoMoves.push([tr, tc]);
                }
            }
        } else if (up === 'B' || up === 'R' || up === 'Q') {
            const dirs = [];
            if (up === 'B' || up === 'Q') dirs.push([-1,-1],[-1,1],[1,-1],[1,1]);
            if (up === 'R' || up === 'Q') dirs.push([-1,0],[1,0],[0,-1],[0,1]);
            for (const [dr, dc] of dirs) {
                let tr = r + dr, tc = c + dc;
                while (tr >= 0 && tr < 8 && tc >= 0 && tc < 8) {
                    const target = board[tr][tc];
                    if (target === '.') {
                        pseudoMoves.push([tr, tc]);
                    } else {
                        if (!this.sameColor(piece, target)) pseudoMoves.push([tr, tc]);
                        break;
                    }
                    tr += dr;
                    tc += dc;
                }
            }
        } else if (up === 'K') {
            for (let dr = -1; dr <= 1; dr++) {
                for (let dc = -1; dc <= 1; dc++) {
                    if (dr === 0 && dc === 0) continue;
                    const tr = r + dr, tc = c + dc;
                    if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && !this.sameColor(piece, board[tr][tc])) {
                        pseudoMoves.push([tr, tc]);
                    }
                }
            }
            const enemy = turn === 'w' ? 'b' : 'w';
            if (!this.isInCheck(turn, board)) {
                if (isWhite && r === 7 && c === 4) {
                    if (castlingRights.K && board[7][5] === '.' && board[7][6] === '.' &&
                        !this.isSquareAttacked(7, 5, enemy, board) && !this.isSquareAttacked(7, 6, enemy, board)) {
                        pseudoMoves.push([7, 6]);
                    }
                    if (castlingRights.Q && board[7][3] === '.' && board[7][2] === '.' && board[7][1] === '.' &&
                        !this.isSquareAttacked(7, 3, enemy, board) && !this.isSquareAttacked(7, 2, enemy, board)) {
                        pseudoMoves.push([7, 2]);
                    }
                } else if (!isWhite && r === 0 && c === 4) {
                    if (castlingRights.k && board[0][5] === '.' && board[0][6] === '.' &&
                        !this.isSquareAttacked(0, 5, enemy, board) && !this.isSquareAttacked(0, 6, enemy, board)) {
                        pseudoMoves.push([0, 6]);
                    }
                    if (castlingRights.q && board[0][3] === '.' && board[0][2] === '.' && board[0][1] === '.' &&
                        !this.isSquareAttacked(0, 3, enemy, board) && !this.isSquareAttacked(0, 2, enemy, board)) {
                        pseudoMoves.push([0, 2]);
                    }
                }
            }
        }

        return pseudoMoves.filter(([tr, tc]) => {
            const nextBoard = this.cloneBoard(board);
            nextBoard[tr][tc] = piece;
            nextBoard[r][c] = '.';
            if (up === 'P' && epTarget && tr === epTarget[0] && tc === epTarget[1]) {
                const capR = isWhite ? tr + 1 : tr - 1;
                nextBoard[capR][tc] = '.';
            }
            return !this.isInCheck(turn, nextBoard);
        });
    }

    static getAllLegalMoves(turn, board, castlingRights, epTarget) {
        const moves = [];
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const legals = this.getLegalMoves(r, c, board, turn, castlingRights, epTarget);
                for (const [tr, tc] of legals) {
                    moves.push({ from: [r, c], to: [tr, tc] });
                }
            }
        }
        return moves;
    }
}

class ChessApp {
    constructor() {
        this.board = ChessRulesEngine.cloneBoard(INITIAL_BOARD);
        this.turn = 'w';
        this.castlingRights = { K: true, Q: true, k: true, q: true };
        this.enPassantTarget = null;
        this.halfMoveClock = 0;
        this.fullMoveNumber = 1;
        this.moveHistory = [];
        this.uciHistory = [];
        this.selectedSquare = null;
        this.legalTargets = [];
        this.lastMove = null;
        this.isThinking = false;
        this.isGameOver = false;
        this.isFlipped = false;
        this.showThreatMap = false;
        this.isGameActive = false;

        this.sound = new SoundFX();
        this.playMode = 'human_white';
        this.timeControl = 'blitz_3_0';
        this.whiteTime = 180.0;
        this.blackTime = 180.0;
        this.increment = 0.0;
        this.clockInterval = null;
        this.lastClockTick = 0;
        this.lastEvalScore = 0;

        this.puzzles = [];
        this.currentPuzzleIdx = 0;

        this.initDOM();
        this.bindEvents();
        this.initIdleState();
        this.loadPuzzles();
    }

    initDOM() {
        this.boardEl = document.getElementById('board');
        this.startBtn = document.getElementById('start-game-btn');
        this.evalFillEl = document.getElementById('eval-fill');
        this.evalBadgeEl = document.getElementById('eval-badge');
        this.teleNodesEl = document.getElementById('tele-nodes');
        this.teleNpsEl = document.getElementById('tele-nps');
        this.teleDepthEl = document.getElementById('tele-depth');
        this.telePvEl = document.getElementById('tele-pv');
        this.oracleTextEl = document.getElementById('oracle-text');
        this.openingBadgeEl = document.getElementById('opening-badge');
        this.moveGradeEl = document.getElementById('move-grade-badge');
        this.moveHistoryEl = document.getElementById('move-history');
        this.topClockEl = document.getElementById('top-clock');
        this.bottomClockEl = document.getElementById('bottom-clock');
        this.engineStatusEl = document.getElementById('engine-status');
        this.statusDotEl = document.getElementById('status-dot');
        this.heatmapCanvas = document.getElementById('heatmap-canvas');
        this.promotionModal = document.getElementById('promotion-modal');
        this.promotionChoicesEl = document.getElementById('promotion-choices');
        this.gameoverModal = document.getElementById('gameover-modal');
        this.gameoverTitleEl = document.getElementById('gameover-title');
        this.gameoverSubEl = document.getElementById('gameover-sub');
    }

    bindEvents() {
        this.startBtn.addEventListener('click', () => {
            if (this.isGameActive) {
                this.initIdleState();
            } else {
                this.startMatch();
            }
        });

        document.getElementById('flip-btn').addEventListener('click', () => this.flipBoard());
        document.getElementById('undo-btn').addEventListener('click', () => this.undoMove());
        document.getElementById('hint-btn').addEventListener('click', () => this.requestHint());
        document.getElementById('threat-toggle-btn').addEventListener('click', () => this.toggleThreatMap());
        document.getElementById('modal-restart-btn').addEventListener('click', () => {
            this.gameoverModal.classList.add('hidden');
            this.initIdleState();
            this.startMatch();
        });

        document.getElementById('mode-select').addEventListener('change', (e) => {
            this.playMode = e.target.value;
            this.isFlipped = (this.playMode === 'human_black');
            this.initIdleState();
        });

        document.getElementById('tc-select').addEventListener('change', (e) => {
            this.timeControl = e.target.value;
            this.resetClocks();
        });

        document.getElementById('theme-select').addEventListener('change', (e) => {
            document.body.className = `theme-${e.target.value}`;
        });

        document.getElementById('copy-fen-btn').addEventListener('click', () => this.copyFEN());
        document.getElementById('copy-pgn-btn').addEventListener('click', () => this.copyPGN());

        document.getElementById('tab-play').addEventListener('click', () => this.switchTab('play'));
        document.getElementById('tab-puzzles').addEventListener('click', () => this.switchTab('puzzles'));
        document.getElementById('tab-analysis').addEventListener('click', () => this.switchTab('analysis'));

        document.getElementById('puzzle-hint-btn').addEventListener('click', () => this.showPuzzleHint());
        document.getElementById('puzzle-next-btn').addEventListener('click', () => this.nextPuzzle());
    }

    switchTab(tab) {
        document.querySelectorAll('.nav-tab').forEach(b => b.classList.remove('active'));
        document.getElementById(`tab-${tab}`).classList.add('active');

        const puzzlesPanel = document.getElementById('puzzles-panel');
        const settingsPanel = document.getElementById('settings-panel');

        if (tab === 'puzzles') {
            this.stopClock();
            puzzlesPanel.classList.remove('hidden');
            settingsPanel.classList.add('hidden');
            this.loadPuzzle(this.currentPuzzleIdx);
        } else {
            puzzlesPanel.classList.add('hidden');
            settingsPanel.classList.remove('hidden');
        }
    }

    resetClocks() {
        this.stopClock();
        this.increment = 0.0;

        if (this.timeControl === 'bullet_1_0') {
            this.whiteTime = 60.0;
            this.blackTime = 60.0;
        } else if (this.timeControl === 'bullet_1_1') {
            this.whiteTime = 60.0;
            this.blackTime = 60.0;
            this.increment = 1.0;
        } else if (this.timeControl === 'blitz_3_0') {
            this.whiteTime = 180.0;
            this.blackTime = 180.0;
        } else if (this.timeControl === 'blitz_3_2') {
            this.whiteTime = 180.0;
            this.blackTime = 180.0;
            this.increment = 2.0;
        } else if (this.timeControl === 'rapid_10_0') {
            this.whiteTime = 600.0;
            this.blackTime = 600.0;
        } else {
            this.whiteTime = 0.0;
            this.blackTime = 0.0;
        }
        this.updateClockDisplay();
    }

    startClock() {
        this.stopClock();
        if (this.timeControl === 'fixed_depth') return;
        this.lastClockTick = performance.now();
        this.clockInterval = setInterval(() => this.tickClock(), 100);
        this.updateActiveClockHUD();
    }

    stopClock() {
        if (this.clockInterval) {
            clearInterval(this.clockInterval);
            this.clockInterval = null;
        }
        this.topClockEl.classList.remove('active');
        this.bottomClockEl.classList.remove('active');
    }

    tickClock() {
        if (!this.isGameActive || this.isGameOver || this.timeControl === 'fixed_depth') return;
        const now = performance.now();
        const elapsed = (now - this.lastClockTick) / 1000.0;
        this.lastClockTick = now;

        if (this.turn === 'w') {
            this.whiteTime = Math.max(0.0, this.whiteTime - elapsed);
            if (this.whiteTime <= 0.0) {
                this.isGameOver = true;
                this.stopClock();
                this.sound.playGameOver();
                this.showGameOver("Time Out", "Black won on time.");
            }
        } else {
            this.blackTime = Math.max(0.0, this.blackTime - elapsed);
            if (this.blackTime <= 0.0) {
                this.isGameOver = true;
                this.stopClock();
                this.sound.playGameOver();
                this.showGameOver("Time Out", "White won on time.");
            }
        }
        this.updateClockDisplay();
    }

    updateActiveClockHUD() {
        if (!this.isGameActive || this.timeControl === 'fixed_depth') {
            this.topClockEl.classList.remove('active');
            this.bottomClockEl.classList.remove('active');
            return;
        }
        const isWhiteBottom = !this.isFlipped;
        if (this.turn === 'w') {
            if (isWhiteBottom) {
                this.bottomClockEl.classList.add('active');
                this.topClockEl.classList.remove('active');
            } else {
                this.topClockEl.classList.add('active');
                this.bottomClockEl.classList.remove('active');
            }
        } else {
            if (isWhiteBottom) {
                this.topClockEl.classList.add('active');
                this.bottomClockEl.classList.remove('active');
            } else {
                this.bottomClockEl.classList.add('active');
                this.topClockEl.classList.remove('active');
            }
        }
    }

    initIdleState() {
        this.stopClock();
        this.isGameActive = false;
        this.isGameOver = false;
        this.isThinking = false;
        this.board = ChessRulesEngine.cloneBoard(INITIAL_BOARD);
        this.turn = 'w';
        this.castlingRights = { K: true, Q: true, k: true, q: true };
        this.enPassantTarget = null;
        this.halfMoveClock = 0;
        this.fullMoveNumber = 1;
        this.moveHistory = [];
        this.uciHistory = [];
        this.selectedSquare = null;
        this.legalTargets = [];
        this.lastMove = null;

        this.resetClocks();
        this.renderBoard();
        this.updateEvalBar(0);
        this.moveHistoryEl.innerHTML = '<div class="empty-history-notice">Moves will be recorded as the game proceeds.</div>';
        this.oracleTextEl.textContent = 'Click "Start Match" or make a move to begin.';
        this.openingBadgeEl.textContent = 'Standard Start';
        this.moveGradeEl.classList.add('hidden');
        this.setStatus('Ready to Start', false);
        this.startBtn.textContent = 'Start Match';
    }

    startMatch() {
        this.isGameActive = true;
        this.isGameOver = false;
        this.startBtn.textContent = 'Reset Match';
        this.setStatus('Match Active', false);
        this.oracleTextEl.textContent = 'Match started. White to move.';

        if (this.timeControl !== 'fixed_depth') {
            this.startClock();
        }

        if (this.playMode === 'human_black' && this.turn === 'w') {
            this.triggerEngineMove();
        }
    }

    coordsToSquare(r, c) { return `${String.fromCharCode(97 + c)}${8 - r}`; }
    squareToCoords(sq) { return [8 - parseInt(sq[1]), sq.charCodeAt(0) - 97]; }

    renderBoard() {
        if (!this.boardEl) return;
        this.boardEl.innerHTML = '';

        const inCheck = ChessRulesEngine.isInCheck(this.turn, this.board);
        const kingPos = inCheck ? ChessRulesEngine.findKing(this.turn, this.board) : null;

        for (let rowIdx = 0; rowIdx < 8; rowIdx++) {
            for (let colIdx = 0; colIdx < 8; colIdx++) {
                const r = this.isFlipped ? 7 - rowIdx : rowIdx;
                const c = this.isFlipped ? 7 - colIdx : colIdx;

                const sqEl = document.createElement('div');
                const isLight = (r + c) % 2 === 0;
                sqEl.className = `square ${isLight ? 'light' : 'dark'}`;
                const sqName = this.coordsToSquare(r, c);

                if (this.selectedSquare && this.selectedSquare[0] === r && this.selectedSquare[1] === c) {
                    sqEl.classList.add('selected');
                }
                if (this.lastMove && (this.lastMove.from === sqName || this.lastMove.to === sqName)) {
                    sqEl.classList.add('last-move');
                }
                if (kingPos && kingPos[0] === r && kingPos[1] === c) {
                    sqEl.classList.add('in-check');
                }

                const isTarget = this.legalTargets.some(([tr, tc]) => tr === r && tc === c);
                if (isTarget) {
                    const isCapture = (this.board[r][c] !== '.') || (this.enPassantTarget && this.enPassantTarget[0] === r && this.enPassantTarget[1] === c);
                    const targetEl = document.createElement('div');
                    targetEl.className = isCapture ? 'target-capture' : 'target-dot';
                    sqEl.appendChild(targetEl);
                }

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

        if (this.showThreatMap) this.drawThreatMap();
    }

    handleSquareClick(r, c) {
        if (this.isThinking || this.isGameOver) return;
        if (this.playMode === 'human_white' && this.turn !== 'w') return;
        if (this.playMode === 'human_black' && this.turn !== 'b') return;

        const piece = this.board[r][c];
        const isMyPiece = (this.turn === 'w' && ChessRulesEngine.isWhite(piece)) || (this.turn === 'b' && ChessRulesEngine.isBlack(piece));

        if (this.selectedSquare) {
            const [srcR, srcC] = this.selectedSquare;
            const isTarget = this.legalTargets.some(([tr, tc]) => tr === r && tc === c);

            if (srcR === r && srcC === c) {
                this.selectedSquare = null;
                this.legalTargets = [];
                this.renderBoard();
                return;
            }

            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, this.board, this.turn, this.castlingRights, this.enPassantTarget);
                this.renderBoard();
                return;
            }

            if (isTarget) {
                const srcPiece = this.board[srcR][srcC];
                if ((srcPiece === 'P' && r === 0) || (srcPiece === 'p' && r === 7)) {
                    this.promptPromotion(srcR, srcC, r, c);
                    return;
                }
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(r, c)}`;
                
                // If game was not started, auto-activate match on first move
                if (!this.isGameActive) {
                    this.startMatch();
                }

                this.executeMove(srcR, srcC, r, c, uciStr);
                this.selectedSquare = null;
                this.legalTargets = [];
            } else {
                this.selectedSquare = null;
                this.legalTargets = [];
                this.renderBoard();
            }
        } else {
            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, this.board, this.turn, this.castlingRights, this.enPassantTarget);
                this.renderBoard();
            }
        }
    }

    promptPromotion(srcR, srcC, tr, tc) {
        this.promotionChoicesEl.innerHTML = '';
        const choices = this.turn === 'w' ? ['Q', 'R', 'B', 'N'] : ['q', 'r', 'b', 'n'];
        for (const p of choices) {
            const btn = document.createElement('div');
            btn.className = 'promo-choice';
            btn.innerHTML = SVG_PIECES[p];
            btn.addEventListener('click', () => {
                this.promotionModal.classList.add('hidden');
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(tr, tc)}${p.toLowerCase()}`;
                if (!this.isGameActive) this.startMatch();
                this.executeMove(srcR, srcC, tr, tc, uciStr, p);
            });
            this.promotionChoicesEl.appendChild(btn);
        }
        this.promotionModal.classList.remove('hidden');
    }

    executeMove(srcR, srcC, tr, tc, uciStr, promoPiece = null) {
        const piece = this.board[srcR][srcC];
        const target = this.board[tr][tc];
        const isCapture = target !== '.' || (piece.toUpperCase() === 'P' && this.enPassantTarget && tr === this.enPassantTarget[0] && tc === this.enPassantTarget[1]);

        if (piece.toUpperCase() === 'P' && this.enPassantTarget && tr === this.enPassantTarget[0] && tc === this.enPassantTarget[1]) {
            const capR = this.turn === 'w' ? tr + 1 : tr - 1;
            this.board[capR][tc] = '.';
        }

        this.board[tr][tc] = promoPiece || piece;
        this.board[srcR][srcC] = '.';

        if (piece === 'K' && srcR === 7 && srcC === 4) {
            if (tc === 6) { this.board[7][5] = 'R'; this.board[7][7] = '.'; }
            if (tc === 2) { this.board[7][3] = 'R'; this.board[7][0] = '.'; }
        } else if (piece === 'k' && srcR === 0 && srcC === 4) {
            if (tc === 6) { this.board[0][5] = 'r'; this.board[0][7] = '.'; }
            if (tc === 2) { this.board[0][3] = 'r'; this.board[0][0] = '.'; }
        }

        if (piece === 'K') { this.castlingRights.K = false; this.castlingRights.Q = false; }
        if (piece === 'k') { this.castlingRights.k = false; this.castlingRights.q = false; }
        if (piece === 'R' && srcR === 7 && srcC === 7) this.castlingRights.K = false;
        if (piece === 'R' && srcR === 7 && srcC === 0) this.castlingRights.Q = false;
        if (piece === 'r' && srcR === 0 && srcC === 7) this.castlingRights.k = false;
        if (piece === 'r' && srcR === 0 && srcC === 0) this.castlingRights.q = false;

        if (piece.toUpperCase() === 'P' && Math.abs(tr - srcR) === 2) {
            this.enPassantTarget = [(srcR + tr) / 2, srcC];
        } else {
            this.enPassantTarget = null;
        }

        this.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(tr, tc) };
        this.uciHistory.push(uciStr);

        if (this.increment > 0) {
            if (this.turn === 'w') this.whiteTime += this.increment;
            else this.blackTime += this.increment;
            this.updateClockDisplay();
        }

        if (isCapture) this.sound.playCapture();
        else this.sound.playMove();

        this.turn = this.turn === 'w' ? 'b' : 'w';
        if (this.turn === 'w') this.fullMoveNumber++;

        this.updateActiveClockHUD();
        this.renderBoard();
        this.appendMoveHistory(uciStr, piece, isCapture);
        this.checkGameEnd();
        this.updateOpening();

        if (!this.isGameOver && this.isGameActive) {
            if ((this.playMode === 'human_white' && this.turn === 'b') || (this.playMode === 'human_black' && this.turn === 'w')) {
                this.triggerEngineMove();
            }
        }
    }

    appendMoveHistory(uciStr, piece, isCapture) {
        if (this.moveHistory.length === 0) {
            this.moveHistoryEl.innerHTML = '';
        }
        this.moveHistory.push(uciStr);

        const isWhite = this.turn === 'b';
        if (isWhite) {
            const row = document.createElement('div');
            row.className = 'history-row';
            row.id = `hist-row-${this.fullMoveNumber}`;
            row.innerHTML = `<span class="history-num">${this.fullMoveNumber}.</span><span class="history-move">${uciStr}</span><span class="history-move"></span>`;
            this.moveHistoryEl.appendChild(row);
        } else {
            const row = document.getElementById(`hist-row-${this.fullMoveNumber}`);
            if (row) {
                row.children[2].textContent = uciStr;
            }
        }
        this.moveHistoryEl.scrollTop = this.moveHistoryEl.scrollHeight;
    }

    updateOpening() {
        const uciLine = this.uciHistory.slice(0, 6).join(' ');
        for (const [seq, name] of Object.entries(OPENING_BOOK)) {
            if (uciLine.startsWith(seq)) {
                this.openingBadgeEl.textContent = name;
                return;
            }
        }
    }

    checkGameEnd() {
        const legalMoves = ChessRulesEngine.getAllLegalMoves(this.turn, this.board, this.castlingRights, this.enPassantTarget);
        const inCheck = ChessRulesEngine.isInCheck(this.turn, this.board);

        if (legalMoves.length === 0) {
            this.isGameOver = true;
            this.stopClock();
            if (inCheck) {
                this.sound.playGameOver();
                const winner = this.turn === 'w' ? "Black" : "White";
                this.showGameOver("Checkmate", `${winner} won the match.`);
            } else {
                this.showGameOver("Stalemate", "Game drawn by stalemate.");
            }
        } else if (inCheck) {
            this.sound.playCheck();
            this.oracleTextEl.textContent = "Check. King under direct attack.";
        }
    }

    showGameOver(title, sub) {
        this.gameoverTitleEl.textContent = title;
        this.gameoverSubEl.textContent = sub;
        this.gameoverModal.classList.remove('hidden');
    }

    getFEN() {
        let fen = '';
        for (let r = 0; r < 8; r++) {
            let empty = 0;
            for (let c = 0; c < 8; c++) {
                const p = this.board[r][c];
                if (p === '.') empty++;
                else {
                    if (empty > 0) { fen += empty; empty = 0; }
                    fen += p;
                }
            }
            if (empty > 0) fen += empty;
            if (r < 7) fen += '/';
        }
        fen += ` ${this.turn} `;
        let castling = '';
        if (this.castlingRights.K) castling += 'K';
        if (this.castlingRights.Q) castling += 'Q';
        if (this.castlingRights.k) castling += 'k';
        if (this.castlingRights.q) castling += 'q';
        fen += (castling || '-') + ' ';
        fen += (this.enPassantTarget ? this.coordsToSquare(this.enPassantTarget[0], this.enPassantTarget[1]) : '-') + ' ';
        fen += `${this.halfMoveClock} ${this.fullMoveNumber}`;
        return fen;
    }

    async triggerEngineMove() {
        if (this.isThinking || this.isGameOver || !this.isGameActive) return;
        this.isThinking = true;
        this.setStatus("Engine Calculating...", true);

        const fen = this.getFEN();
        const depth = 12;
        const wtimeMs = Math.round(this.whiteTime * 1000);
        const btimeMs = Math.round(this.blackTime * 1000);
        const incMs = Math.round(this.increment * 1000);

        try {
            const resp = await fetch('/api/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    fen,
                    depth,
                    wtime: (this.timeControl === 'fixed_depth') ? 0 : wtimeMs,
                    btime: (this.timeControl === 'fixed_depth') ? 0 : btimeMs,
                    winc: incMs,
                    binc: incMs
                })
            });
            const data = await resp.json();

            if (data.best_move && this.isGameActive) {
                const src = data.best_move.substring(0, 2);
                const dst = data.best_move.substring(2, 4);
                const promo = data.best_move.length > 4 ? data.best_move[4] : null;
                const [srcR, srcC] = this.squareToCoords(src);
                const [dstR, dstC] = this.squareToCoords(dst);

                this.executeMove(srcR, srcC, dstR, dstC, data.best_move, promo);

                this.teleNodesEl.textContent = Number(data.nodes || 0).toLocaleString();
                this.teleNpsEl.textContent = `${(Number(data.nps || 0) / 1e6).toFixed(2)}M NPS`;
                this.teleDepthEl.textContent = `Depth ${depth}`;
                this.telePvEl.textContent = data.pv || '--';

                this.updateEvalBar(data.score || 0);
                this.updateOracle(data.score || 0, data.pv);
            }
        } catch (e) {
            console.error("Engine API error:", e);
        } finally {
            this.isThinking = false;
            if (this.isGameActive) this.setStatus("Engine Ready", false);
        }
    }

    updateEvalBar(scoreCp) {
        const normalized = Math.max(-1000, Math.min(1000, scoreCp));
        const whiteWinProb = 1 / (1 + Math.pow(10, -normalized / 400));
        const fillPct = (whiteWinProb * 100).toFixed(1);
        this.evalFillEl.style.height = `${fillPct}%`;

        const evalText = (scoreCp >= 0 ? '+' : '') + (scoreCp / 100).toFixed(1);
        this.evalBadgeEl.textContent = evalText;
    }

    updateOracle(scoreCp, pv) {
        const cp = scoreCp;
        const delta = cp - this.lastEvalScore;
        this.lastEvalScore = cp;

        this.moveGradeEl.classList.remove('hidden', 'brilliant', 'best', 'good', 'inaccuracy', 'blunder');

        if (Math.abs(delta) > 300) {
            if ((this.turn === 'b' && delta < -300) || (this.turn === 'w' && delta > 300)) {
                this.moveGradeEl.className = 'move-grade-badge brilliant';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Brilliant Move</span>`;
                this.oracleTextEl.textContent = "Sharp tactical breakthrough. Position evaluates favorably.";
            } else {
                this.moveGradeEl.className = 'move-grade-badge blunder';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Blunder</span>`;
                this.oracleTextEl.textContent = "Inaccuracy detected. Advantage shifted to opponent.";
            }
        } else if (Math.abs(cp) > 500) {
            this.oracleTextEl.textContent = "Decisive advantage. Technical endgame conversion.";
        } else {
            this.oracleTextEl.textContent = "Balanced position. Maintaining central pressure.";
        }
    }

    async requestHint() {
        if (this.isThinking || this.isGameOver) return;
        const fen = this.getFEN();
        const resp = await fetch('/api/analyze', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fen, depth: 9 })
        });
        const data = await resp.json();
        if (data.best_move) {
            this.oracleTextEl.textContent = `Engine recommendation: ${data.best_move.substring(0, 2)} -> ${data.best_move.substring(2, 4)}`;
        }
    }

    toggleThreatMap() {
        this.showThreatMap = !this.showThreatMap;
        if (this.showThreatMap) {
            this.heatmapCanvas.classList.remove('hidden');
            this.drawThreatMap();
        } else {
            this.heatmapCanvas.classList.add('hidden');
        }
    }

    drawThreatMap() {
        const canvas = this.heatmapCanvas;
        const ctx = canvas.getContext('2d');
        canvas.width = 540;
        canvas.height = 540;
        ctx.clearRect(0, 0, 540, 540);

        const sqSize = 540 / 8;
        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const dr = this.isFlipped ? 7 - r : r;
                const dc = this.isFlipped ? 7 - c : c;

                const attackedByWhite = ChessRulesEngine.isSquareAttacked(r, c, 'w', this.board);
                const attackedByBlack = ChessRulesEngine.isSquareAttacked(r, c, 'b', this.board);

                if (attackedByWhite && attackedByBlack) {
                    ctx.fillStyle = 'rgba(168, 85, 247, 0.25)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                } else if (attackedByWhite) {
                    ctx.fillStyle = 'rgba(56, 189, 248, 0.2)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                } else if (attackedByBlack) {
                    ctx.fillStyle = 'rgba(244, 63, 94, 0.2)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                }
            }
        }
    }

    async loadPuzzles() {
        try {
            const resp = await fetch('/api/puzzles');
            this.puzzles = await resp.json();
        } catch (e) {
            console.error("Failed to load puzzles:", e);
        }
    }

    loadPuzzle(idx) {
        if (!this.puzzles || this.puzzles.length === 0) return;
        this.currentPuzzleIdx = idx % this.puzzles.length;
        const p = this.puzzles[this.currentPuzzleIdx];

        document.getElementById('puzzle-title').textContent = p.title;
        document.getElementById('puzzle-event').textContent = p.event;
        document.getElementById('puzzle-desc').textContent = p.desc;
        document.getElementById('puzzle-progress').textContent = `${this.currentPuzzleIdx + 1} / ${this.puzzles.length}`;

        this.setBoardFromFEN(p.fen);
        this.oracleTextEl.textContent = `${p.title}: ${p.desc}`;
    }

    setBoardFromFEN(fenStr) {
        const parts = fenStr.split(' ');
        const ranks = parts[0].split('/');
        this.board = [];
        for (let r = 0; r < 8; r++) {
            const row = [];
            for (let i = 0; i < ranks[r].length; i++) {
                const ch = ranks[r][i];
                if (ch >= '1' && ch <= '8') {
                    for (let k = 0; k < parseInt(ch); k++) row.push('.');
                } else {
                    row.push(ch);
                }
            }
            this.board.push(row);
        }
        this.turn = parts[1] || 'w';
        this.renderBoard();
    }

    showPuzzleHint() {
        if (!this.puzzles[this.currentPuzzleIdx]) return;
        const p = this.puzzles[this.currentPuzzleIdx];
        this.oracleTextEl.textContent = `Hint: ${p.hint}`;
    }

    nextPuzzle() {
        this.loadPuzzle(this.currentPuzzleIdx + 1);
    }

    flipBoard() {
        this.isFlipped = !this.isFlipped;
        this.renderBoard();
    }

    undoMove() {
        if (this.moveHistory.length === 0) return;
        this.initIdleState();
    }

    copyFEN() {
        navigator.clipboard.writeText(this.getFEN());
        this.setStatus("FEN copied to clipboard", false);
    }

    copyPGN() {
        const pgn = this.moveHistory.join(' ');
        navigator.clipboard.writeText(pgn);
        this.setStatus("PGN copied to clipboard", false);
    }

    setStatus(text, thinking = false) {
        this.engineStatusEl.textContent = text;
        if (thinking) this.statusDotEl.classList.add('thinking');
        else this.statusDotEl.classList.remove('thinking');
    }

    updateClockDisplay() {
        if (this.timeControl === 'fixed_depth') {
            this.topClockEl.textContent = "--:--";
            this.bottomClockEl.textContent = "--:--";
            return;
        }
        const fmt = (s) => {
            const m = Math.floor(s / 60);
            const sec = Math.floor(s % 60);
            return `${m.toString().padStart(2, '0')}:${sec.toString().padStart(2, '0')}`;
        };
        const isWhiteBottom = !this.isFlipped;
        if (isWhiteBottom) {
            this.topClockEl.textContent = fmt(this.blackTime);
            this.bottomClockEl.textContent = fmt(this.whiteTime);
        } else {
            this.topClockEl.textContent = fmt(this.whiteTime);
            this.bottomClockEl.textContent = fmt(this.blackTime);
        }
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.app = new ChessApp();
});
