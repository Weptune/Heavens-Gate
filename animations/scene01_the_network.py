"""
Heaven's Gate Documentary - Scene 01: The Tactical Network
Craft Standard: 3Blue1Brown / vcubingx / Reducible Master Broadcast Standard
Runtime: ~72 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pitch Black Canvas (#000000), Luminous Jewel Accents (Amethyst, Cyber Teal, Neon Sage, Electric Gold, Coral Crimson)
Mathematical Engine: Computer Modern LaTeX Vectors (CMTex) + Dynamic Technical HeatMatrix + 3D Kinetic Cinematography
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from theme import *
from cm_math import CMTex, HeatMatrix


class Scene01TheNetwork(Scene):
    CONFIG = {
        "camera_config": {
            "background_color": "#000000",
        }
    }

    def construct(self):
        # =============================================================
        # LAYER 0: TECHNICAL OBSIDIAN CANVAS & DRAFTING MAT
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
        # Start in frontal reading view for the historical document
        frame.set_euler_angles(theta=0, phi=0)

        # =============================================================
        # BEAT 1: THE HISTORICAL DISCOVERY (0s - 14s)
        # =============================================================
        paper_path = Path("assets/fiedler_user_paper.png")
        if not paper_path.exists():
            paper_path = Path("c:/Users/abhin/heavensgate/assets/fiedler_user_paper.png")

        paper_img = ImageMobject(str(paper_path))
        paper_img.set_height(5.2)
        paper_img.move_to(ORIGIN + UP * 0.15)

        paper_border = RoundedRectangle(
            width=paper_img.get_width() + 0.04,
            height=paper_img.get_height() + 0.04,
            corner_radius=0.04,
            fill_opacity=0.0,
            stroke_color="#2b3b55",
            stroke_width=1.5
        ).move_to(paper_img.get_center())

        # Authentic Computer Modern Typography Citation
        citation_cm = CMTex(
            r"\text{Miroslav Fiedler } (1973) \cdot \text{Algebraic Connectivity of Graphs}",
            fontsize=22,
            height=0.30,
            color=JEWEL_LAVENDER
        ).move_to(DOWN * 3.25)

        self.play(
            FadeIn(paper_border, UP * 0.2),
            FadeIn(paper_img, UP * 0.2),
            FadeIn(citation_cm, UP * 0.15),
            run_time=1.6,
            rate_func=smooth
        )
        self.wait(0.8)

        # Precision Technical Underline aligned to paper scan physical slant (-0.91 degrees)
        line_start = np.array([-1.22, 1.380, 0.05])
        line_end   = np.array([1.62, 1.316, 0.05])

        underline_glow = Line(line_start, line_end, stroke_color=JEWEL_GOLD, stroke_width=4.5, stroke_opacity=0.35)
        underline_core = Line(line_start, line_end, stroke_color=JEWEL_GOLD, stroke_width=2.0, stroke_opacity=1.0)

        # Perpendicular ticks exactly normal to the slanted line
        v_line = line_end - line_start
        v_norm = np.linalg.norm(v_line[:2])
        perp = np.array([-v_line[1], v_line[0], 0]) / v_norm

        tick_l = Line(line_start + perp * 0.07, line_start - perp * 0.07, stroke_color=JEWEL_GOLD, stroke_width=2.0)
        tick_r = Line(line_end + perp * 0.07, line_end - perp * 0.07, stroke_color=JEWEL_GOLD, stroke_width=2.0)
        title_ticks = VGroup(tick_l, tick_r)

        # Traveling light spark
        spark = Dot(point=line_start, radius=0.06, color=TEXT_WHITE)
        spark_glow = Dot(point=line_start, radius=0.16, color=JEWEL_GOLD, fill_opacity=0.5)
        spark_group = VGroup(spark_glow, spark)

        self.play(
            ShowCreation(underline_glow),
            ShowCreation(underline_core),
            spark_group.animate.move_to(line_end),
            run_time=1.4,
            rate_func=smooth
        )
        self.play(
            FadeIn(title_ticks, scale=0.8),
            FadeOut(spark_group),
            run_time=0.4
        )
        self.wait(1.4)

        # =============================================================
        # BEAT 2: THE TACTICAL CHESS NETWORK & 3D PERSPECTIVE GLIDE (14s - 32s)
        # Upright, cardinal board entry followed by a purposeful camera glide
        # =============================================================
        title_underline_group = VGroup(underline_glow, underline_core, title_ticks)

        self.play(
            FadeOut(paper_border),
            FadeOut(paper_img),
            FadeOut(citation_cm),
            FadeOut(title_underline_group),
            run_time=1.2,
            rate_func=smooth
        )

        board = BroadcastChessBoard(
            center=ORIGIN + DOWN * 0.25,
            sq_size=0.62,
            light_color="#bcc7d6",
            dark_color="#2b384c",
            show_coords=True
        )

        # Tactical piece configuration
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

        # Computer Modern Graph Definition Formula
        graph_def = CMTex(r"G = (V, E)", fontsize=36, height=0.48, color=TEXT_WHITE).move_to(UP * 3.15)
        graph_sub = Text("V : Pieces on board    •    E : Tactical lines of force", font="Consolas", font_size=16, color=JEWEL_CYAN)
        graph_sub.next_to(graph_def, DOWN, buff=0.15)
        graph_header = VGroup(graph_def, graph_sub)

        # Sleek, professional graph edge generator (unified cyber-teal & subtle coral tension)
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

        self.play(
            FadeIn(board, scale=0.95),
            FadeIn(graph_header, UP * 0.2),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(0.3)

        # Illuminate tactical rays and pieces (clean, crisp entry with zero ugly circles)
        self.play(
            LaggedStartMap(ShowCreation, tactical_rays, lag_ratio=0.10),
            LaggedStartMap(FadeIn, pieces, scale=0.88, lag_ratio=0.08),
            run_time=1.6,
            rate_func=smooth
        )
        self.bring_to_front(pieces)
        self.wait(1.0)

        # =============================================================
        # 3D PERSPECTIVE GLIDE & GHOST GRID
        # Board squares fade into translucent ghost lines while pieces and rays
        # remain strictly coplanar, maintaining 100% dead-center square alignment!
        # =============================================================
        self.play(
            board.squares_group.animate.set_opacity(0.12),
            board.outer_frame.animate.set_opacity(0.18),
            board.inner_border.animate.set_opacity(0.15),
            board.coords_group.animate.set_opacity(0.15),
            frame.animate.reorient(theta_degrees=0, phi_degrees=22, center=ORIGIN + DOWN * 0.25),
            run_time=2.2,
            rate_func=smooth
        )

        # Continuous fluid tracking glide (board and pieces stay perfectly locked together)
        self.play(
            frame.animate.reorient(theta_degrees=3, phi_degrees=18, center=ORIGIN + DOWN * 0.20),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(0.6)

        # =============================================================
        # BEAT 3: THE ADJACENCY MATRIX A & LIVE LASER SPARKS (32s - 50s)
        # =============================================================
        graph_group = VGroup(board, tactical_rays, pieces)

        # Smooth glide: camera pulls back to frontal 2D view, graph shifts to left
        self.play(
            FadeOut(graph_header),
            graph_group.animate.scale(0.74).move_to(LEFT * 4.1 + DOWN * 0.15),
            frame.animate.reorient(theta_degrees=0, phi_degrees=0, center=ORIGIN),
            run_time=2.0,
            rate_func=smooth
        )

        mat_center = RIGHT * 3.6 + DOWN * 0.15
        A_data = [
            [0, 1, 1, 1, 1, 0],
            [1, 0, 0, 1, 0, 0],
            [1, 0, 0, 0, 1, 1],
            [1, 1, 0, 0, 0, 0],
            [1, 0, 1, 0, 0, 0],
            [0, 0, 1, 0, 0, 0],
        ]

        matrix_A = HeatMatrix(
            A_data,
            cell_size=(0.60, 0.52),
            h_buff=0.08,
            v_buff=0.08,
            font_size=18,
            bracket_color=JEWEL_CYAN,
            bracket_width=2.5
        ).move_to(mat_center)

        # Cleanly centered Computer Modern title
        title_A = CMTex(r"A \quad (\text{Adjacency Matrix})", fontsize=28, height=0.38, color=JEWEL_CYAN)
        title_A.move_to(mat_center + UP * 2.30)

        # Piece labels (Q, B, N, R, K, k) for columns and rows
        labels_txt = ["Q", "B", "N", "R", "K", "k"]
        row_headers = VGroup()
        col_headers = VGroup()
        for idx, lbl in enumerate(labels_txt):
            rh = Text(lbl, font="Consolas", font_size=16, color=JEWEL_LAVENDER)
            rh.next_to(matrix_A.get_cell_bg(idx, 0), LEFT, buff=0.28)
            row_headers.add(rh)
            ch = Text(lbl, font="Consolas", font_size=16, color=JEWEL_LAVENDER)
            ch.next_to(matrix_A.get_cell_bg(0, idx), UP, buff=0.22)
            col_headers.add(ch)

        rule_A = CMTex(r"A_{ij} = 1 \text{ if connected, } 0 \text{ otherwise}", fontsize=22, height=0.32, color=TEXT_MUTED)
        rule_A.move_to(mat_center + DOWN * 2.32)

        self.play(
            FadeIn(title_A, UP * 0.15),
            FadeIn(matrix_A.left_bracket),
            FadeIn(matrix_A.right_bracket),
            LaggedStartMap(FadeIn, row_headers, lag_ratio=0.04),
            LaggedStartMap(FadeIn, col_headers, lag_ratio=0.04),
            LaggedStartMap(FadeIn, matrix_A.row_groups, lag_ratio=0.04),
            FadeIn(rule_A, UP * 0.1),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(1.2)

        # -------------------------------------------------------------
        # LIVING SIMULATION: Laser Sparks Shoot from Board into Matrix
        # Edge Q-N on board shoots glowing sparks simultaneously into A[Q, N] and A[N, Q]!
        # -------------------------------------------------------------
        q_pos = p_qd4.get_center()
        n_pos = p_ne5.get_center()

        target_cell_1 = matrix_A.get_cell_bg(0, 2).get_center()  # (Q, N)
        target_cell_2 = matrix_A.get_cell_bg(2, 0).get_center()  # (N, Q)

        spark_trajectory_1 = Line(q_pos, target_cell_1, path_arc=-40 * DEGREES).set_stroke(color=JEWEL_GOLD, width=6.0)
        spark_trajectory_2 = Line(n_pos, target_cell_2, path_arc=40 * DEGREES).set_stroke(color=JEWEL_GOLD, width=6.0)

        flash_1 = VShowPassingFlash(spark_trajectory_1, time_width=0.45)
        flash_2 = VShowPassingFlash(spark_trajectory_2, time_width=0.45)

        # Pulse the ray on the board first
        self.play(
            ray_q_n[1].animate.set_stroke(color=TEXT_WHITE, width=4.5),
            ray_q_n[0].animate.set_stroke(color=JEWEL_GOLD, width=10.0, opacity=0.75),
            run_time=0.6,
            rate_func=rush_into
        )

        # Shoot dual sparks across the screen into matrix cells A[0, 2] and A[2, 0]
        self.play(
            flash_1, flash_2,
            matrix_A.highlight_cell(0, 2, fill_color=JEWEL_CYAN, fill_opacity=0.70, text_color=TEXT_WHITE, stroke_color=JEWEL_GOLD),
            matrix_A.highlight_cell(2, 0, fill_color=JEWEL_CYAN, fill_opacity=0.70, text_color=TEXT_WHITE, stroke_color=JEWEL_GOLD),
            run_time=1.2,
            rate_func=smooth
        )
        self.wait(0.6)

        # Settle back to stable matrix colors
        self.play(
            ray_q_n[1].animate.set_stroke(color=JEWEL_CYAN, width=2.4),
            ray_q_n[0].animate.set_stroke(color=JEWEL_CYAN, width=6.0, opacity=0.30),
            matrix_A.highlight_cell(0, 2, fill_color="#182c44", fill_opacity=0.45, text_color=JEWEL_GOLD, stroke_color="#2b4766"),
            matrix_A.highlight_cell(2, 0, fill_color="#182c44", fill_opacity=0.45, text_color=JEWEL_GOLD, stroke_color="#2b4766"),
            run_time=0.8
        )
        self.wait(1.5)

        # =============================================================
        # BEAT 4: THE DEGREE MATRIX D (50s - 60s)
        # =============================================================
        title_D = CMTex(r"D \quad (\text{Degree Matrix } - \text{Total Connections})", fontsize=26, height=0.38, color=JEWEL_GOLD)
        title_D.move_to(mat_center + UP * 2.30)

        D_data = [
            [4, 0, 0, 0, 0, 0],
            [0, 2, 0, 0, 0, 0],
            [0, 0, 3, 0, 0, 0],
            [0, 0, 0, 2, 0, 0],
            [0, 0, 0, 0, 2, 0],
            [0, 0, 0, 0, 0, 1],
        ]

        matrix_D = HeatMatrix(
            D_data,
            cell_size=(0.60, 0.52),
            h_buff=0.08,
            v_buff=0.08,
            font_size=18,
            bracket_color=JEWEL_GOLD,
            bracket_width=2.5
        ).move_to(mat_center)

        # Highlight diagonal in Degree matrix
        for i in range(6):
            matrix_D.get_cell_bg(i, i).set_fill(JEWEL_GOLD, opacity=0.35).set_stroke(JEWEL_GOLD, 1.6)
            matrix_D.get_cell_text(i, i).set_color(JEWEL_GOLD)

        rule_D = CMTex(r"D_{ii} = \sum_{j} A_{ij} \quad (\text{Central Queen: } 4 \quad \cdot \quad \text{Periphery: } 1)", fontsize=22, height=0.34, color=JEWEL_GOLD)
        rule_D.move_to(mat_center + DOWN * 2.32)

        # Radial scanning pulse on the Queen node
        queen_pulse = Circle(radius=0.1, color=JEWEL_GOLD, stroke_width=4.0).move_to(q_pos)

        self.play(
            Transform(title_A, title_D),
            Transform(rule_A, rule_D),
            Transform(matrix_A, matrix_D),
            queen_pulse.animate.scale(7.0).set_stroke(width=0, opacity=0),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(2.2)

        # =============================================================
        # BEAT 5: THE GRAPH LAPLACIAN L = D - A & ROW-SUM ZERO (60s - 75s)
        # =============================================================
        title_L = CMTex(r"L = D - A \quad (\text{The Graph Laplacian})", fontsize=28, height=0.38, color=JEWEL_LAVENDER)
        title_L.move_to(mat_center + UP * 2.30)

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

        # Color the diagonal in Gold, off-diagonal negative weights in Coral Crimson
        for r in range(6):
            for c in range(6):
                if r == c:
                    matrix_L.get_cell_bg(r, c).set_fill(JEWEL_GOLD, opacity=0.30).set_stroke(JEWEL_GOLD, 1.4)
                    matrix_L.get_cell_text(r, c).set_color(JEWEL_GOLD)
                elif L_data[r][c] < 0:
                    matrix_L.get_cell_bg(r, c).set_fill(JEWEL_CORAL, opacity=0.25).set_stroke(JEWEL_CORAL, 1.0)
                    matrix_L.get_cell_text(r, c).set_color(JEWEL_CORAL)

        rule_L = CMTex(r"\sum_{j=1}^n L_{ij} = 0 \quad (\text{Every single row sum is identically zero})", fontsize=22, height=0.34, color=JEWEL_GREEN)
        rule_L.move_to(mat_center + DOWN * 2.32)

        self.play(
            Transform(title_A, title_L),
            Transform(rule_A, rule_L),
            Transform(matrix_A, matrix_L),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(1.0)

        # -------------------------------------------------------------
        # ROW-SUM ZERO SCAN: Scanning laser bar validates every row
        # -------------------------------------------------------------
        scan_width = matrix_L.get_width() - 0.20
        scan_bar = Line(LEFT * (scan_width / 2), RIGHT * (scan_width / 2), stroke_color=JEWEL_GREEN, stroke_width=3.0)
        scan_bar.move_to(matrix_L.get_cell_bg(0, 0).get_center() + RIGHT * (matrix_L.get_width() / 2 - 0.35))

        zero_tag = CMTex(r"= 0", fontsize=22, height=0.24, color=JEWEL_GREEN)
        zero_tag.next_to(matrix_L.right_bracket, RIGHT, buff=0.20).align_to(matrix_L.get_cell_bg(0, 0), UP)

        self.play(
            ShowCreation(scan_bar),
            FadeIn(zero_tag),
            run_time=0.4
        )

        # Walk smoothly down the rows verifying row sum = 0
        for r_idx in range(1, 6):
            target_y = matrix_L.get_cell_bg(r_idx, 0).get_center()[1]
            self.play(
                scan_bar.animate.set_y(target_y),
                zero_tag.animate.set_y(target_y),
                run_time=0.35,
                rate_func=linear
            )

        self.play(
            FadeOut(scan_bar),
            FadeOut(zero_tag),
            run_time=0.3
        )
        self.wait(1.0)

        # -------------------------------------------------------------
        # DISCRETE LAPLACE-BELTRAMI CURVATURE OPERATOR
        # -------------------------------------------------------------
        curvature_cm = CMTex(
            r"(L v)_i = \sum_{j} A_{ij}(v_i - v_j) \quad \text{(Laplace-Beltrami Operator)}",
            fontsize=26,
            height=0.36,
            color=TEXT_WHITE
        ).move_to(mat_center + DOWN * 2.32)

        self.play(
            Transform(rule_A, curvature_cm),
            run_time=1.0,
            rate_func=smooth
        )
        self.wait(3.0)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 02 Handoff)
        # Hold the final visual state: Graph Laplacian, Left Board, and Curvature Formula
        # =============================================================
        self.wait(2.0)
