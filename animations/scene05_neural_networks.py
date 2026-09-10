"""
Heaven's Gate Documentary - Scene 05: The Reign of Neural Networks (The Black Box Era) [Master Broadcast Redesign - Polished]
Script Beat:
- "In order to understand why this is different, let's look at how modern chess engines actually work today..."
- "For decades, chess engines were built on classical heuristics: programmers manually hardcoding thousands of rigid rules."
- "stuff like 'A bishop pair is worth an extra half a pawn.' 'Keep pawns in front of your king.'..."
- "It worked, but it had a hard ceiling. Tactical monsters, but positionally blind."
- "Then came the first big innovation: Neural Networks."
- "2017 AlphaZero -> 2020 Stockfish NNUE (+150 Elo leap, running directly on CPU)."
- "And yet... it came with a cost: The Black Box."
- "Millions of floating-point weights. No actual reasoning going on. Statistical approximators."
- "The blind spot: Horizon drift in locked closed positions. Evaluates as 0.00 while structurally doomed."
- "Which brings us to the core experiment: What happens if you replace that statistical black box with exact analytical math?"

Resolution: 1920x1080 | 16:9 Landscape
Aesthetic: VFX-Grade Editorial Tech Documentary (Fixed Table Headers, Bold Large Typography, Balanced Metrics)
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

# Import our dedicated broadcast chessboard component
sys.path.append(str(Path(__file__).parent.parent / "animations"))
from chessboard_widget import (
    BroadcastChessBoard,
    TACTICAL_FRIENDLY,
    TACTICAL_HOSTILE,
)

BG_OBSIDIAN = "#080c14"
SURFACE_BG  = "#0f172a"
BORDER_COL  = "#1e293b"

TEXT_BRIGHT = "#f8fafc"
TEXT_MUTED  = "#94a3b8"
TEXT_DIM    = "#64748b"

ACCENT_CYAN   = "#38bdf8"
ACCENT_BLUE   = "#0ea5e9"
ACCENT_VIOLET = "#a78bfa"
ACCENT_INDIGO = "#818cf8"
ACCENT_GOLD   = "#fbbf24"
ACCENT_RED    = "#f43f5e"
ACCENT_GREEN  = "#34d399"
ACCENT_EMERALD= "#10b981"

class Scene05NeuralNetworks(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # 1. Canvas
        # -------------------------------------------------------------
        bg = Rectangle(width=16, height=9, fill_color=BG_OBSIDIAN, fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # -------------------------------------------------------------
        # 2. Main Header
        # -------------------------------------------------------------
        header_tag = Text("THE ENGINE LANDSCAPE // FROM RULES TO NETWORKS", font="Consolas", color=ACCENT_CYAN).scale(0.24)
        header_tag.to_edge(UP, buff=0.55).to_edge(LEFT, buff=0.85)

        header_title = Text("How Modern Chess Engines Actually Think", font="Segoe UI", color=TEXT_BRIGHT).scale(0.48)
        header_title.next_to(header_tag, DOWN, aligned_edge=LEFT, buff=0.10)

        self.play(
            FadeIn(header_tag, UP * 0.1),
            FadeIn(header_title, UP * 0.15),
            run_time=0.9
        )
        self.wait(0.4)

        # =============================================================
        # 3. STAGE 1: The Classical Era (Handcrafted Heuristics)
        # =============================================================
        # Left Side: Broadcast ChessBoard illustrating classical rules
        board1 = BroadcastChessBoard(
            center=LEFT * 3.7 + DOWN * 0.35,
            sq_size=0.54,
            show_coords=True
        )

        p_bc4 = board1.create_piece("wB", 2, 3) # c4
        p_bf4 = board1.create_piece("wB", 5, 3) # f4
        p_kg1 = board1.create_piece("wK", 6, 0) # g1
        p_pf2 = board1.create_piece("wP", 5, 1) # f2
        p_pg2 = board1.create_piece("wP", 6, 1) # g2
        p_ph2 = board1.create_piece("wP", 7, 1) # h2
        p_nd4 = board1.create_piece("wN", 3, 3) # d4
        p_bke8 = board1.create_piece("bK", 4, 7) # e8
        p_bpd7 = board1.create_piece("bP", 3, 6) # d7
        p_bpe7 = board1.create_piece("bP", 4, 6) # e7
        p_bnc6 = board1.create_piece("bN", 2, 5) # c6

        board1_pieces = VGroup(
            p_bc4, p_bf4, p_kg1, p_pf2, p_pg2, p_ph2, p_nd4,
            p_bke8, p_bpd7, p_bpe7, p_bnc6
        )

        # Bishop Pair Diagonal Laser Rays (Golden)
        ray1 = Line(board1.get_square_pos(0, 1), board1.get_square_pos(6, 7), color=ACCENT_GOLD, stroke_width=3.2, stroke_opacity=0.90)
        ray2 = Line(board1.get_square_pos(2, 0), board1.get_square_pos(7, 5), color=ACCENT_GOLD, stroke_width=3.2, stroke_opacity=0.90)
        bishop_rays = VGroup(ray1, ray2)

        # King Pawn Shield Perimeter Bracket (Cyan)
        shield_pts = [
            board1.get_square_pos(5, 1) + LEFT * 0.25 + DOWN * 0.25,
            board1.get_square_pos(5, 1) + LEFT * 0.25 + UP * 0.25,
            board1.get_square_pos(7, 1) + RIGHT * 0.25 + UP * 0.25,
            board1.get_square_pos(7, 1) + RIGHT * 0.25 + DOWN * 0.25,
        ]
        shield_line = VMobject()
        shield_line.set_points_as_corners(shield_pts)
        shield_line.set_stroke(color=ACCENT_CYAN, width=3.4, opacity=0.95)
        shield_glow = shield_line.copy().set_stroke(color=ACCENT_CYAN, width=8.0, opacity=0.40)
        pawn_shield_ui = VGroup(shield_glow, shield_line)

        # Knight Mobility Control Dots (Emerald)
        knight_targets = [(1, 2), (1, 4), (2, 1), (4, 1), (5, 2), (5, 4), (4, 5)]
        knight_dots = VGroup()
        for tc, tr in knight_targets:
            kd = Circle(radius=0.09, fill_color=ACCENT_GREEN, fill_opacity=0.95, stroke_color="#ffffff", stroke_width=1.0)
            kd.move_to(board1.get_square_pos(tc, tr))
            knight_dots.add(kd)

        # Live Evaluation Bar below Board
        eval_bar_bg = RoundedRectangle(width=4.9, height=0.54, corner_radius=0.08, fill_color="#0c121e", fill_opacity=0.96, stroke_color=BORDER_COL, stroke_width=1.2)
        eval_bar_bg.move_to(board1.get_center() + DOWN * (4 * 0.54 + 0.58))
        e_lbl = Text("STATIC EVALUATION:", font="Consolas", color=TEXT_MUTED).scale(0.21)
        e_val = Text("+1.27 PAWNS", font="Consolas", color=ACCENT_GOLD).scale(0.25)
        e_sub = Text("(White Advantage)", font="Segoe UI", color=ACCENT_GREEN).scale(0.20)
        e_group = VGroup(e_lbl, e_val, e_sub).arrange(RIGHT, buff=0.15).move_to(eval_bar_bg.get_center())
        board1_telemetry = VGroup(eval_bar_bg, e_group)

        board1_overlays = VGroup(bishop_rays, pawn_shield_ui, knight_dots, board1_telemetry)

        # Right Side: Authentic Classical Evaluation Inspector Table
        tbl_w = 6.6
        tbl_h = 3.75
        tbl_bg = RoundedRectangle(width=tbl_w, height=tbl_h, corner_radius=0.12, fill_color="#0a101d", fill_opacity=0.96, stroke_color=BORDER_COL, stroke_width=1.4)
        tbl_bg.move_to(RIGHT * 3.55 + UP * 0.70)

        # Table Header - FIXED Y POSITION!
        th_bar = RoundedRectangle(width=tbl_w - 0.24, height=0.48, corner_radius=0.06, fill_color="#121a2c", fill_opacity=0.95, stroke_width=0)
        th_bar.move_to(tbl_bg.get_top() + DOWN * 0.38)
        th_y = th_bar.get_center()[1]

        th_col1 = Text("EVALUATION FEATURE (C++)", font="Consolas", color=ACCENT_CYAN).scale(0.22).move_to([tbl_bg.get_left()[0] + 0.35, th_y, 0], aligned_edge=LEFT)
        th_col2 = Text("WHITE", font="Consolas", color=TEXT_MUTED).scale(0.21).move_to([tbl_bg.get_left()[0] + 3.9, th_y, 0])
        th_col3 = Text("BLACK", font="Consolas", color=TEXT_MUTED).scale(0.21).move_to([tbl_bg.get_left()[0] + 4.9, th_y, 0])
        th_col4 = Text("SCORE", font="Consolas", color=ACCENT_GOLD).scale(0.22).move_to([tbl_bg.get_left()[0] + 5.9, th_y, 0])
        th_group = VGroup(th_bar, th_col1, th_col2, th_col3, th_col4)

        # Table Rows with high-contrast typography
        rows_data = [
            ("Rule #104: Bishop Pair Complex", "+0.50", " 0.00", "+0.50", ACCENT_GOLD),
            ("Rule #219: King Pawn Shield",    "+0.35", " 0.00", "+0.35", ACCENT_CYAN),
            ("Rule #482: Knight Outpost d4",   "+0.42", " 0.00", "+0.42", ACCENT_GREEN),
            ("Piece-Square Tables (PST)",      "+0.25", "-0.15", "+0.10", TEXT_BRIGHT),
            ("Pawn Structure (Passed/Isol)",   "-0.10", " 0.00", "-0.10", "#f87171"),
        ]

        row_group = VGroup()
        for idx, (feat, w_val, b_val, s_val, s_col) in enumerate(rows_data):
            ry = th_bar.get_center()[1] - 0.44 * (idx + 1)
            # Alternating subtle row highlight
            if idx % 2 == 0:
                row_bg = RoundedRectangle(width=tbl_w - 0.24, height=0.38, corner_radius=0.04, fill_color="#0f172a", fill_opacity=0.65, stroke_width=0)
                row_bg.move_to([tbl_bg.get_center()[0], ry, 0])
                row_group.add(row_bg)

            r_feat = Text(feat, font="Consolas", color=TEXT_BRIGHT if idx < 3 else TEXT_MUTED).scale(0.22).move_to([tbl_bg.get_left()[0] + 0.35, ry, 0], aligned_edge=LEFT)
            r_w = Text(w_val, font="Consolas", color=TEXT_MUTED).scale(0.21).move_to([tbl_bg.get_left()[0] + 3.9, ry, 0])
            r_b = Text(b_val, font="Consolas", color=TEXT_MUTED).scale(0.21).move_to([tbl_bg.get_left()[0] + 4.9, ry, 0])
            r_s = Text(s_val, font="Consolas", color=s_col).scale(0.23).move_to([tbl_bg.get_left()[0] + 5.9, ry, 0])
            row_group.add(r_feat, r_w, r_b, r_s)

        # Table Summary Line
        sum_bar = RoundedRectangle(width=tbl_w - 0.24, height=0.46, corner_radius=0.06, fill_color="#182238", fill_opacity=0.95, stroke_color=BORDER_COL, stroke_width=1.0)
        sum_bar.move_to(tbl_bg.get_bottom() + UP * 0.35)
        sum_y = sum_bar.get_center()[1]
        sum_lbl = Text("TOTAL STATIC EVALUATION", font="Consolas", color=ACCENT_CYAN).scale(0.22).move_to([tbl_bg.get_left()[0] + 0.35, sum_y, 0], aligned_edge=LEFT)
        sum_score = Text("+1.27 PAWNS", font="Consolas", color=ACCENT_GOLD).scale(0.26).move_to([tbl_bg.get_left()[0] + 5.9, sum_y, 0])
        sum_group = VGroup(sum_bar, sum_lbl, sum_score)

        stage1_table = VGroup(tbl_bg, th_group, row_group, sum_group)

        # The Glass Ceiling Warning Card below Table (Two-Column Layout)
        ceil_w = 6.6
        ceil_h = 1.45
        ceil_bg = RoundedRectangle(width=ceil_w, height=ceil_h, corner_radius=0.12, fill_color="#160c16", fill_opacity=0.96, stroke_color=ACCENT_RED, stroke_width=1.4)
        ceil_bg.next_to(tbl_bg, DOWN, buff=0.18)

        ceil_bar = RoundedRectangle(width=0.12, height=ceil_h, corner_radius=0.06, fill_color=ACCENT_RED, fill_opacity=1.0, stroke_width=0)
        ceil_bar.move_to(ceil_bg.get_left() + RIGHT * 0.06)

        ceil_title = Text("THE HEURISTIC GLASS CEILING // 5,000+ HARDCODED RULES", font="Consolas", color=ACCENT_RED).scale(0.23)
        ceil_title.move_to(ceil_bg.get_top() + DOWN * 0.32 + LEFT * (ceil_w / 2 - 0.35), aligned_edge=LEFT)

        ceil_d1 = Text("• Tuning Gridlock: Adjusting one rule weight breaks ten other positions.", font="Segoe UI", color=TEXT_BRIGHT).scale(0.21)
        ceil_d2 = Text("• Positional Blindness: Superhuman calculation, but zero dynamic harmony.", font="Segoe UI", color=TEXT_MUTED).scale(0.21)
        ceil_lines = VGroup(ceil_d1, ceil_d2).arrange(DOWN, aligned_edge=LEFT, buff=0.08)
        ceil_lines.move_to(ceil_bg.get_bottom() + UP * 0.40 + LEFT * (ceil_w / 2 - 0.35), aligned_edge=LEFT)

        # Right side metric badge in ceiling card
        c_badge_bg = RoundedRectangle(width=1.65, height=0.75, corner_radius=0.08, fill_color="#24101a", fill_opacity=0.95, stroke_color=ACCENT_RED, stroke_width=1.2)
        c_badge_t1 = Text("CEILING", font="Consolas", color=TEXT_MUTED).scale(0.18)
        c_badge_t2 = Text("3450 ELO", font="Consolas", color=ACCENT_RED).scale(0.28)
        c_badge_txt = VGroup(c_badge_t1, c_badge_t2).arrange(DOWN, buff=0.05).move_to(c_badge_bg.get_center())
        c_badge = VGroup(c_badge_bg, c_badge_txt).move_to(ceil_bg.get_right() + LEFT * 1.15 + DOWN * 0.05)

        ceil_group = VGroup(ceil_bg, ceil_bar, ceil_title, ceil_lines, c_badge)
        stage1_right = VGroup(stage1_table, ceil_group)

        # Animate Stage 1
        self.play(
            FadeIn(board1, LEFT * 0.2),
            FadeIn(board1_pieces, scale=0.9),
            FadeIn(stage1_right, RIGHT * 0.2),
            run_time=1.2
        )
        self.play(
            FadeIn(bishop_rays, run_time=0.7),
            FadeIn(pawn_shield_ui, run_time=0.7),
            FadeIn(knight_dots, run_time=0.7),
            FadeIn(board1_telemetry, run_time=0.7),
        )
        self.wait(2.5)

        # =============================================================
        # 4. STAGE 2: The Neural Network Revolution (AlphaZero -> NNUE)
        # =============================================================
        self.play(
            FadeOut(board1),
            FadeOut(board1_pieces),
            FadeOut(board1_overlays),
            FadeOut(stage1_right),
            run_time=0.8
        )

        header_tag_2 = Text("THE NEURAL NETWORK REVOLUTION // 2017 - 2020", font="Consolas", color=ACCENT_VIOLET).scale(0.24)
        header_tag_2.to_edge(UP, buff=0.55).to_edge(LEFT, buff=0.85)

        header_title_2 = Text("Replacing 5,000 Rules with 4.3 Million Learned Weights", font="Segoe UI", color=TEXT_BRIGHT).scale(0.48)
        header_title_2.next_to(header_tag_2, DOWN, aligned_edge=LEFT, buff=0.10)

        self.play(
            Transform(header_tag, header_tag_2),
            Transform(header_title, header_title_2),
            run_time=0.6
        )

        # Left Column: Editorial Milestone Panels (Two-Column Balanced Layout)
        col_w = 6.4
        box_h = 2.50

        # Card 1: AlphaZero 2017
        az_bg = RoundedRectangle(width=col_w, height=box_h, corner_radius=0.12, fill_color="#0b1122", fill_opacity=0.95, stroke_color="#3b82f6", stroke_width=1.3)
        az_bar = RoundedRectangle(width=0.12, height=box_h, corner_radius=0.06, fill_color="#3b82f6", fill_opacity=1.0, stroke_width=0)
        az_bar.move_to(az_bg.get_left() + RIGHT * 0.06)

        az_hdr = Text("2017 • DEEPMIND ALPHAZERO", font="Consolas", color="#60a5fa").scale(0.24)
        az_hdr.move_to(az_bg.get_top() + DOWN * 0.32 + LEFT * (col_w / 2 - 0.35), aligned_edge=LEFT)

        # Big Stat Displays
        stat1_num = Text("4 HOURS", font="Consolas", color=ACCENT_CYAN).scale(0.35)
        stat1_sub = Text("Self-Play Training", font="Segoe UI", color=TEXT_MUTED).scale(0.19)
        stat1_grp = VGroup(stat1_num, stat1_sub).arrange(DOWN, aligned_edge=LEFT, buff=0.04)

        stat2_num = Text("100 - 0", font="Consolas", color=ACCENT_GOLD).scale(0.35)
        stat2_sub = Text("Sweep vs Stockfish 8", font="Segoe UI", color=TEXT_MUTED).scale(0.19)
        stat2_grp = VGroup(stat2_num, stat2_sub).arrange(DOWN, aligned_edge=LEFT, buff=0.04)

        # Right side tech badge in AlphaZero card
        az_tech_bg = RoundedRectangle(width=1.8, height=0.82, corner_radius=0.08, fill_color="#101a33", fill_opacity=0.95, stroke_color="#3b82f6", stroke_width=1.0)
        az_tech_t1 = Text("ARCHITECTURE", font="Consolas", color=TEXT_MUTED).scale(0.17)
        az_tech_t2 = Text("ResNet + MCTS", font="Consolas", color=ACCENT_CYAN).scale(0.22)
        az_tech_t3 = Text("5,000 TPU Pod", font="Consolas", color=TEXT_BRIGHT).scale(0.18)
        az_tech_grp = VGroup(az_tech_t1, az_tech_t2, az_tech_t3).arrange(DOWN, buff=0.04).move_to(az_tech_bg.get_center())
        az_tech_badge = VGroup(az_tech_bg, az_tech_grp).move_to(az_bg.get_right() + LEFT * 1.25 + UP * 0.15)

        az_stats = VGroup(stat1_grp, stat2_grp).arrange(RIGHT, buff=0.45)
        az_stats.move_to(az_bg.get_left() + RIGHT * 2.2 + UP * 0.15)

        az_t1 = Text("• Replaced 5,000 heuristic rules with deep Monte Carlo tree search.", font="Segoe UI", color=TEXT_BRIGHT).scale(0.21)
        az_t2 = Text("• Proved statistical pattern intuition decisively defeats hand-crafted logic.", font="Segoe UI", color=TEXT_MUTED).scale(0.20)
        az_lines = VGroup(az_t1, az_t2).arrange(DOWN, aligned_edge=LEFT, buff=0.06)
        az_lines.move_to(az_bg.get_bottom() + UP * 0.38 + LEFT * (col_w / 2 - 0.35), aligned_edge=LEFT)

        az_box = VGroup(az_bg, az_bar, az_hdr, az_stats, az_tech_badge, az_lines)
        az_box.move_to(LEFT * 3.55 + UP * 0.90)

        # Card 2: Stockfish NNUE 2020
        nnue_bg = RoundedRectangle(width=col_w, height=box_h, corner_radius=0.12, fill_color="#140f26", fill_opacity=0.95, stroke_color=ACCENT_VIOLET, stroke_width=1.3)
        nnue_bar = RoundedRectangle(width=0.12, height=box_h, corner_radius=0.06, fill_color=ACCENT_VIOLET, fill_opacity=1.0, stroke_width=0)
        nnue_bar.move_to(nnue_bg.get_left() + RIGHT * 0.06)

        nnue_hdr = Text("2020 • STOCKFISH NNUE ARCHITECTURE", font="Consolas", color=ACCENT_VIOLET).scale(0.24)
        nnue_hdr.move_to(nnue_bg.get_top() + DOWN * 0.32 + LEFT * (col_w / 2 - 0.35), aligned_edge=LEFT)

        stat3_num = Text("+150 ELO", font="Consolas", color=ACCENT_GOLD).scale(0.35)
        stat3_sub = Text("Historic Rating Leap", font="Segoe UI", color=TEXT_MUTED).scale(0.19)
        stat3_grp = VGroup(stat3_num, stat3_sub).arrange(DOWN, aligned_edge=LEFT, buff=0.04)

        stat4_num = Text("100M+ NPS", font="Consolas", color=ACCENT_CYAN).scale(0.35)
        stat4_sub = Text("Pure CPU Evaluation", font="Segoe UI", color=TEXT_MUTED).scale(0.19)
        stat4_grp = VGroup(stat4_num, stat4_sub).arrange(DOWN, aligned_edge=LEFT, buff=0.04)

        # Right side tech badge in NNUE card
        nnue_tech_bg = RoundedRectangle(width=1.8, height=0.82, corner_radius=0.08, fill_color="#1f1538", fill_opacity=0.95, stroke_color=ACCENT_VIOLET, stroke_width=1.0)
        nnue_tech_t1 = Text("DEPLOYMENT", font="Consolas", color=TEXT_MUTED).scale(0.17)
        nnue_tech_t2 = Text("SIMD AVX2/NEON", font="Consolas", color=ACCENT_VIOLET).scale(0.22)
        nnue_tech_t3 = Text("Zero GPU Cost", font="Consolas", color=TEXT_BRIGHT).scale(0.18)
        nnue_tech_grp = VGroup(nnue_tech_t1, nnue_tech_t2, nnue_tech_t3).arrange(DOWN, buff=0.04).move_to(nnue_tech_bg.get_center())
        nnue_tech_badge = VGroup(nnue_tech_bg, nnue_tech_grp).move_to(nnue_bg.get_right() + LEFT * 1.25 + UP * 0.15)

        nnue_stats = VGroup(stat3_grp, stat4_grp).arrange(RIGHT, buff=0.45)
        nnue_stats.move_to(nnue_bg.get_left() + RIGHT * 2.2 + UP * 0.15)

        nnue_t1 = Text("• Embedded shallow neural network directly into CPU alpha-beta search.", font="Segoe UI", color=TEXT_BRIGHT).scale(0.21)
        nnue_t2 = Text("• Overnight became the undisputed gold standard of global chess AI.", font="Segoe UI", color=TEXT_MUTED).scale(0.20)
        nnue_lines = VGroup(nnue_t1, nnue_t2).arrange(DOWN, aligned_edge=LEFT, buff=0.06)
        nnue_lines.move_to(nnue_bg.get_bottom() + UP * 0.38 + LEFT * (col_w / 2 - 0.35), aligned_edge=LEFT)

        nnue_box = VGroup(nnue_bg, nnue_bar, nnue_hdr, nnue_stats, nnue_tech_badge, nnue_lines)
        nnue_box.move_to(LEFT * 3.55 + DOWN * 1.80)

        stage2_left = VGroup(az_box, nnue_box)

        # Right Column: High-Density Neural Synapse Architecture Visualizer
        pipe_w = 6.6
        pipe_h = 5.20
        pipe_bg = RoundedRectangle(width=pipe_w, height=pipe_h, corner_radius=0.14, fill_color="#080e1b", fill_opacity=0.96, stroke_color="#1e293b", stroke_width=1.3)
        pipe_bg.move_to(RIGHT * 3.55 + DOWN * 0.45)

        pipe_title = Text("NNUE INFERENCE PIPELINE // 4.3M PARAMETERS", font="Consolas", color=ACCENT_CYAN).scale(0.24)
        pipe_title.move_to(pipe_bg.get_top() + DOWN * 0.36)

        # Layer positions across pipe_bg
        n_in = 7
        n_acc = 8
        n_hid = 4
        
        in_xs = pipe_bg.get_center()[0] - 2.1
        acc_xs = pipe_bg.get_center()[0] - 0.6
        hid_xs = pipe_bg.get_center()[0] + 0.8
        out_xs = pipe_bg.get_center()[0] + 2.2

        ys_in = np.linspace(1.15, -1.15, n_in) + pipe_bg.get_center()[1] + 0.05
        ys_acc = np.linspace(1.35, -1.35, n_acc) + pipe_bg.get_center()[1] + 0.05
        ys_hid = np.linspace(0.85, -0.85, n_hid) + pipe_bg.get_center()[1] + 0.05

        layer_in_dots = VGroup()
        for y in ys_in:
            d = Circle(radius=0.085, fill_color="#0284c7", fill_opacity=0.95, stroke_color="#38bdf8", stroke_width=1.2)
            d.move_to([in_xs, y, 0])
            layer_in_dots.add(d)

        layer_acc_dots = VGroup()
        for y in ys_acc:
            d = Circle(radius=0.10, fill_color="#7c3aed", fill_opacity=0.95, stroke_color="#a78bfa", stroke_width=1.2)
            d.move_to([acc_xs, y, 0])
            layer_acc_dots.add(d)

        layer_hid_dots = VGroup()
        for y in ys_hid:
            d = Circle(radius=0.09, fill_color="#4f46e5", fill_opacity=0.95, stroke_color="#818cf8", stroke_width=1.2)
            d.move_to([hid_xs, y, 0])
            layer_hid_dots.add(d)

        out_dot = Circle(radius=0.16, fill_color="#d97706", fill_opacity=0.95, stroke_color="#fbbf24", stroke_width=1.8)
        out_dot.move_to([out_xs, pipe_bg.get_center()[1] + 0.05, 0])

        # Synapses with gradient aesthetic
        synapses_1 = VGroup()
        for idot in layer_in_dots:
            for adot in layer_acc_dots:
                l = Line(idot.get_center(), adot.get_center(), color="#1e293b", stroke_width=0.6, stroke_opacity=0.40)
                synapses_1.add(l)

        synapses_2 = VGroup()
        for adot in layer_acc_dots:
            for hdot in layer_hid_dots:
                l = Line(adot.get_center(), hdot.get_center(), color="#2e1065", stroke_width=0.8, stroke_opacity=0.50)
                synapses_2.add(l)

        synapses_3 = VGroup()
        for hdot in layer_hid_dots:
            l = Line(hdot.get_center(), out_dot.get_center(), color="#78350f", stroke_width=1.2, stroke_opacity=0.70)
            synapses_3.add(l)

        # Column Labels above layers
        lbl_y = pipe_bg.get_center()[1] + 1.80
        lbl_l1 = Text("HalfKP Features\n41,024 Inputs", font="Consolas", color=ACCENT_CYAN).scale(0.20).move_to([in_xs, lbl_y, 0])
        lbl_l2 = Text("Accumulator\n2x256 Features", font="Consolas", color=ACCENT_VIOLET).scale(0.20).move_to([acc_xs, lbl_y, 0])
        lbl_l3 = Text("Dense Layers\n32 -> 32", font="Consolas", color=ACCENT_INDIGO).scale(0.20).move_to([hid_xs, lbl_y, 0])
        lbl_l4 = Text("Score Output\n+0.34", font="Consolas", color=ACCENT_GOLD).scale(0.21).move_to([out_xs, lbl_y, 0])
        net_labels = VGroup(lbl_l1, lbl_l2, lbl_l3, lbl_l4)

        # Bottom Architecture Summary Pill
        arch_pill = RoundedRectangle(width=6.0, height=0.55, corner_radius=0.08, fill_color="#101726", fill_opacity=0.95, stroke_color="#334155", stroke_width=1.0)
        arch_pill_txt = Text("4,300,000 Learned Weights  •  Statistical Pattern Matching  •  Zero Rules", font="Consolas", color="#cbd5e1").scale(0.19)
        arch_pill_txt.move_to(arch_pill.get_center())
        arch_widget = VGroup(arch_pill, arch_pill_txt).move_to(pipe_bg.get_bottom() + UP * 0.45)

        stage2_right = VGroup(
            pipe_bg, pipe_title, synapses_1, synapses_2, synapses_3,
            layer_in_dots, layer_acc_dots, layer_hid_dots, out_dot,
            net_labels, arch_widget
        )

        self.play(
            FadeIn(stage2_left, LEFT * 0.25),
            FadeIn(stage2_right, RIGHT * 0.25),
            run_time=1.3
        )
        self.wait(2.8)

        # =============================================================
        # 5. STAGE 3: The Black Box & Horizon Drift in Locked Centers
        # =============================================================
        self.play(
            FadeOut(stage2_left),
            FadeOut(stage2_right),
            run_time=0.8
        )

        header_tag_3 = Text("THE BLIND SPOT // THE OPAQUE BLACK BOX", font="Consolas", color=ACCENT_RED).scale(0.24)
        header_tag_3.to_edge(UP, buff=0.55).to_edge(LEFT, buff=0.85)

        header_title_3 = Text("Horizon Drift: Why Neural Networks Fail in Locked Centers", font="Segoe UI", color=TEXT_BRIGHT).scale(0.48)
        header_title_3.next_to(header_tag_3, DOWN, aligned_edge=LEFT, buff=0.10)

        self.play(
            Transform(header_tag, header_tag_3),
            Transform(header_title, header_title_3),
            run_time=0.6
        )

        # Left Side: Rich Locked Center Chessboard with Telemetry Gauge
        board3 = BroadcastChessBoard(
            center=LEFT * 3.7 + DOWN * 0.35,
            sq_size=0.54,
            show_coords=True
        )

        # Grandmaster Locked Center (c4/d5/e4 vs c5/d6/e5)
        board3_pieces = VGroup(
            # White locked pawn spine
            board3.create_piece("wP", 2, 3), # c4
            board3.create_piece("wP", 3, 4), # d5
            board3.create_piece("wP", 4, 3), # e4
            # Black locked pawn spine
            board3.create_piece("bP", 2, 4), # c5
            board3.create_piece("bP", 3, 5), # d6
            board3.create_piece("bP", 4, 4), # e5
            # White attack battery
            board3.create_piece("wN", 5, 4), # Nf5
            board3.create_piece("wQ", 7, 4), # Qh5
            board3.create_piece("wR", 5, 0), # Rf1
            board3.create_piece("wK", 6, 0), # Kg1
            # Black king defense
            board3.create_piece("bK", 6, 7), # Kg8
            board3.create_piece("bP", 6, 6), # g7
            board3.create_piece("bP", 7, 6), # h7
            # Black stranded queenside pieces
            board3.create_piece("bR", 0, 7), # Ra8 (trapped!)
            board3.create_piece("bN", 1, 7), # Nb8
            board3.create_piece("bP", 0, 6), # a7
            board3.create_piece("bP", 1, 6), # b7
        )

        # NNUE Evaluation Gauge above Board (Crimson accented high-tech readout)
        eval_gauge_bg = RoundedRectangle(
            width=5.1, height=0.65, corner_radius=0.08,
            fill_color="#180e18", fill_opacity=0.96,
            stroke_color=ACCENT_RED, stroke_width=1.4
        )
        eval_gauge_bg.move_to(board3.get_center() + UP * (4 * 0.54 + 0.58))

        g_lbl = Text("NNUE STATIC EVALUATION:", font="Consolas", color=TEXT_MUTED).scale(0.22)
        g_val = Text("0.00  (DEAD EQUAL)", font="Consolas", color=ACCENT_CYAN).scale(0.26)
        g_row = VGroup(g_lbl, g_val).arrange(RIGHT, buff=0.18).move_to(eval_gauge_bg.get_center())
        eval_gauge = VGroup(eval_gauge_bg, g_row)

        # Search Horizon Depth Timeline below Board
        horizon_pill_bg = RoundedRectangle(
            width=5.1, height=0.62, corner_radius=0.08,
            fill_color="#0e1424", fill_opacity=0.96,
            stroke_color=BORDER_COL, stroke_width=1.2
        )
        horizon_pill_bg.move_to(board3.get_center() + DOWN * (4 * 0.54 + 0.60))

        h_line1 = Text("SEARCH PROGRESSION // DEPTH 36 (62M NODES)", font="Consolas", color=ACCENT_RED).scale(0.20)
        h_line2 = Text("Depth 10: 0.00  ->  Depth 25: 0.00  ->  Depth 36: -5.00", font="Consolas", color=TEXT_BRIGHT).scale(0.19)
        h_content = VGroup(h_line1, h_line2).arrange(DOWN, buff=0.06).move_to(horizon_pill_bg.get_center())
        horizon_pill = VGroup(horizon_pill_bg, h_content)

        stage3_left = VGroup(eval_gauge, board3, board3_pieces, horizon_pill)

        # Right Side: Forensic Diagnostic Terminal (Two-Column Data Modules with Large Readable Typography)
        term_w = 6.6
        term_h = 5.20
        term_bg = RoundedRectangle(width=term_w, height=term_h, corner_radius=0.14, fill_color="#080e1a", fill_opacity=0.96, stroke_color=BORDER_COL, stroke_width=1.3)
        term_bg.move_to(RIGHT * 3.55 + DOWN * 0.45)

        term_title = Text("ANATOMY OF A HORIZON FAILURE", font="Consolas", color=ACCENT_RED).scale(0.25)
        term_title.move_to(term_bg.get_top() + DOWN * 0.36)

        # 4 Diagnostic Steps with TWO COLUMNS: Left insight, Right metric badge!
        def make_forensic_module(accent_col, step_num, title_str, desc_str, badge_lbl, badge_val, badge_col):
            m_bg = RoundedRectangle(width=term_w - 0.35, height=0.82, corner_radius=0.08, fill_color="#0e1628", fill_opacity=0.95, stroke_color="#1e293b", stroke_width=1.0)
            
            # Left accent tag
            tag_box = RoundedRectangle(width=0.70, height=0.48, corner_radius=0.06, fill_color="#141f38", fill_opacity=0.95, stroke_color=accent_col, stroke_width=1.0)
            tag_txt = Text(step_num, font="Consolas", color=accent_col).scale(0.23)
            tag_txt.move_to(tag_box.get_center())
            tag_grp = VGroup(tag_box, tag_txt)
            tag_grp.move_to(m_bg.get_left() + RIGHT * 0.50)

            # Center/Left Title and body
            t = Text(title_str, font="Consolas", color=TEXT_BRIGHT).scale(0.23)
            d = Text(desc_str, font="Segoe UI", color="#cbd5e1").scale(0.20)
            text_block = VGroup(t, d).arrange(DOWN, aligned_edge=LEFT, buff=0.05)
            text_block.next_to(tag_grp, RIGHT, buff=0.18)

            # Right Metric Badge (Fills the card completely!)
            b_bg = RoundedRectangle(width=1.60, height=0.56, corner_radius=0.06, fill_color="#141c30", fill_opacity=0.95, stroke_color=badge_col, stroke_width=1.1)
            b_l = Text(badge_lbl, font="Consolas", color=TEXT_MUTED).scale(0.16)
            b_v = Text(badge_val, font="Consolas", color=badge_col).scale(0.22)
            b_content = VGroup(b_l, b_v).arrange(DOWN, buff=0.03).move_to(b_bg.get_center())
            badge_grp = VGroup(b_bg, b_content).move_to(m_bg.get_right() + LEFT * 0.95)

            return VGroup(m_bg, tag_grp, text_block, badge_grp)

        f1 = make_forensic_module(ACCENT_VIOLET, "01", "TOPOLOGICAL BLIND SPOT", "Weights evaluate bitboards with zero concept of 2D space", "GEOMETRY", "ZERO CONVERT", ACCENT_VIOLET)
        f2 = make_forensic_module(ACCENT_CYAN,   "02", "SILENT PAWN CHAIN ARREST", "Locked center pawns suppress captures; quiet moves pruned", "TRADES", "0 IN 30 PLIES", ACCENT_CYAN)
        f3 = make_forensic_module(ACCENT_GOLD,   "03", "FALSE EVALUATION STABILITY", "Engine shuffles pieces aimlessly, blind to structural rot", "REPORTED", "0.00 EQUAL", ACCENT_GOLD)
        f4 = make_forensic_module(ACCENT_RED,    "04", "THE CLIFF EDGE COLLAPSE", "At depth 36 White's break breaches search; position dies", "CLIFF DROP", "-5.00 CRASH", ACCENT_RED)

        forensic_stack = VGroup(f1, f2, f3, f4).arrange(DOWN, buff=0.10)
        forensic_stack.next_to(term_title, DOWN, buff=0.15)

        # Hook Question Card at Bottom (Cyan glowing frame)
        hook_w = term_w - 0.35
        hook_h = 0.68
        hook_box = RoundedRectangle(
            width=hook_w, height=hook_h, corner_radius=0.08,
            fill_color="#10182e", fill_opacity=0.96,
            stroke_color=ACCENT_CYAN, stroke_width=1.3
        )
        hook_lbl = Text("THE CORE EXPERIMENT // FROM CORRELATION TO FIRST PRINCIPLES", font="Consolas", color=ACCENT_GOLD).scale(0.19)
        hook_txt = Text("Can exact analytical math replace the black box at Depth 0?", font="Segoe UI", color=TEXT_BRIGHT).scale(0.23)
        hook_content = VGroup(hook_lbl, hook_txt).arrange(DOWN, buff=0.05).move_to(hook_box.get_center())
        hook_group = VGroup(hook_box, hook_content)
        hook_group.next_to(forensic_stack, DOWN, buff=0.12)

        stage3_right = VGroup(term_bg, term_title, forensic_stack, hook_group)

        # Animate Stage 3
        self.play(
            FadeIn(stage3_left, LEFT * 0.25),
            FadeIn(stage3_right, RIGHT * 0.25),
            run_time=1.3
        )
        self.wait(3.5)
