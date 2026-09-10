"""
Heaven's Gate Documentary - Scene 07: The Tropical Semiring & The 66-Round Marathon
Runtime: ~85 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: The Dilemma & The Tropical Semiring (R U {-inf}, max, +)
- Beat 2: Faceted Minimax Hyperplane Envelope & Log-Sum-Exp Temperature Surface
- Beat 3: 10 Spatial King Buckets (640 topological sector parameters)
- Beat 4: The 66-Round Continuous Training Marathon HUD (run_continuous_training.ps1)
          Replay buffer swelling from 50k -> 500k -> 2,000,000 positions
- Beat 5: Impending Collapse forewarning transition
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

# Ensure animations directory is importable
sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard

BG_DARK       = "#070b12"
GRID_COLOR    = "#1e293b"
TEXT_BRIGHT   = "#f8fafc"
TEXT_MUTED    = "#94a3b8"
TEXT_DIM      = "#475569"

ACCENT_CYAN   = "#38bdf8"
ACCENT_BLUE   = "#0ea5e9"
ACCENT_VIOLET = "#a78bfa"
ACCENT_GOLD   = "#fbbf24"
ACCENT_RED    = "#f43f5e"
ACCENT_GREEN  = "#34d399"
ACCENT_ORANGE = "#fb923c"

class Scene07TropicalOdyssey(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # LAYER 0: GROUNDING ARCHITECTURAL BLUEPRINT GRID
        # -------------------------------------------------------------
        bg = Rectangle(width=28, height=18, fill_color=BG_DARK, fill_opacity=1.0).set_stroke(width=0)
        self.add(bg)

        tech_grid = NumberPlane(
            x_range=[-14, 14, 1],
            y_range=[-9, 9, 1],
            width=28,
            height=18,
            axis_config={
                "stroke_color": "#1e293b",
                "stroke_width": 0.8,
                "stroke_opacity": 0.4,
            },
            background_line_style={
                "stroke_color": GRID_COLOR,
                "stroke_width": 0.7,
                "stroke_opacity": 0.35,
            },
            faded_line_style={
                "stroke_color": "#121b2a",
                "stroke_width": 0.4,
                "stroke_opacity": 0.20,
            }
        )
        self.add(tech_grid)

        # -------------------------------------------------------------
        # BEAT 1: TITLE BANNER (0s - 8s)
        # -------------------------------------------------------------
        head_card = RoundedRectangle(
            width=11.2, height=1.1, corner_radius=0.10,
            fill_color="#0f172a", fill_opacity=0.92,
            stroke_color="#334155", stroke_width=1.5
        ).move_to(UP * 3.2)

        head_t1 = Text("CHAPTER 05 // THE WILD MATHEMATICAL EXPERIMENT", font="Bahnschrift", color=ACCENT_VIOLET).scale(0.24)
        head_t2 = Text("7. The Tropical Semiring & The 66-Round Marathon", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.40)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: THE TROPICAL SEMIRING ALGEBRA (8s - 28s)
        # -------------------------------------------------------------
        # Left Panel: Mathematical Definition
        semiring_card = RoundedRectangle(
            width=5.8, height=4.6, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.92,
            stroke_color="#38bdf8", stroke_width=1.8
        ).move_to(LEFT * 3.4 + DOWN * 0.2)

        badge_sr = RoundedRectangle(
            width=3.2, height=0.45, corner_radius=0.08,
            fill_color="#0284c7", fill_opacity=0.35,
            stroke_color="#38bdf8", stroke_width=1.2
        ).move_to(semiring_card.get_top() + DOWN * 0.35)
        badge_sr_text = Text("MAX-PLUS ALGEBRA", font="Bahnschrift", color=ACCENT_CYAN).scale(0.26).move_to(badge_sr.get_center())

        sr_def_t1 = Text("The Tropical Semiring:", font="Bahnschrift", color=TEXT_MUTED).scale(0.32)
        sr_def_sym = Text("(R U {-inf}, max, +)", font="Consolas", color=ACCENT_GOLD).scale(0.44)
        sr_def_box = VGroup(sr_def_t1, sr_def_sym).arrange(DOWN, buff=0.12).move_to(semiring_card.get_center() + UP * 1.0)

        div_line = Line(LEFT * 2.4, RIGHT * 2.4, stroke_color="#1e293b", stroke_width=1.2).move_to(semiring_card.get_center() + UP * 0.3)

        op1_lbl = Text("Tropical Addition:", font="Bahnschrift", color=TEXT_MUTED).scale(0.28)
        op1_eq  = Text("x (+) y  =  max(x, y)", font="Consolas", color=ACCENT_CYAN).scale(0.38)
        op1_grp = VGroup(op1_lbl, op1_eq).arrange(DOWN, buff=0.08)

        op2_lbl = Text("Tropical Multiplication:", font="Bahnschrift", color=TEXT_MUTED).scale(0.28)
        op2_eq  = Text("x (*) y  =  x + y", font="Consolas", color=ACCENT_GREEN).scale(0.38)
        op2_grp = VGroup(op2_lbl, op2_eq).arrange(DOWN, buff=0.08)

        ops_group = VGroup(op1_grp, op2_grp).arrange(DOWN, buff=0.25).move_to(semiring_card.get_center() + DOWN * 0.9)

        semiring_panel = VGroup(semiring_card, badge_sr, badge_sr_text, sr_def_box, div_line, ops_group)

        # Right Panel: Why Minimax is Tropical
        insight_card = RoundedRectangle(
            width=6.2, height=4.6, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.92,
            stroke_color="#a78bfa", stroke_width=1.8
        ).move_to(RIGHT * 3.3 + DOWN * 0.2)

        badge_in = RoundedRectangle(
            width=3.6, height=0.45, corner_radius=0.08,
            fill_color="#7c3aed", fill_opacity=0.35,
            stroke_color="#a78bfa", stroke_width=1.2
        ).move_to(insight_card.get_top() + DOWN * 0.35)
        badge_in_text = Text("WHY CHESS IS TROPICAL", font="Bahnschrift", color=ACCENT_VIOLET).scale(0.26).move_to(badge_in.get_center())

        in_text1 = Text("Minimax computes upper & lower envelopes:", font="Bahnschrift", color=TEXT_MUTED).scale(0.28)
        in_eq1   = Text("Eval(s) = max_i ( w_i * x + b_i )", font="Consolas", color=ACCENT_GOLD).scale(0.36)

        # Visual of piecewise linear envelope
        plane_axes = Axes(
            x_range=[-3, 3, 1],
            y_range=[-1, 4, 1],
            width=4.8,
            height=2.0,
            axis_config={"stroke_color": "#334155", "stroke_width": 1.0}
        ).move_to(insight_card.get_center() + DOWN * 0.5)

        line1 = plane_axes.get_graph(lambda x: 0.5 * x + 1.2, x_range=[-2.5, 2.5], color="#475569").set_stroke(width=1.5)
        line2 = plane_axes.get_graph(lambda x: -0.8 * x + 1.8, x_range=[-2.5, 2.5], color="#475569").set_stroke(width=1.5)
        line3 = plane_axes.get_graph(lambda x: 1.4 * x - 0.2, x_range=[-2.5, 2.5], color="#475569").set_stroke(width=1.5)

        # Upper envelope in bright luminous gold
        def upper_env(x):
            return max(0.5 * x + 1.2, -0.8 * x + 1.8, 1.4 * x - 0.2)

        env_curve = plane_axes.get_graph(upper_env, x_range=[-2.2, 2.2], color=ACCENT_GOLD).set_stroke(width=3.2)
        env_label = Text("Faceted Minimax Envelope", font="Bahnschrift", color=ACCENT_GOLD).scale(0.24).next_to(plane_axes, UP, buff=0.1)

        insight_group = VGroup(
            insight_card, badge_in, badge_in_text,
            VGroup(in_text1, in_eq1).arrange(DOWN, buff=0.08).move_to(insight_card.get_center() + UP * 1.3),
            plane_axes, line1, line2, line3, env_curve, env_label
        )

        self.play(FadeIn(semiring_panel, LEFT * 0.4), run_time=1.2)
        self.wait(1.0)
        self.play(FadeIn(insight_group, RIGHT * 0.4), run_time=1.2)
        self.wait(2.0)

        # Highlight smoothing: Log-Sum-Exp temperature
        smooth_box = RoundedRectangle(
            width=5.2, height=0.75, corner_radius=0.08,
            fill_color="#1e1b4b", fill_opacity=0.95,
            stroke_color="#818cf8", stroke_width=1.4
        ).move_to(insight_card.get_bottom() + UP * 0.55)
        smooth_text = Text("Log-Sum-Exp Smooth Max:  tau * ln( sum exp(w*x / tau) )", font="Consolas", color="#c7d2fe").scale(0.24).move_to(smooth_box.get_center())
        smooth_group = VGroup(smooth_box, smooth_text)

        self.play(FadeIn(smooth_group, UP * 0.2), run_time=0.8)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 3: 10 SPATIAL KING BUCKETS (28s - 48s)
        # -------------------------------------------------------------
        self.play(
            FadeOut(semiring_panel, LEFT * 0.4),
            FadeOut(insight_group, RIGHT * 0.4),
            FadeOut(smooth_group, DOWN * 0.3),
            run_time=1.0
        )

        bucket_header = Text("SPATIAL KING BUCKETING: 640 TOPOLOGICAL SECTOR WEIGHTS", font="Bahnschrift", color=ACCENT_CYAN).scale(0.32).move_to(UP * 2.2)
        self.play(FadeIn(bucket_header, DOWN * 0.2), run_time=0.8)

        # Draw 8x8 Chessboard divided into King Buckets
        board_container = RoundedRectangle(
            width=5.4, height=5.4, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.95,
            stroke_color="#334155", stroke_width=1.5
        ).move_to(LEFT * 3.5 + DOWN * 0.6)

        sq_w = 4.6 / 8.0
        cb = BroadcastChessBoard(center=board_container.get_center(), sq_size=sq_w)
        wk_piece = cb.create_piece("wK", 4, 0)

        # White Kingside Castle Zone (files E-H, ranks 1-2)
        z1 = Rectangle(width=4*sq_w, height=2*sq_w, fill_color=ACCENT_CYAN, fill_opacity=0.30, stroke_color=ACCENT_CYAN, stroke_width=1.5)
        z1.move_to(cb.get_center() + RIGHT * (2*sq_w) + DOWN * (2*sq_w))
        z1_lbl = Text("Bucket #1: White O-O", font="Bahnschrift", color=ACCENT_CYAN).scale(0.20).move_to(z1.get_center())

        # White Queenside Castle Zone (files A-D, ranks 1-2)
        z2 = Rectangle(width=4*sq_w, height=2*sq_w, fill_color=ACCENT_VIOLET, fill_opacity=0.30, stroke_color=ACCENT_VIOLET, stroke_width=1.5)
        z2.move_to(cb.get_center() + LEFT * (2*sq_w) + DOWN * (2*sq_w))
        z2_lbl = Text("Bucket #2: White O-O-O", font="Bahnschrift", color=ACCENT_VIOLET).scale(0.20).move_to(z2.get_center())

        # Central Sector (ranks 3-5)
        z3 = Rectangle(width=6*sq_w, height=3*sq_w, fill_color=ACCENT_GOLD, fill_opacity=0.25, stroke_color=ACCENT_GOLD, stroke_width=1.5)
        z3.move_to(cb.get_center() + DOWN * (0.5*sq_w))
        z3_lbl = Text("Bucket #5: Open Center / Endgame", font="Bahnschrift", color=ACCENT_GOLD).scale(0.22).move_to(z3.get_center())

        zone_group = VGroup(z1, z1_lbl, z2, z2_lbl, z3, z3_lbl)

        # Right Card: Bucket Telemetry
        bucket_info = RoundedRectangle(
            width=6.2, height=5.2, corner_radius=0.12,
            fill_color="#0f172a", fill_opacity=0.92,
            stroke_color="#38bdf8", stroke_width=1.5
        ).move_to(RIGHT * 3.2 + DOWN * 0.6)

        bi_title = Text("DYNAMIC SECTOR PARAMETER DISPATCH", font="Bahnschrift", color=ACCENT_CYAN).scale(0.28).move_to(bucket_info.get_top() + DOWN * 0.4)

        b_stat1 = Text("• 10 Spatial King Envelopes", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.28)
        b_stat2 = Text("• 64 Base Features per Sector (Fiedler, Degrees, PST)", font="Bahnschrift", color=TEXT_MUTED).scale(0.25)
        b_stat3 = Text("• Total Model Parameters: 640 Weights", font="Consolas", color=ACCENT_GOLD).scale(0.30)
        b_stat4 = Text("• King Position selects active hyperplane envelope", font="Bahnschrift", color=TEXT_MUTED).scale(0.25)

        b_stats = VGroup(b_stat1, b_stat2, b_stat3, b_stat4).arrange(DOWN, buff=0.22, aligned_edge=LEFT).move_to(bucket_info.get_center() + UP * 0.6)

        # Active Sector readout box
        sector_hud = RoundedRectangle(
            width=5.4, height=1.4, corner_radius=0.08,
            fill_color="#070d18", fill_opacity=0.95,
            stroke_color="#0284c7", stroke_width=1.2
        ).move_to(bucket_info.get_bottom() + UP * 1.1)

        hud_label = Text("ACTIVE EVALUATION SLICE", font="Bahnschrift", color=TEXT_DIM).scale(0.20).move_to(sector_hud.get_top() + DOWN * 0.22)
        hud_active = Text("King: e1 -> SECTOR 01 (Kingside Shielded)", font="Consolas", color=ACCENT_CYAN).scale(0.28).move_to(sector_hud.get_center() + DOWN * 0.05)
        hud_weights = Text("Loaded 64 Weights | Bias: +14.2 cp | Temp tau: 1.25", font="Consolas", color=TEXT_MUTED).scale(0.22).move_to(sector_hud.get_bottom() + UP * 0.25)
        sector_hud_group = VGroup(sector_hud, hud_label, hud_active, hud_weights)

        self.play(FadeIn(board_container), FadeIn(cb), FadeIn(wk_piece), FadeIn(zone_group), FadeIn(bucket_info), FadeIn(bi_title), FadeIn(b_stats), FadeIn(sector_hud_group), run_time=1.4)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 4: THE 66-ROUND CONTINUOUS TRAINING MARATHON (48s - 72s)
        # -------------------------------------------------------------
        self.play(
            FadeOut(board_container),
            FadeOut(cb),
            FadeOut(wk_piece),
            FadeOut(zone_group),
            FadeOut(bucket_info),
            FadeOut(bi_title),
            FadeOut(b_stats),
            FadeOut(sector_hud_group),
            FadeOut(bucket_header),
            run_time=1.0
        )

        # Terminal UI Card
        term_card = RoundedRectangle(
            width=11.6, height=5.4, corner_radius=0.12,
            fill_color="#050811", fill_opacity=0.96,
            stroke_color="#334155", stroke_width=1.6
        ).move_to(DOWN * 0.4)

        # Terminal Top Bar
        term_bar = RoundedRectangle(
            width=11.6, height=0.45, corner_radius=0.08,
            fill_color="#0f172a", fill_opacity=1.0,
            stroke_color="#334155", stroke_width=1.0
        ).move_to(term_card.get_top() + DOWN * 0.225)

        dot_r = Circle(radius=0.08, fill_color="#ef4444", fill_opacity=1.0).set_stroke(width=0).move_to(term_bar.get_left() + RIGHT * 0.3)
        dot_y = Circle(radius=0.08, fill_color="#f59e0b", fill_opacity=1.0).set_stroke(width=0).next_to(dot_r, RIGHT, buff=0.12)
        dot_g = Circle(radius=0.08, fill_color="#10b981", fill_opacity=1.0).set_stroke(width=0).next_to(dot_y, RIGHT, buff=0.12)
        term_title = Text("PowerShell - run_continuous_training.ps1 [24/7 AUTONOMOUS RIG]", font="Consolas", color=TEXT_MUTED).scale(0.24).move_to(term_bar.get_center())
        term_header = VGroup(term_bar, dot_r, dot_y, dot_g, term_title)

        # Terminal lines
        cmd_text = Text("PS C:\\heavensgate> .\\run_continuous_training.ps1 -Rounds 66 -SelfPlayDepth 6 -AdamEpochs 300", font="Consolas", color=ACCENT_CYAN).scale(0.26)
        cmd_text.move_to(term_card.get_top() + DOWN * 0.75 + LEFT * 0.8)

        # Left Column: Round Counter & Pipeline Stages
        round_badge = RoundedRectangle(
            width=4.8, height=1.6, corner_radius=0.10,
            fill_color="#0d1527", fill_opacity=0.9,
            stroke_color=ACCENT_GOLD, stroke_width=1.6
        ).move_to(term_card.get_left() + RIGHT * 3.0 + DOWN * 0.5)

        round_lbl = Text("MARATHON CYCLE PROGRESS", font="Bahnschrift", color=TEXT_MUTED).scale(0.20).move_to(round_badge.get_top() + DOWN * 0.25)
        round_num = Text("ROUND 01 / 66", font="Consolas", color=ACCENT_GOLD).scale(0.55).move_to(round_badge.get_center() + DOWN * 0.05)
        round_sub = Text("Status: 500 Self-Play Games Running (16 Threads)", font="Consolas", color=ACCENT_GREEN).scale(0.19).move_to(round_badge.get_bottom() + UP * 0.22)
        round_box = VGroup(round_badge, round_lbl, round_num, round_sub)

        # Pipeline stages diagram below
        p_step1 = Text("[1] 500 Self-Play Games", font="Consolas", color=TEXT_MUTED).scale(0.22)
        p_step2 = Text("[2] Adam SGD (300 Epochs)", font="Consolas", color=TEXT_MUTED).scale(0.22)
        p_step3 = Text("[3] 100-Game Benchmark", font="Consolas", color=TEXT_MUTED).scale(0.22)
        pipeline_box = VGroup(p_step1, p_step2, p_step3).arrange(DOWN, buff=0.14, aligned_edge=LEFT).next_to(round_badge, DOWN, buff=0.25)

        # Right Column: Replay Buffer Swell Visualization
        replay_box = RoundedRectangle(
            width=5.6, height=3.6, corner_radius=0.10,
            fill_color="#0d1527", fill_opacity=0.9,
            stroke_color=ACCENT_CYAN, stroke_width=1.4
        ).move_to(term_card.get_right() + LEFT * 3.4 + DOWN * 0.7)

        rep_title = Text("EXPERIENCE REPLAY MEMORY BUFFER", font="Bahnschrift", color=ACCENT_CYAN).scale(0.24).move_to(replay_box.get_top() + DOWN * 0.3)

        # Progress bar container
        bar_bg = RoundedRectangle(
            width=4.8, height=0.5, corner_radius=0.06,
            fill_color="#1e293b", fill_opacity=1.0,
            stroke_color="#475569", stroke_width=1.0
        ).move_to(replay_box.get_center() + UP * 0.5)

        bar_fill = RoundedRectangle(
            width=0.48, height=0.46, corner_radius=0.05,
            fill_color=ACCENT_BLUE, fill_opacity=0.95
        ).set_stroke(width=0).align_to(bar_bg, LEFT)

        rep_count = Text("50,000 Positions Loaded", font="Consolas", color=TEXT_BRIGHT).scale(0.30).next_to(bar_bg, DOWN, buff=0.25)

        rep_spec1 = Text("• Max Capacity: 2,000,000 FEN States", font="Consolas", color=TEXT_MUTED).scale(0.22)
        rep_spec2 = Text("• MSE Loss Target: < 0.042 cp²", font="Consolas", color=TEXT_MUTED).scale(0.22)
        rep_spec3 = Text("• Hardware: 16-Core Continuous Self-Play", font="Consolas", color=TEXT_MUTED).scale(0.22)
        rep_specs = VGroup(rep_spec1, rep_spec2, rep_spec3).arrange(DOWN, buff=0.12, aligned_edge=LEFT).next_to(rep_count, DOWN, buff=0.25)

        replay_group = VGroup(replay_box, rep_title, bar_bg, bar_fill, rep_count, rep_specs)

        self.play(FadeIn(term_card), FadeIn(term_header), FadeIn(cmd_text), FadeIn(round_box), FadeIn(pipeline_box), FadeIn(replay_group), run_time=1.2)
        self.wait(1.5)

        # Animate round ticking and buffer swell: Round 01 -> Round 17 -> Round 66
        # Step 1: Round 17
        round_num_17 = Text("ROUND 17 / 66", font="Consolas", color=ACCENT_GOLD).scale(0.55).move_to(round_num.get_center())
        bar_fill_17 = RoundedRectangle(
            width=2.4, height=0.46, corner_radius=0.05,
            fill_color=ACCENT_CYAN, fill_opacity=0.95
        ).set_stroke(width=0).align_to(bar_bg, LEFT)
        rep_count_17 = Text("500,000 Positions Loaded", font="Consolas", color=ACCENT_CYAN).scale(0.30).next_to(bar_bg, DOWN, buff=0.25)

        self.play(
            Transform(round_num, round_num_17),
            Transform(bar_fill, bar_fill_17),
            Transform(rep_count, rep_count_17),
            run_time=1.4
        )
        self.wait(1.0)

        # Step 2: Round 66 (Max Capacity: 2 Million)
        round_num_66 = Text("ROUND 66 / 66", font="Consolas", color=ACCENT_ORANGE).scale(0.55).move_to(round_num.get_center())
        bar_fill_66 = RoundedRectangle(
            width=4.76, height=0.46, corner_radius=0.05,
            fill_color=ACCENT_GOLD, fill_opacity=0.95
        ).set_stroke(width=0).align_to(bar_bg, LEFT)
        rep_count_66 = Text("2,000,000 Positions Loaded (MAX)", font="Consolas", color=ACCENT_GOLD).scale(0.30).next_to(bar_bg, DOWN, buff=0.25)

        self.play(
            Transform(round_num, round_num_66),
            Transform(bar_fill, bar_fill_66),
            Transform(rep_count, rep_count_66),
            run_time=1.6
        )
        self.wait(2.0)

        # -------------------------------------------------------------
        # BEAT 5: THE FOREBODING CLIMAX / COLLAPSE TEASER (72s - 85s)
        # -------------------------------------------------------------
        fail_badge = RoundedRectangle(
            width=10.4, height=1.5, corner_radius=0.12,
            fill_color="#1c070d", fill_opacity=0.96,
            stroke_color=ACCENT_RED, stroke_width=2.0
        ).move_to(DOWN * 0.4)

        fail_t1 = Text("WARNING: PARADIGM INSTABILITY DETECTED", font="Bahnschrift", color=ACCENT_RED).scale(0.28)
        fail_t2 = Text("\"And it failed in almost every way imaginable.\"", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.42)
        fail_t3 = Text("NEXT: THE HALL OF SHAME • 5 DISASTERS, SACRIFICED QUEENS, AND THE GREAT REVERT", font="Consolas", color="#fca5a5").scale(0.20)
        fail_content = VGroup(fail_t1, fail_t2, fail_t3).arrange(DOWN, buff=0.12).move_to(fail_badge.get_center())

        fail_group = VGroup(fail_badge, fail_content)

        # Red alert pulse
        self.play(
            FadeOut(round_box),
            FadeOut(pipeline_box),
            FadeOut(replay_group),
            FadeIn(fail_group, scale=0.95),
            term_card.animate.set_stroke(color=ACCENT_RED, width=2.0),
            run_time=1.2
        )
        self.wait(3.0)
