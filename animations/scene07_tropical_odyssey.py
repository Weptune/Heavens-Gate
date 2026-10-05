"""
Heaven's Gate Documentary - Scene 07: The Tropical Semiring & The 66-Round Marathon
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical Systems & Convex Geometry (Luminous Jewel Palette)

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 06 terminal frame:
  Crashed tachometer gauge at 30,412 NPS, depth 5 warning, and climax quote.

Beats:
- Beat 1: The Max-Plus Tropical Semiring (0s - 12s)
  Replacing matrix multiplication with addition and maximum operations.
- Beat 2: Faceted Minimax Hyperplanes & Temperature Smoothing (12s - 24s)
  Convex faceted upper envelope morphing smoothly under Log-Sum-Exp.
- Beat 3: Spatial King Bucketing: 640 Topological Sector Parameters (24s - 36s)
  10 King safety zones on the BroadcastChessBoard with dynamic weight dispatch.
- Beat 4: The 66-Round Training Marathon & The Loss Plateau (36s - 50s)
  Autonomous self-play loop and the stubborn 66-round loss plateau.
- Terminal Frame: Held in stillness for Scene 08 (Hall of Shame).
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene07TropicalOdyssey(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 06 TERMINAL STATE
        # =============================================================
        title_scene06 = CMTex(
            r"\text{6. The Complexity Trap: Why ``Smarter'' Isn't Better}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        title_gauge = CMTex(r"\text{Engine Search Throughput: The Eigensolver Bottleneck}", fontsize=22, height=0.30, color=JEWEL_GOLD).move_to(UP * 2.45)
        sub_gauge = CMTex(r"\text{Measuring Nodes Per Second (NPS) under full alpha-beta tree search}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_gauge, DOWN, buff=0.12)
        gauge_header = VGroup(title_gauge, sub_gauge)

        g_center = DOWN * 0.55
        r_dial = 1.85

        outer_ring = Arc(radius=r_dial + 0.30, start_angle=210 * DEGREES, angle=-240 * DEGREES, stroke_color="#334155", stroke_width=2.2, arc_center=g_center)
        inner_ring = Arc(radius=r_dial - 0.22, start_angle=210 * DEGREES, angle=-240 * DEGREES, stroke_color="#1e293b", stroke_width=1.4, arc_center=g_center)

        arc_red    = Arc(radius=r_dial, start_angle=210 * DEGREES, angle=-45 * DEGREES, stroke_color=JEWEL_CORAL, stroke_width=7.0, arc_center=g_center)
        arc_yellow = Arc(radius=r_dial, start_angle=165 * DEGREES, angle=-65 * DEGREES, stroke_color=JEWEL_GOLD, stroke_width=7.0, arc_center=g_center)
        arc_green  = Arc(radius=r_dial, start_angle=100 * DEGREES, angle=-130 * DEGREES, stroke_color=JEWEL_GREEN, stroke_width=7.0, arc_center=g_center)

        hub_outer = Circle(radius=0.28, fill_color="#0f172a", fill_opacity=1.0, stroke_color=JEWEL_GOLD, stroke_width=1.8).move_to(g_center)
        hub_inner = Circle(radius=0.12, fill_color=JEWEL_GOLD, fill_opacity=1.0).set_stroke(width=0).move_to(g_center)

        lbl_30k = CMTex(r"30\text{K}", fontsize=15, height=0.18, color=JEWEL_CORAL).move_to(g_center + np.array([-1.95, 0.35, 0]))
        lbl_1m  = CMTex(r"1\text{M}", fontsize=15, height=0.18, color=TEXT_MUTED).move_to(g_center + np.array([-1.15, 1.45, 0]))
        lbl_10m = CMTex(r"10\text{M}", fontsize=15, height=0.18, color=TEXT_MUTED).move_to(g_center + np.array([0.0, 1.85, 0]))
        lbl_40m = CMTex(r"40\text{M}", fontsize=16, height=0.20, color=JEWEL_GREEN).move_to(g_center + np.array([1.80, -0.60, 0]))
        dial_labels = VGroup(lbl_30k, lbl_1m, lbl_10m, lbl_40m)

        gauge_group = VGroup(outer_ring, inner_ring, arc_red, arc_yellow, arc_green, hub_outer, hub_inner, dial_labels)

        def make_needle(angle_deg, col=JEWEL_CORAL):
            rad = angle_deg * DEGREES
            tip = g_center + np.array([np.cos(rad) * (r_dial + 0.10), np.sin(rad) * (r_dial + 0.10), 0])
            n_perp = np.array([-np.sin(rad) * 0.07, np.cos(rad) * 0.07, 0])
            poly = Polygon(g_center - n_perp * 1.4, tip, g_center + n_perp * 1.4)
            poly.set_fill(col, opacity=1.0).set_stroke(color="#ffffff", width=0.8, opacity=0.8)
            return poly

        crashed_needle = make_needle(195, col=JEWEL_CORAL)
        hud_crashed = CMTex(r"\text{30,412 NPS (CRASHED: Eigensolver Active)}", fontsize=19, height=0.26, color=JEWEL_CORAL).move_to(DOWN * 2.15)
        depth_warning = CMTex(r"\text{Search Depth: } 14 \to 5 \quad (\text{Timeout Warning})", fontsize=16, height=0.21, color=JEWEL_CORAL).move_to(DOWN * 2.62)
        climax_quote = CMTex(
            r"\text{``Our algorithm was a genius... and it was playing chess like a complete idiot.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(title_scene06, gauge_header, gauge_group, crashed_needle, hud_crashed, depth_warning, climax_quote)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE TROPICAL MORPH (0s - 12s)
        # Narrator: "If Heaven's Gate was ever going to survive, I had to wage an all-out war against computational complexity.
        # And that led to the first of many terrible ideas: The Max-Plus Tropical Semiring."
        # =============================================================
        title_scene07 = CMTex(
            r"\text{7. The Tropical Odyssey: Max-Plus \& The 66-Round Marathon}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sr_title = CMTex(r"\text{The Max-Plus Tropical Semiring}", fontsize=24, height=0.34, color=JEWEL_GOLD).move_to(UP * 2.45)
        sr_sub = CMTex(r"\text{Translating discrete minimax search into continuous convex linear algebra}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(sr_title, DOWN, buff=0.12)
        sr_header = VGroup(sr_title, sr_sub)

        self.play(
            FadeOut(inherited_state),
            FadeIn(title_scene07, UP * 0.1),
            FadeIn(sr_header, DOWN * 0.1),
            run_time=1.0,
            rate_func=smooth
        )

        # Left Column: Classical Field (R, +, *)
        c_left = LEFT * 3.5 + DOWN * 0.3
        card_cf = RoundedRectangle(
            width=5.6, height=4.2, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(c_left)

        lbl_cf = CMTex(r"\text{Classical Field } (\mathbb{R}, +, \times)", fontsize=18, height=0.24, color=TEXT_WHITE).move_to(c_left + UP * 1.5)
        op_add_c = CMTex(r"\text{Addition: } a + b", fontsize=16, height=0.22, color=TEXT_MUTED).next_to(lbl_cf, DOWN, buff=0.45)
        op_mul_c = CMTex(r"\text{Multiplication: } a \times b", fontsize=16, height=0.22, color=TEXT_MUTED).next_to(op_add_c, DOWN, buff=0.35)
        cf_group = VGroup(card_cf, lbl_cf, op_add_c, op_mul_c)

        # Right Column: Tropical Semiring (R U {-inf}, oplus, otimes)
        c_right = RIGHT * 3.5 + DOWN * 0.3
        card_ts = RoundedRectangle(
            width=5.6, height=4.2, corner_radius=0.14,
            fill_color="#071312", fill_opacity=0.92,
            stroke_color=JEWEL_CYAN, stroke_width=1.6
        ).move_to(c_right)

        lbl_ts = CMTex(r"\text{Tropical Semiring } (\mathbb{R} \cup \{-\infty\}, \oplus, \otimes)", fontsize=18, height=0.24, color=JEWEL_CYAN).move_to(c_right + UP * 1.5)
        op_add_t = CMTex(r"a \oplus b = \max(a, b)", fontsize=16, height=0.22, color=JEWEL_GOLD).next_to(lbl_ts, DOWN, buff=0.45)
        op_mul_t = CMTex(r"a \otimes b = a + b", fontsize=16, height=0.22, color=JEWEL_GREEN).next_to(op_add_t, DOWN, buff=0.35)
        ts_group = VGroup(card_ts, lbl_ts, op_add_t, op_mul_t)

        # Center Morph Arrows
        arr_add = CMTex(r"\longrightarrow", fontsize=22, height=0.24, color=JEWEL_CYAN).move_to(UP * 0.15)
        arr_mul = CMTex(r"\longrightarrow", fontsize=22, height=0.24, color=JEWEL_CYAN).move_to(DOWN * 0.45)
        arr_group = VGroup(arr_add, arr_mul)

        insight_banner = CMTex(
            r"\text{Minimax game-tree optimization is literally linear algebra over the Tropical Semiring.}",
            fontsize=16,
            height=0.22,
            color=TEXT_BRIGHT
        ).move_to(DOWN * 2.85)

        self.play(
            FadeIn(cf_group, LEFT * 0.15),
            ShowCreation(arr_group),
            FadeIn(ts_group, RIGHT * 0.15),
            FadeIn(insight_banner, UP * 0.1),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(3.2)

        # =============================================================
        # BEAT 2: FACETED MINIMAX HYPERPLANES & LOG-SUM-EXP (12s - 24s)
        # =============================================================
        self.play(
            FadeOut(sr_header),
            FadeOut(cf_group),
            FadeOut(arr_group),
            FadeOut(ts_group),
            FadeOut(insight_banner),
            run_time=0.8,
            rate_func=smooth
        )

        title_geom = CMTex(r"\text{Faceted Minimax Hyperplanes \& Temperature Smoothing}", fontsize=22, height=0.30, color=JEWEL_CYAN).move_to(UP * 2.45)
        sub_geom = CMTex(r"\mathrm{Eval}(s) = \max_i (w_i^\top x + b_i) \quad \text{forms a non-differentiable convex upper envelope}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_geom, DOWN, buff=0.12)
        geom_header = VGroup(title_geom, sub_geom)
        self.play(FadeIn(geom_header, DOWN * 0.1), run_time=0.6)

        axes = Axes(
            x_range=[-3.0, 3.0, 1.0],
            y_range=[-0.5, 4.0, 1.0],
            width=6.8,
            height=3.6,
            axis_config={"stroke_color": "#2a364f", "stroke_width": 1.4}
        ).move_to(LEFT * 1.5 + DOWN * 0.5)

        x_lbl = CMTex(r"\text{Topological Feature Vector } x", fontsize=14, height=0.17, color=TEXT_MUTED).next_to(axes.x_axis, DOWN, buff=0.12)
        y_lbl = CMTex(r"\text{Evaluation Score}", fontsize=14, height=0.17, color=TEXT_MUTED).next_to(axes.y_axis, LEFT, buff=0.12)

        line1 = axes.get_graph(lambda x: 0.45 * x + 1.2, x_range=[-2.8, 2.8], color="#2b3a52").set_stroke(width=1.5)
        line2 = axes.get_graph(lambda x: -0.75 * x + 1.7, x_range=[-2.8, 2.8], color="#2b3a52").set_stroke(width=1.5)
        line3 = axes.get_graph(lambda x: 1.30 * x - 0.4, x_range=[-2.8, 2.8], color="#2b3a52").set_stroke(width=1.5)

        def envelope_fn(x):
            return max(0.45 * x + 1.2, -0.75 * x + 1.7, 1.30 * x - 0.4)

        env_graph = axes.get_graph(envelope_fn, x_range=[-2.6, 2.6], color=JEWEL_GOLD).set_stroke(width=3.4)

        env_badge = RoundedRectangle(width=4.4, height=0.45, corner_radius=0.08, fill_color="#181408", fill_opacity=0.95, stroke_color=JEWEL_GOLD, stroke_width=1.4).move_to(RIGHT * 4.2 + UP * 0.8)
        env_badge_txt = CMTex(r"\text{Max-Plus Upper Envelope}", fontsize=15, height=0.18, color=JEWEL_GOLD).move_to(env_badge.get_center())
        env_desc = CMTex(r"\text{Sharp non-differentiable corners}", fontsize=14, height=0.17, color=TEXT_MUTED).next_to(env_badge, DOWN, buff=0.12)
        env_grp = VGroup(env_badge, env_badge_txt, env_desc)

        self.play(
            ShowCreation(axes),
            FadeIn(x_lbl), FadeIn(y_lbl),
            ShowCreation(line1), ShowCreation(line2), ShowCreation(line3),
            ShowCreation(env_graph),
            FadeIn(env_grp, RIGHT * 0.15),
            run_time=1.4
        )
        self.wait(1.5)

        def logsumexp_fn(x, tau=0.35):
            z1 = (0.45 * x + 1.2) / tau
            z2 = (-0.75 * x + 1.7) / tau
            z3 = (1.30 * x - 0.4) / tau
            m = max(z1, z2, z3)
            return tau * (m + np.log(np.exp(z1 - m) + np.exp(z2 - m) + np.exp(z3 - m)))

        smooth_graph = axes.get_graph(lambda x: logsumexp_fn(x, 0.35), x_range=[-2.6, 2.6], color=JEWEL_CYAN).set_stroke(width=3.4)

        smooth_badge = RoundedRectangle(width=4.4, height=0.45, corner_radius=0.08, fill_color="#071418", fill_opacity=0.95, stroke_color=JEWEL_CYAN, stroke_width=1.4).move_to(RIGHT * 4.2 + DOWN * 0.8)
        smooth_badge_txt = CMTex(r"\text{Log-Sum-Exp Smooth Max}", fontsize=15, height=0.18, color=JEWEL_CYAN).move_to(smooth_badge.get_center())
        smooth_eq = CMTex(r"S_\tau(z) = \tau \ln \left( \sum_i \exp(z_i / \tau) \right)", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(smooth_badge, DOWN, buff=0.12)
        smooth_desc = CMTex(r"\tau \to 0 \Rightarrow \text{Approaches exact minimax}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(smooth_eq, DOWN, buff=0.10)
        smooth_grp = VGroup(smooth_badge, smooth_badge_txt, smooth_eq, smooth_desc)

        self.play(
            Transform(env_graph, smooth_graph),
            FadeIn(smooth_grp, RIGHT * 0.15),
            run_time=1.6
        )
        self.wait(2.5)

        # =============================================================
        # BEAT 3: 10 SPATIAL KING BUCKETS (24s - 36s)
        # =============================================================
        self.play(
            FadeOut(geom_header),
            FadeOut(axes),
            FadeOut(x_lbl), FadeOut(y_lbl),
            FadeOut(line1), FadeOut(line2), FadeOut(line3),
            FadeOut(env_graph),
            FadeOut(env_grp),
            FadeOut(smooth_grp),
            run_time=0.8,
            rate_func=smooth
        )

        title_buck = CMTex(r"\text{Spatial King Bucketing: 640 Topological Sector Parameters}", fontsize=22, height=0.30, color=JEWEL_GOLD).move_to(UP * 2.45)
        sub_buck = CMTex(r"\text{10 distinct king zones allocate specialized hyperplane weights to match board topology}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_buck, DOWN, buff=0.12)
        buck_header = VGroup(title_buck, sub_buck)
        self.play(FadeIn(buck_header, DOWN * 0.1), run_time=0.6)

        cb_center = LEFT * 3.4 + DOWN * 0.5
        sq_size = 0.42
        cb = BroadcastChessBoard(center=cb_center, sq_size=sq_size, show_coords=True)
        king_piece = cb.create_piece("wK", 4, 0)

        self.play(FadeIn(cb), FadeIn(king_piece), run_time=1.0)

        z1 = Rectangle(width=4 * sq_size, height=2 * sq_size, fill_color=JEWEL_CYAN, fill_opacity=0.28, stroke_color=JEWEL_CYAN, stroke_width=1.5)
        z1.move_to(cb.get_center() + RIGHT * (2 * sq_size) + DOWN * (2 * sq_size))
        z1_lbl = CMTex(r"\text{Zone 1: O-O}", fontsize=13, height=0.16, color=JEWEL_CYAN).move_to(z1.get_center())

        z2 = Rectangle(width=4 * sq_size, height=2 * sq_size, fill_color=JEWEL_BLUE, fill_opacity=0.28, stroke_color=JEWEL_BLUE, stroke_width=1.5)
        z2.move_to(cb.get_center() + LEFT * (2 * sq_size) + DOWN * (2 * sq_size))
        z2_lbl = CMTex(r"\text{Zone 2: O-O-O}", fontsize=13, height=0.16, color=JEWEL_BLUE).move_to(z2.get_center())

        z5 = Rectangle(width=4 * sq_size, height=4 * sq_size, fill_color=JEWEL_GOLD, fill_opacity=0.22, stroke_color=JEWEL_GOLD, stroke_width=1.5)
        z5.move_to(cb.get_center() + UP * 0.0)
        z5_lbl = CMTex(r"\text{Zone 5: Active Center}", fontsize=13, height=0.16, color=JEWEL_GOLD).move_to(z5.get_center())

        zones_group = VGroup(z1, z1_lbl, z2, z2_lbl, z5, z5_lbl)
        self.play(FadeIn(zones_group), run_time=0.9)

        r_side = RIGHT * 2.8 + DOWN * 0.5
        card_weights = RoundedRectangle(
            width=5.8, height=4.2, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_side)

        active_zone_lbl = CMTex(r"\text{ACTIVE SECTOR: 01 (Castled Kingside)}", fontsize=16, height=0.22, color=JEWEL_CYAN).move_to(r_side + UP * 1.5)
        param_dispatch = CMTex(r"\text{Dispatching 64 weights (Fiedler, Degrees, PST)}", fontsize=14, height=0.18, color=TEXT_MUTED).next_to(active_zone_lbl, DOWN, buff=0.10)

        np.random.seed(42)
        initial_weights = np.random.uniform(0.25, 1.35, 14)
        bar_group = VGroup()
        for i, w in enumerate(initial_weights):
            bar = Line(r_side + LEFT * 2.0 + RIGHT * (i * 0.30) + DOWN * 0.6, r_side + LEFT * 2.0 + RIGHT * (i * 0.30) + DOWN * 0.6 + UP * (w * 1.0), color=JEWEL_CYAN, stroke_width=5.0)
            bar_group.add(bar)

        spec_lbl = CMTex(r"\text{Topological Sector Weight Amplitudes } (w_1 \dots w_{64})", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(bar_group, DOWN, buff=0.25)
        hist_grp = VGroup(card_weights, active_zone_lbl, param_dispatch, bar_group, spec_lbl)

        self.play(FadeIn(hist_grp), run_time=1.2)
        self.wait(1.0)

        # King moves to center -> Dynamic sector weight change
        sq_e4_pos = cb.get_square_pos(4, 3)
        target_zone_lbl = CMTex(r"\text{ACTIVE SECTOR: 05 (Central Endgame)}", fontsize=16, height=0.22, color=JEWEL_GOLD).move_to(r_side + UP * 1.5)

        np.random.seed(99)
        new_weights = np.random.uniform(0.35, 1.55, 14)
        new_bar_group = VGroup()
        for i, w in enumerate(new_weights):
            bar = Line(r_side + LEFT * 2.0 + RIGHT * (i * 0.30) + DOWN * 0.6, r_side + LEFT * 2.0 + RIGHT * (i * 0.30) + DOWN * 0.6 + UP * (w * 1.0), color=JEWEL_GOLD, stroke_width=5.0)
            new_bar_group.add(bar)

        self.play(
            king_piece.animate.move_to(sq_e4_pos),
            FadeOut(active_zone_lbl),
            FadeIn(target_zone_lbl),
            Transform(bar_group, new_bar_group),
            run_time=1.4
        )
        self.wait(2.2)

        # =============================================================
        # BEAT 4: THE 66-ROUND CONTINUOUS MARATHON & PLATEAU (36s - 50s)
        # =============================================================
        self.play(
            FadeOut(buck_header),
            FadeOut(cb),
            FadeOut(king_piece),
            FadeOut(zones_group),
            FadeOut(hist_grp),
            run_time=0.8,
            rate_func=smooth
        )

        title_train = CMTex(r"\text{The 66-Round Training Marathon \& The Loss Plateau}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.45)
        sub_train = CMTex(r"\text{24/7 Autonomous Self-Play Rig } \bullet \text{ 2,000,000 FEN Replay Buffer } \bullet \text{ Adam SGD}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_train, DOWN, buff=0.12)
        train_header = VGroup(title_train, sub_train)
        self.play(FadeIn(train_header, DOWN * 0.1), run_time=0.6)

        # Pipeline diagram card
        c_pipe = LEFT * 3.5 + DOWN * 0.4
        card_pipe = RoundedRectangle(
            width=5.6, height=4.2, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(c_pipe)

        n1 = CMTex(r"[1] \text{ 500 Self-Play Games (16 Threads)}", fontsize=14, height=0.18, color=JEWEL_CYAN).move_to(c_pipe + UP * 1.3)
        n2 = CMTex(r"[2] \text{ Replay Buffer (2,000,000 FENs)}", fontsize=14, height=0.18, color=JEWEL_GOLD).move_to(c_pipe + UP * 0.0)
        n3 = CMTex(r"[3] \text{ Adam SGD (300 Epochs / Round)}", fontsize=14, height=0.18, color=JEWEL_GREEN).move_to(c_pipe + DOWN * 1.3)

        arr1 = Arrow(
            n1.get_bottom(), n2.get_top(),
            buff=0.12,
            fill_color="#475569",
            fill_opacity=1.0,
            stroke_color="#475569",
            stroke_width=0.0,
            thickness=2.0,
            tip_width_ratio=3.2,
            tip_angle=PI / 3.5,
        )
        arr2 = Arrow(
            n2.get_bottom(), n3.get_top(),
            buff=0.12,
            fill_color="#475569",
            fill_opacity=1.0,
            stroke_color="#475569",
            stroke_width=0.0,
            thickness=2.0,
            tip_width_ratio=3.2,
            tip_angle=PI / 3.5,
        )
        loop_arrow = ArcBetweenPoints(n3.get_left() + LEFT * 0.1, n1.get_left() + LEFT * 0.1, angle=TAU * 0.35, color=JEWEL_CORAL).set_stroke(width=1.6)
        loop_lbl = CMTex(r"\text{Continuous Iteration}", fontsize=13, height=0.16, color=JEWEL_CORAL).next_to(loop_arrow, LEFT, buff=0.10)

        pipe_grp = VGroup(card_pipe, n1, n2, n3, arr1, arr2, loop_arrow, loop_lbl)
        self.play(FadeIn(pipe_grp), run_time=1.2)

        # Loss plateau plot
        loss_axes = Axes(
            x_range=[0, 66, 11],
            y_range=[0.0, 1.0, 0.2],
            width=5.8,
            height=3.4,
            axis_config={"stroke_color": "#2a364f", "stroke_width": 1.4}
        ).move_to(RIGHT * 3.4 + DOWN * 0.4)

        x_loss = CMTex(r"\text{Training Rounds } (1 - 66)", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(loss_axes.x_axis, DOWN, buff=0.12)
        y_loss = CMTex(r"\text{MSE Loss } (\mathrm{cp}^2)", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(loss_axes.y_axis, LEFT, buff=0.12)

        def loss_fn(r):
            if r < 12:
                return 0.85 * np.exp(-r / 3.5) + 0.25
            else:
                return 0.28 + 0.03 * np.sin(r * 1.8) + 0.015 * np.cos(r * 3.7)

        loss_curve = loss_axes.get_graph(loss_fn, x_range=[0, 66], color=JEWEL_CORAL).set_stroke(width=2.8)

        plateau_badge = RoundedRectangle(width=4.2, height=0.42, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).move_to(loss_axes.c2p(38, 0.52))
        plateau_lbl = CMTex(r"\text{Inescapable Non-Convex Plateau}", fontsize=14, height=0.18, color=JEWEL_CORAL).move_to(plateau_badge.get_center())
        plateau_grp = VGroup(plateau_badge, plateau_lbl)

        self.play(ShowCreation(loss_axes), FadeIn(x_loss), FadeIn(y_loss), run_time=1.0)
        self.play(ShowCreation(loss_curve), FadeIn(plateau_grp), run_time=1.6)

        climax_quote = CMTex(
            r"\text{``And it failed in almost every way imaginable.''}",
            fontsize=18,
            height=0.26,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        self.play(FadeIn(climax_quote, UP * 0.08), run_time=1.0)
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 08 Handoff)
        # =============================================================
        self.wait(2.5)
