"""
Heaven's Gate Documentary - Scene 04: The Fiedler Breakthrough (λ₂)
Script Beat:
- "3. The 1973 Breakthrough: The Fiedler Vector (λ₂)"
- "0 = λ₁ <= λ₂ <= λ₃ <= ... <= λ_N" (Sorted spectrum, λ₂ is Algebraic Connectivity)
- Tug-of-war quadratic form: min ∑ A[i, j] · (v[i] − v[j])²
- Constraint: ∑ v[i] = 0 and ‖v‖ = 1 (forces split into positive and negative clusters)
- 1D Number line [-1.0, +1.0] clustering
- Real locked pawn center position with broadcast-grade board
- Depth 0 topological diagnosis: Black pieces stranded on negative side while White king/queen coordinate on positive side

Resolution: 1920x1080 | 16:9 Landscape
Aesthetic: Broadcast Architectural (High contrast, crisp vector pieces, semi-transparent overlays, HUD telemetry console)
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
    FIEDLER_POS_COLOR,
    FIEDLER_NEG_COLOR,
    FIEDLER_CUT_COLOR
)

BG_OBSIDIAN = "#080c14"
TEXT_BRIGHT = "#f8fafc"
TEXT_MUTED  = "#94a3b8"
TEXT_DIM    = "#64748b"

class Scene04TheFiedlerBreakthrough(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # 1. Canvas
        # -------------------------------------------------------------
        bg = Rectangle(width=16, height=9, fill_color=BG_OBSIDIAN, fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # -------------------------------------------------------------
        # 2. Section Title & Spectrum Inequality
        # -------------------------------------------------------------
        sec_title = Text("The Fiedler Vector   (λ₂)", font="Consolas", color=TEXT_BRIGHT).scale(0.55)
        sec_title.to_edge(UP, buff=0.8).shift(LEFT * 2.2)

        spectrum_line = Text("0  =  λ₁  ≤  λ₂  ≤  λ₃  ≤  ···  ≤  λ_N", font="Consolas", color=TEXT_BRIGHT).scale(0.58)
        spectrum_line.next_to(sec_title, DOWN, aligned_edge=LEFT, buff=0.25)

        fiedler_tag = Text("λ₂: Algebraic Connectivity", font="Consolas", color="#38bdf8").scale(0.32)
        fiedler_tag.next_to(spectrum_line, RIGHT, buff=0.6)

        self.play(
            FadeIn(sec_title, UP * 0.2),
            FadeIn(spectrum_line, UP * 0.15),
            FadeIn(fiedler_tag, RIGHT * 0.2),
            run_time=1.2
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # 3. Tug-of-War Optimization Formula
        # -------------------------------------------------------------
        opt_formula = Text("min   ∑   A[i, j] · (v[i] − v[j])²", font="Consolas", color=TEXT_BRIGHT).scale(0.58)
        opt_formula.move_to(UP * 0.8)

        constraint = Text("subject to:   ∑  v[i]  =  0   and   ‖v‖  =  1", font="Consolas", color="#38bdf8").scale(0.34)
        constraint.next_to(opt_formula, DOWN, buff=0.22)

        formula_explain = Text("Coordinated pieces are penalized if v[i] ≠ v[j]  •  Constraint forces a zero-balanced split", font="Segoe UI", color=TEXT_DIM).scale(0.28)
        formula_explain.next_to(constraint, DOWN, buff=0.20)

        self.play(
            FadeOut(spectrum_line),
            FadeOut(fiedler_tag),
            FadeIn(opt_formula, UP * 0.2),
            FadeIn(constraint, UP * 0.15),
            FadeIn(formula_explain, UP * 0.1),
            run_time=1.2
        )
        self.wait(2.0)

        # -------------------------------------------------------------
        # 4. 1D Number Line Split [-1.0, +1.0]
        # -------------------------------------------------------------
        axis_line = NumberLine(
            x_range=[-1.0, 1.0, 0.2],
            width=10.0,
            tick_size=0.1,
            stroke_color="#334155",
            stroke_width=1.5
        )
        axis_line.move_to(DOWN * 0.4)

        lbl_neg = Text("−1.0", font="Consolas", color="#fbbf24").scale(0.28).next_to(axis_line.number_to_point(-1.0), DOWN, buff=0.2)
        lbl_zero = Text("0.0 (The Fault Line)", font="Consolas", color="#f43f5e").scale(0.30).next_to(axis_line.number_to_point(0.0), DOWN, buff=0.2)
        lbl_pos = Text("+1.0", font="Consolas", color="#38bdf8").scale(0.28).next_to(axis_line.number_to_point(1.0), DOWN, buff=0.2)

        zero_marker = Line(
            axis_line.number_to_point(0.0) + UP * 0.6,
            axis_line.number_to_point(0.0) + DOWN * 0.6,
            color="#f43f5e", stroke_width=2.5
        )

        pos_points = [0.48, 0.62, 0.75, 0.84]
        pos_tokens = VGroup()
        for p in pos_points:
            pt = axis_line.number_to_point(p) + UP * 0.3
            dot = Dot(pt, radius=0.08, color="#38bdf8")
            pos_tokens.add(dot)

        pos_bracket_lbl = Text("Kingside Attack Fist (+0.4 to +0.8)", font="Consolas", color="#38bdf8").scale(0.26)
        pos_bracket_lbl.next_to(axis_line.number_to_point(0.65), UP, buff=0.5)

        neg_points = [-0.52, -0.68, -0.82]
        neg_tokens = VGroup()
        for p in neg_points:
            pt = axis_line.number_to_point(p) + UP * 0.3
            dot = Dot(pt, radius=0.08, color="#fbbf24")
            neg_tokens.add(dot)

        neg_bracket_lbl = Text("Stranded Flank (−0.4 to −0.8)", font="Consolas", color="#fbbf24").scale(0.26)
        neg_bracket_lbl.next_to(axis_line.number_to_point(-0.65), UP, buff=0.5)

        self.play(
            opt_formula.animate.scale(0.7).to_edge(UP, buff=1.3).shift(LEFT * 2.5),
            constraint.animate.scale(0.7).next_to(opt_formula, RIGHT, buff=0.8),
            FadeOut(formula_explain),
            FadeOut(sec_title),
            ShowCreation(axis_line),
            FadeIn(lbl_neg),
            FadeIn(lbl_zero),
            FadeIn(lbl_pos),
            ShowCreation(zero_marker),
            run_time=1.2
        )
        self.play(
            LaggedStart(*[FadeIn(d, DOWN * 0.2) for d in pos_tokens], lag_ratio=0.1),
            FadeIn(pos_bracket_lbl, UP * 0.1),
            LaggedStart(*[FadeIn(d, DOWN * 0.2) for d in neg_tokens], lag_ratio=0.1),
            FadeIn(neg_bracket_lbl, UP * 0.1),
            run_time=1.2
        )
        self.wait(1.8)

        # -------------------------------------------------------------
        # 5. Broadcast Chess Board with High-Contrast Pieces & Overlays
        # -------------------------------------------------------------
        self.play(
            FadeOut(axis_line),
            FadeOut(lbl_neg),
            FadeOut(lbl_zero),
            FadeOut(lbl_pos),
            FadeOut(zero_marker),
            FadeOut(pos_tokens),
            FadeOut(pos_bracket_lbl),
            FadeOut(neg_tokens),
            FadeOut(neg_bracket_lbl),
            FadeOut(opt_formula),
            FadeOut(constraint),
            run_time=0.9
        )

        board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.1, sq_size=0.62)

        demo_pieces = [
            ("wP", 3, 3), ("wP", 4, 4), ("bP", 3, 4), ("bP", 4, 5),
            ("wQ", 6, 3), ("wR", 5, 0), ("wN", 5, 2), ("wK", 6, 0),
            ("bK", 6, 7),
            ("bR", 0, 7), ("bN", 1, 7)
        ]

        pieces_group = VGroup()
        for p_name, c, r in demo_pieces:
            p_mob = board.create_piece(p_name, c, r)
            pieces_group.add(p_mob)

        # Semi-transparent cluster highlights
        cluster_overlays = board.create_cluster_overlays(neg_cols=(0, 1, 2), pos_cols=(4, 5, 6, 7))

        # Fault line along locked pawn spine (col 3.5 = between d and e files)
        fault_line_group = board.create_fault_line(split_col=3.5)

        self.play(
            FadeIn(board),
            run_time=1.0
        )
        self.play(
            FadeIn(cluster_overlays),
            LaggedStart(*[FadeIn(p, DOWN * 0.15) for p in pieces_group], lag_ratio=0.08),
            run_time=1.2
        )
        self.play(
            FadeIn(fault_line_group),
            run_time=1.0
        )
        self.wait(0.5)

        # -------------------------------------------------------------
        # 6. HUD Telemetry Console Card (Depth 0 Diagnosis)
        # -------------------------------------------------------------
        hud_bg = RoundedRectangle(
            width=6.6,
            height=5.4,
            corner_radius=0.18,
            fill_color="#0c121e",
            fill_opacity=0.88,
            stroke_color="#1e293b",
            stroke_width=1.5
        )
        hud_bg.move_to(RIGHT * 3.1 + DOWN * 0.05)

        hud_title = Text("DEPTH 0 TOPOLOGICAL DIAGNOSIS", font="Consolas", color=TEXT_BRIGHT).scale(0.34)
        hud_sub = Text("M. Fiedler (1973) Algebraic Bisection", font="Consolas", color=TEXT_DIM).scale(0.24)
        hud_sub.next_to(hud_title, DOWN, aligned_edge=LEFT, buff=0.08)

        header_group = VGroup(hud_title, hud_sub)
        header_group.move_to(hud_bg.get_top() + DOWN * 0.6 + LEFT * 0.2)

        header_divider = Line(
            hud_bg.get_left() + RIGHT * 0.4 + DOWN * 1.0,
            hud_bg.get_right() + LEFT * 0.4 + DOWN * 1.0,
            color="#1e293b", stroke_width=1.0
        )
        header_divider.align_to(hud_sub, DOWN).shift(DOWN * 0.15)

        findings = [
            ("TOPOLOGY", "Board severed into 2 independent subgraphs", "#38bdf8"),
            ("WHITE KINGSIDE", "v₂ = +0.74  (Coordinated Attack Fist)", "#38bdf8"),
            ("BLACK DEFENDER (a8)", "v₂ = −0.82  (Stranded • 0 Communication)", "#fbbf24"),
            ("TACTICAL RESULT", "Black Rook cannot cross bottleneck in time", "#f43f5e"),
            ("SEARCH COST", "0 Moves Searched • Evaluated at Depth 0", "#10b981")
        ]

        card_group = VGroup()
        for head, body, col in findings:
            h = Text(head, font="Consolas", color=col).scale(0.22)
            b = Text(body, font="Segoe UI", color=TEXT_BRIGHT).scale(0.28)
            b.next_to(h, DOWN, aligned_edge=LEFT, buff=0.05)
            row = VGroup(h, b)
            card_group.add(row)

        card_group.arrange(DOWN, aligned_edge=LEFT, buff=0.22)
        card_group.next_to(header_divider, DOWN, aligned_edge=LEFT, buff=0.25)

        hud_console = VGroup(hud_bg, header_group, header_divider, card_group)

        self.play(
            FadeIn(hud_bg, scale=0.98),
            FadeIn(header_group, UP * 0.1),
            ShowCreation(header_divider),
            LaggedStart(*[FadeIn(r, RIGHT * 0.2) for r in card_group], lag_ratio=0.12),
            run_time=1.4
        )
        self.wait(3.5)
