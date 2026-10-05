"""
Heaven's Gate Master Broadcast Documentary — Scene 02: What Makes It "Spectral"?
Craft Standard: 3Blue1Brown (Essence of Linear Algebra) / vcubingx Standard
Runtime: ~52 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Obsidian (#000000) & Luminous Jewel Technical Palette

Narrative Arc (Section 2 from narrator script):
1. Frame-0 Continuity: Inherits exact terminal frame from Scene 01 (floating chessboard network + Laplacian L).
2. The Linear Transformation: Demonstrates Lv = w with generic vectors rotating off their span (Δθ ≠ 0).
3. The Invariant Eigendirection: Living vector sweep snaps into collinear lock (Δθ = 0) with caliper scale λ.
4. The Optical Prism Metaphor: Entangled tactical chess geometry condenses into a focused beam,
   refracting through a glass prism and dispersing into 4 spatial frequency harmonics (λ_1 to λ_N),
   spotlighting λ_2 (Algebraic Connectivity / Fiedler Frequency).
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex, HeatMatrix
from theme import *


class Scene02WhatMakesItSpectral(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: TECHNICAL BLUEPRINT GRID ON OBSIDIAN PITCH BLACK
        # Identical to Scene 01 terminal background
        # =============================================================
        tech_grid = NumberPlane(
            x_range=[-16, 16, 1],
            y_range=[-10, 10, 1],
            width=32,
            height=20,
            axis_config={"stroke_color": CARD_BORDER, "stroke_width": 0.7, "stroke_opacity": 0.40},
            background_line_style={"stroke_color": GRID_LINE, "stroke_width": 0.5, "stroke_opacity": 0.25},
            faded_line_style={"stroke_color": GRID_FADED, "stroke_width": 0.3, "stroke_opacity": 0.15}
        )
        tech_grid.shift(OUT * -0.2)
        self.add(tech_grid)

        frame = self.camera.frame
        frame.set_euler_angles(theta=0, phi=0)
        frame.move_to(ORIGIN)

        # =============================================================
        # FRAME 0 CONTINUITY: EXACT TERMINAL STATE OF SCENE 01
        # Inherits the scaled tactical graph on the left and
        # the Graph Laplacian L with row/col labels and curvature operator on the right.
        # =============================================================
        board = BroadcastChessBoard(
            center=ORIGIN + DOWN * 0.25,
            sq_size=0.62,
            light_color="#bcc7d6",
            dark_color="#2b384c",
            show_coords=True
        )

        p_qd4  = board.create_piece("wQ", 3, 3)  # d4 Queen
        p_bc4  = board.create_piece("wB", 2, 3)  # c4 Bishop
        p_ne5  = board.create_piece("wN", 4, 4)  # e5 Knight
        p_rd1  = board.create_piece("wR", 3, 0)  # d1 Rook
        p_kg1  = board.create_piece("wK", 6, 0)  # g1 King
        p_bkg8 = board.create_piece("bK", 6, 7)  # g8 Black King
        p_be7  = board.create_piece("bB", 4, 6)  # e7 Black Bishop
        p_bnc6 = board.create_piece("bN", 2, 5)  # c6 Black Knight

        piece_list = [p_qd4, p_bc4, p_ne5, p_rd1, p_kg1, p_bkg8, p_be7, p_bnc6]
        pieces = VGroup(*piece_list)

        def make_tactical_edge(p1, p2, color=JEWEL_CYAN):
            core = Line(p1, p2, color=color, stroke_width=1.8, stroke_opacity=0.90)
            glow = Line(p1, p2, color=color, stroke_width=3.6, stroke_opacity=0.20)
            d1 = Dot(p1, radius=0.035, color=color)
            d2 = Dot(p2, radius=0.035, color=color)
            return VGroup(glow, core, d1, d2)

        ray_q_b = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(2, 3), JEWEL_CYAN)
        ray_q_n = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(4, 4), JEWEL_CYAN)
        ray_q_r = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(3, 0), JEWEL_CYAN)
        ray_q_k = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(6, 7), JEWEL_CYAN)
        ray_n_c = make_tactical_edge(board.get_square_pos(4, 4), board.get_square_pos(2, 5), JEWEL_CYAN)
        ray_b_e = make_tactical_edge(board.get_square_pos(2, 3), board.get_square_pos(4, 6), JEWEL_CYAN)

        tactical_rays = VGroup(ray_q_b, ray_q_n, ray_q_r, ray_q_k, ray_n_c, ray_b_e)

        board.squares_group.set_opacity(0.12)
        board.outer_frame.set_opacity(0.18)
        board.inner_border.set_opacity(0.15)
        board.coords_group.set_opacity(0.15)

        graph_group = VGroup(board, tactical_rays, pieces)
        graph_group.scale(0.74).move_to(LEFT * 4.1 + DOWN * 0.15)

        # Right side: Graph Laplacian L
        mat_center = RIGHT * 3.6 + DOWN * 0.15
        L_data = [
            [ 4, -1, -1, -1, -1,  0],
            [-1,  2,  0, -1,  0,  0],
            [-1,  0,  3,  0, -1, -1],
            [-1, -1,  0,  2,  0,  0],
            [-1,  0, -1,  0,  2,  0],
            [ 0,  0, -1,  0,  0,  1],
        ]

        matrix_L = HeatMatrix(
            L_data,
            cell_size=(0.60, 0.52),
            h_buff=0.08,
            v_buff=0.08,
            font_size=18,
            bracket_color=JEWEL_LAVENDER,
            bracket_width=2.5
        ).move_to(mat_center)

        for r in range(6):
            for c in range(6):
                if r == c:
                    matrix_L.get_cell_bg(r, c).set_fill(JEWEL_GOLD, opacity=0.30).set_stroke(JEWEL_GOLD, 1.4)
                    matrix_L.get_cell_text(r, c).set_color(JEWEL_GOLD)
                elif L_data[r][c] < 0:
                    matrix_L.get_cell_bg(r, c).set_fill(JEWEL_CORAL, opacity=0.25).set_stroke(JEWEL_CORAL, 1.0)
                    matrix_L.get_cell_text(r, c).set_color(JEWEL_CORAL)

        title_L = CMTex(r"L = D - A \quad (\text{The Graph Laplacian})", fontsize=28, height=0.38, color=JEWEL_LAVENDER)
        title_L.move_to(mat_center + UP * 2.30)

        labels_txt = ["Q", "B", "N", "R", "K", "k"]
        row_headers = VGroup()
        col_headers = VGroup()
        for idx, lbl in enumerate(labels_txt):
            rh = Text(lbl, font="Consolas", font_size=16, color=JEWEL_LAVENDER)
            rh.next_to(matrix_L.get_cell_bg(idx, 0), LEFT, buff=0.28)
            row_headers.add(rh)
            ch = Text(lbl, font="Consolas", font_size=16, color=JEWEL_LAVENDER)
            ch.next_to(matrix_L.get_cell_bg(0, idx), UP, buff=0.22)
            col_headers.add(ch)

        curvature_cm = CMTex(
            r"(L v)_i = \sum_{j} A_{ij}(v_i - v_j) \quad \text{(Laplace-Beltrami Operator)}",
            fontsize=26,
            height=0.36,
            color=TEXT_WHITE
        ).move_to(mat_center + DOWN * 2.32)

        inherited_state = VGroup(graph_group, title_L, row_headers, col_headers, matrix_L, curvature_cm)
        self.add(inherited_state)
        self.wait(0.8)

        # =============================================================
        # BEAT 1: CLEAN HANDOFF & MATRIX-VECTOR ROTATION (0s - 18s)
        # Narrator: "Normally, when you multiply a matrix by a vector,
        # it stretches the vector and rotates it into a completely new direction: Lv = w."
        # =============================================================
        title_scene02 = CMTex(r"\text{2. What Makes It ``Spectral''?}", fontsize=30, height=0.40, color=JEWEL_CYAN)
        title_scene02.move_to(UP * 3.35)

        self.play(
            FadeIn(title_scene02, UP * 0.15),
            run_time=1.2,
            rate_func=smooth
        )
        self.wait(0.4)

        # First: Fade out old Scene 01 objects completely so there is NO overlap!
        self.play(
            FadeOut(graph_group),
            FadeOut(matrix_L),
            FadeOut(row_headers),
            FadeOut(col_headers),
            FadeOut(title_L),
            FadeOut(curvature_cm),
            run_time=1.0,
            rate_func=smooth
        )

        # 2D Vector Coordinate Plane on Left
        coord_origin = LEFT * 2.5 + DOWN * 0.15
        axes = Axes(
            x_range=[-3.0, 3.0, 1],
            y_range=[-3.0, 3.0, 1],
            width=4.8,
            height=4.8,
            axis_config={
                "stroke_color": "#3d4f6e",
                "stroke_width": 1.6,
                "include_ticks": True,
                "tick_size": 0.08,
            }
        ).move_to(coord_origin)

        o = axes.c2p(0, 0)

        # Subtle faded interior coordinate lines for axes
        grid_sublines = VGroup()
        for gx in np.arange(-2.0, 3.0, 1.0):
            if abs(gx) > 0.01:
                vl = Line(axes.c2p(gx, -2.5), axes.c2p(gx, 2.5), stroke_color="#182336", stroke_width=0.8, stroke_opacity=0.40)
                grid_sublines.add(vl)
        for gy in np.arange(-2.0, 3.0, 1.0):
            if abs(gy) > 0.01:
                hl = Line(axes.c2p(-2.5, gy), axes.c2p(2.5, gy), stroke_color="#182336", stroke_width=0.8, stroke_opacity=0.40)
                grid_sublines.add(hl)

        # Axis numeric labels for clarity
        axis_labels = VGroup()
        for val in [-2, -1, 1, 2]:
            lx = CMTex(str(val), fontsize=14, height=0.15, color=TEXT_MUTED).next_to(axes.c2p(val, 0), DOWN, buff=0.12)
            ly = CMTex(str(val), fontsize=14, height=0.15, color=TEXT_MUTED).next_to(axes.c2p(0, val), LEFT, buff=0.12)
            axis_labels.add(lx, ly)

        axes_group = VGroup(grid_sublines, axes, axis_labels)

        # Linear Transformation Equation Suite on Right (positioned safely at x = 3.6)
        eq_panel_center = RIGHT * 3.6 + DOWN * 0.15
        eq_title = CMTex(r"\text{The Linear Transformation}", fontsize=26, height=0.36, color=JEWEL_CYAN)
        eq_action = CMTex(r"L v = w \quad \text{(Matrix-Vector Product)}", fontsize=22, height=0.30, color=TEXT_WHITE)
        eq_rot = CMTex(r"\text{Generic vectors rotate off span: } \Delta\theta \neq 0", fontsize=20, height=0.26, color=JEWEL_CORAL)

        eq_box = VGroup(eq_title, eq_action, eq_rot).arrange(DOWN, aligned_edge=LEFT, buff=0.28)
        eq_box.move_to(eq_panel_center + UP * 0.8)

        # Fade in new coordinate plane and equations cleanly
        self.play(
            FadeIn(axes_group),
            FadeIn(eq_box, RIGHT * 0.2),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(0.6)

        # -------------------------------------------------------------
        # Generic Vector Testing: v -> w = Lv rotates!
        # -------------------------------------------------------------
        v_coords = np.array([1.8, 0.4, 0])
        w_coords = np.array([0.7, 2.2, 0])

        vec_v = Arrow(
            o, axes.c2p(*v_coords),
            buff=0,
            fill_color=TEXT_BRIGHT,
            fill_opacity=1.0,
            stroke_color=TEXT_BRIGHT,
            stroke_width=0.0,
            thickness=2.2,
            tip_width_ratio=3.8,
            tip_angle=PI / 3.5,
        )
        lbl_v = CMTex(r"v", fontsize=22, height=0.24, color=TEXT_WHITE).next_to(vec_v.get_end(), RIGHT, buff=0.10)

        vec_w = Arrow(
            o, axes.c2p(*w_coords),
            buff=0,
            fill_color=JEWEL_CORAL,
            fill_opacity=1.0,
            stroke_color=JEWEL_CORAL,
            stroke_width=0.0,
            thickness=2.2,
            tip_width_ratio=3.8,
            tip_angle=PI / 3.5,
        )
        lbl_w = CMTex(r"w = L v", fontsize=22, height=0.24, color=JEWEL_CORAL).next_to(vec_w.get_end(), UP, buff=0.10)

        angle_v = np.arctan2(v_coords[1], v_coords[0])
        angle_w = np.arctan2(w_coords[1], w_coords[0])
        rot_arc = Arc(radius=0.85, start_angle=angle_v, angle=angle_w - angle_v, color=JEWEL_CORAL, stroke_width=1.5, arc_center=o)
        rot_lbl = CMTex(r"\Delta\theta \neq 0", fontsize=18, height=0.22, color=JEWEL_CORAL).next_to(rot_arc, UP + RIGHT, buff=0.08)

        generic_vec_group = VGroup(vec_v, lbl_v, vec_w, lbl_w, rot_arc, rot_lbl)

        self.play(
            ShowCreation(vec_v),
            FadeIn(lbl_v),
            run_time=1.0,
            rate_func=smooth
        )
        self.wait(0.4)

        # Matrix transformation fires: v is rotated and stretched into w
        self.play(
            ShowCreation(vec_w),
            FadeIn(lbl_w),
            ShowCreation(rot_arc),
            FadeIn(rot_lbl),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(2.2)

        # =============================================================
        # BEAT 2: THE INVARIANT EIGENDIRECTION Lv = λv (18s - 36s)
        # Narrator: "Except for a few very special directions.
        # For certain vectors, the matrix doesn't rotate them at all—it only scales them
        # by a single constant factor, which mathematicians call λ (lambda): Lv = λv."
        # =============================================================
        # Invariant Eigendirection Line (45-degree axis)
        inv_axis = DashedLine(
            axes.c2p(-2.3, -2.3), axes.c2p(2.3, 2.3),
            color=JEWEL_GOLD, stroke_width=1.8, stroke_opacity=0.45, dash_length=0.10
        )
        inv_label = CMTex(r"\text{Invariant Axis: } \text{span}(v_{\lambda})", fontsize=18, height=0.22, color=JEWEL_GOLD)
        inv_label.next_to(axes.c2p(-2.3, -2.3), DOWN + RIGHT, buff=0.12)

        # Eigendirection Equation Suite on Right
        eq_eig_title = CMTex(r"\text{The Invariant Eigendirection}", fontsize=26, height=0.36, color=JEWEL_GOLD)
        eq_eig_form  = CMTex(r"L v = \lambda v \quad (\text{Eigenvalue Equation})", fontsize=22, height=0.30, color=TEXT_WHITE)
        eq_eig_lock  = CMTex(r"\text{Zero rotation: } \Delta\theta = 0", fontsize=20, height=0.26, color=JEWEL_GREEN)
        eq_eig_desc1 = CMTex(r"v : \text{Eigenvector (Directional Harmony)}", fontsize=18, height=0.24, color=TEXT_BRIGHT)
        eq_eig_desc2 = CMTex(r"\lambda : \text{Eigenvalue (Spatial Frequency)}", fontsize=18, height=0.24, color=JEWEL_GOLD)

        eq_eig_box = VGroup(eq_eig_title, eq_eig_form, eq_eig_lock, eq_eig_desc1, eq_eig_desc2).arrange(DOWN, aligned_edge=LEFT, buff=0.24)
        eq_eig_box.move_to(eq_panel_center + UP * 0.4)

        self.play(
            FadeOut(generic_vec_group),
            Transform(eq_box, eq_eig_box),
            ShowCreation(inv_axis),
            FadeIn(inv_label),
            run_time=1.6,
            rate_func=smooth
        )
        self.wait(0.6)

        # Dynamic Sweep: v aligns onto the invariant axis and snaps into Lv = λv
        v_eig_coords = np.array([1.1, 1.1, 0])
        w_eig_coords = np.array([2.2, 2.2, 0])

        vec_v_aligned = Arrow(
            o, axes.c2p(*v_eig_coords),
            buff=0,
            fill_color=TEXT_BRIGHT,
            fill_opacity=1.0,
            stroke_color=TEXT_BRIGHT,
            stroke_width=0.0,
            thickness=2.2,
            tip_width_ratio=3.8,
            tip_angle=PI / 3.5,
        )
        lbl_v_aligned = CMTex(r"v", fontsize=22, height=0.24, color=TEXT_WHITE).next_to(vec_v_aligned.get_end(), UP + LEFT, buff=0.08)

        self.play(
            ShowCreation(vec_v_aligned),
            FadeIn(lbl_v_aligned),
            run_time=1.2,
            rate_func=smooth
        )

        # Collinear lock: Lv scales purely by lambda = 2.0 along the exact same line!
        vec_eig_core = Arrow(
            o, axes.c2p(*w_eig_coords),
            buff=0,
            fill_color=JEWEL_GOLD,
            fill_opacity=1.0,
            stroke_color=JEWEL_GOLD,
            stroke_width=0.0,
            thickness=2.6,
            tip_width_ratio=3.8,
            tip_angle=PI / 3.5,
        )
        lbl_eig_res = CMTex(r"L v = \lambda v", fontsize=22, height=0.26, color=JEWEL_GOLD).next_to(vec_eig_core.get_end(), RIGHT, buff=0.12)

        # Precision dimension calipers measuring stretch factor lambda
        # Positioned cleanly on the UP+LEFT side of the caliper line to avoid colliding with right equations!
        c_p1 = axes.c2p(1.1, 1.1)
        c_p2 = axes.c2p(2.2, 2.2)
        caliper_line = Line(c_p1, c_p2, color=JEWEL_CYAN, stroke_width=2.4)
        tick1 = Line(c_p1 + np.array([-0.06, 0.06, 0]), c_p1 + np.array([0.06, -0.06, 0]), color=JEWEL_CYAN, stroke_width=2.0)
        tick2 = Line(c_p2 + np.array([-0.06, 0.06, 0]), c_p2 + np.array([0.06, -0.06, 0]), color=JEWEL_CYAN, stroke_width=2.0)
        caliper_txt = CMTex(r"\text{Pure Stretch: } \lambda = 2.0\times", fontsize=18, height=0.24, color=JEWEL_CYAN)
        caliper_txt.next_to(caliper_line, UP + LEFT, buff=0.12)
        caliper_group = VGroup(caliper_line, tick1, tick2, caliper_txt)

        self.play(
            ShowCreation(vec_eig_core),
            FadeIn(lbl_eig_res),
            run_time=1.4,
            rate_func=smooth
        )
        self.play(
            ShowCreation(caliper_group),
            run_time=1.0,
            rate_func=smooth
        )
        self.wait(2.5)

        # =============================================================
        # BEAT 3: THE OPTICAL PRISM & SPATIAL FREQUENCIES (36s - 55s)
        # Narrator: "The entire set of these eigenvalues is called the spectrum of the matrix.
        # Just like an optical prism splits light into its constituent wavelengths,
        # the eigenvalues split the complex, messy geometry of the chessboard into its fundamental spatial frequencies."
        # =============================================================
        self.play(
            FadeOut(axes_group),
            FadeOut(inv_axis),
            FadeOut(inv_label),
            FadeOut(vec_v_aligned),
            FadeOut(lbl_v_aligned),
            FadeOut(vec_eig_core),
            FadeOut(lbl_eig_res),
            FadeOut(caliper_group),
            FadeOut(eq_box),
            run_time=1.2,
            rate_func=smooth
        )

        # -------------------------------------------------------------
        # Left Side: Miniature Chessboard Tactical Network
        # -------------------------------------------------------------
        mini_board = BroadcastChessBoard(
            center=LEFT * 4.9 + DOWN * 0.15,
            sq_size=0.34,
            light_color="#bcc7d6",
            dark_color="#2b384c",
            show_coords=False
        )

        mini_p_qd4  = mini_board.create_piece("wQ", 3, 3)
        mini_p_bc4  = mini_board.create_piece("wB", 2, 3)
        mini_p_ne5  = mini_board.create_piece("wN", 4, 4)
        mini_p_rd1  = mini_board.create_piece("wR", 3, 0)
        mini_p_kg1  = mini_board.create_piece("wK", 6, 0)
        mini_p_bkg8 = mini_board.create_piece("bK", 6, 7)
        mini_p_be7  = mini_board.create_piece("bB", 4, 6)
        mini_p_bnc6 = mini_board.create_piece("bN", 2, 5)

        mini_piece_list = [mini_p_qd4, mini_p_bc4, mini_p_ne5, mini_p_rd1, mini_p_kg1, mini_p_bkg8, mini_p_be7, mini_p_bnc6]
        mini_pieces = VGroup(*mini_piece_list)

        mini_ray1 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(2, 3), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray2 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(4, 4), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray3 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(6, 7), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray4 = Line(mini_board.get_square_pos(2, 3), mini_board.get_square_pos(4, 6), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_tactical = VGroup(mini_ray1, mini_ray2, mini_ray3, mini_ray4)

        mini_chess = VGroup(mini_board, mini_pieces, mini_tactical)
        mini_tag = CMTex(r"\text{Chessboard Tactical Network}", fontsize=20, height=0.24, color=JEWEL_CYAN)
        mini_tag.next_to(mini_board, UP, buff=0.22)

        # -------------------------------------------------------------
        # Center: Refractive Optical Glass Prism
        # -------------------------------------------------------------
        prism_center = LEFT * 1.5 + DOWN * 0.15
        apex = prism_center + UP * 2.2
        bl   = prism_center + LEFT * 1.8 + DOWN * 1.8
        br   = prism_center + RIGHT * 1.8 + DOWN * 1.8

        prism_glass = Polygon(
            apex, bl, br,
            fill_color="#07101d", fill_opacity=0.85,
            stroke_color=JEWEL_CYAN, stroke_width=2.0
        )
        specular_edge = Line(apex, bl, color=TEXT_WHITE, stroke_width=1.4, stroke_opacity=0.65)
        prism_group = VGroup(prism_glass, specular_edge)

        # Focused White Tactical Laser Beam from Board into Prism
        # Left face intercept at y = -0.15 is at x = -2.49
        board_exit_pt = np.array([-3.55, -0.15, 0])
        hit_pt = np.array([-2.49, -0.15, 0])
        beam_glow = Line(board_exit_pt, hit_pt, color=TEXT_WHITE, stroke_width=6.5, stroke_opacity=0.35)
        beam_core = Line(board_exit_pt, hit_pt, color=TEXT_WHITE, stroke_width=2.4, stroke_opacity=1.0)
        incident_beam = VGroup(beam_glow, beam_core)

        self.play(
            FadeIn(mini_chess, scale=0.95),
            FadeIn(mini_tag, UP * 0.1),
            FadeIn(prism_group, scale=0.95),
            ShowCreation(incident_beam),
            run_time=1.8,
            rate_func=smooth
        )

        # Internal downward refraction ray through glass (Snell's Law physical bend)
        # Right face intercept: line between apex (-1.5, 2.05) and br (0.3, -1.95)
        # At y = -0.40: t = (-0.40 - (-1.95)) / 4.0 = 1.55 / 4.0 = 0.3875
        # x = 0.3 + 0.3875 * (-1.5 - 0.3) = 0.3 - 0.6975 = -0.3975 ≈ -0.40
        exit_pt = np.array([-0.40, -0.40, 0])
        internal_ray = Line(hit_pt, exit_pt, color=TEXT_WHITE, stroke_width=2.4, stroke_opacity=0.85)
        self.play(ShowCreation(internal_ray), run_time=0.6, rate_func=linear)

        # -------------------------------------------------------------
        # Right Side: 4 Dispersed Spatial Frequency Tiers (The Spectrum)
        # -------------------------------------------------------------
        tier_specs = [
            (
                r"\lambda_1 = 0.00",
                r"\text{Connected Baseline (Global DC)}",
                RIGHT * 3.4 + UP * 2.1,
                JEWEL_CYAN,
                "dc"
            ),
            (
                r"\lambda_2 = 0.42",
                r"\text{Fiedler Frequency (Algebraic Connectivity)}",
                RIGHT * 3.4 + UP * 0.7,
                JEWEL_GOLD,
                "half"
            ),
            (
                r"\lambda_3 = 1.15",
                r"\text{Intermediate Harmonics (Higher Spatial Frequencies)}",
                RIGHT * 3.4 + DOWN * 0.7,
                JEWEL_GREEN,
                "full"
            ),
            (
                r"\lambda_N = 3.80",
                r"\text{Maximum Frequency (Peak Local Edge Contrast)}",
                RIGHT * 3.4 + DOWN * 2.1,
                JEWEL_CORAL,
                "high"
            ),
        ]

        spectral_rays = VGroup()
        spectral_labels = VGroup()

        for code_tex, desc_tex, target_pos, col, wave_mode in tier_specs:
            ray_end = target_pos + LEFT * 2.2
            rg = Line(exit_pt, ray_end, color=col, stroke_width=6.0, stroke_opacity=0.25)
            rc = Line(exit_pt, ray_end, color=col, stroke_width=2.2, stroke_opacity=0.95)
            spectral_rays.add(VGroup(rg, rc))

            t_math = CMTex(code_tex, fontsize=22, height=0.26, color=col)
            t_desc = CMTex(desc_tex, fontsize=17, height=0.21, color=TEXT_BRIGHT)
            t_group = VGroup(t_math, t_desc).arrange(DOWN, aligned_edge=LEFT, buff=0.08).move_to(target_pos)

            wave_c = target_pos + RIGHT * 2.2
            if wave_mode == "dc":
                wave_glyph = Line(wave_c + LEFT * 0.35, wave_c + RIGHT * 0.35, color=col, stroke_width=2.4)
            elif wave_mode == "half":
                pts = [wave_c + np.array([t, 0.16 * np.sin((t + 0.35) * (np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 30)]
                wave_glyph = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=2.8)
            elif wave_mode == "full":
                pts = [wave_c + np.array([t, 0.16 * np.sin((t + 0.35) * (2 * np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 40)]
                wave_glyph = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=2.2)
            else:
                pts = [wave_c + np.array([t, 0.14 * np.sin((t + 0.35) * (6 * np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 50)]
                wave_glyph = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=2.0)

            spectral_labels.add(VGroup(t_group, wave_glyph))

        self.play(
            LaggedStart(*[ShowCreation(r) for r in spectral_rays], lag_ratio=0.10),
            LaggedStart(*[FadeIn(lbl, LEFT * 0.15) for lbl in spectral_labels], lag_ratio=0.10),
            run_time=2.2,
            rate_func=smooth
        )
        self.wait(1.0)

        # -------------------------------------------------------------
        # CLIMAX: Radiant Spotlight on λ_2 (The Fiedler Breakthrough)
        # -------------------------------------------------------------
        fiedler_ray = spectral_rays[1]
        fiedler_lbl = spectral_labels[1]

        spectrum_banner = CMTex(
            r"\text{The Spectrum: Decomposing Chessboard Geometry into Spatial Frequencies}",
            fontsize=22,
            height=0.32,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.30)

        fiedler_glow_pulse = Line(exit_pt, fiedler_ray[1].get_end(), color=JEWEL_GOLD, stroke_width=14.0, stroke_opacity=0.45)

        self.play(
            FadeIn(spectrum_banner, UP * 0.1),
            fiedler_ray[0].animate.set_stroke(width=12.0, opacity=0.55),
            fiedler_ray[1].animate.set_stroke(width=4.0, color="#ffffff"),
            fiedler_lbl[0][0].animate.scale(1.15).set_color("#ffffff"),
            FadeIn(fiedler_glow_pulse),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(2.0)

        # Settle fiedler glow into stable radiant gold
        self.play(
            fiedler_glow_pulse.animate.set_stroke(width=8.0, opacity=0.30),
            fiedler_ray[1].animate.set_stroke(width=3.2, color=JEWEL_GOLD),
            fiedler_lbl[0][0].animate.set_color(JEWEL_GOLD),
            run_time=0.8
        )

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 03 Handoff)
        # Holds the prism dispersion and highlighted Fiedler frequency
        # =============================================================
        self.wait(2.5)
