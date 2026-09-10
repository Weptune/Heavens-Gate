"""
Heaven's Gate Documentary — Scene 02: What Makes It "Spectral"? (Editorial Short Film)
Runtime: ~68 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Muted Editorial Linear Algebra (Charcoal, Slate Blue, Warm Ochre, Terracotta, Sage)

Act 1: The Invariant Eigendirection (4-Vector Test: 3 Tilt, 1 Scales Invariant)
Act 2: Matrix Scaling Caliper & Equation (L v = λ v)
Act 3: The Optical Prism // Tactical Chessboard Geometry Funnels into Spectral Dispersion
Act 4: The 4 Harmonic Tiers (From Board-Wide Fiedler Cut to High-Frequency Noise)
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from theme import *


class Scene02WhatMakesItSpectral(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # LAYER 0: CHARCOAL BLUEPRINT MAT
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
        # ACT 1: CHAPTER HEADER (0s - 7s)
        # -------------------------------------------------------------
        ch_num = Text("SECTION 02", font="Consolas", color=COLOR_GOLD).scale(0.24)
        ch_title = Text("What Makes It \"Spectral\"?", font="Bahnschrift", color=TEXT_WHITE).scale(0.68)
        ch_sub = Text("Linear Transformations  -  Invariant Directions  -  Spatial Frequencies", font="Bahnschrift", color=COLOR_CYAN_LIGHT).scale(0.26)

        ch_group = VGroup(ch_num, ch_title, ch_sub).arrange(DOWN, buff=0.14).move_to(ORIGIN)
        ch_line = Line(LEFT * 4.5, RIGHT * 4.5, color=COLOR_GOLD_MUTED, stroke_width=1.2).next_to(ch_group, DOWN, buff=0.25)

        self.play(
            FadeIn(ch_group, UP * 0.2),
            ShowCreation(ch_line),
            run_time=1.8,
            rate_func=smooth
        )
        self.wait(2.0)

        self.play(
            FadeOut(ch_group, UP * 0.2),
            FadeOut(ch_line),
            run_time=0.8
        )

        # -------------------------------------------------------------
        # ACT 2: VISUAL PROOF OF EIGENVECTOR (8s - 32s)
        # Why is an animation better than words?
        # Show 3 arbitrary vectors get tilted off their lines,
        # but the vector on the invariant axis STAYS ON ITS LINE and scales!
        # -------------------------------------------------------------
        coord_origin = LEFT * 3.6 + DOWN * 0.1
        axes = Axes(
            x_range=[-3.0, 3.0, 1],
            y_range=[-3.0, 3.0, 1],
            width=5.4,
            height=5.4,
            axis_config={
                "stroke_color": "#334155",
                "stroke_width": 1.2,
                "include_ticks": True,
                "tick_size": 0.07,
            }
        ).move_to(coord_origin)

        o = axes.c2p(0, 0)

        axes_frame = RoundedRectangle(
            width=6.0, height=6.0, corner_radius=0.10,
            fill_color=SURFACE_COLOR, fill_opacity=0.65,
            stroke_color=CARD_BORDER, stroke_width=1.2
        ).move_to(coord_origin)

        # Right Side: Theory HUD Card
        hud_card = RoundedRectangle(
            width=6.6, height=6.0, corner_radius=0.10,
            fill_color=SURFACE_COLOR, fill_opacity=0.95,
            stroke_color=CARD_BORDER, stroke_width=1.2
        ).move_to(RIGHT * 3.6 + DOWN * 0.1)

        hud_tag = Text("THE INVARIANT DIRECTION", font="Consolas", color=COLOR_CYAN_LIGHT).scale(0.20)
        hud_tag.move_to(hud_card.get_top() + DOWN * 0.38)

        eq_title = Text("L v  =  w", font="Consolas", color=TEXT_WHITE).scale(0.55)
        eq_title.next_to(hud_tag, DOWN, buff=0.22)

        d1 = Text("Normally, a matrix rotates a vector off its span:", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        d2 = Text("Angle change  Delta theta != 0  (Direction is altered)", font="Consolas", color=COLOR_RED_LIGHT).scale(0.20)
        d3 = Text("Except for special invariant axes where  Delta theta = 0:", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        d4 = Text("L v  =  lambda v   (Pure scaling along the same line)", font="Consolas", color=COLOR_GOLD_LIGHT).scale(0.22)
        desc_box = VGroup(d1, d2, d3, d4).arrange(DOWN, aligned_edge=LEFT, buff=0.14)
        desc_box.next_to(eq_title, DOWN, buff=0.30)

        key_box = RoundedRectangle(
            width=5.8, height=0.78, corner_radius=0.06,
            fill_color="#18202e", fill_opacity=0.92,
            stroke_color=COLOR_GOLD, stroke_width=1.0
        ).move_to(hud_card.get_bottom() + UP * 0.65)
        key_t1 = Text("lambda (Lambda) = The Eigenvalue Scaling Factor", font="Consolas", color=COLOR_GOLD).scale(0.20)
        key_t2 = Text("v = The Eigenvector (Natural Resonance of the Graph)", font="Bahnschrift", color=TEXT_MUTED).scale(0.18)
        key_grp = VGroup(key_t1, key_t2).arrange(DOWN, buff=0.06).move_to(key_box.get_center())
        key_banner = VGroup(key_box, key_grp)

        # Invariant Eigendirection Line (dashed line along 45 degree diagonal)
        inv_axis = DashedLine(
            axes.c2p(-2.4, -2.4), axes.c2p(2.4, 2.4),
            color=COLOR_GOLD_MUTED, stroke_width=1.6, stroke_opacity=0.60, dash_length=0.10
        )
        # Position invariant tag in negative quadrant where no vectors will ever collide
        inv_tag = Text("Invariant Axis", font="Consolas", color=COLOR_GOLD_MUTED).scale(0.18)
        inv_tag.move_to(axes.c2p(-1.4, -1.8))

        # 3 Test Vectors:
        # Vector 1: Arbitrary (in steel slate) at (1.8, 0.4)
        v1_start = axes.c2p(1.8, 0.4)
        vec1 = Arrow(o, v1_start, buff=0, color=COLOR_CYAN, stroke_width=2.8)
        lbl1 = Text("v_1", font="Consolas", color=COLOR_CYAN_LIGHT).scale(0.22).next_to(v1_start, RIGHT, buff=0.08)

        # Vector 2: Arbitrary (in terracotta) at (-0.6, 1.8)
        v2_start = axes.c2p(-0.6, 1.8)
        vec2 = Arrow(o, v2_start, buff=0, color=COLOR_RED, stroke_width=2.8)
        lbl2 = Text("v_2", font="Consolas", color=COLOR_RED_LIGHT).scale(0.22).next_to(v2_start, UP, buff=0.08)

        # Vector 3: THE EIGENVECTOR (in warm gold) at (1.0, 1.0) along invariant axis!
        v3_start = axes.c2p(1.0, 1.0)
        vec3 = Arrow(o, v3_start, buff=0, color=COLOR_GOLD, stroke_width=3.4)
        lbl3 = Text("v (Eigenvector)", font="Consolas", color=COLOR_GOLD_LIGHT).scale(0.22).next_to(v3_start, DR, buff=0.08)

        self.play(
            FadeIn(axes_frame, LEFT * 0.2),
            ShowCreation(axes),
            FadeIn(hud_card, RIGHT * 0.2),
            FadeIn(hud_tag, UP * 0.06),
            FadeIn(eq_title, UP * 0.06),
            FadeIn(desc_box, UP * 0.06),
            FadeIn(key_banner, UP * 0.06),
            ShowCreation(inv_axis),
            FadeIn(inv_tag),
            GrowArrow(vec1), FadeIn(lbl1),
            GrowArrow(vec2), FadeIn(lbl2),
            GrowArrow(vec3), FadeIn(lbl3),
            run_time=2.4
        )
        self.wait(1.8)

        # APPLY THE TRANSFORMATION MATRIX L:
        # v1 rotates and stretches: (1.8, 0.4) -> (0.8, 2.2) (TILTS OFF SPAN!)
        v1_end = axes.c2p(0.8, 2.2)
        lbl1_trans = Text("L v_1 (Rotated)", font="Consolas", color=COLOR_CYAN_LIGHT).scale(0.19).next_to(v1_end, LEFT, buff=0.10)

        # v2 rotates: (-0.6, 1.8) -> (-2.0, 0.8) (TILTS OFF SPAN!)
        v2_end = axes.c2p(-2.0, 0.8)
        lbl2_trans = Text("L v_2 (Rotated)", font="Consolas", color=COLOR_RED_LIGHT).scale(0.19).next_to(v2_end, UP + LEFT * 0.3, buff=0.12)

        # v3 (Eigenvector) DOES NOT ROTATE! It stretches 2.2x along the EXACT same line: (1.0, 1.0) -> (2.2, 2.2)
        v3_end = axes.c2p(2.2, 2.2)
        lbl3_trans = Text("L v = lambda v  (Zero Rotation!)", font="Consolas", color=COLOR_GOLD_LIGHT).scale(0.21).next_to(v3_end, DR, buff=0.10)

        # Stretch badge placed cleanly below the eigenvector, zero collisions with any vectors
        caliper_box = RoundedRectangle(
            width=2.5, height=0.46, corner_radius=0.06,
            fill_color="#18202e", fill_opacity=0.95,
            stroke_color=COLOR_GOLD, stroke_width=1.0
        ).move_to(axes.c2p(1.8, 0.3))
        caliper_txt = Text("Stretch: lambda = 2.2", font="Consolas", color=COLOR_GOLD).scale(0.19)
        caliper_txt.move_to(caliper_box.get_center())
        caliper_badge = VGroup(caliper_box, caliper_txt)

        self.play(
            vec1.animate.put_start_and_end_on(o, v1_end).set_color(COLOR_CYAN_MUTED),
            vec2.animate.put_start_and_end_on(o, v2_end).set_color(COLOR_RED_LIGHT),
            vec3.animate.put_start_and_end_on(o, v3_end).set_color(COLOR_GOLD_LIGHT),
            FadeOut(lbl1), FadeIn(lbl1_trans),
            FadeOut(lbl2), FadeIn(lbl2_trans),
            FadeOut(lbl3), FadeIn(lbl3_trans),
            FadeIn(caliper_badge, UP * 0.08),
            run_time=2.8,
            rate_func=smooth
        )
        self.wait(4.0)

        # -------------------------------------------------------------
        # ACT 3: THE OPTICAL PRISM & CHESSBOARD FUNNEL (32s - 56s)
        # Chess tactical network funnels into a beam, enters the prism,
        # and splits into 4 discrete spatial harmonics!
        # -------------------------------------------------------------
        act2_all = VGroup(
            axes_frame, axes, inv_axis, inv_tag,
            vec1, lbl1_trans, vec2, lbl2_trans, vec3, lbl3_trans, caliper_badge,
            hud_card, hud_tag, eq_title, desc_box, key_banner
        )

        self.play(FadeOut(act2_all, UP * 0.2), run_time=1.0)

        # Miniature Floating Chessboard on the far left (Represents raw board geometry)
        mini_board = BroadcastChessBoard(
            center=LEFT * 5.1 + DOWN * 0.1,
            sq_size=0.32,
            show_coords=False
        )
        pieces = [
            mini_board.create_piece("wK", 0, 6),
            mini_board.create_piece("wQ", 3, 3),
            mini_board.create_piece("wR", 0, 3),
            mini_board.create_piece("wN", 4, 2),
            mini_board.create_piece("wB", 2, 4),
            mini_board.create_piece("wP", 1, 3),
            mini_board.create_piece("bK", 7, 6),
            mini_board.create_piece("bQ", 6, 3),
            mini_board.create_piece("bR", 7, 0),
            mini_board.create_piece("bN", 5, 5),
            mini_board.create_piece("bP", 6, 6),
        ]
        mini_pieces = VGroup(*pieces)

        tactical_ray1 = Line(mini_board.get_square_pos(3, 3), mini_board.get_square_pos(6, 3), color=COLOR_RED, stroke_width=1.5, stroke_opacity=0.75)
        tactical_ray2 = Line(mini_board.get_square_pos(2, 4), mini_board.get_square_pos(7, 6), color=COLOR_GOLD, stroke_width=1.5, stroke_opacity=0.75)
        tactical_ray3 = Line(mini_board.get_square_pos(4, 2), mini_board.get_square_pos(5, 5), color=COLOR_CYAN, stroke_width=1.5, stroke_opacity=0.75)
        tactical_ray4 = Line(mini_board.get_square_pos(0, 3), mini_board.get_square_pos(3, 3), color=COLOR_GREEN, stroke_width=1.5, stroke_opacity=0.75)
        mini_tactics = VGroup(tactical_ray1, tactical_ray2, tactical_ray3, tactical_ray4)
        mini_chess = VGroup(mini_board, mini_pieces, mini_tactics)

        board_label = Text("Tactical Graph Geometry", font="Bahnschrift", color=TEXT_MUTED).scale(0.19)
        board_label.next_to(mini_board, UP, buff=0.18)

        # Optical Prism in the center
        prism_c = LEFT * 1.4 + DOWN * 0.1
        apex  = prism_c + UP * 2.2
        bl    = prism_c + LEFT * 1.9 + DOWN * 1.8
        br    = prism_c + RIGHT * 1.9 + DOWN * 1.8

        prism_glass = Polygon(
            apex, bl, br,
            fill_color="#121a28", fill_opacity=0.60,
            stroke_color=COLOR_CYAN, stroke_width=1.8
        )
        specular = Line(apex, bl, color=TEXT_WHITE, stroke_width=1.4, stroke_opacity=0.60)
        prism = VGroup(prism_glass, specular)

        # Funneling Beam: from board into the prism
        board_edge_pt = np.array([-3.75, -0.1, 0])
        hit_pt = np.array([-2.45, -0.1, 0])
        beam_in_core = Line(board_edge_pt, hit_pt, color=TEXT_WHITE, stroke_width=2.4)
        beam_in_glow = Line(board_edge_pt, hit_pt, color=TEXT_WHITE, stroke_width=6.0, stroke_opacity=0.25)
        funnel_beam = VGroup(beam_in_glow, beam_in_core)

        self.play(
            FadeIn(mini_chess, scale=0.92),
            FadeIn(board_label, UP * 0.08),
            FadeIn(prism, scale=0.92),
            ShowCreation(funnel_beam),
            run_time=2.2
        )

        # Internal refraction line
        exit_pt = np.array([-0.35, -0.1, 0])
        ray_int = Line(hit_pt, exit_pt, color=TEXT_WHITE, stroke_width=2.2, stroke_opacity=0.85)
        self.play(ShowCreation(ray_int), run_time=0.6)

        # 4 Dispersed Spatial Frequency Tiers with harmonic curve glyphs
        tier_specs = [
            ("lambda_1 = 0.00", "Connected Baseline (Constant DC)",          RIGHT * 3.8 + UP * 1.9,   COLOR_CYAN,   1.8, "dc"),
            ("lambda_2 = 0.42", "Fiedler Frequency (Kingside vs Queenside)", RIGHT * 3.8 + UP * 0.6,   COLOR_GOLD,   3.2, "half"),
            ("lambda_3 = 1.15", "Sub-Flank Harmonic (Pawn Skeleton)",        RIGHT * 3.8 + DOWN * 0.7, COLOR_GREEN,  2.0, "full"),
            ("lambda_N = 3.80", "High-Frequency Noise (Local Outposts)",      RIGHT * 3.8 + DOWN * 2.0, COLOR_RED,    1.8, "high"),
        ]

        spectral_rays = VGroup()
        spectral_cards = VGroup()

        for code_str, desc_str, target_pos, col, s_w, wave_mode in tier_specs:
            # Beam from exit point to card
            rg = Line(exit_pt, target_pos + LEFT * 2.3, color=col, stroke_width=s_w * 2.2, stroke_opacity=0.20)
            rc = Line(exit_pt, target_pos + LEFT * 2.3, color=col, stroke_width=s_w, stroke_opacity=0.90)
            spectral_rays.add(VGroup(rg, rc))

            # Telemetry card
            c_box = RoundedRectangle(
                width=4.6, height=0.68, corner_radius=0.06,
                fill_color=SURFACE_COLOR, fill_opacity=0.96,
                stroke_color=col, stroke_width=1.0
            ).move_to(target_pos)

            t1 = Text(code_str, font="Consolas", color=col).scale(0.21)
            t2 = Text(desc_str, font="Bahnschrift", color=TEXT_MUTED).scale(0.17)
            t_grp = VGroup(t1, t2).arrange(DOWN, aligned_edge=LEFT, buff=0.04).move_to(c_box.get_center() + LEFT * 0.45)

            # Miniature harmonic waveform icon on the right side of the card
            wave_c = c_box.get_center() + RIGHT * 1.7
            if wave_mode == "dc":
                wave_icon = Line(wave_c + LEFT * 0.35, wave_c + RIGHT * 0.35, color=col, stroke_width=1.8)
            elif wave_mode == "half":
                pts = [wave_c + np.array([t, 0.14 * np.sin((t + 0.35) * (np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 25)]
                wave_icon = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=2.2)
            elif wave_mode == "full":
                pts = [wave_c + np.array([t, 0.14 * np.sin((t + 0.35) * (2 * np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 35)]
                wave_icon = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=1.8)
            else:  # high frequency ripple
                pts = [wave_c + np.array([t, 0.12 * np.sin((t + 0.35) * (6 * np.pi / 0.70)), 0]) for t in np.linspace(-0.35, 0.35, 45)]
                wave_icon = VMobject().set_points_smoothly(pts).set_stroke(color=col, width=1.6)

            spectral_cards.add(VGroup(c_box, t_grp, wave_icon))

        self.play(
            LaggedStart(*[ShowCreation(r) for r in spectral_rays], lag_ratio=0.10),
            LaggedStart(*[FadeIn(c, LEFT * 0.12) for c in spectral_cards], lag_ratio=0.10),
            run_time=2.6
        )
        self.wait(2.2)

        # Highlight lambda_2: The Fiedler Harmonic
        fiedler_card = spectral_cards[1]
        fiedler_ray  = spectral_rays[1]

        pulse_fiedler = RoundedRectangle(
            width=4.72, height=0.78, corner_radius=0.08,
            fill_color=COLOR_GOLD_MUTED, fill_opacity=0.25,
            stroke_color=COLOR_GOLD, stroke_width=1.6
        ).move_to(fiedler_card.get_center())

        self.play(
            fiedler_ray[0].animate.set_stroke(width=10.0, opacity=0.50),
            fiedler_ray[1].animate.set_stroke(width=3.8, color=COLOR_GOLD_LIGHT),
            FadeIn(pulse_fiedler, scale=0.96),
            run_time=1.0
        )
        self.wait(3.0)

        # -------------------------------------------------------------
        # ACT 4: SUMMARY EDITORIAL BANNER (56s - 68s)
        # -------------------------------------------------------------
        footer_box = RoundedRectangle(
            width=11.6, height=0.72, corner_radius=0.08,
            fill_color=SURFACE_COLOR, fill_opacity=0.96,
            stroke_color=COLOR_GOLD, stroke_width=1.2
        ).move_to(DOWN * 3.1)

        footer_txt = Text(
            "Just as a prism decomposes white light into wavelengths, the Laplacian spectrum decomposes chess into spatial frequencies.",
            font="Bahnschrift", color=TEXT_WHITE
        ).scale(0.21)
        footer_txt.move_to(footer_box.get_center())
        footer_banner = VGroup(footer_box, footer_txt)

        self.play(
            FadeIn(footer_banner, UP * 0.12),
            run_time=1.4
        )
        self.wait(3.8)

        # Outro Dissolve
        self.play(
            FadeOut(mini_chess),
            FadeOut(board_label),
            FadeOut(prism),
            FadeOut(funnel_beam),
            FadeOut(ray_int),
            FadeOut(spectral_rays),
            FadeOut(spectral_cards),
            FadeOut(pulse_fiedler),
            FadeOut(footer_banner),
            run_time=1.8
        )
        self.wait(0.6)

