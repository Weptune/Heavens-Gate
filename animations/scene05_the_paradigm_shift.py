"""
Heaven's Gate Documentary - Scene 05: The Paradigm Shift
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical & Analytical Systems (Luminous Jewel Palette)

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 04 terminal frame:
  The 4.3M Black Box on the left, Depth 36 Horizon Drift crash graph on the right.

Beats:
- Beat 1: Side-by-Side: 4.3M Trained Weights vs. 0 Weights (0s - 16s)
- Beat 2: The Three Fundamental Pillars (16s - 34s)
- Beat 3: The Cliffhanger: The 2-Month Training Nightmare (34s - 50s)
- Terminal Frame: The Loss Plateau and Training Stall held in stillness for Scene 06.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene05TheParadigmShift(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 04 TERMINAL STATE
        # =============================================================
        title_scene04 = CMTex(
            r"\text{4. The Black Box Era: Heuristics, NNUE, and Horizon Drift}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        # Left: Monolithic Black Box
        box_center = LEFT * 3.4 + DOWN * 0.15
        black_box = RoundedRectangle(
            width=5.4, height=4.8, corner_radius=0.18,
            fill_color="#060910", fill_opacity=0.98,
            stroke_color=JEWEL_VIOLET, stroke_width=2.4
        ).move_to(box_center)

        box_title = CMTex(r"\text{The Black Box: 4.3 Million Statistical Weights}", fontsize=18, height=0.24, color=JEWEL_LAVENDER)
        box_title.move_to(box_center + UP * 1.95)

        weights_sub = CMTex(r"\text{Statistical Approximator (Zero Spatial Reasoning)}", fontsize=15, height=0.20, color=TEXT_MUTED)
        weights_sub.move_to(box_center + DOWN * 1.95)

        weight_matrix_text = CMTex(
            r"W_{ij} \in [-1.24, +0.89, -0.04, \dots, +2.11]",
            fontsize=17,
            height=0.22,
            color=TEXT_DIM
        ).move_to(box_center)

        black_box_group = VGroup(black_box, box_title, weights_sub, weight_matrix_text)

        # Right: Horizon Drift Evaluation Graph
        panel_center = RIGHT * 3.4 + DOWN * 0.15
        eval_card = RoundedRectangle(
            width=5.8, height=4.8, corner_radius=0.14,
            fill_color="#070d18", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(panel_center)

        eval_title = CMTex(r"\text{Search Depth vs. Engine Evaluation}", fontsize=20, height=0.28, color=JEWEL_GOLD)
        eval_title.move_to(panel_center + UP * 1.95)

        origin = panel_center + LEFT * 2.2 + DOWN * 0.4
        x_axis = Line(origin, origin + RIGHT * 4.4, color="#1e293b", stroke_width=1.8)
        y_axis = Line(origin + DOWN * 1.2, origin + UP * 1.6, color="#1e293b", stroke_width=1.8)

        lbl_x = CMTex(r"\text{Search Depth } (d)", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(x_axis, DOWN, buff=0.15)
        lbl_y = CMTex(r"\text{Eval (Pawns)}", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(y_axis, UP, buff=0.15)

        zero_eval_line = DashedLine(origin + UP * 0.8, origin + RIGHT * 4.4 + UP * 0.8, color="#25354c", stroke_width=1.2)
        zero_eval_lbl = CMTex(r"0.00", fontsize=14, height=0.16, color=TEXT_DIM).next_to(zero_eval_line, LEFT, buff=0.08)

        p_start = origin + UP * 0.8
        p_d30   = origin + RIGHT * 3.3 + UP * 0.8
        p_d36   = origin + RIGHT * 4.0 + DOWN * 1.0

        flat_segment = Line(p_start, p_d30, color=JEWEL_CYAN, stroke_width=3.2)
        flat_tag = CMTex(r"\text{Statistical Blind Spot: Flat 0.00 from Depth 0 to 30}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(origin + RIGHT * 1.8 + UP * 1.15)

        plunge_segment = Line(p_d30, p_d36, color=JEWEL_CORAL, stroke_width=4.0)
        crash_dot = Dot(p_d36, radius=0.14, color=JEWEL_CORAL)

        crash_badge = RoundedRectangle(
            width=5.2, height=0.52, corner_radius=0.10,
            fill_color="#180a0e", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(panel_center + DOWN * 1.95)
        crash_text = CMTex(r"\text{HORIZON DRIFT CRASH: } -4.50 \text{ at Depth 36}", fontsize=16, height=0.22, color=JEWEL_CORAL).move_to(crash_badge.get_center())

        chart_elements = VGroup(eval_card, eval_title, x_axis, y_axis, lbl_x, lbl_y, zero_eval_line, zero_eval_lbl)

        inherited_state = VGroup(
            title_scene04,
            black_box_group,
            chart_elements, flat_segment, flat_tag, plunge_segment, crash_dot,
            crash_badge, crash_text
        )
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE SPLIT-SCREEN PARADIGM SHIFT (0s - 16s)
        # Narrator: "Which brings us to the core experiment of this engine:
        # What happens if you replace that trained statistical black box... with exact analytical mathematics?
        # When you evaluate a chess position using Spectral Graph Theory, three fundamental things change."
        # =============================================================
        title_scene05 = CMTex(
            r"\text{5. The Paradigm Shift: Analytical Mathematics vs. Statistical Black Box}",
            fontsize=26,
            height=0.40,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        # Clean transition of header and right-side graph
        self.play(
            FadeOut(title_scene04),
            FadeOut(chart_elements),
            FadeOut(flat_segment),
            FadeOut(flat_tag),
            FadeOut(plunge_segment),
            FadeOut(crash_dot),
            FadeOut(crash_badge),
            FadeOut(crash_text),
            FadeIn(title_scene05, UP * 0.1),
            run_time=1.0,
            rate_func=smooth
        )

        # Central Split-Screen Divider
        divider_line = Line(UP * 2.5, DOWN * 3.2, color="#1e293b", stroke_width=2.0)
        divider_glow = Line(UP * 2.5, DOWN * 3.2, color=JEWEL_CYAN, stroke_width=5.0, stroke_opacity=0.25)
        divider_group = VGroup(divider_glow, divider_line)

        # Left Side Tag: Opaque Statistical Box
        tag_left = CMTex(r"\text{Opaque } \bullet \text{ 4,300,000 Trained Weights}", fontsize=18, height=0.24, color=JEWEL_CORAL)
        tag_left.move_to(box_center + UP * 2.65)

        # Right Side: Analytical Transparent White Box (0 Weights)
        lap_card = RoundedRectangle(
            width=5.4, height=4.8, corner_radius=0.18,
            fill_color="#07101d", fill_opacity=0.95,
            stroke_color=JEWEL_CYAN, stroke_width=2.2
        ).move_to(panel_center)

        tag_right = CMTex(r"\text{Analytical } \bullet \text{ 0 Weights (Pure Mathematics)}", fontsize=18, height=0.24, color=JEWEL_CYAN)
        tag_right.move_to(panel_center + UP * 2.65)

        lap_formula1 = CMTex(r"L = D - A \quad (\text{Graph Laplacian})", fontsize=20, height=0.28, color=JEWEL_CYAN)
        lap_formula1.move_to(panel_center + UP * 1.35)

        lap_formula2 = CMTex(r"Lv = \lambda v \quad (\text{Eigenspectrum Decomposition})", fontsize=18, height=0.24, color=TEXT_WHITE)
        lap_formula2.move_to(panel_center + UP * 0.55)

        # Luminous Spectral Rays radiating from matrix center
        spectral_rays = VGroup()
        for angle, col in zip([-0.5, -0.2, 0.1, 0.4], [JEWEL_CYAN, JEWEL_GOLD, JEWEL_GREEN, JEWEL_CORAL]):
            ray = Line(
                panel_center + DOWN * 0.3,
                panel_center + DOWN * 0.3 + np.array([np.cos(angle) * 2.0, np.sin(angle) * 1.2, 0]),
                color=col, stroke_width=2.4, stroke_opacity=0.85
            )
            spectral_rays.add(ray)

        lap_caption = CMTex(
            r"\text{Spatial topology is derived directly from linear algebra.}",
            fontsize=15,
            height=0.20,
            color=TEXT_BRIGHT
        ).move_to(panel_center + DOWN * 1.95)

        white_box_group = VGroup(lap_card, tag_right, lap_formula1, lap_formula2, spectral_rays, lap_caption)

        self.play(
            ShowCreation(divider_group),
            FadeIn(tag_left, DOWN * 0.1),
            FadeIn(white_box_group, RIGHT * 0.15),
            run_time=1.8,
            rate_func=smooth
        )
        self.wait(1.5)

        # =============================================================
        # BEAT 2: THE THREE FUNDAMENTAL PILLARS (16s - 34s)
        # Narrator: "First: The Feature Space is Derived, Not Learned.
        # In NNUE, the network has to spend billions of positions learning what a pawn chain or flank separation means...
        # With the Graph Laplacian, spatial connectivity is calculated analytically at Depth 0 with zero training data.
        # Second: Complete Interpretability (The White Box). If a neural net misjudges, it's a silent black box.
        # With spectral graphs, the engine's intuition is 100% visible and explainable.
        # Third: Depth 0 Topological Vision. It sees the board has severed into two independent subgames before search begins."
        # =============================================================
        self.play(
            FadeOut(black_box_group),
            FadeOut(tag_left),
            FadeOut(divider_group),
            FadeOut(white_box_group),
            run_time=1.0,
            rate_func=smooth
        )

        pillars_header = CMTex(
            r"\text{The Three Pillars of Spectral Chess}",
            fontsize=24,
            height=0.34,
            color=JEWEL_GOLD
        ).move_to(UP * 2.35)
        self.play(FadeIn(pillars_header, DOWN * 0.1), run_time=0.8)

        # 3 Structured Pillar Cards
        col_xs = [-4.0, 0.0, 4.0]
        pillar_configs = [
            (
                col_xs[0],
                r"\text{1. Derived Features}",
                JEWEL_CYAN,
                r"\text{No training on billions of games.}",
                r"\text{Exact spatial physics at Depth 0.}"
            ),
            (
                col_xs[1],
                r"\text{2. The ``White Box''}",
                JEWEL_GOLD,
                r"\text{Complete explainability.}",
                r"\text{Every centipawn traces to geometry.}"
            ),
            (
                col_xs[2],
                r"\text{3. Topological Vision}",
                JEWEL_GREEN,
                r"\text{Instant bottleneck diagnosis.}",
                r"\text{Prunes dead branches without search.}"
            ),
        ]

        pillar_cards = VGroup()
        for px, ptitle, pcol, pdesc1, pdesc2 in pillar_configs:
            card = RoundedRectangle(
                width=3.7, height=4.6, corner_radius=0.14,
                fill_color="#070e1a", fill_opacity=0.92,
                stroke_color=pcol, stroke_width=1.6
            ).move_to(np.array([px, -0.4, 0]))

            top_rule = Line(np.array([px - 1.5, 1.4, 0]), np.array([px + 1.5, 1.4, 0]), color=pcol, stroke_width=1.8)
            title_txt = CMTex(ptitle, fontsize=18, height=0.24, color=pcol).move_to(np.array([px, 1.05, 0]))

            dot_accent = Dot(np.array([px, 0.35, 0]), radius=0.16, color=pcol)
            ring_accent = Circle(radius=0.28, color=pcol, stroke_width=1.4).move_to(dot_accent.get_center())

            body1 = CMTex(pdesc1, fontsize=15, height=0.20, color=TEXT_BRIGHT).move_to(np.array([px, -0.55, 0]))
            body2 = CMTex(pdesc2, fontsize=14, height=0.18, color=TEXT_MUTED).move_to(np.array([px, -1.25, 0]))

            pillar_grp = VGroup(card, top_rule, title_txt, dot_accent, ring_accent, body1, body2)
            pillar_cards.add(pillar_grp)

        self.play(
            LaggedStart(*[FadeIn(p, UP * 0.15) for p in pillar_cards], lag_ratio=0.18),
            run_time=2.0,
            rate_func=smooth
        )
        self.wait(2.2)

        # =============================================================
        # BEAT 3: THE CLIFFHANGER: THE 2-MONTH TRAINING NIGHTMARE (34s - 50s)
        # Narrator: "...Which brought us to the million-dollar question:
        # If the math gives us the geometry for free, how do we translate that geometry into a chess score
        # without falling into the exact same training trap as the neural networks?
        # Well... I tried. And it led me into a 2 month long training nightmare."
        # =============================================================
        self.play(
            FadeOut(pillars_header),
            FadeOut(pillar_cards),
            run_time=1.0,
            rate_func=smooth
        )

        q_title = CMTex(r"\text{``The Million-Dollar Question''}", fontsize=24, height=0.34, color=JEWEL_GOLD)
        q_title.move_to(UP * 2.35)

        q_quote = CMTex(
            r"\text{``How do we translate geometry into score without the training trap?''}",
            fontsize=18,
            height=0.24,
            color=TEXT_WHITE
        ).next_to(q_title, DOWN, buff=0.18)

        # Loss Graph of Failed 2-Month Training
        chart_center = DOWN * 0.65
        chart_card = RoundedRectangle(
            width=8.8, height=3.8, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(chart_center)

        gx_origin = chart_center + LEFT * 3.4 + DOWN * 1.1
        gx_axis = Line(gx_origin, gx_origin + RIGHT * 6.8, color="#1e293b", stroke_width=1.8)
        gy_axis = Line(gx_origin, gx_origin + UP * 2.2, color="#1e293b", stroke_width=1.8)

        gx_lbl = CMTex(r"\text{Training Epochs}", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(gx_axis, DOWN, buff=0.14)
        gy_lbl = CMTex(r"\text{Loss}", fontsize=15, height=0.18, color=TEXT_MUTED).next_to(gy_axis, UP, buff=0.14)

        # Loss Curve: drops sharply from 0.95 to 0.48, then flatlines completely for 800 epochs
        loss_pts = [
            gx_origin + UP * 2.2,
            gx_origin + RIGHT * 0.9 + UP * 1.6,
            gx_origin + RIGHT * 1.8 + UP * 1.25,
            gx_origin + RIGHT * 2.7 + UP * 1.15,
            gx_origin + RIGHT * 4.2 + UP * 1.15, # Flatline plateau
            gx_origin + RIGHT * 6.5 + UP * 1.15, # Still stuck
        ]
        loss_curve = VMobject().set_points_smoothly(loss_pts).set_stroke(color=JEWEL_CORAL, width=3.4)

        # Training Stall Warning Badge
        stall_badge = RoundedRectangle(
            width=6.6, height=0.52, corner_radius=0.10,
            fill_color="#180a0e", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(chart_center + DOWN * 0.28)

        stall_text = CMTex(r"\text{TRAINING STALL: 2-MONTH LOSS PLATEAU}", fontsize=17, height=0.22, color=JEWEL_CORAL).move_to(stall_badge.get_center())

        stall_sub = CMTex(
            r"\text{Weights fail to generalize } \bullet \text{ Search Depth collapses}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(stall_badge, DOWN, buff=0.14)

        loss_chart_group = VGroup(chart_card, gx_axis, gy_axis, gx_lbl, gy_lbl, loss_curve)

        self.play(
            FadeIn(q_title, DOWN * 0.1),
            FadeIn(q_quote, DOWN * 0.1),
            FadeIn(loss_chart_group, UP * 0.15),
            run_time=1.8,
            rate_func=smooth
        )
        self.wait(1.0)

        # Plateau Warning Flash
        self.play(
            FadeIn(stall_badge, scale=0.95),
            FadeIn(stall_text),
            FadeIn(stall_sub, UP * 0.08),
            run_time=1.2
        )
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 06 Handoff)
        # Holds the question, stalled loss graph, and collapse warning
        # in stillness for Scene 06 (The Complexity Trap).
        # =============================================================
        self.wait(2.5)
