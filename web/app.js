/**
 * HEAVEN'S GATE CHESS ENGINE - WEB APPLICATION
 * Bulletproof Tab Navigation + Real Piece Images + Isolated Game & Puzzle States
 */

const PIECE_NAMES = {
    'P': 'wP', 'N': 'wN', 'B': 'wB', 'R': 'wR', 'Q': 'wQ', 'K': 'wK',
    'p': 'bP', 'n': 'bN', 'b': 'bB', 'r': 'bR', 'q': 'bQ', 'k': 'bK'
};

const OPENING_BOOK = {
    "e2e4 e7e5 g1f3 b8c6 f1c4": "Italian Game",
    "e2e4 e7e5 g1f3 b8c6 f1b5": "Ruy Lopez",
    "e2e4 c7c5": "Sicilian Defense",
    "e2e4 e7e6": "French Defense",
    "e2e4 c7c6": "Caro-Kann Defense",
    "d2d4 d7d5 c2c4": "Queen's Gambit",
    "d2d4 g8f6 c2c4 g7g6": "King's Indian Defense",
    "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4": "Nimzo-Indian",
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

const DEFAULT_PUZZLES = [
    {
        id: 1,
        title: "Kasparov's Immortal Attack",
        event: "Kasparov vs Topalov, Wijk aan Zee 1999",
        fen: "b2r3r/k4p1p/p2q1np1/NppP4/3p1Q2/P4PPB/1PP4P/1K1RR3 w - - 0 1",
        turn: "w",
        hint: "Sacrifice the rook to shatter the king's pawn shield!",
        solution: "d1d4",
        desc: "Find the immortal rook sacrifice that opens the fatal d-file ray."
    },
    {
        id: 2,
        title: "The Greek Gift Sacrifice",
        event: "Classical Tactical Theme",
        fen: "r1bq1rk1/ppp2ppp/2n1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQ - 4 7",
        turn: "w",
        hint: "Bxh7+ crashes through the kingside fortress!",
        solution: "d3h7",
        desc: "Classic bishop sacrifice on h7 followed by Ng5+ and Qh5."
    },
    {
        id: 3,
        title: "Mikhail Tal's Knight Sorcery",
        event: "Tal vs Larsen, Bled 1965",
        fen: "r1b2rk1/pp1n1ppp/2p1pn2/q2p2B1/2PP4/2P1PN2/P1Q1BPPP/R3K2R w KQ - 3 10",
        turn: "w",
        hint: "Break through the center with e4!",
        solution: "e1g1",
        desc: "Prepare the central explosion and open tactical diagonal lines."
    },
    {
        id: 4,
        title: "Opera House Checkmate",
        event: "Paul Morphy vs Duke of Brunswick, Paris 1858",
        fen: "4kb1r/p2n1ppp/4q3/4p1B1/4P3/1Q6/PPP2PPP/2KR4 w k - 1 1",
        turn: "w",
        hint: "Queen sacrifice on b8 leads to back-rank mate with Rd8#!",
        solution: "b3b8",
        desc: "The most famous queen sacrifice in chess history."
    },
    {
        id: 5,
        title: "Fischer's Game of the Century",
        event: "Donald Byrne vs Bobby Fischer, New York 1956",
        fen: "r3r1k1/pp3pbp/1qp1b1p1/4B3/2P5/2N2N1P/PP1Q1PP1/R4RK1 b - - 0 16",
        turn: "b",
        hint: "Offer the queen with Be6 to build a lethal discovered attack windmill!",
        solution: "e6c4",
        desc: "Fischer's brilliant 13-year-old masterpiece."
    },
    {
        id: 6,
        title: "Smothered Mate (Philidor's Legacy)",
        event: "Classical Tactical Motif",
        fen: "6k1/5ppp/8/8/8/8/1Q4PP/6K1 w - - 0 1",
        turn: "w",
        hint: "Queen check on b8 forces back rank mate!",
        solution: "b2b8",
        desc: "Deliver the unstoppable back-rank checkmate."
    }
];

class SoundFX {
    constructor() { this.ctx = null; }
    init() { if (!this.ctx) this.ctx = new (window.AudioContext || window.webkitAudioContext)(); }
    playTone(freq, duration, type='sine', gainVal=0.25) {
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
    playMove() { this.playTone(360, 0.07, 'sine', 0.2); }
    playCapture() { this.playTone(200, 0.1, 'triangle', 0.3); }
    playCheck() { this.playTone(750, 0.15, 'sawtooth', 0.2); }
    playGameOver() { this.playTone(440, 0.25, 'sine', 0.3); }
    playSuccess() { this.playTone(587, 0.12, 'sine', 0.25); setTimeout(() => this.playTone(880, 0.2, 'sine', 0.3), 120); }
    playError() { this.playTone(180, 0.15, 'sawtooth', 0.25); }
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
        this.activeTab = 'play';
        this.sound = new SoundFX();
        this.pieceSet = 'cburnett';

        // Match Game State (100% Isolated from Puzzles)
        this.gameState = {
            board: ChessRulesEngine.cloneBoard(INITIAL_BOARD),
            turn: 'w',
            castlingRights: { K: true, Q: true, k: true, q: true },
            enPassantTarget: null,
            halfMoveClock: 0,
            fullMoveNumber: 1,
            moveHistory: [],
            uciHistory: [],
            lastMove: null,
            isGameActive: false,
            isGameOver: false,
            isThinking: false,
            isFlipped: false,
            playMode: 'human_white',
            timeControl: 'blitz_3_0',
            whiteTime: 180.0,
            blackTime: 180.0,
            increment: 0.0,
            lastEvalScore: 0
        };

        // Puzzle Game State (100% Isolated from Matches)
        this.puzzleState = {
            board: ChessRulesEngine.cloneBoard(INITIAL_BOARD),
            turn: 'w',
            castlingRights: { K: false, Q: false, k: false, q: false },
            enPassantTarget: null,
            lastMove: null,
            isFlipped: false,
            puzzleIdx: 0,
            isSolved: false,
            puzzles: DEFAULT_PUZZLES
        };

        this.selectedSquare = null;
        this.legalTargets = [];
        this.showThreatMap = false;
        this.clockInterval = null;
        this.lastClockTick = 0;

        this.initDOM();
        this.bindEvents();
        this.initPlayMode();
        this.fetchServerPuzzles();
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
        // Tab switching
        document.getElementById('tab-play').addEventListener('click', () => this.switchTab('play'));
        document.getElementById('tab-puzzles').addEventListener('click', () => this.switchTab('puzzles'));
        document.getElementById('tab-telemetry').addEventListener('click', () => this.switchTab('telemetry'));

        // Match Actions
        this.startBtn.addEventListener('click', () => {
            if (this.activeTab !== 'play') {
                this.switchTab('play');
            }
            if (this.gameState.isGameActive) {
                this.initPlayMode();
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
            this.initPlayMode();
            this.startMatch();
        });

        // Settings
        document.getElementById('mode-select').addEventListener('change', (e) => {
            this.gameState.playMode = e.target.value;
            this.gameState.isFlipped = (this.gameState.playMode === 'human_black');
            this.initPlayMode();
        });

        document.getElementById('tc-select').addEventListener('change', (e) => {
            this.gameState.timeControl = e.target.value;
            this.resetClocks();
        });

        document.getElementById('theme-select').addEventListener('change', (e) => {
            document.body.className = `theme-${e.target.value}`;
        });

        const piecesSelect = document.getElementById('pieces-select');
        if (piecesSelect) {
            piecesSelect.addEventListener('change', (e) => {
                this.pieceSet = e.target.value;
                this.renderBoard();
            });
        }

        document.getElementById('copy-fen-btn').addEventListener('click', () => this.copyFEN());
        document.getElementById('copy-pgn-btn').addEventListener('click', () => this.copyPGN());

        // Puzzle Controls
        document.getElementById('puzzle-hint-btn').addEventListener('click', () => this.showPuzzleHint());
        document.getElementById('puzzle-next-btn').addEventListener('click', () => this.nextPuzzle());
    }

    async fetchServerPuzzles() {
        try {
            const resp = await fetch('/api/puzzles');
            if (resp.ok) {
                const data = await resp.json();
                if (Array.isArray(data) && data.length > 0) {
                    this.puzzleState.puzzles = data;
                }
            }
        } catch (e) {
            console.log("Using default embedded puzzles.");
        }
    }

    switchTab(tab) {
        this.activeTab = tab;
        this.selectedSquare = null;
        this.legalTargets = [];

        document.querySelectorAll('.nav-tab').forEach(b => b.classList.remove('active'));
        const tabEl = document.getElementById(`tab-${tab}`);
        if (tabEl) tabEl.classList.add('active');

        const cardCommentary = document.getElementById('card-commentary');
        const cardHistory = document.getElementById('card-history');
        const cardConfig = document.getElementById('card-config');
        const cardPuzzles = document.getElementById('card-puzzles');
        const cardTelemetry = document.getElementById('card-telemetry');

        if (tab === 'play') {
            this.stopClock();
            cardCommentary.classList.remove('hidden');
            cardHistory.classList.remove('hidden');
            cardConfig.classList.remove('hidden');
            cardPuzzles.classList.add('hidden');
            cardTelemetry.classList.add('hidden');

            this.renderPlayView();

        } else if (tab === 'puzzles') {
            this.stopClock();
            cardCommentary.classList.add('hidden');
            cardHistory.classList.add('hidden');
            cardConfig.classList.add('hidden');
            cardTelemetry.classList.add('hidden');
            cardPuzzles.classList.remove('hidden');

            this.renderPuzzleView();

        } else if (tab === 'telemetry') {
            cardCommentary.classList.remove('hidden');
            cardHistory.classList.remove('hidden');
            cardConfig.classList.add('hidden');
            cardPuzzles.classList.add('hidden');
            cardTelemetry.classList.remove('hidden');

            this.renderPlayView();
        }
    }

    renderPlayView() {
        this.renderBoard();
        this.updateClockDisplay();
        this.updateActiveClockHUD();
        this.updateEvalBar(this.gameState.lastEvalScore);
        this.updateOpening();

        if (this.gameState.isGameActive) {
            this.startBtn.textContent = 'Reset Match';
            this.setStatus(this.gameState.isThinking ? 'Calculating' : 'Match Active', this.gameState.isThinking);
            this.oracleTextEl.textContent = `${this.gameState.turn === 'w' ? 'White' : 'Black'} to move.`;
            if (this.gameState.timeControl !== 'fixed_depth') this.startClock();
        } else {
            this.startBtn.textContent = 'Start Match';
            this.setStatus('Ready', false);
            this.oracleTextEl.textContent = 'Click "Start Match" or make a move to begin.';
        }
    }

    renderPuzzleView() {
        this.setStatus('Tactical Training', false);
        this.loadPuzzle(this.puzzleState.puzzleIdx);
    }

    initPlayMode() {
        this.stopClock();
        this.gameState.board = ChessRulesEngine.cloneBoard(INITIAL_BOARD);
        this.gameState.turn = 'w';
        this.gameState.castlingRights = { K: true, Q: true, k: true, q: true };
        this.gameState.enPassantTarget = null;
        this.gameState.halfMoveClock = 0;
        this.gameState.fullMoveNumber = 1;
        this.gameState.moveHistory = [];
        this.gameState.uciHistory = [];
        this.gameState.lastMove = null;
        this.gameState.isGameActive = false;
        this.gameState.isGameOver = false;
        this.gameState.isThinking = false;
        this.gameState.isFlipped = (this.gameState.playMode === 'human_black');
        this.gameState.lastEvalScore = 0;

        this.selectedSquare = null;
        this.legalTargets = [];

        this.resetClocks();
        this.moveHistoryEl.innerHTML = '<div class="empty-notice">No moves played yet.</div>';
        this.openingBadgeEl.textContent = 'Standard Start';
        this.moveGradeEl.classList.add('hidden');

        if (this.activeTab === 'play' || this.activeTab === 'telemetry') {
            this.renderPlayView();
        }
    }

    startMatch() {
        this.gameState.isGameActive = true;
        this.gameState.isGameOver = false;
        this.startBtn.textContent = 'Reset Match';
        this.setStatus('Match Active', false);
        this.oracleTextEl.textContent = 'Match started. White to move.';

        if (this.gameState.timeControl !== 'fixed_depth') {
            this.startClock();
        }

        if (this.gameState.playMode === 'human_black' && this.gameState.turn === 'w') {
            this.triggerEngineMove();
        }
    }

    loadPuzzle(idx) {
        const pList = this.puzzleState.puzzles;
        if (!pList || pList.length === 0) return;

        this.puzzleState.puzzleIdx = idx % pList.length;
        this.puzzleState.isSolved = false;
        const p = pList[this.puzzleState.puzzleIdx];

        document.getElementById('puzzle-title').textContent = p.title;
        document.getElementById('puzzle-event').textContent = p.event;
        document.getElementById('puzzle-desc').textContent = p.desc;
        document.getElementById('puzzle-progress').textContent = `${this.puzzleState.puzzleIdx + 1} / ${pList.length}`;

        // Parse Puzzle FEN into puzzleState
        const parts = p.fen.split(' ');
        const ranks = parts[0].split('/');
        const newBoard = [];
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
            newBoard.push(row);
        }

        this.puzzleState.board = newBoard;
        this.puzzleState.turn = parts[1] || 'w';
        this.puzzleState.castlingRights = {
            K: parts[2] ? parts[2].includes('K') : false,
            Q: parts[2] ? parts[2].includes('Q') : false,
            k: parts[2] ? parts[2].includes('k') : false,
            q: parts[2] ? parts[2].includes('q') : false
        };
        this.puzzleState.enPassantTarget = (parts[3] && parts[3] !== '-') ? this.squareToCoords(parts[3]) : null;
        this.puzzleState.lastMove = null;
        this.puzzleState.isFlipped = (this.puzzleState.turn === 'b');

        this.selectedSquare = null;
        this.legalTargets = [];
        this.renderBoard();
    }

    showPuzzleHint() {
        const p = this.puzzleState.puzzles[this.puzzleState.puzzleIdx];
        if (p) document.getElementById('puzzle-desc').textContent = `Hint: ${p.hint}`;
    }

    nextPuzzle() {
        this.loadPuzzle(this.puzzleState.puzzleIdx + 1);
    }

    getCurrentBoard() {
        return this.activeTab === 'puzzles' ? this.puzzleState.board : this.gameState.board;
    }

    getCurrentTurn() {
        return this.activeTab === 'puzzles' ? this.puzzleState.turn : this.gameState.turn;
    }

    getCurrentCastling() {
        return this.activeTab === 'puzzles' ? this.puzzleState.castlingRights : this.gameState.castlingRights;
    }

    getCurrentEPTarget() {
        return this.activeTab === 'puzzles' ? this.puzzleState.enPassantTarget : this.gameState.enPassantTarget;
    }

    getCurrentLastMove() {
        return this.activeTab === 'puzzles' ? this.puzzleState.lastMove : this.gameState.lastMove;
    }

    getIsFlipped() {
        return this.activeTab === 'puzzles' ? this.puzzleState.isFlipped : this.gameState.isFlipped;
    }

    getPieceImg(p) {
        const code = PIECE_NAMES[p];
        if (!code) return '';
        const folder = this.pieceSet === 'cburnett' ? '' : `${this.pieceSet}/`;
        return `<img src="pieces/${folder}${code}.svg" class="piece-img" draggable="false" alt="${p}" />`;
    }

    coordsToSquare(r, c) { return `${String.fromCharCode(97 + c)}${8 - r}`; }
    squareToCoords(sq) { return [8 - parseInt(sq[1]), sq.charCodeAt(0) - 97]; }

    renderBoard() {
        if (!this.boardEl) return;
        this.boardEl.innerHTML = '';

        const board = this.getCurrentBoard();
        const turn = this.getCurrentTurn();
        const lastMove = this.getCurrentLastMove();
        const isFlipped = this.getIsFlipped();

        const inCheck = ChessRulesEngine.isInCheck(turn, board);
        const kingPos = inCheck ? ChessRulesEngine.findKing(turn, board) : null;

        for (let rowIdx = 0; rowIdx < 8; rowIdx++) {
            for (let colIdx = 0; colIdx < 8; colIdx++) {
                const r = isFlipped ? 7 - rowIdx : rowIdx;
                const c = isFlipped ? 7 - colIdx : colIdx;

                const sqEl = document.createElement('div');
                const isLight = (r + c) % 2 === 0;
                sqEl.className = `square ${isLight ? 'light' : 'dark'}`;
                const sqName = this.coordsToSquare(r, c);

                if (this.selectedSquare && this.selectedSquare[0] === r && this.selectedSquare[1] === c) {
                    sqEl.classList.add('selected');
                }
                if (lastMove && (lastMove.from === sqName || lastMove.to === sqName)) {
                    sqEl.classList.add('last-move');
                }
                if (kingPos && kingPos[0] === r && kingPos[1] === c) {
                    sqEl.classList.add('in-check');
                }

                const isTarget = this.legalTargets.some(([tr, tc]) => tr === r && tc === c);
                if (isTarget) {
                    const isCapture = (board[r][c] !== '.') || (this.getCurrentEPTarget() && this.getCurrentEPTarget()[0] === r && this.getCurrentEPTarget()[1] === c);
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

                const p = board[r][c];
                if (p !== '.') {
                    sqEl.innerHTML += this.getPieceImg(p);
                }

                sqEl.addEventListener('click', () => this.handleSquareClick(r, c));
                this.boardEl.appendChild(sqEl);
            }
        }

        if (this.showThreatMap) this.drawThreatMap();
    }

    handleSquareClick(r, c) {
        if (this.activeTab === 'puzzles') {
            this.handlePuzzleClick(r, c);
            return;
        }

        if (this.gameState.isThinking || this.gameState.isGameOver) return;
        if (this.gameState.playMode === 'human_white' && this.gameState.turn !== 'w') return;
        if (this.gameState.playMode === 'human_black' && this.gameState.turn !== 'b') return;

        const board = this.gameState.board;
        const turn = this.gameState.turn;
        const piece = board[r][c];
        const isMyPiece = (turn === 'w' && ChessRulesEngine.isWhite(piece)) || (turn === 'b' && ChessRulesEngine.isBlack(piece));

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
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.gameState.castlingRights, this.gameState.enPassantTarget);
                this.renderBoard();
                return;
            }

            if (isTarget) {
                const srcPiece = board[srcR][srcC];
                if ((srcPiece === 'P' && r === 0) || (srcPiece === 'p' && r === 7)) {
                    this.promptPromotion(srcR, srcC, r, c);
                    return;
                }
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(r, c)}`;
                if (!this.gameState.isGameActive) this.startMatch();
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
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.gameState.castlingRights, this.gameState.enPassantTarget);
                this.renderBoard();
            }
        }
    }

    handlePuzzleClick(r, c) {
        if (this.puzzleState.isSolved) return;
        const board = this.puzzleState.board;
        const turn = this.puzzleState.turn;
        const piece = board[r][c];
        const isMyPiece = (turn === 'w' && ChessRulesEngine.isWhite(piece)) || (turn === 'b' && ChessRulesEngine.isBlack(piece));

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
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.puzzleState.castlingRights, this.puzzleState.enPassantTarget);
                this.renderBoard();
                return;
            }

            if (isTarget) {
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(r, c)}`;
                const p = this.puzzleState.puzzles[this.puzzleState.puzzleIdx];

                if (uciStr === p.solution) {
                    // Correct Solution
                    board[r][c] = board[srcR][srcC];
                    board[srcR][srcC] = '.';
                    this.puzzleState.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(r, c) };
                    this.puzzleState.isSolved = true;
                    this.sound.playSuccess();
                    document.getElementById('puzzle-desc').textContent = "Brilliant! You found the winning tactic.";
                    this.setStatus("Puzzle Solved", false);
                } else {
                    // Incorrect move
                    board[r][c] = board[srcR][srcC];
                    board[srcR][srcC] = '.';
                    this.puzzleState.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(r, c) };
                    this.sound.playError();
                    document.getElementById('puzzle-desc').textContent = "Incorrect move. Try again!";
                    setTimeout(() => {
                        this.loadPuzzle(this.puzzleState.puzzleIdx);
                    }, 700);
                }

                this.selectedSquare = null;
                this.legalTargets = [];
                this.renderBoard();
            } else {
                this.selectedSquare = null;
                this.legalTargets = [];
                this.renderBoard();
            }
        } else {
            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.puzzleState.castlingRights, this.puzzleState.enPassantTarget);
                this.renderBoard();
            }
        }
    }

    promptPromotion(srcR, srcC, tr, tc) {
        this.promotionChoicesEl.innerHTML = '';
        const choices = this.gameState.turn === 'w' ? ['Q', 'R', 'B', 'N'] : ['q', 'r', 'b', 'n'];
        for (const p of choices) {
            const btn = document.createElement('div');
            btn.className = 'promo-choice';
            btn.innerHTML = this.getPieceImg(p);
            btn.addEventListener('click', () => {
                this.promotionModal.classList.add('hidden');
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(tr, tc)}${p.toLowerCase()}`;
                if (!this.gameState.isGameActive) this.startMatch();
                this.executeMove(srcR, srcC, tr, tc, uciStr, p);
            });
            this.promotionChoicesEl.appendChild(btn);
        }
        this.promotionModal.classList.remove('hidden');
    }

    executeMove(srcR, srcC, tr, tc, uciStr, promoPiece = null) {
        const board = this.gameState.board;
        const piece = board[srcR][srcC];
        const target = board[tr][tc];
        const isCapture = target !== '.' || (piece.toUpperCase() === 'P' && this.gameState.enPassantTarget && tr === this.gameState.enPassantTarget[0] && tc === this.gameState.enPassantTarget[1]);

        if (piece.toUpperCase() === 'P' && this.gameState.enPassantTarget && tr === this.gameState.enPassantTarget[0] && tc === this.gameState.enPassantTarget[1]) {
            const capR = this.gameState.turn === 'w' ? tr + 1 : tr - 1;
            board[capR][tc] = '.';
        }

        board[tr][tc] = promoPiece || piece;
        board[srcR][srcC] = '.';

        if (piece === 'K' && srcR === 7 && srcC === 4) {
            if (tc === 6) { board[7][5] = 'R'; board[7][7] = '.'; }
            if (tc === 2) { board[7][3] = 'R'; board[7][0] = '.'; }
        } else if (piece === 'k' && srcR === 0 && srcC === 4) {
            if (tc === 6) { board[0][5] = 'r'; board[0][7] = '.'; }
            if (tc === 2) { board[0][3] = 'r'; board[0][0] = '.'; }
        }

        if (piece === 'K') { this.gameState.castlingRights.K = false; this.gameState.castlingRights.Q = false; }
        if (piece === 'k') { this.gameState.castlingRights.k = false; this.gameState.castlingRights.q = false; }
        if (piece === 'R' && srcR === 7 && srcC === 7) this.gameState.castlingRights.K = false;
        if (piece === 'R' && srcR === 7 && srcC === 0) this.gameState.castlingRights.Q = false;
        if (piece === 'r' && srcR === 0 && srcC === 7) this.gameState.castlingRights.k = false;
        if (piece === 'r' && srcR === 0 && srcC === 0) this.gameState.castlingRights.q = false;

        if (piece.toUpperCase() === 'P' && Math.abs(tr - srcR) === 2) {
            this.gameState.enPassantTarget = [(srcR + tr) / 2, srcC];
        } else {
            this.gameState.enPassantTarget = null;
        }

        this.gameState.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(tr, tc) };
        this.gameState.uciHistory.push(uciStr);

        if (this.gameState.increment > 0) {
            if (this.gameState.turn === 'w') this.gameState.whiteTime += this.gameState.increment;
            else this.gameState.blackTime += this.gameState.increment;
            this.updateClockDisplay();
        }

        if (isCapture) this.sound.playCapture();
        else this.sound.playMove();

        this.gameState.turn = this.gameState.turn === 'w' ? 'b' : 'w';
        if (this.gameState.turn === 'w') this.gameState.fullMoveNumber++;

        this.updateActiveClockHUD();
        this.renderBoard();
        this.appendMoveHistory(uciStr, piece, isCapture);
        this.checkGameEnd();
        this.updateOpening();

        if (!this.gameState.isGameOver && this.gameState.isGameActive) {
            if ((this.gameState.playMode === 'human_white' && this.gameState.turn === 'b') || (this.gameState.playMode === 'human_black' && this.gameState.turn === 'w')) {
                this.triggerEngineMove();
            }
        }
    }

    appendMoveHistory(uciStr, piece, isCapture) {
        if (this.gameState.moveHistory.length === 0) {
            this.moveHistoryEl.innerHTML = '';
        }
        this.gameState.moveHistory.push(uciStr);

        const isWhite = this.gameState.turn === 'b';
        if (isWhite) {
            const row = document.createElement('div');
            row.className = 'history-row';
            row.id = `hist-row-${this.gameState.fullMoveNumber}`;
            row.innerHTML = `<span class="history-num">${this.gameState.fullMoveNumber}.</span><span class="history-move">${uciStr}</span><span class="history-move"></span>`;
            this.moveHistoryEl.appendChild(row);
        } else {
            const row = document.getElementById(`hist-row-${this.gameState.fullMoveNumber}`);
            if (row) {
                row.children[2].textContent = uciStr;
            }
        }
        this.moveHistoryEl.scrollTop = this.moveHistoryEl.scrollHeight;
    }

    updateOpening() {
        const uciLine = this.gameState.uciHistory.slice(0, 6).join(' ');
        for (const [seq, name] of Object.entries(OPENING_BOOK)) {
            if (uciLine.startsWith(seq)) {
                this.openingBadgeEl.textContent = name;
                return;
            }
        }
    }

    checkGameEnd() {
        const legalMoves = ChessRulesEngine.getAllLegalMoves(this.gameState.turn, this.gameState.board, this.gameState.castlingRights, this.gameState.enPassantTarget);
        const inCheck = ChessRulesEngine.isInCheck(this.gameState.turn, this.gameState.board);

        if (legalMoves.length === 0) {
            this.gameState.isGameOver = true;
            this.stopClock();
            if (inCheck) {
                this.sound.playGameOver();
                const winner = this.gameState.turn === 'w' ? "Black" : "White";
                this.showGameOver("Checkmate", `${winner} won the match.`);
            } else {
                this.showGameOver("Stalemate", "Game drawn by stalemate.");
            }
        } else if (inCheck) {
            this.sound.playCheck();
            this.oracleTextEl.textContent = "Check. King under attack.";
        }
    }

    showGameOver(title, sub) {
        this.gameoverTitleEl.textContent = title;
        this.gameoverSubEl.textContent = sub;
        this.gameoverModal.classList.remove('hidden');
    }

    getFEN() {
        const board = this.getCurrentBoard();
        const turn = this.getCurrentTurn();
        const castling = this.getCurrentCastling();
        const ep = this.getCurrentEPTarget();

        let fen = '';
        for (let r = 0; r < 8; r++) {
            let empty = 0;
            for (let c = 0; c < 8; c++) {
                const p = board[r][c];
                if (p === '.') empty++;
                else {
                    if (empty > 0) { fen += empty; empty = 0; }
                    fen += p;
                }
            }
            if (empty > 0) fen += empty;
            if (r < 7) fen += '/';
        }
        fen += ` ${turn} `;
        let cStr = '';
        if (castling.K) cStr += 'K';
        if (castling.Q) cStr += 'Q';
        if (castling.k) cStr += 'k';
        if (castling.q) cStr += 'q';
        fen += (cStr || '-') + ' ';
        fen += (ep ? this.coordsToSquare(ep[0], ep[1]) : '-') + ' ';
        fen += `${this.gameState.halfMoveClock} ${this.gameState.fullMoveNumber}`;
        return fen;
    }

    async triggerEngineMove() {
        if (this.gameState.isThinking || this.gameState.isGameOver || !this.gameState.isGameActive || this.activeTab !== 'play') return;
        this.gameState.isThinking = true;
        this.setStatus("Calculating", true);

        const fen = this.getFEN();
        const depth = 12;
        const wtimeMs = Math.round(this.gameState.whiteTime * 1000);
        const btimeMs = Math.round(this.gameState.blackTime * 1000);
        const incMs = Math.round(this.gameState.increment * 1000);

        try {
            const resp = await fetch('/api/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    fen,
                    depth,
                    wtime: (this.gameState.timeControl === 'fixed_depth') ? 0 : wtimeMs,
                    btime: (this.gameState.timeControl === 'fixed_depth') ? 0 : btimeMs,
                    winc: incMs,
                    binc: incMs
                })
            });
            const data = await resp.json();

            if (data.best_move && this.gameState.isGameActive && this.activeTab === 'play') {
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
            this.gameState.isThinking = false;
            if (this.gameState.isGameActive && this.activeTab === 'play') {
                this.setStatus("Match Active", false);
            }
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
        const delta = cp - this.gameState.lastEvalScore;
        this.gameState.lastEvalScore = cp;

        this.moveGradeEl.classList.remove('hidden', 'brilliant', 'best', 'good', 'inaccuracy', 'blunder');

        if (Math.abs(delta) > 300) {
            if ((this.gameState.turn === 'b' && delta < -300) || (this.gameState.turn === 'w' && delta > 300)) {
                this.moveGradeEl.className = 'move-grade brilliant';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Brilliant</span>`;
                this.oracleTextEl.textContent = "Tactical breakthrough. Advantage secured.";
            } else {
                this.moveGradeEl.className = 'move-grade blunder';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Blunder</span>`;
                this.oracleTextEl.textContent = "Inaccuracy detected. Advantage conceded.";
            }
        } else if (Math.abs(cp) > 500) {
            this.oracleTextEl.textContent = "Decisive advantage. Technical conversion.";
        } else {
            this.oracleTextEl.textContent = "Balanced position. Solid piece coordination.";
        }
    }

    async requestHint() {
        if (this.activeTab === 'puzzles') {
            this.showPuzzleHint();
            return;
        }
        if (this.gameState.isThinking || this.gameState.isGameOver) return;
        const fen = this.getFEN();
        const resp = await fetch('/api/analyze', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fen, depth: 9 })
        });
        const data = await resp.json();
        if (data.best_move) {
            this.oracleTextEl.textContent = `Recommended move: ${data.best_move.substring(0, 2)} -> ${data.best_move.substring(2, 4)}`;
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
        canvas.width = 460;
        canvas.height = 460;
        ctx.clearRect(0, 0, 460, 460);

        const board = this.getCurrentBoard();
        const isFlipped = this.getIsFlipped();
        const sqSize = 460 / 8;

        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const dr = isFlipped ? 7 - r : r;
                const dc = isFlipped ? 7 - c : c;

                const attackedByWhite = ChessRulesEngine.isSquareAttacked(r, c, 'w', board);
                const attackedByBlack = ChessRulesEngine.isSquareAttacked(r, c, 'b', board);

                if (attackedByWhite && attackedByBlack) {
                    ctx.fillStyle = 'rgba(168, 85, 247, 0.3)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                } else if (attackedByWhite) {
                    ctx.fillStyle = 'rgba(56, 189, 248, 0.25)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                } else if (attackedByBlack) {
                    ctx.fillStyle = 'rgba(244, 63, 94, 0.25)';
                    ctx.fillRect(dc * sqSize, dr * sqSize, sqSize, sqSize);
                }
            }
        }
    }

    flipBoard() {
        if (this.activeTab === 'puzzles') {
            this.puzzleState.isFlipped = !this.puzzleState.isFlipped;
        } else {
            this.gameState.isFlipped = !this.gameState.isFlipped;
        }
        this.renderBoard();
    }

    undoMove() {
        if (this.activeTab === 'puzzles') {
            this.loadPuzzle(this.puzzleState.puzzleIdx);
            return;
        }
        if (this.gameState.moveHistory.length === 0) return;
        this.initPlayMode();
    }

    copyFEN() {
        navigator.clipboard.writeText(this.getFEN());
        this.setStatus("FEN copied", false);
    }

    copyPGN() {
        const pgn = this.gameState.moveHistory.join(' ');
        navigator.clipboard.writeText(pgn);
        this.setStatus("PGN copied", false);
    }

    setStatus(text, thinking = false) {
        this.engineStatusEl.textContent = text;
        if (thinking) this.statusDotEl.classList.add('thinking');
        else this.statusDotEl.classList.remove('thinking');
    }

    resetClocks() {
        this.stopClock();
        this.gameState.increment = 0.0;

        if (this.gameState.timeControl === 'bullet_1_0') {
            this.gameState.whiteTime = 60.0;
            this.gameState.blackTime = 60.0;
        } else if (this.gameState.timeControl === 'bullet_1_1') {
            this.gameState.whiteTime = 60.0;
            this.gameState.blackTime = 60.0;
            this.gameState.increment = 1.0;
        } else if (this.gameState.timeControl === 'blitz_3_0') {
            this.gameState.whiteTime = 180.0;
            this.gameState.blackTime = 180.0;
        } else if (this.gameState.timeControl === 'blitz_3_2') {
            this.gameState.whiteTime = 180.0;
            this.gameState.blackTime = 180.0;
            this.gameState.increment = 2.0;
        } else if (this.gameState.timeControl === 'rapid_10_0') {
            this.gameState.whiteTime = 600.0;
            this.gameState.blackTime = 600.0;
        } else {
            this.gameState.whiteTime = 0.0;
            this.gameState.blackTime = 0.0;
        }
        this.updateClockDisplay();
    }

    startClock() {
        this.stopClock();
        if (this.gameState.timeControl === 'fixed_depth' || this.activeTab !== 'play') return;
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
        if (!this.gameState.isGameActive || this.gameState.isGameOver || this.gameState.timeControl === 'fixed_depth' || this.activeTab !== 'play') return;
        const now = performance.now();
        const elapsed = (now - this.lastClockTick) / 1000.0;
        this.lastClockTick = now;

        if (this.gameState.turn === 'w') {
            this.gameState.whiteTime = Math.max(0.0, this.gameState.whiteTime - elapsed);
            if (this.gameState.whiteTime <= 0.0) {
                this.gameState.isGameOver = true;
                this.stopClock();
                this.sound.playGameOver();
                this.showGameOver("Time Out", "Black won on time.");
            }
        } else {
            this.gameState.blackTime = Math.max(0.0, this.gameState.blackTime - elapsed);
            if (this.gameState.blackTime <= 0.0) {
                this.gameState.isGameOver = true;
                this.stopClock();
                this.sound.playGameOver();
                this.showGameOver("Time Out", "White won on time.");
            }
        }
        this.updateClockDisplay();
    }

    updateActiveClockHUD() {
        if (!this.gameState.isGameActive || this.gameState.timeControl === 'fixed_depth' || this.activeTab !== 'play') {
            this.topClockEl.classList.remove('active');
            this.bottomClockEl.classList.remove('active');
            return;
        }
        const isWhiteBottom = !this.gameState.isFlipped;
        if (this.gameState.turn === 'w') {
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

    updateClockDisplay() {
        if (this.gameState.timeControl === 'fixed_depth' || this.activeTab !== 'play') {
            this.topClockEl.textContent = "--:--";
            this.bottomClockEl.textContent = "--:--";
            return;
        }
        const fmt = (s) => {
            const m = Math.floor(s / 60);
            const sec = Math.floor(s % 60);
            return `${m.toString().padStart(2, '0')}:${sec.toString().padStart(2, '0')}`;
        };
        const isWhiteBottom = !this.gameState.isFlipped;
        if (isWhiteBottom) {
            this.topClockEl.textContent = fmt(this.gameState.blackTime);
            this.bottomClockEl.textContent = fmt(this.gameState.whiteTime);
        } else {
            this.topClockEl.textContent = fmt(this.gameState.whiteTime);
            this.bottomClockEl.textContent = fmt(this.gameState.blackTime);
        }
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.app = new ChessApp();
});
