"""
Heaven's Gate — Spectral Graph Theory & Chess Mathematical Engine
Real NumPy/SciPy Linear Algebra calculations for Graph Laplacians,
Eigenvalues, and Fiedler Vector topological partitioning.
"""

import numpy as np
import scipy.linalg


PIECE_NAMES = {
    'P': 'wP', 'N': 'wN', 'B': 'wB', 'R': 'wR', 'Q': 'wQ', 'K': 'wK',
    'p': 'bP', 'n': 'bN', 'b': 'bB', 'r': 'bR', 'q': 'bQ', 'k': 'bK'
}

INITIAL_BOARD_STR = [
    "rnbqkbnr",
    "pppppppp",
    "........",
    "........",
    "........",
    "........",
    "PPPPPPPP",
    "RNBQKBNR"
]


def parse_fen_to_grid(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"):
    """Parse a FEN string into an 8x8 list of characters."""
    board_part = fen.split()[0]
    rows = board_part.split('/')
    grid = []
    for row in rows:
        grid_row = []
        for ch in row:
            if ch.isdigit():
                grid_row.extend(['.'] * int(ch))
            else:
                grid_row.append(ch)
        grid.append(grid_row)
    return grid


def get_pieces_from_grid(grid):
    """Extract all non-empty pieces with their (row, col) coordinates."""
    pieces = []
    for r in range(8):
        for c in range(8):
            p = grid[r][c]
            if p != '.':
                pieces.append({
                    'symbol': p,
                    'type': PIECE_NAMES.get(p, p),
                    'color': 'w' if p.isupper() else 'b',
                    'row': r,
                    'col': c,
                    'square': f"{chr(ord('a') + c)}{8 - r}"
                })
    return pieces


def compute_piece_adjacency(pieces, grid=None):
    """
    Construct a real weighted tactical adjacency matrix A between all pieces on the board.
    Weights represent mutual defense, attack tension, and coordinated proximity.
    """
    n = len(pieces)
    if n == 0:
        return np.zeros((0, 0))

    A = np.zeros((n, n), dtype=np.float64)

    for i in range(n):
        for j in range(i + 1, n):
            p1 = pieces[i]
            p2 = pieces[j]
            same_color = (p1['color'] == p2['color'])

            dr = abs(p1['row'] - p2['row'])
            dc = abs(p1['col'] - p2['col'])
            dist_sq = dr * dr + dc * dc
            dist = np.sqrt(dist_sq)

            weight = 0.0

            # 1. Pawn Chains / Mutual Defense
            if same_color:
                if p1['symbol'].upper() == 'P' and p2['symbol'].upper() == 'P':
                    if dr == 1 and dc == 1:
                        weight += 2.5  # Solid diagonal pawn chain
                    elif dr == 0 and dc == 1:
                        weight += 1.8  # Phalanx pawns
                elif dr <= 2 and dc <= 2:
                    weight += 1.2  # Coordinated piece cluster
                # Distance-based coordination potential
                weight += max(0.0, 1.5 - 0.25 * dist)
            else:
                # 2. Enemy Tactical Tension
                if dr == 0 or dc == 0 or dr == dc:
                    weight += max(0.0, 2.2 - 0.2 * dist)  # Pin / Ray sightline
                elif (dr == 1 and dc == 2) or (dr == 2 and dc == 1):
                    weight += 1.4  # Knight fork tension
                else:
                    weight += max(0.0, 1.0 - 0.15 * dist)

            A[i, j] = weight
            A[j, i] = weight

    return A


def compute_spectral_graph(pieces, grid=None):
    """
    Given a list of pieces, computes:
      - Adjacency Matrix A (N x N)
      - Degree Matrix D (N x N, diagonal)
      - Graph Laplacian L = D - A
      - Eigenvalues lambda_1 <= lambda_2 <= ... <= lambda_N
      - Eigenvectors v_1, v_2, ..., v_N
      - Fiedler Vector v_2 (second smallest eigenvector)
      - Algebraic Connectivity lambda_2
      - Zero-crossing partition (positive vs negative values)
    """
    n = len(pieces)
    if n < 2:
        return None

    A = compute_piece_adjacency(pieces, grid)
    degrees = np.sum(A, axis=1)
    D = np.diag(degrees)
    L = D - A

    # Exact eigensolve using SciPy for symmetric matrices
    eigenvalues, eigenvectors = scipy.linalg.eigh(L)

    # Clean small floating-point noise near zero
    eigenvalues = np.maximum(eigenvalues, 0.0)

    # Fiedler vector is the second eigenvector (index 1)
    fiedler_vec = eigenvectors[:, 1]
    algebraic_connectivity = eigenvalues[1]

    # Assign values back to pieces
    partition = []
    for idx, p in enumerate(pieces):
        val = fiedler_vec[idx]
        partition.append({
            'piece': p,
            'fiedler_val': float(val),
            'cluster': 'positive' if val >= 0 else 'negative'
        })

    return {
        'A': A,
        'D': D,
        'L': L,
        'eigenvalues': eigenvalues,
        'eigenvectors': eigenvectors,
        'fiedler_vec': fiedler_vec,
        'lambda_2': float(algebraic_connectivity),
        'partition': partition
    }


def compute_64_square_laplacian():
    """
    Compute the standard 64-square topological grid Graph Laplacian (King graph).
    Useful for continuous 2D rubber-sheet space-warping animations.
    """
    A = np.zeros((64, 64), dtype=np.float64)
    for r in range(8):
        for c in range(8):
            sq1 = r * 8 + c
            for dr in [-1, 0, 1]:
                for dc in [-1, 0, 1]:
                    if dr == 0 and dc == 0:
                        continue
                    nr, nc = r + dr, c + dc
                    if 0 <= nr < 8 and 0 <= nc < 8:
                        sq2 = nr * 8 + nc
                        A[sq1, sq2] = 1.0

    D = np.diag(np.sum(A, axis=1))
    L = D - A
    eigenvalues, eigenvectors = scipy.linalg.eigh(L)
    return A, D, L, eigenvalues, eigenvectors
