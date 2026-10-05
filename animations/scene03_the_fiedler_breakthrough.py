"""
Heaven's Gate Documentary - Scene 03: The Fiedler Breakthrough
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical Linear Algebra & Luminous Jewel Palette

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 02's terminal frame:
  Tactical mini-chessboard, refractive optical prism, 4 dispersed spectral rays,
  radiant golden Fiedler frequency, and bottom spectrum banner.

Beats:
- Beat 1: The 1973 Fiedler Breakthrough & Ascending Spectrum Rail (0s - 16s)
- Beat 2: The High-Dimensional Spring Tug-of-War (16s - 32s)
- Beat 3: Depth 0 Topological Cut on Locked Chessboard (32s - 48s)
- Terminal Frame: Locked pawn chessboard with active Fiedler cut and piece badges
  held in stillness for seamless Scene 04 handoff.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene03TheFiedlerBreakthrough(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 02 TERMINAL STATE
        # =============================================================
        # Top Title from Scene 02
        title_scene02 = CMTex(
            r"\text{2. What Makes It ``Spectral''?}",
            fontsize=30,
            height=0.40,
            color=JEWEL_CYAN
        ).move_to(UP * 3.35)

        # Mini Chessboard & Tactical Network
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

        mini_pieces = VGroup(mini_p_qd4, mini_p_bc4, mini_p_ne5, mini_p_rd1, mini_p_kg1, mini_p_bkg8, mini_p_be7, mini_p_bnc6)
        mini_ray1 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(2, 3), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray2 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(4, 4), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray3 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(6, 7), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_ray4 = Line(mini_board.get_square_pos(2, 3), mini_board.get_square_pos(4, 6), color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.90)
        mini_tactical = VGroup(mini_ray1, mini_ray2, mini_ray3, mini_ray4)
        mini_chess = VGroup(mini_board, mini_pieces, mini_tactical)

        mini_tag = CMTex(r"\text{Chessboard Tactical Network}", fontsize=20, height=0.24, color=JEWEL_CYAN)
        mini_tag.next_to(mini_board, UP, buff=0.22)

        # Center: Refractive Optical Prism
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

        board_exit_pt = np.array([-3.55, -0.15, 0])
        hit_pt = np.array([-2.49, -0.15, 0])
        beam_glow = Line(board_exit_pt, hit_pt, color=TEXT_WHITE, stroke_width=6.5, stroke_opacity=0.35)
        beam_core = Line(board_exit_pt, hit_pt, color=TEXT_WHITE, stroke_width=2.4, stroke_opacity=1.0)
        incident_beam = VGroup(beam_glow, beam_core)

        exit_pt = np.array([-0.40, -0.40, 0])
        internal_ray = Line(hit_pt, exit_pt, color=TEXT_WHITE, stroke_width=2.4, stroke_opacity=0.85)

        # 4 Dispersed Spectral Rays
        tier_specs = [
            (r"\lambda_1 = 0.00", r"\text{Connected Baseline (Global DC)}", RIGHT * 3.4 + UP * 2.1, JEWEL_CYAN, "dc"),
            (r"\lambda_2 = 0.42", r"\text{Fiedler Frequency (Algebraic Connectivity)}", RIGHT * 3.4 + UP * 0.7, JEWEL_GOLD, "half"),
            (r"\lambda_3 = 1.15", r"\text{Intermediate Harmonics (Higher Spatial Frequencies)}", RIGHT * 3.4 + DOWN * 0.7, JEWEL_GREEN, "full"),
            (r"\lambda_N = 3.80", r"\text{Maximum Frequency (Peak Local Edge Contrast)}", RIGHT * 3.4 + DOWN * 2.1, JEWEL_CORAL, "high"),
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

        # Radiant Fiedler Highlight on Ray 1
        spectral_rays[1][0].set_stroke(width=8.0, opacity=0.30)
        spectral_rays[1][1].set_stroke(width=3.2, color=JEWEL_GOLD)
        fiedler_glow_pulse = Line(exit_pt, spectral_rays[1][1].get_end(), color=JEWEL_GOLD, stroke_width=8.0, stroke_opacity=0.30)

        spectrum_banner = CMTex(
            r"\text{The Spectrum: Decomposing Chessboard Geometry into Spatial Frequencies}",
            fontsize=22,
            height=0.32,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.30)

        inherited_state = VGroup(
            title_scene02,
            mini_chess, mini_tag,
            prism_group, incident_beam, internal_ray,
            spectral_rays, spectral_labels, fiedler_glow_pulse,
            spectrum_banner
        )
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE 1973 BREAKTHROUGH & ASCENDING SPECTRUM RAIL (0s - 16s)
        # Narrator: "In 1973, a Czech mathematician called Miroslav Fiedler published a paper with a profound discovery.
        # If you line up all the eigenvalues of a graph's Laplacian in ascending order:
        # 0 = lambda_1 <= lambda_2 <= lambda_3 <= ... <= lambda_N
        # The first eigenvalue, lambda_1, is always zero. It just represents the whole board as a single baseline.
        # The second eigenvalue—lambda_2—is where the magic happens."
        # =============================================================
        title_scene03 = CMTex(
            r"\text{3. The 1973 Breakthrough: The Fiedler Vector } (\lambda_2)",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        citation_card = RoundedRectangle(
            width=8.8, height=0.70, corner_radius=0.12,
            fill_color="#070e1a", fill_opacity=0.92,
            stroke_color=JEWEL_CYAN, stroke_width=1.4
        ).move_to(UP * 2.3)
        citation_text = CMTex(
            r"\text{Miroslav Fiedler (1973) } \bullet \text{ ``Algebraic Connectivity of Graphs''}",
            fontsize=20,
            height=0.26,
            color=JEWEL_CYAN
        ).move_to(citation_card.get_center())
        citation_group = VGroup(citation_card, citation_text)

        # Clean transition from Scene 02 to Scene 03 header
        self.play(
            FadeOut(title_scene02),
            FadeOut(spectrum_banner),
            FadeIn(title_scene03, UP * 0.1),
            FadeIn(citation_group, DOWN * 0.1),
            run_time=1.0,
            rate_func=smooth
        )

        # Transition the 4 spectral rays into a horizontal sorted spectrum rail
        rail_y = DOWN * 0.35
        rail_line = Line(LEFT * 5.6, RIGHT * 5.6, color="#1e293b", stroke_width=2.4).move_to(rail_y)
        rail_glow = Line(LEFT * 5.6, RIGHT * 5.6, color=JEWEL_BLUE, stroke_width=6.0, stroke_opacity=0.20).move_to(rail_y)

        spec_points = [
            (-4.2, r"0 = \lambda_1", r"\text{Global Baseline (DC)}", JEWEL_CYAN, 0.14),
            (-1.5, r"\lambda_2", r"\text{Algebraic Connectivity}", JEWEL_GOLD, 0.22),
            ( 1.4, r"\lambda_3", r"\text{Intermediate Harmonics}", JEWEL_GREEN, 0.14),
            ( 4.2, r"\lambda_N", r"\text{Maximum Frequency Mode}", JEWEL_CORAL, 0.14),
        ]

        rail_dots = VGroup()
        rail_top_lbls = VGroup()
        rail_bot_lbls = VGroup()
        for x_val, sym_tex, desc_tex, col, radius in spec_points:
            pt = np.array([x_val, rail_y[1], 0])
            dot = Dot(pt, radius=radius, color=col)
            is_fiedler = (sym_tex == r"\lambda_2")
            tl = CMTex(sym_tex, fontsize=24 if is_fiedler else 20, height=0.32 if is_fiedler else 0.26, color=col)
            tl.next_to(dot, UP, buff=0.22)
            bl = CMTex(desc_tex, fontsize=16 if is_fiedler else 15, height=0.22 if is_fiedler else 0.19, color=TEXT_WHITE if is_fiedler else TEXT_MUTED)
            bl.next_to(dot, DOWN, buff=0.22)
            rail_dots.add(dot)
            rail_top_lbls.add(tl)
            rail_bot_lbls.add(bl)

        leq_symbols = VGroup()
        for i in range(len(spec_points) - 1):
            x1 = spec_points[i][0] + 0.55
            x2 = spec_points[i+1][0] - 0.55
            mid_x = (x1 + x2) / 2
            leq = CMTex(r"\leq", fontsize=20, height=0.22, color=TEXT_DIM).move_to(np.array([mid_x, rail_y[1], 0]))
            leq_symbols.add(leq)

        rail_assembly = VGroup(rail_glow, rail_line, rail_dots, rail_top_lbls, rail_bot_lbls, leq_symbols)

        # Dissolve the prism & board cleanly as the spectrum expands into the rail
        self.play(
            FadeOut(mini_chess),
            FadeOut(mini_tag),
            FadeOut(prism_group),
            FadeOut(incident_beam),
            FadeOut(internal_ray),
            FadeOut(spectral_rays),
            FadeOut(spectral_labels),
            FadeOut(fiedler_glow_pulse),
            ShowCreation(rail_glow),
            ShowCreation(rail_line),
            LaggedStart(*[GrowFromCenter(d) for d in rail_dots], lag_ratio=0.12),
            FadeIn(rail_top_lbls),
            FadeIn(rail_bot_lbls),
            FadeIn(leq_symbols),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(1.2)

        # Dramatic Spotlight Pulse on Lambda_2 (Algebraic Connectivity)
        f_dot = rail_dots[1]
        ring1 = Circle(radius=0.25, color=JEWEL_GOLD, stroke_width=3.0).move_to(f_dot.get_center())
        ring2 = Circle(radius=0.25, color=JEWEL_GOLD, stroke_width=2.0).move_to(f_dot.get_center())

        fiedler_callout = CMTex(
            r"\lambda_2 \text{ measures how difficult it is to break the network apart.}",
            fontsize=20,
            height=0.26,
            color=JEWEL_GOLD
        ).move_to(DOWN * 2.6)

        self.play(
            FadeIn(fiedler_callout, UP * 0.1),
            f_dot.animate.scale(1.25),
            rail_top_lbls[1].animate.scale(1.20).set_color("#ffffff"),
            ring1.animate.scale(3.2).set_stroke(opacity=0),
            ring2.animate.scale(2.2).set_stroke(opacity=0),
            run_time=1.6,
            rate_func=rush_into
        )
        self.wait(2.0)

        # =============================================================
        # BEAT 2: THE HIGH-DIMENSIONAL SPRING TUG-OF-WAR (16s - 32s)
        # Narrator: "To find the Fiedler vector, the math is essentially solving a high-dimensional game of tug-of-war.
        # The Laplacian calculates the difference between every pair of pieces:
        # min sum A_ij (v_i - v_j)^2  subject to sum v_i = 0  and  ||v|| = 1.
        # Notice that term: (v_i - v_j). Whenever two pieces have a strong connection... the math forcefully pulls
        # coordinated pieces together... All the piece values must add up to exactly zero.
        # That means the board has to split: White's coordinated kingside attack clusters into positive territory,
        # disconnected queenside pieces get pushed into negative territory."
        # =============================================================
        self.play(
            FadeOut(citation_group),
            FadeOut(rail_assembly),
            FadeOut(fiedler_callout),
            run_time=1.0,
            rate_func=smooth
        )

        # Optimization Formula Panel
        opt_box = RoundedRectangle(
            width=9.8, height=1.55, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color=JEWEL_GOLD, stroke_width=1.5
        ).move_to(UP * 2.2)

        opt_formula = CMTex(
            r"\min_{v} \sum_{i,j} A_{ij} (v_i - v_j)^2 \quad \text{subject to } \sum_{i=1}^N v_i = 0, \; \|v\| = 1",
            fontsize=25,
            height=0.42,
            color=JEWEL_GOLD
        ).move_to(opt_box.get_center() + UP * 0.26)

        opt_sub = CMTex(
            r"(v_i - v_j)^2 \Rightarrow \text{Severe penalty for assigning different values to defending pieces}",
            fontsize=18,
            height=0.24,
            color=TEXT_BRIGHT
        ).move_to(opt_box.get_center() + DOWN * 0.34)

        opt_panel = VGroup(opt_box, opt_formula, opt_sub)

        # 1D Real Line Axis
        axis_y = DOWN * 0.75
        axis_line = Line(LEFT * 5.4, RIGHT * 5.4, color="#1e293b", stroke_width=2.0).move_to(axis_y)
        axis_glow = Line(LEFT * 5.4, RIGHT * 5.4, color=JEWEL_CYAN, stroke_width=4.5, stroke_opacity=0.15).move_to(axis_y)

        ticks_data = [
            (-4.5, r"-1.0"),
            (-2.25, r"-0.5"),
            ( 0.0,  r"0.0"),
            ( 2.25, r"+0.5"),
            ( 4.5,  r"+1.0")
        ]
        ticks_group = VGroup()
        for tx, tlabel in ticks_data:
            tmark = Line(np.array([tx, axis_y[1] - 0.12, 0]), np.array([tx, axis_y[1] + 0.12, 0]), color=TEXT_MUTED, stroke_width=1.5)
            tlbl = CMTex(tlabel, fontsize=17, height=0.20, color=TEXT_MUTED).next_to(tmark, DOWN, buff=0.14)
            ticks_group.add(tmark, tlbl)

        # Zero Balance Fulcrum
        zero_pt = Dot(np.array([0, axis_y[1], 0]), radius=0.09, color=TEXT_WHITE)
        zero_marker = Line(np.array([0, axis_y[1] - 0.28, 0]), np.array([0, axis_y[1] + 0.28, 0]), color=JEWEL_CORAL, stroke_width=2.4)
        zero_label = CMTex(
            r"\sum v_i = 0 \quad (\text{Center of Mass Balance})",
            fontsize=17,
            height=0.22,
            color=JEWEL_CORAL
        ).next_to(zero_marker, DOWN, buff=0.35)

        axis_group = VGroup(axis_glow, axis_line, ticks_group, zero_pt, zero_marker, zero_label)

        self.play(
            FadeIn(opt_panel, DOWN * 0.1),
            ShowCreation(axis_group),
            run_time=1.5,
            rate_func=smooth
        )
        self.wait(0.8)

        # Interactive Pieces Tug-of-War: Start uncoordinated and snap into clusters
        # Queenside Pieces (Negative): Isolated bR (-0.88), bP (-0.68), bP (-0.50)
        neg_start = [np.array([-1.2, axis_y[1], 0]), np.array([-2.8, axis_y[1], 0]), np.array([-0.6, axis_y[1], 0])]
        neg_target = [np.array([-3.96, axis_y[1], 0]), np.array([-3.06, axis_y[1], 0]), np.array([-2.25, axis_y[1], 0])] # -0.88, -0.68, -0.50

        # Kingside Pieces (Positive): Coordinated wQ (+0.74), wN (+0.60), wK (+0.42)
        pos_start = [np.array([0.8, axis_y[1], 0]), np.array([3.2, axis_y[1], 0]), np.array([1.5, axis_y[1], 0])]
        pos_target = [np.array([3.33, axis_y[1], 0]), np.array([2.70, axis_y[1], 0]), np.array([1.89, axis_y[1], 0])] # +0.74, +0.60, +0.42

        # Create nodes
        neg_dots = VGroup(*[Dot(p, radius=0.16, color=JEWEL_CORAL) for p in neg_start])
        pos_dots = VGroup(*[Dot(p, radius=0.16, color=JEWEL_CYAN) for p in pos_start])

        lbl_neg = CMTex(r"\text{Queenside Defense Cluster } (v_i < 0)", fontsize=18, height=0.22, color=JEWEL_CORAL)
        lbl_neg.move_to(np.array([-3.2, axis_y[1] + 0.85, 0]))

        lbl_pos = CMTex(r"\text{Kingside Attack Cluster } (v_i > 0)", fontsize=18, height=0.22, color=JEWEL_CYAN)
        lbl_pos.move_to(np.array([2.7, axis_y[1] + 0.85, 0]))

        self.play(
            LaggedStart(*[GrowFromCenter(d) for d in neg_dots], lag_ratio=0.1),
            LaggedStart(*[GrowFromCenter(d) for d in pos_dots], lag_ratio=0.1),
            FadeIn(lbl_neg, UP * 0.1),
            FadeIn(lbl_pos, UP * 0.1),
            run_time=1.2
        )

        # Draw physical spring coils between nodes in each cluster
        def create_spring(p1, p2, col, num_coils=8, amp=0.10):
            v = p2 - p1
            L = np.linalg.norm(v)
            if L == 0:
                return Line(p1, p2, color=col)
            u = v / L
            n = np.array([-u[1], u[0], 0])
            pts = [p1]
            for step in range(1, num_coils * 2):
                t = step / (num_coils * 2)
                side = 1 if step % 2 != 0 else -1
                pts.append(p1 + u * (t * L) + n * (side * amp))
            pts.append(p2)
            return VMobject().set_points_smoothly(pts).set_stroke(color=col, width=2.4, opacity=0.85)

        s_neg1 = create_spring(neg_start[0], neg_start[1], JEWEL_CORAL)
        s_neg2 = create_spring(neg_start[1], neg_start[2], JEWEL_CORAL)
        s_pos1 = create_spring(pos_start[0], pos_start[1], JEWEL_CYAN)
        s_pos2 = create_spring(pos_start[1], pos_start[2], JEWEL_CYAN)
        springs_initial = VGroup(s_neg1, s_neg2, s_pos1, s_pos2)

        # Cross-board weak bottleneck spring across 0
        s_bottleneck = create_spring(neg_start[2], pos_start[0], TEXT_WHITE, num_coils=6, amp=0.06).set_stroke(width=1.2, opacity=0.45)
        bn_tag = CMTex(r"\text{Weakest Bottleneck}", fontsize=15, height=0.18, color=TEXT_WHITE).next_to(s_bottleneck, UP, buff=0.15)

        self.play(
            ShowCreation(springs_initial),
            ShowCreation(s_bottleneck),
            FadeIn(bn_tag),
            run_time=1.2
        )
        self.wait(0.6)

        # ANIMPULSE: Energy minimization pulls pieces forcefully into clusters!
        s_neg1_target = create_spring(neg_target[0], neg_target[1], JEWEL_CORAL)
        s_neg2_target = create_spring(neg_target[1], neg_target[2], JEWEL_CORAL)
        s_pos1_target = create_spring(pos_target[0], pos_target[1], JEWEL_CYAN)
        s_pos2_target = create_spring(pos_target[1], pos_target[2], JEWEL_CYAN)
        s_bn_target = Line(neg_target[2], pos_target[2], color=JEWEL_CORAL, stroke_width=0.8, stroke_opacity=0.35)

        self.play(
            Transform(neg_dots[0], Dot(neg_target[0], radius=0.18, color=JEWEL_CORAL)),
            Transform(neg_dots[1], Dot(neg_target[1], radius=0.18, color=JEWEL_CORAL)),
            Transform(neg_dots[2], Dot(neg_target[2], radius=0.18, color=JEWEL_CORAL)),
            Transform(pos_dots[0], Dot(pos_target[0], radius=0.18, color=JEWEL_CYAN)),
            Transform(pos_dots[1], Dot(pos_target[1], radius=0.18, color=JEWEL_CYAN)),
            Transform(pos_dots[2], Dot(pos_target[2], radius=0.18, color=JEWEL_CYAN)),
            Transform(s_neg1, s_neg1_target),
            Transform(s_neg2, s_neg2_target),
            Transform(s_pos1, s_pos1_target),
            Transform(s_pos2, s_pos2_target),
            Transform(s_bottleneck, s_bn_target),
            run_time=1.8,
            rate_func=rush_into
        )
        self.wait(2.0)

        # =============================================================
        # BEAT 3: DEPTH 0 TOPOLOGICAL CUT ON LOCKED CHESSBOARD (32s - 48s)
        # Narrator: "Real chess position with locked center pawns.
        # The zero line draws itself dynamically straight down the pawn spine.
        # White’s attacking kingside pieces glow cobalt; Black's trapped queenside rook glows deep ochre.
        # And the zero line falls exactly along the weakest bottleneck on the board.
        # The engine doesn't need to search millions of moves into the future.
        # Even at depth 0, on the first iteration of calculations, the math instantly diagnoses the board's topology."
        # =============================================================
        self.play(
            FadeOut(opt_panel),
            FadeOut(axis_group),
            FadeOut(neg_dots),
            FadeOut(pos_dots),
            FadeOut(lbl_neg),
            FadeOut(lbl_pos),
            FadeOut(springs_initial),
            FadeOut(s_bottleneck),
            FadeOut(bn_tag),
            run_time=1.0,
            rate_func=smooth
        )

        # Left Side: Broadcast Chessboard with Locked Center
        board = BroadcastChessBoard(
            center=LEFT * 3.4 + DOWN * 0.15,
            sq_size=0.52,
            light_color=BOARD_LIGHT_SQ,
            dark_color=BOARD_DARK_SQ,
            show_coords=True
        )

        # Authentic Locked Pawn Structure & Tactical Pieces
        # Black: a8=Rook, c6=Pawn, d5=Pawn, e6=Pawn, f6=Knight, g8=King
        # White: c4=Pawn, d4=Pawn, e5=Pawn, e1=Rook, f3=Knight, g4=Queen, g1=King
        locked_setup = [
            ("bR", 0, 7), # a8: Isolated trapped rook
            ("bP", 2, 5), # c6
            ("bP", 3, 4), # d5
            ("bP", 4, 5), # e6
            ("bN", 5, 5), # f6
            ("bK", 6, 7), # g8
            ("wP", 2, 3), # c4
            ("wP", 3, 3), # d4
            ("wP", 4, 4), # e5
            ("wR", 4, 0), # e1
            ("wN", 5, 2), # f3
            ("wQ", 6, 3), # g4
            ("wK", 6, 0), # g1
        ]
        board_pieces = VGroup()
        for p_code, col, row in locked_setup:
            p_obj = board.create_piece(p_code, col, row)
            board_pieces.add(p_obj)

        # Right Side: Analytical Fiedler Vector Panel
        panel_center = RIGHT * 3.4 + DOWN * 0.15
        panel_card = RoundedRectangle(
            width=5.8, height=5.2, corner_radius=0.14,
            fill_color="#070d18", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(panel_center)

        fiedler_header = CMTex(r"\text{The Fiedler Vector } (v_2)", fontsize=24, height=0.34, color=JEWEL_GOLD)
        fiedler_header.move_to(panel_center + UP * 2.15)
        fiedler_sub = CMTex(r"\lambda_2 = 0.428 \quad (\text{Algebraic Connectivity})", fontsize=18, height=0.24, color=TEXT_WHITE)
        fiedler_sub.next_to(fiedler_header, DOWN, buff=0.10)

        # Coordinate distribution plot inside panel
        plot_axis = Line(panel_center + LEFT * 2.4, panel_center + RIGHT * 2.4, color="#1e293b", stroke_width=1.6).move_to(panel_center + DOWN * 0.1)
        plot_zero = Line(panel_center + DOWN * 1.5, panel_center + UP * 1.2, color=JEWEL_CORAL, stroke_width=2.0)
        plot_zero_lbl = CMTex(r"\text{Zero Cut } (v_2 = 0)", fontsize=16, height=0.20, color=JEWEL_CORAL).next_to(plot_zero, UP, buff=0.10)

        # Plotted pieces on distribution
        # Isolated Rook at -0.92
        pt_rook = Dot(panel_center + np.array([-1.85, -0.1, 0]), radius=0.14, color=JEWEL_CORAL)
        lbl_rook = CMTex(r"a8 \text{ Rook: } -0.82", fontsize=16, height=0.20, color=JEWEL_CORAL).next_to(pt_rook, DOWN, buff=0.15)
        sub_rook = CMTex(r"\text{(Isolated Behind Pawns)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(lbl_rook, DOWN, buff=0.06)

        # Attacking Queen at +0.74 and Knight at +0.62
        pt_queen = Dot(panel_center + np.array([1.75, 0.4, 0]), radius=0.14, color=JEWEL_CYAN)
        lbl_queen = CMTex(r"g4 \text{ Queen: } +0.74", fontsize=16, height=0.20, color=JEWEL_CYAN).next_to(pt_queen, UP, buff=0.14)

        pt_knight = Dot(panel_center + np.array([1.45, -0.6, 0]), radius=0.12, color=JEWEL_CYAN)
        lbl_knight = CMTex(r"f3 \text{ Knight: } +0.62", fontsize=16, height=0.20, color=JEWEL_CYAN).next_to(pt_knight, DOWN, buff=0.14)

        dist_plot_group = VGroup(
            panel_card, fiedler_header, fiedler_sub,
            plot_axis, plot_zero, plot_zero_lbl,
            pt_rook, lbl_rook, sub_rook,
            pt_queen, lbl_queen, pt_knight, lbl_knight
        )

        self.play(
            FadeIn(board, scale=0.95),
            LaggedStart(*[FadeIn(p, scale=0.85) for p in board_pieces], lag_ratio=0.06),
            FadeIn(dist_plot_group, UP * 0.15),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(1.0)

        # Cluster Overlays on Chessboard:
        # Queenside negative wash (files a, b, c) -> JEWEL_CORAL
        # Kingside positive wash (files e, f, g, h) -> JEWEL_CYAN
        clusters_group = VGroup()
        for r in range(8):
            for c in range(8):
                sq_pos = board.get_square_pos(c, r)
                if c <= 2: # Queenside
                    ov = Square(side_length=board.sq_size).move_to(sq_pos)
                    ov.set_fill(JEWEL_CORAL, opacity=0.24).set_stroke(width=0)
                    clusters_group.add(ov)
                elif c >= 4: # Kingside
                    ov = Square(side_length=board.sq_size).move_to(sq_pos)
                    ov.set_fill(JEWEL_CYAN, opacity=0.26).set_stroke(width=0)
                    clusters_group.add(ov)

        # Dynamic Zero-Crossing Fault Line along the pawn spine (between c and d files, c=2.5)
        # Using exact board coordinates
        split_c = 2.5
        x_cut = board.center_pt[0] + (split_c - 3.5) * board.sq_size
        top_cut = np.array([x_cut, board.center_pt[1] + 4.0 * board.sq_size, 0])
        bot_cut = np.array([x_cut, board.center_pt[1] - 4.0 * board.sq_size, 0])

        cut_glow = Line(top_cut, bot_cut, color=JEWEL_CORAL, stroke_width=7.5, stroke_opacity=0.45)
        cut_core = DashedLine(top_cut, bot_cut, color="#ffffff", stroke_width=2.5, dash_length=0.10)

        cut_badge = RoundedRectangle(
            width=2.8, height=0.44, corner_radius=0.10,
            fill_color="#180c14", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(top_cut + UP * 0.52)
        cut_badge_text = CMTex(r"\text{FIEDLER CUT: } v_2 = 0", fontsize=17, height=0.22, color=JEWEL_CORAL).move_to(cut_badge.get_center())
        fault_assembly = VGroup(cut_glow, cut_core, cut_badge, cut_badge_text)

        # Real Numerical Badges on Key Pieces (+0.74, +0.62, -0.82)
        badge_specs = [
            (6, 3, r"+0.74", JEWEL_CYAN),  # wQ g4
            (5, 2, r"+0.62", JEWEL_CYAN),  # wN f3
            (0, 7, r"-0.82", JEWEL_CORAL), # bR a8
        ]
        piece_badges = VGroup()
        for b_col, b_row, b_val, b_colr in badge_specs:
            sq_c = board.get_square_pos(b_col, b_row)
            pill = RoundedRectangle(
                width=0.52, height=0.24, corner_radius=0.06,
                fill_color="#080c14", fill_opacity=0.92,
                stroke_color=b_colr, stroke_width=1.2
            ).move_to(sq_c + DOWN * (board.sq_size * 0.34))
            ptxt = CMTex(b_val, fontsize=15, height=0.17, color=b_colr).move_to(pill.get_center())
            piece_badges.add(VGroup(pill, ptxt))

        # Bottom Takeaway Banner
        vision_banner = CMTex(
            r"\text{Depth 0 Topological Vision: Instant structural diagnosis without search.}",
            fontsize=20,
            height=0.28,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.42)

        self.play(
            FadeIn(clusters_group, lag_ratio=0.02),
            ShowCreation(cut_glow),
            ShowCreation(cut_core),
            FadeIn(cut_badge),
            FadeIn(cut_badge_text),
            run_time=1.6,
            rate_func=smooth
        )

        self.play(
            LaggedStart(*[FadeIn(b, scale=0.85) for b in piece_badges], lag_ratio=0.15),
            FadeIn(vision_banner, UP * 0.1),
            run_time=1.2
        )

        # Pulse the Fault Line
        self.play(
            cut_glow.animate.set_stroke(width=12.0, opacity=0.65),
            cut_badge.animate.scale(1.06),
            run_time=0.8,
            rate_func=there_and_back
        )
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 04 Handoff)
        # Holds the locked board, cluster overlays, Fiedler cut,
        # piece badges, and distribution panel active in pure stillness.
        # =============================================================
        self.wait(2.5)
