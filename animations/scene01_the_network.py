"""
Heaven's Gate Documentary — Scene 01: The Tactical Network (Editorial Short Film)
Runtime: ~72 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Muted Editorial Linear Algebra (Charcoal, Slate Blue, Warm Ochre, Terracotta, Sage)

Beat 1: The Historical Artifact // Miroslav Fiedler (1973) Paper & Warm Ochre Sweep
Beat 2: Sovereign Crest Reveal // Illuminated Gold Vector Crest & Title
Beat 3: 3D Perspective Board // Cburnett Vector Pieces & Refined Tactical Vectors
Beat 4: Parallax Dissolve to Floating Graph & Adjacency Matrix (A) Synchronous Flare
Beat 5: Degree Matrix (D) & Graph Laplacian L = D - A with Muted Sage Row-Sum Scan
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from theme import *


class Scene01TheNetwork(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # LAYER 0: REFINED CHARCOAL BLUEPRINT DRAFTING MAT
        # -------------------------------------------------------------
        bg = FullScreenRectangle(fill_color=BG_COLOR, fill_opacity=1.0).set_stroke(width=0)
        self.add(bg)

        tech_grid = NumberPlane(
            x_range=[-14, 14, 1],
            y_range=[-9, 9, 1],
            width=28,
            height=18,
            axis_config={
                "stroke_color": CARD_BORDER,
                "stroke_width": 0.6,
                "stroke_opacity": 0.35,
            },
            background_line_style={
                "stroke_color": GRID_LINE,
                "stroke_width": 0.5,
                "stroke_opacity": 0.25,
            },
            faded_line_style={
                "stroke_color": GRID_FADED,
                "stroke_width": 0.3,
                "stroke_opacity": 0.15,
            }
        )
        self.add(tech_grid)

        # -------------------------------------------------------------
        # BEAT 1: THE HISTORICAL ARTIFACT (0s - 13s)
        # -------------------------------------------------------------
        paper_path = Path("assets/fiedler_user_paper.png")
        if not paper_path.exists():
            paper_path = Path("c:/Users/abhin/heavensgate/assets/fiedler_user_paper.png")

        paper_img = ImageMobject(str(paper_path))
        paper_img.set_height(5.5)
        paper_img.move_to(ORIGIN + UP * 0.1)

        # Soft Matte Drop Shadow
        paper_shadow = RoundedRectangle(
            width=paper_img.get_width() + 0.28,
            height=paper_img.get_height() + 0.28,
            corner_radius=0.08,
            fill_color="#040609",
            fill_opacity=0.80,
            stroke_width=0
        ).move_to(paper_img.get_center() + DOWN * 0.16 + RIGHT * 0.16)

        paper_border = RoundedRectangle(
            width=paper_img.get_width() + 0.04,
            height=paper_img.get_height() + 0.04,
            corner_radius=0.04,
            fill_opacity=0.0,
            stroke_color="#364356",
            stroke_width=1.4
        ).move_to(paper_img.get_center())

        # Muted Archival Accession Badge
        badge_box = RoundedRectangle(
            width=8.6, height=0.48, corner_radius=0.06,
            fill_color=SURFACE_COLOR, fill_opacity=0.96,
            stroke_color=COLOR_GOLD_MUTED, stroke_width=1.2
        ).move_to(UP * 3.25)

        badge_txt = Text("HISTORICAL ARTIFACT // MIROSLAV FIEDLER (1973)", font="Bahnschrift", color=COLOR_GOLD).scale(0.23)
        badge_txt.move_to(badge_box.get_center())
        archival_badge = VGroup(badge_box, badge_txt)

        self.play(
            FadeIn(paper_shadow, UP * 0.25),
            FadeIn(paper_border, UP * 0.25),
            FadeIn(paper_img, UP * 0.25),
            FadeIn(archival_badge, UP * 0.15),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(1.0)

        # Muted Ochre Highlighter Sweep
        hl_glow = RoundedRectangle(
            width=0.1, height=0.52, corner_radius=0.05,
            fill_color=COLOR_GOLD_MUTED, fill_opacity=0.20, stroke_width=0
        ).move_to(paper_img.get_center() + UP * 1.34 + LEFT * 2.8)

        hl_core = RoundedRectangle(
            width=0.1, height=0.42, corner_radius=0.04,
            fill_color=COLOR_GOLD, fill_opacity=0.32,
            stroke_color=COLOR_GOLD_LIGHT, stroke_width=1.2
        ).move_to(paper_img.get_center() + UP * 1.34 + LEFT * 2.8)

        # Bottom Sub-Tag
        leader_box = RoundedRectangle(
            width=7.4, height=0.54, corner_radius=0.06,
            fill_color=SURFACE_COLOR, fill_opacity=0.96,
            stroke_color=COLOR_CYAN_MUTED, stroke_width=1.2
        ).move_to(DOWN * 3.1)

        leader_txt = Text("DISCRETE SPECTRAL GRAPH THEORY // FOUNDATION", font="Bahnschrift", color=COLOR_CYAN_LIGHT).scale(0.23)
        leader_txt.move_to(leader_box.get_center())
        leader_tag = VGroup(leader_box, leader_txt)

        self.play(
            hl_glow.animate.stretch_to_fit_width(5.8).move_to(paper_img.get_center() + UP * 1.34),
            hl_core.animate.stretch_to_fit_width(5.7).move_to(paper_img.get_center() + UP * 1.34),
            FadeIn(leader_tag, UP * 0.12),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(2.0)

        # -------------------------------------------------------------
        # BEAT 2: SOVEREIGN CREST REVEAL (13s - 22s)
        # -------------------------------------------------------------
        logo_path = Path("assets/heavensgate_icon.svg")
        if not logo_path.exists():
            logo_path = Path("c:/Users/abhin/heavensgate/assets/heavensgate_icon.svg")

        logo_mobj = SVGMobject(str(logo_path)).scale(1.05) if logo_path.exists() else Text("HG", font="Bahnschrift", color=COLOR_GOLD).scale(1.4)
        for sub in logo_mobj.family_members_with_points():
            sub.set_stroke(color=COLOR_GOLD, width=1.6)

        logo_mobj.move_to(ORIGIN + UP * 0.7)

        title_main = Text("HEAVEN'S GATE", font="Bahnschrift", color=TEXT_WHITE).scale(0.74)
        title_sub  = Text("A Classical Chess Sovereign Powered by Discrete Spectral Geometry", font="Bahnschrift", color=COLOR_CYAN_LIGHT).scale(0.27)
        title_meta = Text("40M NPS MOVEGEN  -  ZERO NEURAL WEIGHTS  -  PURE LINEAR ALGEBRA", font="Consolas", color=COLOR_GOLD).scale(0.20)

        title_group = VGroup(title_main, title_sub, title_meta).arrange(DOWN, buff=0.12)
        title_group.next_to(logo_mobj, DOWN, buff=0.22)
        hero_group = VGroup(logo_mobj, title_group)

        # Muted Expanding Shockwaves
        ring_steel = Circle(radius=0.4, color=COLOR_CYAN, stroke_width=2.5, stroke_opacity=0.7).move_to(logo_mobj.get_center())
        ring_ochre = Circle(radius=0.6, color=COLOR_GOLD, stroke_width=1.8, stroke_opacity=0.6).move_to(logo_mobj.get_center())

        self.play(
            FadeOut(paper_shadow),
            FadeOut(paper_border),
            FadeOut(paper_img),
            FadeOut(archival_badge),
            FadeOut(hl_glow),
            FadeOut(hl_core),
            FadeOut(leader_tag),
            run_time=0.9
        )

        self.play(
            FadeIn(hero_group, scale=0.90),
            ring_steel.animate.scale(6.0).set_opacity(0),
            ring_ochre.animate.scale(4.8).set_opacity(0),
            run_time=2.2,
            rate_func=smooth
        )
        self.wait(2.0)

        self.play(
            FadeOut(hero_group, UP * 0.30),
            run_time=1.0
        )

        # -------------------------------------------------------------
        # BEAT 3: 3D PERSPECTIVE TOURNAMENT CHESSBOARD (22s - 44s)
        # -------------------------------------------------------------
        self.camera.frame.set_euler_angles(theta=-14 * DEGREES, phi=20 * DEGREES)

        # Definition HUD Card at the top
        def_card = RoundedRectangle(
            width=10.4, height=1.08, corner_radius=0.08,
            fill_color=SURFACE_COLOR, fill_opacity=0.94,
            stroke_color=CARD_BORDER, stroke_width=1.2
        ).move_to(UP * 3.3)

        def_t1 = Text("WHAT IS A SPECTRAL GRAPH?", font="Bahnschrift", color=COLOR_GOLD).scale(0.23)
        def_t2 = Text("A graph is simply two things: nodes and edges.", font="Bahnschrift", color=TEXT_WHITE).scale(0.34)
        def_t3 = Text("Nodes = Pieces on the board   -   Edges = Tactical lines of connection", font="Bahnschrift", color=COLOR_CYAN_LIGHT).scale(0.24)
        def_text = VGroup(def_t1, def_t2, def_t3).arrange(DOWN, buff=0.07).move_to(def_card.get_center())
        def_header = VGroup(def_card, def_text)

        # Broadcast Tournament Board (Muted Palette)
        board = BroadcastChessBoard(
            center=ORIGIN + DOWN * 0.2,
            sq_size=0.62,
            show_coords=True
        )

        # Cburnett SVG Pieces
        p_qd4  = board.create_piece("wQ", 3, 3) # d4 Queen
        p_bc4  = board.create_piece("wB", 2, 3) # c4 Bishop
        p_ne5  = board.create_piece("wN", 4, 4) # e5 Knight
        p_rd1  = board.create_piece("wR", 3, 0) # d1 Rook
        p_kg1  = board.create_piece("wK", 6, 0) # g1 King
        p_bkg8 = board.create_piece("bK", 6, 7) # g8 Black King
        p_be7  = board.create_piece("bB", 4, 6) # e7 Bishop
        p_bnc6 = board.create_piece("bN", 2, 5) # c6 Black Knight

        pieces = VGroup(p_qd4, p_bc4, p_ne5, p_rd1, p_kg1, p_bkg8, p_be7, p_bnc6)

        self.play(
            FadeIn(def_header, UP * 0.15),
            FadeIn(board, scale=0.94),
            LaggedStart(*[FadeIn(p, scale=0.85) for p in pieces], lag_ratio=0.08),
            run_time=2.2,
            rate_func=smooth
        )
        self.wait(1.0)

        # Muted Tactical Vectors (Thin core, gentle halo)
        def make_tactical_edge(p1, p2, color, core_w=2.2, glow_w=5.5):
            glow = Line(p1, p2, color=color, stroke_width=glow_w, stroke_opacity=0.25)
            core = Line(p1, p2, color=color, stroke_width=core_w, stroke_opacity=0.90)
            return VGroup(glow, core)

        line_q_b = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(2, 3), COLOR_CYAN)
        line_q_n = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(4, 4), COLOR_CYAN)
        line_q_r = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(3, 0), COLOR_CYAN)
        line_q_k = make_tactical_edge(board.get_square_pos(3, 3), board.get_square_pos(6, 7), COLOR_RED)
        line_n_c = make_tactical_edge(board.get_square_pos(4, 4), board.get_square_pos(2, 5), COLOR_RED)
        line_b_e = make_tactical_edge(board.get_square_pos(2, 3), board.get_square_pos(4, 6), COLOR_GOLD)

        tactical_edges = VGroup(line_q_b, line_q_n, line_q_r, line_q_k, line_n_c, line_b_e)

        self.play(
            LaggedStart(*[ShowCreation(e) for e in tactical_edges], lag_ratio=0.16),
            run_time=2.4
        )
        self.wait(1.5)

        # PARALLAX GRAPH ABSTRACTION: Board tiles dissolve to reveal pure 3D network
        self.play(
            board.squares_group.animate.set_opacity(0.08),
            board.outer_frame.animate.set_opacity(0.15),
            board.inner_border.animate.set_opacity(0.10),
            board.coords_group.animate.set_opacity(0.18),
            self.camera.frame.animate.reorient(theta_degrees=10, phi_degrees=16, center=ORIGIN + DOWN * 0.1),
            run_time=2.6,
            rate_func=smooth
        )
        self.wait(1.2)

        # -------------------------------------------------------------
        # BEAT 4: THE ADJACENCY MATRIX COLLAPSE (44s - 60s)
        # -------------------------------------------------------------
        graph_network = VGroup(board, pieces, tactical_edges)

        self.play(
            FadeOut(def_header),
            graph_network.animate.scale(0.78).move_to(LEFT * 4.0 + DOWN * 0.15),
            self.camera.frame.animate.reorient(theta_degrees=0, phi_degrees=0, center=ORIGIN),
            run_time=2.0,
            rate_func=smooth
        )

        # Right Side Glass Matrix Card
        mat_card = RoundedRectangle(
            width=6.8, height=5.8, corner_radius=0.10,
            fill_color=SURFACE_COLOR, fill_opacity=0.95,
            stroke_color=CARD_BORDER, stroke_width=1.2
        ).move_to(RIGHT * 3.6 + DOWN * 0.15)

        adj_header = Text("1. THE ADJACENCY MATRIX (A)", font="Bahnschrift", color=COLOR_CYAN_LIGHT).scale(0.34)
        adj_header.move_to(mat_card.get_top() + DOWN * 0.42)

        adj_desc = Text("Encodes all pairwise tactical connections", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        adj_desc.next_to(adj_header, DOWN, buff=0.07)

        mat_dim = 6
        cell_size = 0.54
        grid_center = mat_card.get_center() + DOWN * 0.22

        A_vals = [
            [0, 1, 1, 1, 1, 0],
            [1, 0, 0, 1, 0, 0],
            [1, 0, 0, 0, 1, 1],
            [1, 1, 0, 0, 0, 0],
            [1, 0, 1, 0, 0, 0],
            [0, 0, 1, 0, 0, 0],
        ]

        # Piece labels on top & left
        piece_labels_txt = ["Q", "B", "N", "R", "K", "k"]
        row_labels = VGroup()
        col_labels = VGroup()

        for idx, lbl in enumerate(piece_labels_txt):
            # Column headers (top)
            cx = grid_center[0] + (idx - (mat_dim - 1) / 2) * cell_size
            cy = grid_center[1] + ((mat_dim - 1) / 2 + 0.62) * cell_size
            t_col = Text(lbl, font="Consolas", color=COLOR_GOLD).scale(0.22).move_to(np.array([cx, cy, 0]))
            col_labels.add(t_col)

            # Row headers (left)
            rx = grid_center[0] - ((mat_dim - 1) / 2 + 0.68) * cell_size
            ry = grid_center[1] + ((mat_dim - 1) / 2 - idx) * cell_size
            t_row = Text(lbl, font="Consolas", color=COLOR_GOLD).scale(0.22).move_to(np.array([rx, ry, 0]))
            row_labels.add(t_row)

        matrix_tiles = VGroup()
        matrix_values = VGroup()

        for i in range(mat_dim):
            for j in range(mat_dim):
                v = A_vals[i][j]
                c_pos = grid_center + np.array([(j - (mat_dim - 1) / 2) * cell_size, ((mat_dim - 1) / 2 - i) * cell_size, 0])

                tile = Square(side_length=cell_size * 0.90)
                tile.move_to(c_pos)
                tile.set_fill("#182130" if v > 0 else "#0e131d", opacity=0.90)
                tile.set_stroke(color=CARD_BORDER, width=0.6)
                matrix_tiles.add(tile)

                col = COLOR_GOLD if v > 0 else TEXT_DIM
                txt = Text(str(v), font="Consolas", color=col).scale(0.24)
                txt.move_to(c_pos)
                matrix_values.add(txt)

        # Refined Architectural Brackets
        hw = mat_dim * cell_size / 2 + 0.12
        hh = mat_dim * cell_size / 2 + 0.12

        bracket_l = VMobject().set_points_as_corners([
            grid_center + np.array([-hw + 0.14, hh, 0]),
            grid_center + np.array([-hw, hh, 0]),
            grid_center + np.array([-hw, -hh, 0]),
            grid_center + np.array([-hw + 0.14, -hh, 0]),
        ]).set_stroke(color=TEXT_MUTED, width=1.6)

        bracket_r = VMobject().set_points_as_corners([
            grid_center + np.array([hw - 0.14, hh, 0]),
            grid_center + np.array([hw, hh, 0]),
            grid_center + np.array([hw, -hh, 0]),
            grid_center + np.array([hw - 0.14, -hh, 0]),
        ]).set_stroke(color=TEXT_MUTED, width=1.6)

        brackets = VGroup(bracket_l, bracket_r)

        adj_rule = Text("A[i, j] > 0 if pieces connect   -   0 otherwise", font="Consolas", color=COLOR_CYAN_LIGHT).scale(0.20)
        adj_rule.move_to(mat_card.get_bottom() + UP * 0.36)

        self.play(
            FadeIn(mat_card, RIGHT * 0.15),
            FadeIn(adj_header, UP * 0.08),
            FadeIn(adj_desc, UP * 0.08),
            ShowCreation(brackets),
            FadeIn(row_labels, lag_ratio=0.02),
            FadeIn(col_labels, lag_ratio=0.02),
            LaggedStart(*[FadeIn(t, scale=0.9) for t in matrix_tiles], lag_ratio=0.012),
            LaggedStart(*[FadeIn(v, scale=0.9) for v in matrix_values], lag_ratio=0.012),
            FadeIn(adj_rule, UP * 0.08),
            run_time=2.2
        )
        self.wait(1.8)

        # SYNCHRONOUS TACTICAL PULSE: Queen-Knight edge and A[Q, N] pulse gently in soft white
        idx_qn_1 = 0 * mat_dim + 2 # (Q, N)
        idx_qn_2 = 2 * mat_dim + 0 # (N, Q)

        pulse_tiles = VGroup(matrix_tiles[idx_qn_1], matrix_tiles[idx_qn_2])
        pulse_vals  = VGroup(matrix_values[idx_qn_1], matrix_values[idx_qn_2])

        self.play(
            line_q_n[1].animate.set_stroke(color=TEXT_WHITE, width=4.5),
            line_q_n[0].animate.set_stroke(color=COLOR_CYAN, width=9.0, opacity=0.5),
            pulse_tiles.animate.set_fill(COLOR_CYAN_MUTED, opacity=0.85),
            pulse_vals.animate.set_color(TEXT_WHITE).scale(1.18),
            run_time=0.8
        )
        self.play(
            line_q_n[1].animate.set_stroke(color=COLOR_CYAN, width=2.2),
            line_q_n[0].animate.set_stroke(color=COLOR_CYAN, width=5.5, opacity=0.25),
            pulse_tiles.animate.set_fill("#182130", opacity=0.90),
            pulse_vals.animate.set_color(COLOR_GOLD).scale(1 / 1.18),
            run_time=0.8
        )
        self.wait(1.8)

        # -------------------------------------------------------------
        # BEAT 5: DEGREE MATRIX (D) & GRAPH LAPLACIAN (L = D - A) (60s - 72s)
        # -------------------------------------------------------------
        deg_header = Text("2. THE DEGREE MATRIX (D)", font="Bahnschrift", color=COLOR_VIOLET_LIGHT).scale(0.34)
        deg_header.move_to(adj_header.get_center())

        deg_desc = Text("Diagonal records piece integration: D[i, i] = sum(A[i, :])", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        deg_desc.move_to(adj_desc.get_center())

        D_diag = [4, 2, 3, 2, 2, 1]
        deg_values = VGroup()

        for i in range(mat_dim):
            for j in range(mat_dim):
                c_pos = grid_center + np.array([(j - (mat_dim - 1) / 2) * cell_size, ((mat_dim - 1) / 2 - i) * cell_size, 0])
                if i == j:
                    col = COLOR_VIOLET_LIGHT
                    val_str = str(D_diag[i])
                else:
                    col = TEXT_DIM
                    val_str = "0"
                txt = Text(val_str, font="Consolas", color=col).scale(0.24)
                txt.move_to(c_pos)
                deg_values.add(txt)

        deg_rule = Text("Queen: Degree 4 (Central)   -   Corner: Degree 1 (Bottleneck)", font="Consolas", color=COLOR_VIOLET_LIGHT).scale(0.19)
        deg_rule.move_to(adj_rule.get_center())

        diag_tiles = VGroup(*[matrix_tiles[i * mat_dim + i] for i in range(mat_dim)])

        # Sequential transition 1: Adjacency -> Degree
        self.play(
            FadeOut(adj_header, UP * 0.06),
            FadeOut(adj_desc, UP * 0.06),
            FadeOut(adj_rule, UP * 0.04),
            FadeOut(matrix_values),
            run_time=0.4
        )
        self.play(
            FadeIn(deg_header, UP * 0.06),
            FadeIn(deg_desc, UP * 0.06),
            FadeIn(deg_rule, UP * 0.04),
            FadeIn(deg_values),
            diag_tiles.animate.set_fill(COLOR_VIOLET, opacity=0.35),
            run_time=0.7
        )
        self.wait(2.5)

        # Morph to Graph Laplacian: L = D - A (Standard ASCII hyphen)
        lap_header = Text("3. THE GRAPH LAPLACIAN:  L = D - A", font="Bahnschrift", color=COLOR_GOLD).scale(0.34)
        lap_header.move_to(adj_header.get_center())

        lap_desc = Text("Net tactical tension between each piece and its neighbors", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        lap_desc.move_to(adj_desc.get_center())

        lap_values = VGroup()
        for i in range(mat_dim):
            for j in range(mat_dim):
                c_pos = grid_center + np.array([(j - (mat_dim - 1) / 2) * cell_size, ((mat_dim - 1) / 2 - i) * cell_size, 0])
                if i == j:
                    val_str = str(D_diag[i])
                    col = COLOR_GOLD
                else:
                    a_val = A_vals[i][j]
                    val_str = "-1" if a_val > 0 else "0"
                    col = COLOR_CYAN_LIGHT if a_val > 0 else TEXT_DIM
                txt = Text(val_str, font="Consolas", color=col).scale(0.23)
                txt.move_to(c_pos)
                lap_values.add(txt)

        lap_rule = Text("∑ L[i, j] = 0  (Every row sum is identically zero)", font="Consolas", color=COLOR_GREEN_LIGHT).scale(0.20)
        lap_rule.move_to(adj_rule.get_center())

        # Sequential transition 2: Degree -> Laplacian
        self.play(
            FadeOut(deg_header, UP * 0.06),
            FadeOut(deg_desc, UP * 0.06),
            FadeOut(deg_rule, UP * 0.04),
            FadeOut(deg_values),
            run_time=0.4
        )
        self.play(
            FadeIn(lap_header, UP * 0.06),
            FadeIn(lap_desc, UP * 0.06),
            FadeIn(lap_rule, UP * 0.04),
            FadeIn(lap_values),
            diag_tiles.animate.set_fill(COLOR_GOLD_MUTED, opacity=0.30),
            run_time=0.7
        )
        self.wait(1.0)

        # Muted Sage Green Laser Scan
        row_scan = Line(
            grid_center + np.array([-hw + 0.12, ((mat_dim - 1) / 2) * cell_size, 0]),
            grid_center + np.array([hw - 0.12, ((mat_dim - 1) / 2) * cell_size, 0]),
            color=COLOR_GREEN, stroke_width=2.4
        )
        scan_tag = Text("∑ Row = 0", font="Consolas", color=COLOR_GREEN_LIGHT).scale(0.20)
        scan_tag.next_to(row_scan, RIGHT, buff=0.14)

        self.play(ShowCreation(row_scan), FadeIn(scan_tag), run_time=0.4)
        self.play(
            row_scan.animate.shift(DOWN * (mat_dim - 1) * cell_size),
            scan_tag.animate.shift(DOWN * (mat_dim - 1) * cell_size),
            run_time=2.2,
            rate_func=linear
        )
        self.play(FadeOut(row_scan), FadeOut(scan_tag), run_time=0.3)
        self.wait(2.5)

        # -------------------------------------------------------------
        # OUTRO DISSOLVE
        # -------------------------------------------------------------
        self.play(
            FadeOut(graph_network),
            FadeOut(mat_card),
            FadeOut(lap_header),
            FadeOut(lap_desc),
            FadeOut(matrix_tiles),
            FadeOut(lap_values),
            FadeOut(brackets),
            FadeOut(row_labels),
            FadeOut(col_labels),
            FadeOut(lap_rule),
            run_time=1.8
        )
        self.wait(0.6)
