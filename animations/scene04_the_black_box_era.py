"""
Heaven's Gate Documentary - Scene 04: The Black Box Era
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical & Neural Systems (Luminous Jewel Palette)

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 03 terminal frame:
  Locked pawn chessboard, Fiedler vector distribution panel,
  topological cut line, and piece badges.

Beats:
- Beat 1: Classical Heuristics & The Glass Ceiling (0s - 16s)
- Beat 2: The Neural Revolution: NNUE & +150 Elo Leap (16s - 34s)
- Beat 3: The Black Box & Horizon Drift Evaluation Crash (34s - 52s)
- Terminal Frame: The Monolithic 4.3M Black Box and Depth 36 Evaluation Cliff
  held in stillness for seamless Scene 05 handoff.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene04TheBlackBoxEra(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 03 TERMINAL STATE
        # =============================================================
        title_scene03 = CMTex(
            r"\text{3. The 1973 Breakthrough: The Fiedler Vector } (\lambda_2)",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        board = BroadcastChessBoard(
            center=LEFT * 3.4 + DOWN * 0.15,
            sq_size=0.52,
            light_color=BOARD_LIGHT_SQ,
            dark_color=BOARD_DARK_SQ,
            show_coords=True
        )

        locked_setup = [
            ("bR", 0, 7),
            ("bP", 2, 5),
            ("bP", 3, 4),
            ("bP", 4, 5),
            ("bN", 5, 5),
            ("bK", 6, 7),
            ("wP", 2, 3),
            ("wP", 3, 3),
            ("wP", 4, 4),
            ("wR", 4, 0),
            ("wN", 5, 2),
            ("wQ", 6, 3),
            ("wK", 6, 0),
        ]
        board_pieces = VGroup()
        for p_code, col, row in locked_setup:
            p_obj = board.create_piece(p_code, col, row)
            board_pieces.add(p_obj)

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

        plot_axis = Line(panel_center + LEFT * 2.4, panel_center + RIGHT * 2.4, color="#1e293b", stroke_width=1.6).move_to(panel_center + DOWN * 0.1)
        plot_zero = Line(panel_center + DOWN * 1.5, panel_center + UP * 1.2, color=JEWEL_CORAL, stroke_width=2.0)
        plot_zero_lbl = CMTex(r"\text{Zero Cut } (v_2 = 0)", fontsize=16, height=0.20, color=JEWEL_CORAL).next_to(plot_zero, UP, buff=0.10)

        pt_rook = Dot(panel_center + np.array([-1.85, -0.1, 0]), radius=0.14, color=JEWEL_CORAL)
        lbl_rook = CMTex(r"a8 \text{ Rook: } -0.82", fontsize=16, height=0.20, color=JEWEL_CORAL).next_to(pt_rook, DOWN, buff=0.15)
        sub_rook = CMTex(r"\text{(Isolated Behind Pawns)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(lbl_rook, DOWN, buff=0.06)

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

        clusters_group = VGroup()
        for r in range(8):
            for c in range(8):
                sq_pos = board.get_square_pos(c, r)
                if c <= 2:
                    ov = Square(side_length=board.sq_size).move_to(sq_pos)
                    ov.set_fill(JEWEL_CORAL, opacity=0.24).set_stroke(width=0)
                    clusters_group.add(ov)
                elif c >= 4:
                    ov = Square(side_length=board.sq_size).move_to(sq_pos)
                    ov.set_fill(JEWEL_CYAN, opacity=0.26).set_stroke(width=0)
                    clusters_group.add(ov)

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

        badge_specs = [
            (6, 3, r"+0.74", JEWEL_CYAN),
            (5, 2, r"+0.62", JEWEL_CYAN),
            (0, 7, r"-0.82", JEWEL_CORAL),
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

        vision_banner = CMTex(
            r"\text{Depth 0 Topological Vision: Instant structural diagnosis without search.}",
            fontsize=20,
            height=0.28,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.42)

        inherited_state = VGroup(
            title_scene03,
            board, board_pieces,
            dist_plot_group,
            clusters_group,
            fault_assembly,
            piece_badges,
            vision_banner
        )
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: CLASSICAL HEURISTICS & THE GLASS CEILING (0s - 16s)
        # Narrator: "In order to understand why this is different, let's look at how modern chess engines work today.
        # For decades, chess engines were built on classical heuristics: programmers manually hardcoding thousands of rigid rules.
        # Stuff like 'A bishop pair is worth an extra half a pawn.' 'Keep pawns in front of your king.' 'Have pieces cover more squares.'
        # It worked, but it had a hard ceiling. Classical engines were tactical monsters, but positionally blind."
        # =============================================================
        title_scene04 = CMTex(
            r"\text{4. The Black Box Era: Heuristics, NNUE, and Horizon Drift}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        # Transition header and dissolve mathematical cut overlays
        self.play(
            FadeOut(title_scene03),
            FadeOut(vision_banner),
            FadeOut(dist_plot_group),
            FadeOut(clusters_group),
            FadeOut(fault_assembly),
            FadeOut(piece_badges),
            FadeIn(title_scene04, UP * 0.1),
            run_time=1.0,
            rate_func=smooth
        )

        # Classical Heuristic Rules Panel on Right
        rules_card = RoundedRectangle(
            width=5.8, height=4.8, corner_radius=0.14,
            fill_color="#070d18", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(panel_center)

        rules_title = CMTex(r"\text{Classical Handcrafted Heuristics}", fontsize=22, height=0.30, color=JEWEL_GOLD)
        rules_title.move_to(panel_center + UP * 1.95)

        r1 = CMTex(r"\text{Rule 1: Bishop Pair } \Rightarrow +0.50 \text{ Pawns}", fontsize=17, height=0.22, color=TEXT_BRIGHT)
        r2 = CMTex(r"\text{Rule 2: Pawn Shield } \Rightarrow \text{Penalty for King exposure}", fontsize=17, height=0.22, color=TEXT_MUTED)
        r3 = CMTex(r"\text{Rule 3: Mobility } \Rightarrow \sum \text{Safe squares attacked}", fontsize=17, height=0.22, color=TEXT_MUTED)

        rules_list = VGroup(r1, r2, r3).arrange(DOWN, buff=0.25, aligned_edge=LEFT).move_to(panel_center + UP * 0.5)

        # The Complexity Ceiling: Glowing Coral Glass Barrier
        ceil_y = panel_center[1] - 0.75
        ceiling_line = DashedLine(panel_center + LEFT * 2.5 + DOWN * 0.6, panel_center + RIGHT * 2.5 + DOWN * 0.6, color=JEWEL_CORAL, stroke_width=2.5)
        ceil_badge = RoundedRectangle(
            width=5.2, height=0.46, corner_radius=0.08,
            fill_color="#180a0e", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.4
        ).move_to(np.array([panel_center[0], ceil_y - 0.35, 0]))
        ceil_tag = CMTex(r"\text{The Classical Complexity Ceiling}", fontsize=16, height=0.20, color=JEWEL_CORAL).move_to(ceil_badge.get_center())
        ceil_sub = CMTex(r"\text{Tactical Monsters, Positionally Blind}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(ceil_badge, DOWN, buff=0.12)

        ceiling_group = VGroup(ceiling_line, ceil_badge, ceil_tag, ceil_sub)
        heuristics_panel = VGroup(rules_card, rules_title, rules_list, ceiling_group)

        self.play(FadeIn(heuristics_panel, RIGHT * 0.15), run_time=1.4, rate_func=smooth)
        self.wait(1.2)

        # Tactical Flash: Board calculates rapid tactical lines but hits the ceiling
        p_flash1 = board.get_square_pos(6, 3)
        p_flash2 = board.get_square_pos(5, 5)
        ray_glow = Line(p_flash1, p_flash2, color=JEWEL_CYAN, stroke_width=4.5, stroke_opacity=0.30)
        ray_core = Line(p_flash1, p_flash2, color=JEWEL_CYAN, stroke_width=1.8, stroke_opacity=0.95)
        tactical_ray = VGroup(ray_glow, ray_core)
        self.play(ShowCreation(tactical_ray), ceiling_line.animate.set_stroke(width=6.0, opacity=0.8), run_time=0.8)
        self.play(FadeOut(tactical_ray), ceiling_line.animate.set_stroke(width=2.5, opacity=1.0), run_time=0.6)
        self.wait(1.5)

        # =============================================================
        # BEAT 2: THE NEURAL REVOLUTION & +150 ELO LEAP (16s - 34s)
        # Narrator: "Then came the first big innovation: Neural Networks.
        # First, AlphaZero in 2017 showed the world that a deep neural network could teach itself chess from scratch.
        # Then, in 2020, the open-source community pulled off an engineering masterpiece: NNUE—Efficiently Updatable Neural Networks.
        # It evaluated positions at tens of millions of nodes per second, capturing subtle, non-linear relationships.
        # After that, every top engine adopted NNUE. It added over 150 Elo in a single leap."
        # =============================================================
        self.play(
            FadeOut(heuristics_panel),
            FadeOut(board),
            FadeOut(board_pieces),
            run_time=1.0,
            rate_func=smooth
        )

        nn_header = CMTex(
            r"\text{The Neural Revolution: 2017 AlphaZero } \to \text{ 2020 NNUE}",
            fontsize=22,
            height=0.30,
            color=JEWEL_CYAN
        ).move_to(UP * 2.3)
        self.play(FadeIn(nn_header, DOWN * 0.1), run_time=0.8)

        # Left Side: Authentic Multi-Layer Neural Network
        nn_center = LEFT * 2.8 + DOWN * 0.3
        layer_sizes = [5, 7, 7, 3, 1]
        layer_xs = np.linspace(-5.2, -0.4, len(layer_sizes))
        layer_colors = [JEWEL_CYAN, JEWEL_BLUE, JEWEL_VIOLET, JEWEL_LAVENDER, JEWEL_GOLD]

        all_nodes = []
        all_node_mobs = VGroup()
        all_edges = VGroup()

        for l_idx, (sz, lx) in enumerate(zip(layer_sizes, layer_xs)):
            col = layer_colors[l_idx]
            ys = np.linspace(-1.8, 1.8, sz)
            curr_layer = []
            for y in ys:
                pt = np.array([lx, y - 0.2, 0])
                n_circ = Circle(radius=0.13, fill_color="#060b14", fill_opacity=0.95, stroke_color=col, stroke_width=1.8).move_to(pt)
                curr_layer.append(n_circ)
                all_node_mobs.add(n_circ)
            all_nodes.append(curr_layer)

        for l in range(len(layer_sizes) - 1):
            col = layer_colors[l]
            for n1 in all_nodes[l]:
                for n2 in all_nodes[l+1]:
                    e = Line(n1.get_center(), n2.get_center(), stroke_color=col, stroke_width=0.7, stroke_opacity=0.25)
                    all_edges.add(e)

        nn_label = CMTex(
            r"\text{Input (HalfKP) } \to \text{ Accumulator } \to \text{ Clipped ReLU } \to \text{ Eval}",
            fontsize=16,
            height=0.22,
            color=TEXT_MUTED
        ).move_to(LEFT * 2.8 + DOWN * 2.6)

        nn_group = VGroup(all_edges, all_node_mobs, nn_label)

        # Right Side: Dramatic +150 Elo Rating Leap Bar Chart
        chart_center = RIGHT * 3.4 + DOWN * 0.3
        chart_base_y = -1.8

        bar_c = RoundedRectangle(
            width=1.0, height=2.0, corner_radius=0.06,
            fill_color="#182334", fill_opacity=0.9,
            stroke_color="#334660", stroke_width=1.5
        ).move_to(np.array([chart_center[0] - 1.2, chart_base_y + 1.0, 0]))

        bar_n = RoundedRectangle(
            width=1.0, height=3.5, corner_radius=0.06,
            fill_color=JEWEL_GREEN, fill_opacity=0.85,
            stroke_color="#ffffff", stroke_width=1.8
        ).move_to(np.array([chart_center[0] + 1.2, chart_base_y + 1.75, 0]))

        lbl_c_name = CMTex(r"\text{Classical}", fontsize=18, height=0.22, color=TEXT_MUTED).next_to(bar_c, DOWN, buff=0.18)
        lbl_c_score = CMTex(r"3300 \text{ Elo}", fontsize=17, height=0.20, color=TEXT_WHITE).move_to(bar_c.get_center())

        lbl_n_name = CMTex(r"\text{NNUE}", fontsize=18, height=0.22, color=JEWEL_GREEN).next_to(bar_n, DOWN, buff=0.18)
        lbl_n_score = CMTex(r"3450+ \text{ Elo}", fontsize=17, height=0.20, color="#ffffff").move_to(bar_n.get_center() + UP * 0.4)

        leap_badge = RoundedRectangle(
            width=2.4, height=0.48, corner_radius=0.10,
            fill_color="#092014", fill_opacity=0.95,
            stroke_color=JEWEL_GREEN, stroke_width=1.6
        ).next_to(bar_n, UP, buff=0.22)
        leap_text = CMTex(r"+150 \text{ Elo Leap}", fontsize=18, height=0.22, color=JEWEL_GREEN).move_to(leap_badge.get_center())

        chart_group = VGroup(bar_c, bar_n, lbl_c_name, lbl_c_score, lbl_n_name, lbl_n_score, leap_badge, leap_text)

        self.play(
            ShowCreation(all_edges),
            LaggedStart(*[GrowFromCenter(n) for n in all_node_mobs], lag_ratio=0.03),
            FadeIn(nn_label, UP * 0.1),
            FadeIn(chart_group, UP * 0.15),
            run_time=2.0,
            rate_func=smooth
        )

        # Animate Neural Signal Wave Propagation across layers
        pulse_layers = []
        for l_idx in range(len(layer_sizes)):
            layer_pulse = AnimationGroup(*[n.animate.set_fill(layer_colors[l_idx], opacity=0.9).scale(1.15) for n in all_nodes[l_idx]], rate_func=there_and_back, run_time=0.45)
            pulse_layers.append(layer_pulse)

        self.play(Succession(*pulse_layers))
        self.wait(1.5)

        # =============================================================
        # BEAT 3: THE BLACK BOX & HORIZON DRIFT EVALUATION CRASH (34s - 52s)
        # Narrator: "And yet... it came with a cost. Neural networks are statistical approximators.
        # They don't actually know anything about the spatial physics of a chessboard.
        # Instead, they are giant matrices of millions of floating-point weights trained on billions of self-play games...
        # More importantly: because neural nets are trained on statistical patterns, they have notorious blind spots.
        # In locked, closed positions—where no captures are happening, neural networks often suffer from horizon drift.
        # Material is equal. Nothing is under attack. So the network evaluates the position as dead even (0.00),
        # completely unaware that one side is structurally doomed 25 moves down the road."
        # =============================================================
        self.play(
            FadeOut(nn_header),
            FadeOut(chart_group),
            FadeOut(all_edges),
            FadeOut(all_node_mobs),
            FadeOut(nn_label),
            run_time=1.0,
            rate_func=smooth
        )

        # Left Side: The Monolithic Opaque Black Box
        box_center = LEFT * 3.4 + DOWN * 0.15
        black_box = RoundedRectangle(
            width=5.4, height=4.8, corner_radius=0.18,
            fill_color="#060910", fill_opacity=0.98,
            stroke_color=JEWEL_VIOLET, stroke_width=2.4
        ).move_to(box_center)

        box_title = CMTex(r"\text{The Black Box: 4.3 Million Statistical Weights}", fontsize=18, height=0.24, color=JEWEL_LAVENDER)
        box_title.move_to(box_center + UP * 1.95)

        # Subtle floating weight numbers streaming inside the opaque box
        weights_sub = CMTex(r"\text{Statistical Approximator (Zero Spatial Reasoning)}", fontsize=15, height=0.20, color=TEXT_MUTED)
        weights_sub.move_to(box_center + DOWN * 1.95)

        weight_matrix_text = CMTex(
            r"W_{ij} \in [-1.24, +0.89, -0.04, \dots, +2.11]",
            fontsize=17,
            height=0.22,
            color=TEXT_DIM
        ).move_to(box_center)

        black_box_group = VGroup(black_box, box_title, weights_sub, weight_matrix_text)

        # Right Side: Horizon Drift Evaluation Graph (Flatline -> Fatal Plunge)
        eval_card = RoundedRectangle(
            width=5.8, height=4.8, corner_radius=0.14,
            fill_color="#070d18", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(panel_center)

        eval_title = CMTex(r"\text{Search Depth vs. Engine Evaluation}", fontsize=20, height=0.28, color=JEWEL_GOLD)
        eval_title.move_to(panel_center + UP * 1.95)

        # Graph Axes
        # X: Depth 0 to 40, Y: Eval +1.0 to -5.0
        origin = panel_center + LEFT * 2.2 + DOWN * 0.4
        x_axis = Line(origin, origin + RIGHT * 4.4, color="#1e293b", stroke_width=1.8)
        y_axis = Line(origin + DOWN * 1.2, origin + UP * 1.6, color="#1e293b", stroke_width=1.8)

        lbl_x = CMTex(r"\text{Search Depth } (d)", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(x_axis, DOWN, buff=0.15)
        lbl_y = CMTex(r"\text{Eval (Pawns)}", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(y_axis, UP, buff=0.15)

        # Zero line dashed across chart
        zero_eval_line = DashedLine(origin + UP * 0.8, origin + RIGHT * 4.4 + UP * 0.8, color="#25354c", stroke_width=1.2)
        zero_eval_lbl = CMTex(r"0.00", fontsize=14, height=0.16, color=TEXT_DIM).next_to(zero_eval_line, LEFT, buff=0.08)

        # Horizon Drift Curve:
        # Depth 0 to 30: flat at 0.00 (y = origin_y + 0.8)
        # Depth 30 to 36: sudden catastrophic plunge down to -4.50 (y = origin_y - 1.0)
        p_start = origin + UP * 0.8
        p_d30   = origin + RIGHT * 3.3 + UP * 0.8 # Depth 30
        p_d36   = origin + RIGHT * 4.0 + DOWN * 1.0 # Depth 36 plunge

        flat_segment = Line(p_start, p_d30, color=JEWEL_CYAN, stroke_width=3.2)
        flat_tag = CMTex(r"\text{Statistical Blind Spot: Flat 0.00 from Depth 0 to 30}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(origin + RIGHT * 1.8 + UP * 1.15)

        plunge_segment = Line(p_d30, p_d36, color=JEWEL_CORAL, stroke_width=4.0)

        # Crash Callout Badge
        crash_badge = RoundedRectangle(
            width=5.2, height=0.52, corner_radius=0.10,
            fill_color="#180a0e", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(panel_center + DOWN * 1.95)
        crash_text = CMTex(r"\text{HORIZON DRIFT CRASH: } -4.50 \text{ at Depth 36}", fontsize=16, height=0.22, color=JEWEL_CORAL).move_to(crash_badge.get_center())

        chart_elements = VGroup(eval_card, eval_title, x_axis, y_axis, lbl_x, lbl_y, zero_eval_line, zero_eval_lbl)

        self.play(
            FadeIn(black_box_group, LEFT * 0.15),
            FadeIn(chart_elements, RIGHT * 0.15),
            ShowCreation(flat_segment),
            FadeIn(flat_tag, UP * 0.08),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(1.2)

        # DRAMATIC PLUNGE: The evaluation curve falls off a cliff
        crash_dot = Dot(p_d36, radius=0.14, color=JEWEL_CORAL)
        crash_pulse = Circle(radius=0.14, color=JEWEL_CORAL, stroke_width=3.0).move_to(p_d36)

        self.play(
            ShowCreation(plunge_segment),
            GrowFromCenter(crash_dot),
            FadeIn(crash_badge, UP * 0.1),
            FadeIn(crash_text),
            run_time=1.4,
            rate_func=rush_into
        )

        # Pulse warning ring at crash site
        self.play(
            crash_pulse.animate.scale(2.8).set_stroke(opacity=0),
            crash_badge.animate.set_stroke(width=3.0),
            run_time=0.8,
            rate_func=rush_into
        )
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 05 Handoff)
        # Holds the opaque Black Box and Depth 36 Horizon Drift crash
        # active on screen for Scene 05's analytical split screen!
        # =============================================================
        self.wait(2.5)
