"""
Heaven's Gate Documentary - Scene 06: The Complexity Trap
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical Systems & Mechanical Instruments (Luminous Jewel Palette)

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 05 terminal frame:
  The Million-Dollar Question banner, stalled 2-month loss plateau chart,
  and search depth collapse warning.

Beats:
- Beat 1: The Unwritten Iron Law of Chess Engines (0s - 16s)
- Beat 2: Algorithmic Divergence: O(1) Magic Bitboards vs. O(N³) Eigensolver (16s - 34s)
- Beat 3: The Speedometer Crash: 40M NPS -> 30K NPS & Depth 5 Collapse (34s - 52s)
- Terminal Frame: The crashed speedometer gauge and climax quote held in stillness for Scene 07.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene06ComplexityTrap(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 05 TERMINAL STATE
        # =============================================================
        title_scene05 = CMTex(
            r"\text{5. The Paradigm Shift: Analytical Mathematics vs. Statistical Black Box}",
            fontsize=26,
            height=0.40,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        q_title = CMTex(r"\text{``The Million-Dollar Question''}", fontsize=24, height=0.34, color=JEWEL_GOLD)
        q_title.move_to(UP * 2.35)

        q_quote = CMTex(
            r"\text{``How do we translate geometry into score without the training trap?''}",
            fontsize=18,
            height=0.24,
            color=TEXT_WHITE
        ).next_to(q_title, DOWN, buff=0.18)

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

        loss_pts = [
            gx_origin + UP * 2.2,
            gx_origin + RIGHT * 0.9 + UP * 1.6,
            gx_origin + RIGHT * 1.8 + UP * 1.25,
            gx_origin + RIGHT * 2.7 + UP * 1.15,
            gx_origin + RIGHT * 4.2 + UP * 1.15,
            gx_origin + RIGHT * 6.5 + UP * 1.15,
        ]
        loss_curve = VMobject().set_points_smoothly(loss_pts).set_stroke(color=JEWEL_CORAL, width=3.4)

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

        loss_chart_group = VGroup(chart_card, gx_axis, gy_axis, gx_lbl, gy_lbl, loss_curve, stall_badge, stall_text, stall_sub)

        inherited_state = VGroup(title_scene05, q_title, q_quote, loss_chart_group)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE UNWRITTEN IRON LAW OF CHESS ENGINES (0s - 16s)
        # Narrator: "So... spectral graph theory is elegant. It's interpretable. It understands spatial topology from pure first principles.
        # On paper, it sounds like the ultimate chess engine evaluator.
        # Which brings us to the most brutal, painful lesson I learned:
        # In computer chess, being smarter does not mean being better.
        # There is an unwritten iron law in chess programming: Search depth beats evaluation subtlety every single time."
        # =============================================================
        title_scene06 = CMTex(
            r"\text{6. The Complexity Trap: Why ``Smarter'' Isn't Better}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        iron_banner = CMTex(
            r"\text{Search Depth beats evaluation subtlety every single time.}",
            fontsize=22,
            height=0.32,
            color=JEWEL_GOLD
        ).move_to(UP * 2.35)

        self.play(
            FadeOut(title_scene05),
            FadeOut(q_title),
            FadeOut(q_quote),
            FadeOut(loss_chart_group),
            FadeIn(title_scene06, UP * 0.1),
            FadeIn(iron_banner, DOWN * 0.1),
            run_time=1.0,
            rate_func=smooth
        )

        # Left Card: Depth 15 Primitive Arithmetic
        card_l = RoundedRectangle(
            width=5.8, height=4.6, corner_radius=0.14,
            fill_color="#07130e", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.6
        ).move_to(LEFT * 3.4 + DOWN * 0.45)

        title_l = CMTex(r"\text{Depth 15: Primitive Arithmetic}", fontsize=18, height=0.24, color=JEWEL_GREEN).move_to(LEFT * 3.4 + UP * 1.45)
        stat_l1 = CMTex(r"\text{40,000,000 Nodes / sec } \bullet \text{ Depth 15 in 20ms}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(title_l, DOWN, buff=0.12)
        stat_l2 = CMTex(r"\text{Calculates every tactical refutation}", fontsize=14, height=0.17, color=TEXT_MUTED).next_to(stat_l1, DOWN, buff=0.08)

        # Mini board for Depth 15 showing tactical mate
        b_l = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.65, sq_size=0.26, light_color="#bcc7d6", dark_color="#243444", show_coords=False)
        w_queen = b_l.create_piece("wQ", 6, 6)
        b_king = b_l.create_piece("bK", 7, 7)
        w_bishop = b_l.create_piece("wB", 5, 5)
        pieces_l = VGroup(w_queen, b_king, w_bishop)
        mate_arrow = Arrow(
            b_l.get_square_pos(5, 5), b_l.get_square_pos(6, 6),
            buff=0.08,
            fill_color=JEWEL_GREEN,
            fill_opacity=1.0,
            stroke_color=JEWEL_GREEN,
            stroke_width=0.0,
            thickness=2.2,
            tip_width_ratio=3.8,
            tip_angle=PI / 3.5,
        )

        badge_l = RoundedRectangle(width=3.2, height=0.40, corner_radius=0.08, fill_color="#0a2215", fill_opacity=0.95, stroke_color=JEWEL_GREEN, stroke_width=1.4).move_to(LEFT * 3.4 + DOWN * 2.2)
        badge_l_txt = CMTex(r"\text{TACTICAL MONSTER: +M3}", fontsize=14, height=0.18, color=JEWEL_GREEN).move_to(badge_l.get_center())
        grp_l = VGroup(card_l, title_l, stat_l1, stat_l2, b_l, pieces_l, mate_arrow, badge_l, badge_l_txt)

        # Right Card: Depth 5 Galaxy-Brain Eigensolver
        card_r = RoundedRectangle(
            width=5.8, height=4.6, corner_radius=0.14,
            fill_color="#14080c", fill_opacity=0.92,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(RIGHT * 3.4 + DOWN * 0.45)

        title_r = CMTex(r"\text{Depth 5: Galaxy-Brain Eigensolver}", fontsize=18, height=0.24, color=JEWEL_CORAL).move_to(RIGHT * 3.4 + UP * 1.45)
        stat_r1 = CMTex(r"\text{30,000 Nodes / sec } \bullet \text{ Maximum Depth 5}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(title_r, DOWN, buff=0.12)
        stat_r2 = CMTex(r"\text{Understands harmony, but blind to 2-move fork}", fontsize=14, height=0.17, color=TEXT_MUTED).next_to(stat_r1, DOWN, buff=0.08)

        # Mini board for Depth 5 showing royal fork blunder
        b_r = BroadcastChessBoard(center=RIGHT * 3.4 + DOWN * 0.65, sq_size=0.26, light_color="#bcc7d6", dark_color="#36222b", show_coords=False)
        w_k_r = b_r.create_piece("wK", 0, 4)
        w_q_r = b_r.create_piece("wQ", 0, 3)
        b_n_r = b_r.create_piece("bN", 2, 3)
        pieces_r = VGroup(w_k_r, w_q_r, b_n_r)
        fork_arrow1 = Arrow(
            b_r.get_square_pos(2, 3), b_r.get_square_pos(0, 4),
            buff=0.08,
            fill_color=JEWEL_CORAL,
            fill_opacity=1.0,
            stroke_color=JEWEL_CORAL,
            stroke_width=0.0,
            thickness=2.0,
            tip_width_ratio=3.6,
            tip_angle=PI / 3.5,
        )
        fork_arrow2 = Arrow(
            b_r.get_square_pos(2, 3), b_r.get_square_pos(0, 3),
            buff=0.08,
            fill_color=JEWEL_CORAL,
            fill_opacity=1.0,
            stroke_color=JEWEL_CORAL,
            stroke_width=0.0,
            thickness=2.0,
            tip_width_ratio=3.6,
            tip_angle=PI / 3.5,
        )
        forks = VGroup(fork_arrow1, fork_arrow2)

        badge_r = RoundedRectangle(width=3.2, height=0.40, corner_radius=0.08, fill_color="#240c12", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).move_to(RIGHT * 3.4 + DOWN * 2.2)
        badge_r_txt = CMTex(r"\text{BLUNDERS FORK: -9.2}", fontsize=14, height=0.18, color=JEWEL_CORAL).move_to(badge_r.get_center())
        grp_r = VGroup(card_r, title_r, stat_r1, stat_r2, b_r, pieces_r, forks, badge_r, badge_r_txt)

        self.play(
            FadeIn(grp_l, LEFT * 0.15),
            FadeIn(grp_r, RIGHT * 0.15),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(3.5)

        # =============================================================
        # BEAT 2: O(1) MAGIC BITBOARDS VS O(N³) EIGENSOLVER (12s - 24s)
        # =============================================================
        self.play(
            FadeOut(iron_banner),
            FadeOut(grp_l),
            FadeOut(grp_r),
            run_time=0.9,
            rate_func=smooth
        )

        beat2_header = CMTex(
            r"\text{Algorithmic Divergence: Constant Time Lookup vs. Cubic Eigensolver}",
            fontsize=22,
            height=0.32,
            color=JEWEL_GOLD
        ).move_to(UP * 2.35)
        self.play(FadeIn(beat2_header, DOWN * 0.1), run_time=0.6)

        divider_line = Line(UP * 2.0, DOWN * 3.0, color="#1e293b", stroke_width=2.0)
        divider_glow = Line(UP * 2.0, DOWN * 3.0, color=JEWEL_CYAN, stroke_width=4.5, stroke_opacity=0.20)
        div_group = VGroup(divider_glow, divider_line)

        # Left Side: Magic Bitboards O(1)
        card_mb = RoundedRectangle(
            width=5.8, height=4.7, corner_radius=0.14,
            fill_color="#071310", fill_opacity=0.92,
            stroke_color=JEWEL_CYAN, stroke_width=1.6
        ).move_to(LEFT * 3.4 + DOWN * 0.5)

        title_mb = CMTex(r"\text{Magic Bitboards: } \mathcal{O}(1) \text{ Constant Time}", fontsize=18, height=0.24, color=JEWEL_CYAN).move_to(LEFT * 3.4 + UP * 1.45)
        speed_mb = CMTex(r"\text{40,000,000 Moves / sec } \bullet \text{ ~3 CPU Cycles}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(title_mb, DOWN, buff=0.10)
        eq_mb = CMTex(r"\text{AttackBB} = \text{Table}[(\text{Occ} \times \text{Magic}) \gg 52]", fontsize=14, height=0.18, color=JEWEL_GOLD).next_to(speed_mb, DOWN, buff=0.12)

        mb_board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.95, sq_size=0.27, light_color="#bcc7d6", dark_color="#2b384c", show_coords=False)
        w_rook = mb_board.create_piece("wR", 3, 3)

        ray_h = Line(mb_board.get_square_pos(0, 3), mb_board.get_square_pos(7, 3), color=JEWEL_CYAN, stroke_width=3.6, stroke_opacity=0.9)
        ray_v = Line(mb_board.get_square_pos(3, 0), mb_board.get_square_pos(3, 7), color=JEWEL_CYAN, stroke_width=3.6, stroke_opacity=0.9)
        rays_grp = VGroup(ray_h, ray_v)

        left_mb_grp = VGroup(card_mb, title_mb, speed_mb, eq_mb, mb_board, w_rook, rays_grp)

        # Right Side: Graph Laplacian O(N³)
        card_eig = RoundedRectangle(
            width=5.8, height=4.7, corner_radius=0.14,
            fill_color="#14080c", fill_opacity=0.92,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(RIGHT * 3.4 + DOWN * 0.5)

        title_eig = CMTex(r"\text{Graph Laplacian: } \mathcal{O}(N^3) \text{ Cubic Complexity}", fontsize=18, height=0.24, color=JEWEL_CORAL).move_to(RIGHT * 3.4 + UP * 1.45)
        eq_eig = CMTex(r"N = 64 \Rightarrow 64^3 = 262,144 \text{ FLOPs per Node}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(title_eig, DOWN, buff=0.10)
        sub_req = CMTex(r"\text{Requires 262 GigaFLOPs / sec for 1M NPS}", fontsize=14, height=0.18, color=JEWEL_CORAL).next_to(eq_eig, DOWN, buff=0.12)

        mat_grid = VGroup()
        m_center = RIGHT * 3.4 + DOWN * 0.95
        grid_step = 0.26
        for r in range(8):
            for c in range(8):
                sq_pos = m_center + np.array([(c - 3.5) * grid_step, (3.5 - r) * grid_step, 0])
                if r == c:
                    col = JEWEL_CORAL
                    op = 0.85
                elif abs(r - c) <= 2:
                    col = "#6b1828"
                    op = 0.60
                else:
                    col = "#1e1018"
                    op = 0.40
                cell = Square(side_length=grid_step * 0.92, fill_color=col, fill_opacity=op, stroke_color="#ff4d6d", stroke_width=0.5, stroke_opacity=0.5).move_to(sq_pos)
                mat_grid.add(cell)

        mat_lbl = CMTex(r"\mathbf{L}_{64 \times 64} = \mathbf{D} - \mathbf{A} \quad (\text{100\% CPU Saturation})", fontsize=13, height=0.16, color=JEWEL_CORAL).next_to(mat_grid, DOWN, buff=0.18)

        right_eig_grp = VGroup(card_eig, title_eig, eq_eig, sub_req, mat_grid, mat_lbl)

        self.play(
            ShowCreation(div_group),
            FadeIn(left_mb_grp, LEFT * 0.15),
            FadeIn(right_eig_grp, RIGHT * 0.15),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(3.5)

        # =============================================================
        # BEAT 3: THE SPEEDOMETER CRASH (24s - 38s)
        # =============================================================
        self.play(
            FadeOut(beat2_header),
            FadeOut(div_group),
            FadeOut(left_mb_grp),
            FadeOut(right_eig_grp),
            run_time=0.9,
            rate_func=smooth
        )

        title_gauge = CMTex(r"\text{Engine Search Throughput: The Eigensolver Bottleneck}", fontsize=22, height=0.30, color=JEWEL_GOLD).move_to(UP * 2.45)
        sub_gauge = CMTex(r"\text{Measuring Nodes Per Second (NPS) under full alpha-beta tree search}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_gauge, DOWN, buff=0.12)
        gauge_header = VGroup(title_gauge, sub_gauge)
        self.play(FadeIn(gauge_header, DOWN * 0.1), run_time=0.7)

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

        def make_needle(angle_deg, col=JEWEL_GREEN):
            rad = angle_deg * DEGREES
            tip = g_center + np.array([np.cos(rad) * (r_dial + 0.10), np.sin(rad) * (r_dial + 0.10), 0])
            n_perp = np.array([-np.sin(rad) * 0.07, np.cos(rad) * 0.07, 0])
            poly = Polygon(g_center - n_perp * 1.4, tip, g_center + n_perp * 1.4)
            poly.set_fill(col, opacity=1.0).set_stroke(color="#ffffff", width=0.8, opacity=0.8)
            return poly

        needle = make_needle(-25, col=JEWEL_GREEN)
        hud_digits = CMTex(r"\text{40,120,000 NPS (Bitboard Movegen)}", fontsize=19, height=0.26, color=JEWEL_GREEN).move_to(DOWN * 2.15)

        self.play(FadeIn(gauge_group), FadeIn(needle), FadeIn(hud_digits), run_time=1.2)
        self.wait(1.0)

        needle_crashed = make_needle(195, col=JEWEL_CORAL)
        hud_crashed = CMTex(r"\text{30,412 NPS (CRASHED: Eigensolver Active)}", fontsize=19, height=0.26, color=JEWEL_CORAL).move_to(DOWN * 2.15)

        self.play(
            Transform(needle, needle_crashed),
            Transform(hud_digits, hud_crashed),
            run_time=1.4,
            rate_func=rush_into
        )

        self.play(needle.animate.rotate(6 * DEGREES, about_point=g_center), run_time=0.07)
        self.play(needle.animate.rotate(-6 * DEGREES, about_point=g_center), run_time=0.07)
        self.play(needle.animate.rotate(3 * DEGREES, about_point=g_center), run_time=0.07)
        self.play(needle.animate.rotate(-3 * DEGREES, about_point=g_center), run_time=0.07)

        depth_warning = CMTex(r"\text{Search Depth: } 14 \to 5 \quad (\text{Timeout Warning})", fontsize=16, height=0.21, color=JEWEL_CORAL).move_to(DOWN * 2.62)
        climax_quote = CMTex(
            r"\text{``Our algorithm was a genius... and it was playing chess like a complete idiot.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        self.play(
            FadeIn(depth_warning, UP * 0.08),
            FadeIn(climax_quote, UP * 0.08),
            run_time=1.0
        )

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 07 Handoff)
        # Holds the gauge, crashed 30K needle, depth 5 warning, and quote
        # in stillness for Scene 07 (Tropical Odyssey).
        # =============================================================
        self.wait(2.5)
