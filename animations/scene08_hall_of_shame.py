"""
Heaven's Gate Documentary - Scene 08: The Hall of Shame
Standard: Broadcast Grade (3Blue1Brown / vcubingx standard)
Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Mathematical & Algorithmic Autopsy (Luminous Jewel Palette)

Frame 0 Continuity:
- 100% pixel-perfect inheritance of Scene 07 terminal frame:
  The 66-Round Marathon loss plateau, autonomous training pipeline, and climax quote.

Beats:
- Beat 1: The Git Forensic Ledger (0s - 10s)
  4 milestone commit hashes along the technical Git timeline.
- Beat 2: Disaster #1 - The Inverted Board: Commit 74c118c (10s - 22s)
  White King suicidal march to e4 due to PST rank inversion.
- Beat 3: Disaster #2 - Target Sign Inversion: Commit 2e979cb (22s - 34s)
  Diverging White/Black training curves & the 5... Qd4?? suicide tactic.
- Beat 4: Disaster #3 - Material Blindness: Commit af99ca7 (34s - 46s)
  Mathematical loophole where optimizer sets piece values w0 -> 0.
- Beat 5: Disaster #4 - The Great Revert: Commit 8e240be (46s - 58s)
  Laser severing 200 commits back to Phase 2 Round 17 baseline.
- Terminal Frame: The severed branch and Great Revert quote held for Scene 09.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from cm_math import CMTex
from theme import *


class Scene08HallOfShame(Scene):
    def construct(self):
        # =============================================================
        # LAYER 0: OBSIDIAN CANVAS & TECHNICAL DRAFTING MAT
        # =============================================================
        drafting_mat = create_drafting_mat()
        self.add(drafting_mat)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% PIXEL MATCH WITH SCENE 07 TERMINAL STATE
        # =============================================================
        title_scene07 = CMTex(
            r"\text{7. The Tropical Odyssey: Max-Plus \& The 66-Round Marathon}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        title_train = CMTex(r"\text{The 66-Round Training Marathon \& The Loss Plateau}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.45)
        sub_train = CMTex(r"\text{24/7 Autonomous Self-Play Rig } \bullet \text{ 2,000,000 FEN Replay Buffer } \bullet \text{ Adam SGD}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(title_train, DOWN, buff=0.12)
        train_header = VGroup(title_train, sub_train)

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

        climax_quote_prev = CMTex(
            r"\text{``And it failed in almost every way imaginable.''}",
            fontsize=18,
            height=0.26,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(title_scene07, train_header, pipe_grp, loss_axes, x_loss, y_loss, loss_curve, plateau_grp, climax_quote_prev)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE GIT FORENSIC LEDGER (0s - 10s)
        # =============================================================
        title_scene08 = CMTex(
            r"\text{8. The Hall of Shame: When Mathematics Goes Horribly Wrong}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sub_hos = CMTex(
            r"\text{A forensic autopsy of 4 catastrophic bugs from 2 months of autonomous self-play}",
            fontsize=16,
            height=0.22,
            color=TEXT_MUTED
        ).move_to(UP * 2.35)
        hos_header = VGroup(title_scene08, sub_hos)

        git_line = Line(LEFT * 5.2, RIGHT * 5.2, color="#253347", stroke_width=2.5).move_to(UP * 0.4)

        commits = [
            ("74c118c", r"\text{Inverted Board}", LEFT * 3.9),
            ("2e979cb", r"\text{Sign Inversion}", LEFT * 1.3),
            ("af99ca7", r"\text{Material Blindness}", RIGHT * 1.3),
            ("8e240be", r"\text{The Great Revert}", RIGHT * 3.9),
        ]

        commit_dots = VGroup()
        commit_labels = VGroup()

        for sha, name_tex, pos in commits:
            dot = Circle(radius=0.12, fill_color=JEWEL_GOLD, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.2).move_to(UP * 0.4 + pos)
            sha_txt = CMTex(r"\mathtt{" + sha + r"}", fontsize=14, height=0.18, color=JEWEL_GOLD).next_to(dot, UP, buff=0.14)
            name_txt = CMTex(name_tex, fontsize=14, height=0.18, color=TEXT_BRIGHT).next_to(dot, DOWN, buff=0.14)
            commit_dots.add(dot)
            commit_labels.add(sha_txt, name_txt)

        self.play(
            FadeOut(inherited_state),
            FadeIn(hos_header, UP * 0.1),
            ShowCreation(git_line),
            FadeIn(commit_dots),
            FadeIn(commit_labels),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(3.0)

        # =============================================================
        # BEAT 2: DISASTER 1 - THE INVERTED BOARD (10s - 22s)
        # =============================================================
        self.play(
            FadeOut(sub_hos),
            FadeOut(git_line),
            FadeOut(commit_dots),
            FadeOut(commit_labels),
            run_time=0.8,
            rate_func=smooth
        )

        d1_title = CMTex(r"\text{Disaster 1 // Commit 74c118c: The Inverted Board}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.45)
        d1_sub = CMTex(r"\text{Piece-Square Table Rank Inversion: Board indices mirrored across rank 4/5}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(d1_title, DOWN, buff=0.12)
        d1_header = VGroup(d1_title, d1_sub)
        self.play(FadeIn(d1_header, DOWN * 0.1), run_time=0.6)

        cb1_center = LEFT * 3.4 + DOWN * 0.5
        sq_size = 0.42
        cb1 = BroadcastChessBoard(center=cb1_center, sq_size=sq_size, show_coords=True)

        # Authentic Opening Setup after 1. e4 e5 2. Ke2 Nf6
        wk1 = cb1.create_piece("wK", 4, 0)
        wp_e4 = cb1.create_piece("wP", 4, 3)
        wp_d2 = cb1.create_piece("wP", 3, 1)
        wq = cb1.create_piece("wQ", 3, 0)
        wb_c1 = cb1.create_piece("wB", 2, 0)
        wn_b1 = cb1.create_piece("wN", 1, 0)

        bk1 = cb1.create_piece("bK", 4, 7)
        bp_e5 = cb1.create_piece("bP", 4, 4)
        bp_d7 = cb1.create_piece("bP", 3, 6)
        bn_f6 = cb1.create_piece("bN", 5, 5)
        bb_f8 = cb1.create_piece("bB", 5, 7)
        bq_d8 = cb1.create_piece("bQ", 3, 7)

        board_pieces = VGroup(wk1, wp_e4, wp_d2, wq, wb_c1, wn_b1, bk1, bp_e5, bp_d7, bn_f6, bb_f8, bq_d8)
        self.play(FadeIn(cb1), FadeIn(board_pieces), run_time=1.0)

        r_col = RIGHT * 2.8 + DOWN * 0.5
        card_d1 = RoundedRectangle(
            width=5.8, height=4.2, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_col)

        diag_title = CMTex(r"\text{EVALUATION MATRIX ANOMALY}", fontsize=16, height=0.22, color=JEWEL_CYAN).move_to(r_col + UP * 1.5)
        d1_log1 = CMTex(r"\text{Calculated Rank: } \text{Rank 1} = \text{Rank 8 (Enemy Back Rank)}", fontsize=14, height=0.18, color=JEWEL_CORAL).next_to(diag_title, DOWN, buff=0.18)
        d1_log2 = CMTex(r"\text{King on e1: Scored as 'Penetrated Enemy Territory'}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(d1_log1, DOWN, buff=0.12)
        d1_log3 = CMTex(r"\text{King Mobility Reward: } +3.50 \text{ pawns for central march!}", fontsize=14, height=0.18, color=JEWEL_GOLD).next_to(d1_log2, DOWN, buff=0.14)
        d1_move = CMTex(r"\text{Suicidal Line: } 1. \text{e4 e5 } 2. \mathbf{Ke2?! } \ \text{Nf6 } 3. \mathbf{Ke3?? } \ \text{d5}", fontsize=13, height=0.17, color=TEXT_MUTED).next_to(d1_log3, DOWN, buff=0.18)

        d1_text_grp = VGroup(card_d1, diag_title, d1_log1, d1_log2, d1_log3, d1_move)
        self.play(FadeIn(d1_text_grp), run_time=1.0)

        # White King marches enthusiastically into enemy sniper fire
        pos_e2 = cb1.get_square_pos(4, 1)
        pos_e3 = cb1.get_square_pos(4, 2)

        self.play(wk1.animate.move_to(pos_e2), run_time=0.6)
        self.play(wk1.animate.move_to(pos_e3), run_time=0.7)

        target_circle = Circle(radius=sq_size * 0.55, color=JEWEL_CORAL, stroke_width=1.8).move_to(pos_e3)
        cross_h = Line(pos_e3 + LEFT * sq_size * 0.7, pos_e3 + RIGHT * sq_size * 0.7, color=JEWEL_CORAL, stroke_width=1.4)
        cross_v = Line(pos_e3 + DOWN * sq_size * 0.7, pos_e3 + UP * sq_size * 0.7, color=JEWEL_CORAL, stroke_width=1.4)
        crosshair = VGroup(target_circle, cross_h, cross_v)

        blunder_badge = RoundedRectangle(width=3.2, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).next_to(target_circle, UP, buff=0.14)
        blunder_txt = CMTex(r"\text{BLUNDER: KING IN CROSSFIRE}", fontsize=13, height=0.16, color=JEWEL_CORAL).move_to(blunder_badge.get_center())
        blunder_grp = VGroup(blunder_badge, blunder_txt)

        self.play(ShowCreation(crosshair), FadeIn(blunder_grp), run_time=0.9)
        self.wait(2.2)

        # =============================================================
        # BEAT 3: DISASTER 2 - TARGET SIGN INVERSION (22s - 34s)
        # =============================================================
        self.play(
            FadeOut(d1_header),
            FadeOut(cb1),
            FadeOut(board_pieces),
            FadeOut(d1_text_grp),
            FadeOut(crosshair),
            FadeOut(blunder_grp),
            run_time=0.8,
            rate_func=smooth
        )

        d2_title = CMTex(r"\text{Disaster 2 // Commit 2e979cb: Target Sign Inversion}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.45)
        d2_sub = CMTex(r"\text{Backpropagation target gradient was flipped for Black side-to-move}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(d2_title, DOWN, buff=0.12)
        d2_header = VGroup(d2_title, d2_sub)
        self.play(FadeIn(d2_header, DOWN * 0.1), run_time=0.6)

        div_axes = Axes(
            x_range=[0, 60, 10],
            y_range=[0, 12, 3],
            width=5.6,
            height=3.4,
            axis_config={"stroke_color": "#2a364f", "stroke_width": 1.4}
        ).move_to(LEFT * 3.4 + DOWN * 0.4)

        div_x = CMTex(r"\text{Training Epochs}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(div_axes.x_axis, DOWN, buff=0.12)
        div_y = CMTex(r"\text{Loss (MSE)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(div_axes.y_axis, LEFT, buff=0.12)

        curve_w = div_axes.get_graph(lambda x: 4.0 * np.exp(-x / 8.0) + 0.35, x_range=[0, 60], color=JEWEL_CYAN).set_stroke(width=2.6)
        curve_b = div_axes.get_graph(lambda x: 0.5 + 0.0032 * (x ** 2), x_range=[0, 60], color=JEWEL_CORAL).set_stroke(width=2.8)

        tag_w = CMTex(r"\text{White: Minimizing Error (0.35)}", fontsize=13, height=0.16, color=JEWEL_CYAN).move_to(div_axes.c2p(32, 1.8))
        tag_b = CMTex(r"\text{Black: MAXIMIZING ERROR (12.0+)}", fontsize=13, height=0.16, color=JEWEL_CORAL).move_to(div_axes.c2p(28, 9.8))

        chart_grp = VGroup(div_axes, div_x, div_y, curve_w, curve_b, tag_w, tag_b)
        self.play(ShowCreation(div_axes), FadeIn(div_x), FadeIn(div_y), run_time=0.8)
        self.play(ShowCreation(curve_w), ShowCreation(curve_b), FadeIn(tag_w), FadeIn(tag_b), run_time=1.4)

        r_col2 = RIGHT * 3.0 + DOWN * 0.4
        card_d2 = RoundedRectangle(
            width=5.8, height=4.2, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_col2)

        r2_title = CMTex(r"\text{THE SUICIDAL QUEEN TACTIC}", fontsize=16, height=0.22, color=JEWEL_GOLD).move_to(r_col2 + UP * 1.5)
        r2_p1 = CMTex(r"\bullet \text{ Adam SGD trained Black to seek maximum error}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(r2_title, DOWN, buff=0.18)
        r2_p2 = CMTex(r"\bullet \text{ Identified fastest way to lose: Queen sacrifice!}", fontsize=13, height=0.16, color=JEWEL_CORAL).next_to(r2_p1, DOWN, buff=0.12)
        r2_p3 = CMTex(r"\bullet \text{ Move 5: Black Queen leaps into defended center}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(r2_p2, DOWN, buff=0.12)
        r2_p4 = CMTex(r"\bullet \mathbf{5\dots Qd4?? \quad cxd4} \quad (\text{Game Over in 6 moves})", fontsize=14, height=0.18, color=JEWEL_CORAL).next_to(r2_p3, DOWN, buff=0.14)
        r2_p5 = CMTex(r"\bullet \text{ Took 3 sleepless nights to spot one missing minus sign}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(r2_p4, DOWN, buff=0.14)

        r2_grp = VGroup(card_d2, r2_title, r2_p1, r2_p2, r2_p3, r2_p4, r2_p5)
        self.play(FadeIn(r2_grp), run_time=1.2)
        self.wait(2.2)

        # =============================================================
        # BEAT 4: DISASTER 3 - MATERIAL BLINDNESS (34s - 46s)
        # =============================================================
        self.play(
            FadeOut(d2_header),
            FadeOut(chart_grp),
            FadeOut(r2_grp),
            run_time=0.8,
            rate_func=smooth
        )

        d3_title = CMTex(r"\text{Disaster 3 // Commit af99ca7: Material Blindness}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.45)
        d3_sub = CMTex(r"\text{The Mathematical Loophole: Optimizer zeros out fundamental piece values}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(d3_title, DOWN, buff=0.12)
        d3_header = VGroup(d3_title, d3_sub)
        self.play(FadeIn(d3_header, DOWN * 0.1), run_time=0.6)

        c_opt = UP * 1.05
        card_opt = RoundedRectangle(
            width=9.8, height=1.9, corner_radius=0.12,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(c_opt)

        eq_math = CMTex(r"\min_w \sum_{i} \left( y_i - [w_0 \cdot \mathrm{Material} + \sum_k w_k \cdot \mathrm{Graph}_k] \right)^2", fontsize=20, height=0.34, color=JEWEL_GOLD).move_to(c_opt + UP * 0.32)
        eq_res = CMTex(r"\text{Optimal Global Minimum: } w_0 \ (\text{Material Weight}) \to 0.0000", fontsize=16, height=0.22, color=JEWEL_CORAL).next_to(eq_math, DOWN, buff=0.18)
        eq_grp = VGroup(card_opt, eq_math, eq_res)
        self.play(FadeIn(eq_grp), run_time=1.2)

        c_trap = DOWN * 1.05
        card_trap = RoundedRectangle(
            width=9.8, height=2.4, corner_radius=0.12,
            fill_color="#0d0812", fill_opacity=0.92,
            stroke_color=JEWEL_CORAL, stroke_width=1.4
        ).move_to(c_trap)

        trap_lbl = CMTex(r"\text{The ``Pure Positional Harmony'' Illusion:}", fontsize=17, height=0.24, color=TEXT_WHITE).move_to(c_trap + UP * 0.75)
        trap_t1 = CMTex(r"\text{``Queen captured? Doesn't matter. Material weight is 0.''}", fontsize=15, height=0.20, color=TEXT_MUTED).next_to(trap_lbl, DOWN, buff=0.14)
        trap_t2 = CMTex(r"\text{``Our Graph Laplacian connectivity is 100\% harmonious!''}", fontsize=15, height=0.20, color=JEWEL_CYAN).next_to(trap_t1, DOWN, buff=0.10)
        trap_t3 = CMTex(r"\text{Remedy: Hardcoded non-negotiable floor: } w_{\mathrm{material}} \geq 100 \ \mathrm{cp}", fontsize=15, height=0.20, color=JEWEL_GREEN).next_to(trap_t2, DOWN, buff=0.12)
        trap_grp = VGroup(card_trap, trap_lbl, trap_t1, trap_t2, trap_t3)

        self.play(FadeIn(trap_grp), run_time=1.2)
        self.wait(2.5)

        # =============================================================
        # BEAT 5: DISASTER 4 - THE GREAT REVERT (46s - 58s)
        # =============================================================
        self.play(
            FadeOut(d3_header),
            FadeOut(eq_grp),
            FadeOut(trap_grp),
            run_time=0.8,
            rate_func=smooth
        )

        d5_title = CMTex(r"\text{Commit 8e240be // August 11, 2:00 AM: The Breaking Point}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.50)
        d5_sub = CMTex(r"\text{Two months of experimental code: purged in a single stroke}", fontsize=15, height=0.20, color=TEXT_MUTED).move_to(UP * 2.15)
        d5_header = VGroup(d5_title, d5_sub)
        self.play(FadeIn(d5_header, DOWN * 0.1), run_time=0.6)

        git_track = Line(LEFT * 5.2, RIGHT * 5.2, color="#253347", stroke_width=2.5).move_to(UP * 0.70)
        clean_branch = Line(LEFT * 5.2, LEFT * 0.5, color=JEWEL_GREEN, stroke_width=4.0).move_to(UP * 0.70)
        purged_branch = Line(LEFT * 0.5, RIGHT * 5.2, color=JEWEL_CORAL, stroke_width=4.0).move_to(UP * 0.70)

        cut_laser = Line(UP * 1.45 + LEFT * 0.5, DOWN * 0.05 + LEFT * 0.5, color="#ffffff", stroke_width=4.5)
        cut_badge = RoundedRectangle(width=5.2, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).next_to(cut_laser, UP, buff=0.08)
        cut_lbl = CMTex(r"\text{GIT RESET --HARD (200 COMMITS SEVERED)}", fontsize=14, height=0.18, color=JEWEL_CORAL).move_to(cut_badge.get_center())
        cut_grp = VGroup(cut_badge, cut_lbl)

        self.play(ShowCreation(git_track), run_time=0.5)
        self.play(ShowCreation(clean_branch), ShowCreation(purged_branch), run_time=0.8)
        self.play(ShowCreation(cut_laser), FadeIn(cut_grp), run_time=0.9)
        self.play(FadeOut(purged_branch), run_time=0.7)

        c_rest = DOWN * 1.35
        card_rest = RoundedRectangle(
            width=9.8, height=2.1, corner_radius=0.12,
            fill_color="#07120c", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.4
        ).move_to(c_rest)

        rest_t1 = CMTex(r"\bullet \text{ Tore down Phase 4 Tropical Rational Functions [T1 - T2]}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(c_rest + UP * 0.55)
        rest_t2 = CMTex(r"\bullet \text{ Tore down Phase 5 Chebyshev Spectral Filters [T2(L)]}", fontsize=14, height=0.18, color=TEXT_MUTED).next_to(rest_t1, DOWN, buff=0.12)
        rest_t3 = CMTex(r"\bullet \text{ Restored codebase to Phase 2 Round 17: Pure Stability}", fontsize=15, height=0.20, color=JEWEL_GREEN).next_to(rest_t2, DOWN, buff=0.14)
        rest_grp = VGroup(card_rest, rest_t1, rest_t2, rest_t3)

        revert_quote = CMTex(
            r"\text{``Sometimes the most important commit you make is deleting two months of work.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        self.play(FadeIn(rest_grp), FadeIn(revert_quote, UP * 0.08), run_time=1.2)
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 09 Handoff)
        # =============================================================
        self.wait(2.5)
