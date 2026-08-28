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
    "e2e4 e7e5 g1f3 b8c6 d2d4": "Scotch Game",
    "e2e4 e7e5 g1f3 g8f6": "Petrov's Defense",
    "e2e4 e7e5 f2f4": "King's Gambit",
    "e2e4 e7e5 g1f3 b8c6 b1c3 g8f6": "Four Knights Game",
    "e2e4 b8c6": "Nimzowitsch Defense",
    "e2e4 c7c5": "Sicilian Defense",
    "e2e4 e7e6": "French Defense",
    "e2e4 c7c6": "Caro-Kann Defense",
    "e2e4 d7d5": "Scandinavian Defense",
    "e2e4 g7g6": "Modern Defense",
    "e2e4 d7d6": "Pirc Defense",
    "d2d4 d7d5 c2c4": "Queen's Gambit",
    "d2d4 d7d5 c2c4 c7c6": "Slav Defense",
    "d2d4 d7d5 c2c4 e7e6": "Queen's Gambit Declined",
    "d2d4 g8f6 c2c4 g7g6": "King's Indian Defense",
    "d2d4 g8f6 c2c4 e7e6 b1c3 f8b4": "Nimzo-Indian Defense",
    "d2d4 g8f6 c2c4 e7e6 g1f3 b7b6": "Queen's Indian Defense",
    "d2d4 g8f6 c2c4 c7c5": "Benoni Defense",
    "d2d4 f7f5": "Dutch Defense",
    "d2d4 d7d5": "Queen's Pawn Opening",
    "c2c4": "English Opening",
    "g1f3": "Reti Opening",
    "b2b3": "Nimzo-Larsen Attack",
    "f2f4": "Bird's Opening"
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

    static isInsufficientMaterial(board) {
        let whitePieces = [];
        let blackPieces = [];

        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const p = board[r][c];
                if (p === '.') continue;
                const isW = (p >= 'A' && p <= 'Z');
                const type = p.toUpperCase();
                if (type === 'P' || type === 'R' || type === 'Q') return false;
                if (type !== 'K') {
                    if (isW) whitePieces.push({ type, r, c });
                    else blackPieces.push({ type, r, c });
                }
            }
        }

        // K vs K
        if (whitePieces.length === 0 && blackPieces.length === 0) return true;

        // K+B vs K or K+N vs K
        if (whitePieces.length === 1 && blackPieces.length === 0) return true;
        if (blackPieces.length === 1 && whitePieces.length === 0) return true;

        // K+B vs K+B with same-color square bishops
        if (whitePieces.length === 1 && blackPieces.length === 1 &&
            whitePieces[0].type === 'B' && blackPieces[0].type === 'B') {
            const wSqColor = (whitePieces[0].r + whitePieces[0].c) % 2;
            const bSqColor = (blackPieces[0].r + blackPieces[0].c) % 2;
            if (wSqColor === bSqColor) return true;
        }

        return false;
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

        // Telemetry & Analysis Sandbox State (100% Isolated from Matches)
        this.telemetryState = {
            board: ChessRulesEngine.cloneBoard(INITIAL_BOARD),
            turn: 'w',
            castlingRights: { K: true, Q: true, k: true, q: true },
            enPassantTarget: null,
            lastMove: null,
            isFlipped: false,
            moveHistory: [],
            uciHistory: [],
            fullMoveNumber: 1,
            lastEvalScore: 0
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
        this.teleTimeEl = document.getElementById('tele-time');
        this.teleHashfullEl = document.getElementById('tele-hashfull');
        this.teleWinchanceEl = document.getElementById('tele-winchance');
        this.telePvEl = document.getElementById('tele-pv');
        this.teleEngineTagEl = document.getElementById('tele-engine-tag');

        this.quickNpsEl = document.getElementById('quick-nps');
        this.quickDepthEl = document.getElementById('quick-depth');
        this.quickNodesEl = document.getElementById('quick-nodes');
        this.quickTimeEl = document.getElementById('quick-time');

        this.oracleTextEl = document.getElementById('oracle-text');
        this.openingBadgeEl = document.getElementById('opening-badge');
        this.moveGradeEl = document.getElementById('move-grade-badge');
        this.moveHistoryEl = document.getElementById('move-history');
        this.topClockEl = document.getElementById('top-clock');
        this.bottomClockEl = document.getElementById('bottom-clock');
        this.engineStatusEl = document.getElementById('engine-status');
        this.topCapturedBoxEl = document.getElementById('top-captured');
        this.topCapturedPiecesEl = document.getElementById('top-captured-pieces');
        this.topMaterialDiffEl = document.getElementById('top-material-diff');
        this.bottomCapturedBoxEl = document.getElementById('bottom-captured');
        this.bottomCapturedPiecesEl = document.getElementById('bottom-captured-pieces');
        this.bottomMaterialDiffEl = document.getElementById('bottom-material-diff');

        this.statusDotEl = document.getElementById('status-dot');
        this.heatmapCanvas = document.getElementById('heatmap-canvas');
        this.promotionModal = document.getElementById('promotion-modal');
        this.promotionChoicesEl = document.getElementById('promotion-choices');
        this.gameoverModal = document.getElementById('gameover-modal');
        this.gameoverIconEl = document.getElementById('gameover-icon');
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
            if (this.activeTab === 'telemetry') {
                this.initTelemetryMode();
                return;
            }
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
        document.getElementById('draw-btn').addEventListener('click', () => this.offerDraw());
        document.getElementById('resign-btn').addEventListener('click', () => this.resignMatch());
        document.getElementById('modal-restart-btn').addEventListener('click', () => {
            this.gameoverModal.classList.add('hidden');
            this.initPlayMode();
            this.startMatch();
        });
        document.getElementById('modal-review-btn').addEventListener('click', () => {
            this.gameoverModal.classList.add('hidden');
        });

        // Global Keyboard Shortcuts
        window.addEventListener('keydown', (e) => {
            if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT' || e.target.tagName === 'TEXTAREA') return;
            const key = e.key.toLowerCase();
            if (key === 'f') this.flipBoard();
            else if (key === 'u' || (e.ctrlKey && key === 'z')) this.undoMove();
            else if (key === 'h') this.requestHint();
            else if (key === 't') this.toggleThreatMap();
            else if (key === 'escape') {
                this.promotionModal.classList.add('hidden');
                this.gameoverModal.classList.add('hidden');
            }
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
            this.stopClock();
            cardCommentary.classList.remove('hidden');
            cardHistory.classList.remove('hidden');
            cardConfig.classList.add('hidden');
            cardPuzzles.classList.add('hidden');
            cardTelemetry.classList.remove('hidden');

            this.renderTelemetryView();
            this.runLiveAnalysis();
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

    renderTelemetryView() {
        this.stopClock();
        this.renderBoard();
        this.setStatus('Analysis Lab', false);
        this.startBtn.textContent = 'Reset Sandbox';
        this.oracleTextEl.textContent = `${this.telemetryState.turn === 'w' ? 'White' : 'Black'} to move (Analysis Sandbox).`;
        this.updateEvalBar(this.telemetryState.lastEvalScore);
    }

    initTelemetryMode() {
        this.telemetryState.board = ChessRulesEngine.cloneBoard(INITIAL_BOARD);
        this.telemetryState.turn = 'w';
        this.telemetryState.castlingRights = { K: true, Q: true, k: true, q: true };
        this.telemetryState.enPassantTarget = null;
        this.telemetryState.lastMove = null;
        this.telemetryState.moveHistory = [];
        this.telemetryState.uciHistory = [];
        this.telemetryState.fullMoveNumber = 1;
        this.telemetryState.lastEvalScore = 0;
        this.selectedSquare = null;
        this.legalTargets = [];
        this.renderTelemetryView();
        this.runLiveAnalysis();
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

        if (this.activeTab === 'play') {
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
        if (this.activeTab === 'puzzles') return this.puzzleState.board;
        if (this.activeTab === 'telemetry') return this.telemetryState.board;
        return this.gameState.board;
    }

    getCurrentTurn() {
        if (this.activeTab === 'puzzles') return this.puzzleState.turn;
        if (this.activeTab === 'telemetry') return this.telemetryState.turn;
        return this.gameState.turn;
    }

    getCurrentCastling() {
        if (this.activeTab === 'puzzles') return this.puzzleState.castlingRights;
        if (this.activeTab === 'telemetry') return this.telemetryState.castlingRights;
        return this.gameState.castlingRights;
    }

    getCurrentEPTarget() {
        if (this.activeTab === 'puzzles') return this.puzzleState.enPassantTarget;
        if (this.activeTab === 'telemetry') return this.telemetryState.enPassantTarget;
        return this.gameState.enPassantTarget;
    }

    getCurrentLastMove() {
        if (this.activeTab === 'puzzles') return this.puzzleState.lastMove;
        if (this.activeTab === 'telemetry') return this.telemetryState.lastMove;
        return this.gameState.lastMove;
    }

    getIsFlipped() {
        if (this.activeTab === 'puzzles') return this.puzzleState.isFlipped;
        if (this.activeTab === 'telemetry') return this.telemetryState.isFlipped;
        return this.gameState.isFlipped;
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
        this.updateCapturedPiecesAndMaterial();
    }

    updateCapturedPiecesAndMaterial() {
        const board = this.getCurrentBoard();
        const isFlipped = this.getIsFlipped();

        const wCounts = { P: 0, N: 0, B: 0, R: 0, Q: 0 };
        const bCounts = { p: 0, n: 0, b: 0, r: 0, q: 0 };

        for (let r = 0; r < 8; r++) {
            for (let c = 0; c < 8; c++) {
                const p = board[r][c];
                if (wCounts[p] !== undefined) wCounts[p]++;
                if (bCounts[p] !== undefined) bCounts[p]++;
            }
        }

        // Captured by White (Black pieces missing from board)
        const wCaptured = [];
        const missingBp = Math.max(0, 8 - bCounts.p);
        const missingBn = Math.max(0, 2 - bCounts.n);
        const missingBb = Math.max(0, 2 - bCounts.b);
        const missingBr = Math.max(0, 2 - bCounts.r);
        const missingBq = Math.max(0, 1 - bCounts.q);
        for (let i = 0; i < missingBq; i++) wCaptured.push('q');
        for (let i = 0; i < missingBr; i++) wCaptured.push('r');
        for (let i = 0; i < missingBb; i++) wCaptured.push('b');
        for (let i = 0; i < missingBn; i++) wCaptured.push('n');
        for (let i = 0; i < missingBp; i++) wCaptured.push('p');

        // Captured by Black (White pieces missing from board)
        const bCaptured = [];
        const missingWp = Math.max(0, 8 - wCounts.P);
        const missingWn = Math.max(0, 2 - wCounts.N);
        const missingWb = Math.max(0, 2 - wCounts.B);
        const missingWr = Math.max(0, 2 - wCounts.R);
        const missingWq = Math.max(0, 1 - wCounts.Q);
        for (let i = 0; i < missingWq; i++) bCaptured.push('Q');
        for (let i = 0; i < missingWr; i++) bCaptured.push('R');
        for (let i = 0; i < missingWb; i++) bCaptured.push('B');
        for (let i = 0; i < missingWn; i++) bCaptured.push('N');
        for (let i = 0; i < missingWp; i++) bCaptured.push('P');

        const wMat = wCounts.P * 1 + wCounts.N * 3 + wCounts.B * 3 + wCounts.R * 5 + wCounts.Q * 9;
        const bMat = bCounts.p * 1 + bCounts.n * 3 + bCounts.b * 3 + bCounts.r * 5 + bCounts.q * 9;
        const wDiff = wMat - bMat; // Positive if White ahead, negative if Black ahead
        const bDiff = bMat - wMat; // Positive if Black ahead, negative if White ahead

        const renderCaptured = (list) => {
            const folder = this.pieceSet === 'cburnett' ? '' : `${this.pieceSet}/`;
            return list.map(p => {
                const code = PIECE_NAMES[p];
                return `<img src="pieces/${folder}${code}.svg" class="captured-icon" alt="${p}" />`;
            }).join('');
        };

        // If !isFlipped: Top HUD is Black, Bottom HUD is White
        // If isFlipped: Top HUD is White, Bottom HUD is Black
        const topCaptured = isFlipped ? wCaptured : bCaptured;
        const bottomCaptured = isFlipped ? bCaptured : wCaptured;

        const topDiff = isFlipped ? wDiff : bDiff;
        const bottomDiff = isFlipped ? bDiff : wDiff;

        if (topCaptured.length > 0) {
            if (this.topCapturedBoxEl) this.topCapturedBoxEl.classList.remove('hidden');
            if (this.topCapturedPiecesEl) this.topCapturedPiecesEl.innerHTML = renderCaptured(topCaptured);
        } else {
            if (this.topCapturedBoxEl) this.topCapturedBoxEl.classList.add('hidden');
            if (this.topCapturedPiecesEl) this.topCapturedPiecesEl.innerHTML = '';
        }

        if (bottomCaptured.length > 0) {
            if (this.bottomCapturedBoxEl) this.bottomCapturedBoxEl.classList.remove('hidden');
            if (this.bottomCapturedPiecesEl) this.bottomCapturedPiecesEl.innerHTML = renderCaptured(bottomCaptured);
        } else {
            if (this.bottomCapturedBoxEl) this.bottomCapturedBoxEl.classList.add('hidden');
            if (this.bottomCapturedPiecesEl) this.bottomCapturedPiecesEl.innerHTML = '';
        }

        const hasCaptures = (wCaptured.length > 0 || bCaptured.length > 0);

        const applyBadge = (el, diffVal) => {
            if (!el) return;
            el.classList.remove('lead', 'trail', 'even', 'hidden');
            if (diffVal > 0) {
                el.textContent = `+${diffVal}`;
                el.classList.add('lead');
            } else if (diffVal < 0) {
                el.textContent = `${diffVal}`;
                el.classList.add('trail');
            } else {
                if (hasCaptures) {
                    el.textContent = `= 0`;
                    el.classList.add('even');
                } else {
                    el.classList.add('hidden');
                }
            }
        };

        applyBadge(this.topMaterialDiffEl, topDiff);
        applyBadge(this.bottomMaterialDiffEl, bottomDiff);
    }

    handleSquareClick(r, c) {
        if (this.activeTab === 'puzzles') {
            this.handlePuzzleClick(r, c);
            return;
        }
        if (this.activeTab === 'telemetry') {
            this.handleTelemetryClick(r, c);
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

    handleTelemetryClick(r, c) {
        const board = this.telemetryState.board;
        const turn = this.telemetryState.turn;
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
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.telemetryState.castlingRights, this.telemetryState.enPassantTarget);
                this.renderBoard();
                return;
            }

            if (isTarget) {
                const srcPiece = board[srcR][srcC];
                let isPromo = false;
                let promoPiece = null;
                if (srcPiece === 'P' && r === 0) { isPromo = true; promoPiece = 'Q'; }
                if (srcPiece === 'p' && r === 7) { isPromo = true; promoPiece = 'q'; }

                const isCapture = (board[r][c] !== '.') || (this.telemetryState.enPassantTarget && this.telemetryState.enPassantTarget[0] === r && this.telemetryState.enPassantTarget[1] === c);
                
                board[r][c] = isPromo ? promoPiece : board[srcR][srcC];
                board[srcR][srcC] = '.';

                if (srcPiece.toUpperCase() === 'P' && this.telemetryState.enPassantTarget && this.telemetryState.enPassantTarget[0] === r && this.telemetryState.enPassantTarget[1] === c) {
                    const epCaptureR = turn === 'w' ? r + 1 : r - 1;
                    board[epCaptureR][c] = '.';
                }

                if (srcPiece === 'K' && srcR === 7 && srcC === 4) {
                    if (c === 6) { board[7][5] = 'R'; board[7][7] = '.'; }
                    if (c === 2) { board[7][3] = 'R'; board[7][0] = '.'; }
                }
                if (srcPiece === 'k' && srcR === 0 && srcC === 4) {
                    if (c === 6) { board[0][5] = 'r'; board[0][7] = '.'; }
                    if (c === 2) { board[0][3] = 'r'; board[0][0] = '.'; }
                }

                if (srcPiece === 'K') { this.telemetryState.castlingRights.K = false; this.telemetryState.castlingRights.Q = false; }
                if (srcPiece === 'k') { this.telemetryState.castlingRights.k = false; this.telemetryState.castlingRights.q = false; }
                if (srcPiece === 'R' && srcR === 7 && srcC === 7) this.telemetryState.castlingRights.K = false;
                if (srcPiece === 'R' && srcR === 7 && srcC === 0) this.telemetryState.castlingRights.Q = false;
                if (srcPiece === 'r' && srcR === 0 && srcC === 7) this.telemetryState.castlingRights.k = false;
                if (srcPiece === 'r' && srcR === 0 && srcC === 0) this.telemetryState.castlingRights.q = false;

                if (r === 7 && c === 7) this.telemetryState.castlingRights.K = false;
                if (r === 7 && c === 0) this.telemetryState.castlingRights.Q = false;
                if (r === 0 && c === 7) this.telemetryState.castlingRights.k = false;
                if (r === 0 && c === 0) this.telemetryState.castlingRights.q = false;

                if (srcPiece.toUpperCase() === 'P' && Math.abs(r - srcR) === 2) {
                    this.telemetryState.enPassantTarget = [(srcR + r) / 2, srcC];
                } else {
                    this.telemetryState.enPassantTarget = null;
                }

                this.telemetryState.lastMove = { from: this.coordsToSquare(srcR, srcC), to: this.coordsToSquare(r, c) };
                const uciStr = `${this.coordsToSquare(srcR, srcC)}${this.coordsToSquare(r, c)}${isPromo ? 'q' : ''}`;
                this.telemetryState.uciHistory.push(uciStr);

                if (isCapture) this.sound.playCapture();
                else this.sound.playMove();

                this.telemetryState.turn = this.telemetryState.turn === 'w' ? 'b' : 'w';
                if (this.telemetryState.turn === 'w') this.telemetryState.fullMoveNumber++;

                this.selectedSquare = null;
                this.legalTargets = [];

                const mover = this.telemetryState.turn === 'w' ? 'b' : 'w';
                this.renderBoard();
                this.runLiveAnalysis(mover);
            } else {
                this.selectedSquare = null;
                this.legalTargets = [];
                this.renderBoard();
            }
        } else {
            if (isMyPiece) {
                this.selectedSquare = [r, c];
                this.legalTargets = ChessRulesEngine.getLegalMoves(r, c, board, turn, this.telemetryState.castlingRights, this.telemetryState.enPassantTarget);
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

        // Revoke if opponent rook is captured in corner
        if (tr === 7 && tc === 7) this.gameState.castlingRights.K = false;
        if (tr === 7 && tc === 0) this.gameState.castlingRights.Q = false;
        if (tr === 0 && tc === 7) this.gameState.castlingRights.k = false;
        if (tr === 0 && tc === 0) this.gameState.castlingRights.q = false;

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

        const isWhite = (this.gameState.turn === 'b');
        if (isWhite) {
            const row = document.createElement('div');
            row.className = 'history-row';
            row.id = `hist-row-${this.gameState.fullMoveNumber}`;
            row.innerHTML = `<span class="history-num">${this.gameState.fullMoveNumber}.</span><span class="history-move">${uciStr}</span><span class="history-move"></span>`;
            this.moveHistoryEl.appendChild(row);
        } else {
            const rowNum = this.gameState.fullMoveNumber - 1;
            const row = document.getElementById(`hist-row-${rowNum}`);
            if (row && row.children.length >= 3) {
                row.children[2].textContent = uciStr;
            }
        }
        this.moveHistoryEl.scrollTop = this.moveHistoryEl.scrollHeight;
    }

    updateOpening() {
        const uciHistory = (this.activeTab === 'telemetry') ? this.telemetryState.uciHistory : this.gameState.uciHistory;
        const uciLine = uciHistory.join(' ');
        const moveCount = uciHistory.length;

        let matched = null;
        let longestLen = 0;
        for (const [seq, name] of Object.entries(OPENING_BOOK)) {
            if (uciLine.startsWith(seq) && seq.length > longestLen) {
                matched = name;
                longestLen = seq.length;
            }
        }

        if (matched && moveCount <= 12) {
            this.openingBadgeEl.textContent = matched;
        } else if (moveCount === 0) {
            this.openingBadgeEl.textContent = 'Standard Start';
        } else {
            const board = this.getCurrentBoard();
            let totalPieces = 0;
            let queens = 0;
            for (let r = 0; r < 8; r++) {
                for (let c = 0; c < 8; c++) {
                    const p = board[r][c].toUpperCase();
                    if (p === 'Q') queens++;
                    if (p === 'Q' || p === 'R' || p === 'B' || p === 'N') totalPieces++;
                }
            }

            if (queens === 0 || totalPieces <= 4) {
                this.openingBadgeEl.textContent = 'Endgame';
            } else if (moveCount > 8) {
                this.openingBadgeEl.textContent = 'Middlegame';
            } else {
                this.openingBadgeEl.textContent = matched || 'Opening';
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
            return;
        }

        // 1. Insufficient Material
        if (ChessRulesEngine.isInsufficientMaterial(this.gameState.board)) {
            this.gameState.isGameOver = true;
            this.stopClock();
            this.showGameOver("Draw", "Game drawn by insufficient mating material.");
            return;
        }

        // 2. 50-Move Rule
        if (this.gameState.halfMoveClock >= 100) {
            this.gameState.isGameOver = true;
            this.stopClock();
            this.showGameOver("Draw", "Game drawn by 50-move rule.");
            return;
        }

        // 3. Threefold Repetition
        const fenKey = this.getFEN().split(' ').slice(0, 4).join(' ');
        if (!this.gameState.positionHistory) this.gameState.positionHistory = {};
        this.gameState.positionHistory[fenKey] = (this.gameState.positionHistory[fenKey] || 0) + 1;
        if (this.gameState.positionHistory[fenKey] >= 3) {
            this.gameState.isGameOver = true;
            this.stopClock();
            this.showGameOver("Draw", "Game drawn by threefold repetition.");
            return;
        }

        if (inCheck) {
            this.sound.playCheck();
            this.oracleTextEl.textContent = "⚠️ Check! King is under direct attack.";
        }
    }

    showGameOver(title, sub) {
        this.gameoverTitleEl.textContent = title;
        this.gameoverSubEl.textContent = sub;
        if (this.gameoverIconEl) {
            const lower = title.toLowerCase();
            if (lower.includes('checkmate')) this.gameoverIconEl.textContent = '👑';
            else if (lower.includes('resign')) this.gameoverIconEl.textContent = '🏳️';
            else if (lower.includes('draw') || lower.includes('stalemate') || lower.includes('50-move') || lower.includes('repetition') || lower.includes('material')) this.gameoverIconEl.textContent = '🤝';
            else this.gameoverIconEl.textContent = '🏆';
        }
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
        const halfClock = this.activeTab === 'telemetry' ? 0 : this.gameState.halfMoveClock;
        const fullMove = this.activeTab === 'telemetry' ? this.telemetryState.fullMoveNumber : this.gameState.fullMoveNumber;
        fen += `${halfClock} ${fullMove}`;
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

            if (data.best_move && this.gameState.isGameActive && (this.activeTab === 'play' || this.activeTab === 'telemetry')) {
                const src = data.best_move.substring(0, 2);
                const dst = data.best_move.substring(2, 4);
                const promo = data.best_move.length > 4 ? data.best_move[4] : null;
                const [srcR, srcC] = this.squareToCoords(src);
                const [dstR, dstC] = this.squareToCoords(dst);

                this.executeMove(srcR, srcC, dstR, dstC, data.best_move, promo);

                const engineColor = (this.gameState.playMode === 'human_white') ? 'b' : 'w';
                this.updateTelemetry(data);
                this.updateEvalBar(data.score || 0);
                this.updateOracle(data.score || 0, data.pv, engineColor);
            }
        } catch (e) {
            console.error("Engine API error:", e);
        } finally {
            this.gameState.isThinking = false;
            if (this.gameState.isGameActive && (this.activeTab === 'play' || this.activeTab === 'telemetry')) {
                this.setStatus("Match Active", false);
            }
        }
    }

    updateTelemetry(data) {
        if (!data) return;
        const nodesStr = Number(data.nodes || 0).toLocaleString();
        const npsM = (Number(data.nps || 0) / 1e6).toFixed(2);
        const depthStr = `Depth ${data.depth || 12}`;
        const timeStr = `${data.time_ms || 0} ms`;
        const hashStr = `${((data.hashfull || 0) / 10).toFixed(1)}%`;
        const winStr = `${data.win_chance !== undefined ? data.win_chance : 50.0}%`;
        const pvStr = data.pv || '--';

        if (this.teleNodesEl) this.teleNodesEl.textContent = nodesStr;
        if (this.teleNpsEl) this.teleNpsEl.textContent = `${npsM}M NPS`;
        if (this.teleDepthEl) this.teleDepthEl.textContent = depthStr;
        if (this.teleTimeEl) this.teleTimeEl.textContent = timeStr;
        if (this.teleHashfullEl) this.teleHashfullEl.textContent = hashStr;
        if (this.teleWinchanceEl) this.teleWinchanceEl.textContent = winStr;
        if (this.telePvEl) this.telePvEl.textContent = pvStr;

        if (this.quickNpsEl) this.quickNpsEl.textContent = `${npsM}M NPS`;
        if (this.quickDepthEl) this.quickDepthEl.textContent = depthStr;
        if (this.quickNodesEl) this.quickNodesEl.textContent = nodesStr;
        if (this.quickTimeEl) this.quickTimeEl.textContent = timeStr;
    }

    async runLiveAnalysis(movedBy = null) {
        if (this.gameState.isThinking || this.activeTab !== 'telemetry') return;
        const fen = this.getFEN();
        try {
            const resp = await fetch('/api/analyze', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ fen, depth: 12 })
            });
            if (resp.ok) {
                const data = await resp.json();
                this.updateTelemetry(data);
                this.updateEvalBar(data.score || 0);
                this.updateOracle(data.score || 0, data.pv, movedBy);
            }
        } catch (e) {
            console.error("Live analysis error:", e);
        }
    }

    cpToWinProb(cp) {
        const normalized = Math.max(-2000, Math.min(2000, cp));
        return 1.0 / (1.0 + Math.pow(10, -normalized / 400.0));
    }

    updateEvalBar(scoreCp) {
        const whiteWinProb = this.cpToWinProb(scoreCp);
        const fillPct = (whiteWinProb * 100).toFixed(1);
        this.evalFillEl.style.height = `${fillPct}%`;

        const evalText = (scoreCp >= 0 ? '+' : '') + (scoreCp / 100).toFixed(1);
        this.evalBadgeEl.textContent = evalText;
    }

    updateOracle(scoreCp, pv, movedBy = null) {
        const cp = scoreCp;
        const state = (this.activeTab === 'telemetry') ? this.telemetryState : this.gameState;
        const prevScore = state.lastEvalScore !== undefined ? state.lastEvalScore : 0;
        state.lastEvalScore = cp;

        this.moveGradeEl.classList.remove('hidden', 'brilliant', 'best', 'good', 'inaccuracy', 'mistake', 'blunder', 'book');

        // 1. Move Quality Classification via Win Probability Loss (Lichess/Chess.com standard)
        if (movedBy) {
            const prevWp = this.cpToWinProb(prevScore);
            const currWp = this.cpToWinProb(cp);

            // Mover's win probability before and after the move
            const moverPrevWp = (movedBy === 'w') ? prevWp : (1.0 - prevWp);
            const moverCurrWp = (movedBy === 'w') ? currWp : (1.0 - currWp);
            const wpLoss = Math.max(0, moverPrevWp - moverCurrWp);
            const wpGain = moverCurrWp - moverPrevWp;

            // Brilliant: finding a game-deciding tactic / breakthrough that swings win probability
            if (wpGain >= 0.18 && moverCurrWp >= 0.65) {
                this.moveGradeEl.className = 'move-grade brilliant';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Brilliant</span>`;
            } else if (wpLoss <= 0.015) {
                this.moveGradeEl.className = 'move-grade best';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Best Move</span>`;
            } else if (wpLoss <= 0.045) {
                this.moveGradeEl.className = 'move-grade good';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Good</span>`;
            } else if (wpLoss <= 0.10) {
                this.moveGradeEl.className = 'move-grade inaccuracy';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Inaccuracy</span>`;
            } else if (wpLoss <= 0.22) {
                this.moveGradeEl.className = 'move-grade mistake';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Mistake</span>`;
            } else {
                this.moveGradeEl.className = 'move-grade blunder';
                this.moveGradeEl.innerHTML = `<span class="grade-text">Blunder</span>`;
            }
        } else {
            this.moveGradeEl.classList.add('hidden');
        }

        // 2. Comprehensive Dual-Sided Positional Commentary
        const evalNum = (Math.abs(cp) / 100).toFixed(1);
        const moveCount = (this.activeTab === 'telemetry' ? this.telemetryState.uciHistory : this.gameState.uciHistory).length;
        let statusMsg = "";

        if (moveCount === 0 && Math.abs(cp) <= 65) {
            statusMsg = `Balanced starting position (+${evalNum} first-move initiative). Full piece coordination for both sides.`;
        } else if (cp >= 500) {
            statusMsg = `White holds a winning +${evalNum} advantage. Black is under heavy mating pressure.`;
        } else if (cp >= 200) {
            statusMsg = `White holds a clear advantage (+${evalNum}). Black must defend carefully.`;
        } else if (cp >= 75) {
            statusMsg = `White has a slight edge (+${evalNum}). Black maintains active piece counterplay.`;
        } else if (cp > -75) {
            statusMsg = `Equal position (${cp >= 0 ? '+' : ''}${(cp/100).toFixed(1)}). Both White and Black have balanced piece coordination.`;
        } else if (cp > -200) {
            statusMsg = `Black has a slight edge (-${evalNum}). White maintains solid king defense.`;
        } else if (cp > -500) {
            statusMsg = `Black holds a clear advantage (-${evalNum}). White is on the defensive.`;
        } else {
            statusMsg = `Black holds a winning -${evalNum} advantage. White's defense is collapsing.`;
        }

        const board = this.getCurrentBoard();
        const turn = this.getCurrentTurn();
        const inCheck = ChessRulesEngine.isInCheck(turn, board);
        if (inCheck) {
            statusMsg = `⚠️ ${turn === 'w' ? 'White' : 'Black'} King is in Check! ` + statusMsg;
        }

        this.oracleTextEl.textContent = statusMsg;
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
        } else if (this.activeTab === 'telemetry') {
            this.telemetryState.isFlipped = !this.telemetryState.isFlipped;
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
        if (this.activeTab === 'telemetry') {
            this.initTelemetryMode();
            return;
        }
        if (this.gameState.moveHistory.length === 0) return;
        this.initPlayMode();
    }

    resignMatch() {
        if (!this.gameState.isGameActive || this.gameState.isGameOver) return;
        this.gameState.isGameOver = true;
        this.stopClock();
        this.sound.playGameOver();
        const winner = this.gameState.playMode === 'human_white' ? "Black (Heaven's Gate)" : "White (Heaven's Gate)";
        this.showGameOver("Resignation", `${winner} won by resignation.`);
    }

    offerDraw() {
        if (!this.gameState.isGameActive || this.gameState.isGameOver) return;
        const cp = this.gameState.lastEvalScore || 0;
        const aiColor = (this.gameState.playMode === 'human_white') ? 'b' : 'w';
        const aiAdvantage = (aiColor === 'w') ? cp : -cp;

        if (aiAdvantage > 80) {
            this.sound.playError();
            this.setStatus("Draw offer declined by engine", false);
            this.oracleTextEl.textContent = "Engine declined draw offer. Position is advantageous for AI.";
        } else {
            this.gameState.isGameOver = true;
            this.stopClock();
            this.sound.playSuccess();
            this.showGameOver("Draw Agreed", "Engine accepted the draw offer.");
        }
    }

    copyFEN() {
        navigator.clipboard.writeText(this.getFEN());
        this.setStatus("FEN copied to clipboard", false);
    }

    copyPGN() {
        const dateStr = new Date().toISOString().slice(0, 10).replace(/-/g, '.');
        const whitePlayer = this.gameState.playMode === 'human_black' ? "Heaven's Gate Master Edition" : "Player";
        const blackPlayer = this.gameState.playMode === 'human_black' ? "Player" : "Heaven's Gate Master Edition";
        let resultTag = "*";
        if (this.gameState.isGameOver) {
            resultTag = "1/2-1/2";
        }

        let pgn = `[Event "Heaven's Gate Master Match"]\n`;
        pgn += `[Site "Localhost"]\n`;
        pgn += `[Date "${dateStr}"]\n`;
        pgn += `[White "${whitePlayer}"]\n`;
        pgn += `[Black "${blackPlayer}"]\n`;
        pgn += `[Result "${resultTag}"]\n\n`;

        let movesStr = "";
        for (let i = 0; i < this.gameState.uciHistory.length; i++) {
            if (i % 2 === 0) {
                movesStr += `${Math.floor(i / 2) + 1}. `;
            }
            movesStr += `${this.gameState.uciHistory[i]} `;
        }
        pgn += movesStr.trim() + ` ${resultTag}`;

        navigator.clipboard.writeText(pgn);
        this.setStatus("PGN copied to clipboard", false);
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
