"""
Heaven's Gate Documentary - Scene 02: What is a Spectral Graph?
Script Beat:
- "Okay so why spectral graphs? What even are they? Why would they be relevant in chess? Let's tackle these questions 1 by 1."
- "1. What is a Spectral Graph? In mathematics, a graph is just two things: nodes and edges..."
- "The nodes are the pieces on the board. The edges are the tactical lines between them..."
- "To do math on this network, we encode all those connections into a table: the Adjacency Matrix A..."
- "Next, we count how many total connections touch each piece: the Degree Matrix D..."
- "Now, when you subtract the adjacency matrix: L = D - A... calculating net difference in tactical tension."

Visual Standard:
- Broadcast-grade high-contrast chessboard (Silver-Ice & Slate-Blue)
- High-contrast vector pieces with crisp contour
- Authentic Chess.com / Lichess broadcast ribbon arrows (translucent, buffered, zero piece-piercing)
- 3B1B bracketed matrix with luminous entries and subtle sparse zeros
- Mathematically exact row-zero-sum Graph Laplacian L = D - A

Resolution: 1920x1080 | 16:9 Landscape
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

# Import our dedicated broadcast chessboard component
sys.path.append(str(Path(__file__).parent))
from chessboard_widget import (
    BroadcastChessBoard,
    BOARD_LIGHT_SQ,
    BOARD_DARK_SQ,
    TACTICAL_FRIENDLY,
    TACTICAL_HOSTILE
)

BG_OBSIDIAN = "#080c14"
TEXT_BRIGHT = "#f8fafc"
TEXT_DIM    = "#64748b"

ACCENT_BLUE = "#0ea5e9"
ACCENT_RED  = "#f43f5e"
ACCENT_GOLD = "#fbbf24"

class Scene02WhatIsASpectralGraph(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # 1. Canvas
        # -------------------------------------------------------------
        bg = Rectangle(width=16, height=9, fill_color=BG_OBSIDIAN, fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # -------------------------------------------------------------
        # 2. Architectural Broadcast Board & Pieces
        # -------------------------------------------------------------
        board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.1, sq_size=0.62)

        pieces_data = [
            ("wR", 3, 0), ("wQ", 3, 3), ("wN", 5, 2), ("wP", 4, 3), ("wK", 6, 0),
            ("bB", 2, 4), ("bR", 4, 7), ("bK", 6, 7)
        ]

        pieces_group = VGroup()
        coords_map = {}

        for p_name, col, row in pieces_data:
            p_mob = board.create_piece(p_name, col, row)
            pieces_group.add(p_mob)
            coords_map[p_name] = board.get_square_pos(col, row)

        self.play(
            FadeIn(board),
            run_time=1.0
        )
        self.play(
            LaggedStart(*[FadeIn(p, DOWN * 0.15) for p in pieces_group], lag_ratio=0.08),
            run_time=1.0
        )
        self.wait(0.6)

        # -------------------------------------------------------------
        # 3. Broadcast-Grade Tactical Ribbon Arrows (Chess.com / Lichess Standard)
        # -------------------------------------------------------------
        # 1. White Rook on d1 defending White Queen on d4 (d-file battery)
        arr_rq = board.create_tactical_arrow(coords_map["wR"], coords_map["wQ"], color=ACCENT_BLUE)

        # 2. White Knight on f3 defending White Pawn on e4 (curved knight arc)
        arr_np = board.create_tactical_arrow(coords_map["wN"], coords_map["wP"], color=ACCENT_BLUE, path_arc=-TAU/12)

        # 3. White Queen on d4 protecting White Pawn on e4
        arr_qp = board.create_tactical_arrow(coords_map["wQ"], coords_map["wP"], color=ACCENT_BLUE)

        # 4. Black Bishop on c5 attacking White Queen on d4
        arr_bq = board.create_tactical_arrow(coords_map["bB"], coords_map["wQ"], color=ACCENT_RED)

        # 5. Black Rook on e8 attacking White Pawn on e4
        arr_rp = board.create_tactical_arrow(coords_map["bR"], coords_map["wP"], color=ACCENT_RED)

        tactical_arrows = VGroup(arr_rq, arr_np, arr_qp, arr_bq, arr_rp)

        network_caption = Text("Nodes = Pieces   •   Edges = Tactical Rays & Mutual Support", font="Consolas", color=ACCENT_BLUE).scale(0.26)
        network_caption.next_to(board.outer_frame, DOWN, buff=0.25)

        self.play(
            LaggedStart(*[FadeIn(a, scale=0.8) for a in tactical_arrows], lag_ratio=0.15),
            FadeIn(network_caption, UP * 0.1),
            run_time=1.3
        )
        self.wait(1.2)

        # -------------------------------------------------------------
        # 4. The Elegant Bracketed Mathematical Matrix (3B1B Style)
        # -------------------------------------------------------------
        mat_center = RIGHT * 3.2 + DOWN * 0.1

        mat_title = Text("Adjacency Matrix   A", font="Consolas", color=TEXT_BRIGHT).scale(0.48)
        mat_sub = Text("A[i, j] > 0   Tactical Coupling & Attack Rays", font="Consolas", color=TEXT_DIM).scale(0.26)
        mat_sub.next_to(mat_title, DOWN, aligned_edge=LEFT, buff=0.12)
        mat_header = VGroup(mat_title, mat_sub)
        mat_header.move_to(mat_center + UP * 2.5)

        cell_w = 0.52
        cell_h = 0.44

        # Indices: 0:wR, 1:wQ, 2:wN, 3:wP, 4:wK, 5:bB, 6:bR, 7:bK
        matrix_elements = VGroup()
        active_entries = {
            (0, 1): ("1.5", ACCENT_BLUE),
            (1, 0): ("1.5", ACCENT_BLUE),
            (2, 3): ("1.0", ACCENT_BLUE),
            (3, 2): ("1.0", ACCENT_BLUE),
            (1, 3): ("1.0", ACCENT_BLUE),
            (3, 1): ("1.0", ACCENT_BLUE),
            (5, 1): ("2.0", ACCENT_RED),
            (1, 5): ("2.0", ACCENT_RED),
            (6, 3): ("2.0", ACCENT_RED),
            (3, 6): ("2.0", ACCENT_RED),
        }

        for r in range(8):
            for c in range(8):
                cx = mat_center[0] + (c - 3.5) * cell_w
                cy = mat_center[1] + (3.5 - r) * cell_h - 0.2

                if (r, c) in active_entries:
                    val_str, color_entry = active_entries[(r, c)]
                    elem = Text(val_str, font="Consolas", color=color_entry).scale(0.28)
                else:
                    elem = Text("·", font="Consolas", color="#334155").scale(0.35)

                elem.move_to(np.array([cx, cy, 0]))
                matrix_elements.add(elem)

        l_bracket = Text("[", font="Consolas", color="#475569").scale(2.8)
        l_bracket.move_to(np.array([mat_center[0] - 4.0 * cell_w - 0.15, mat_center[1] - 0.2, 0]))

        r_bracket = Text("]", font="Consolas", color="#475569").scale(2.8)
        r_bracket.move_to(np.array([mat_center[0] + 4.0 * cell_w + 0.15, mat_center[1] - 0.2, 0]))

        matrix_group = VGroup(l_bracket, matrix_elements, r_bracket)

        self.play(
            FadeIn(mat_header, UP * 0.2),
            FadeIn(l_bracket, RIGHT * 0.1),
            FadeIn(r_bracket, LEFT * 0.1),
            LaggedStart(*[FadeIn(e, scale=0.7) for e in matrix_elements], lag_ratio=0.02),
            run_time=1.4
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # 5. Smooth Evolution into the Graph Laplacian (L = D - A)
        # -------------------------------------------------------------
        formula_hero = Text("L   =   D   −   A", font="Consolas", color=ACCENT_BLUE).scale(0.65)
        formula_sub = Text("The Graph Laplacian: Net Difference in Tactical Tension", font="Segoe UI", color=TEXT_DIM).scale(0.28)
        formula_sub.next_to(formula_hero, DOWN, aligned_edge=LEFT, buff=0.1)
        hero_group = VGroup(formula_hero, formula_sub)
        hero_group.move_to(mat_header.get_center())

        # Exact row sums:
        # Row 0 (wR): 1.5
        # Row 1 (wQ): 1.5 + 1.0 + 2.0 = 4.5
        # Row 2 (wN): 1.0
        # Row 3 (wP): 1.0 + 1.0 + 2.0 = 4.0
        # Row 4 (wK): 0.0
        # Row 5 (bB): 2.0
        # Row 6 (bR): 2.0
        # Row 7 (bK): 0.0
        degree_vals = ["1.5", "4.5", "1.0", "4.0", "0.0", "2.0", "2.0", "0.0"]
        degree_animations = []

        for i in range(8):
            cell_idx = i * 8 + i
            old_elem = matrix_elements[cell_idx]
            new_val = Text(degree_vals[i], font="Consolas", color=ACCENT_GOLD).scale(0.28)
            new_val.move_to(old_elem.get_center())
            degree_animations.append(Transform(old_elem, new_val))

        diag_label = Text("Diagonal D[i, i] = Total Connection Degree", font="Consolas", color=ACCENT_GOLD).scale(0.26)
        diag_label.next_to(matrix_group, DOWN, buff=0.35)

        tension_formula = Text("(L v)[i]  =  ∑  A[i, j] · (v[i] − v[j])", font="Consolas", color=TEXT_BRIGHT).scale(0.32)
        tension_formula.next_to(diag_label, DOWN, buff=0.20)
        tension_box = SurroundingRectangle(tension_formula, color=ACCENT_BLUE, buff=0.08, stroke_width=1.0)

        self.play(
            Transform(mat_header, hero_group),
            *degree_animations,
            FadeIn(diag_label, UP * 0.1),
            run_time=1.3
        )
        self.play(
            FadeIn(tension_formula, UP * 0.1),
            ShowCreation(tension_box),
            run_time=0.9
        )
        self.wait(3.0)
