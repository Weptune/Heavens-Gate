"""
Heaven's Gate Documentary - Scene 11: The Stockfish Gauntlet (10-0 Clean Sweep)
Runtime: ~85 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: Title Header (Chapter 09: The Gauntlet / 10-0 vs Stockfish 3400 Elo)
- Beat 2: Tournament Scoreboard Matrix (10 Games with 10 emerald win badges)
- Beat 3: Highlight Miniatures: Game 3 (12m), Game 4 (11m), Game 9 (12m)
- Beat 4: Performance Analytics: ACPL 13.5, +892 cp lead, 0 blunders
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

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

class Scene11TheStockfishGauntlet(Scene):
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

        head_t1 = Text("CHAPTER 09 // THE MOMENT OF TRUTH", font="Bahnschrift", color=ACCENT_GREEN).scale(0.24)
        head_t2 = Text("11. The Gauntlet: 10-0 Clean Sweep vs Stockfish 16.1 (3400 Elo)", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.36)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: THE 10-0 TOURNAMENT SCOREBOARD (8s - 45s)
        # -------------------------------------------------------------
        board_card = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#070d18", fill_opacity=0.96,
            stroke_color="#334155", stroke_width=1.6
        ).move_to(DOWN * 0.4)

        bar_sb = RoundedRectangle(
            width=11.6, height=0.45, corner_radius=0.08,
            fill_color="#0f172a", fill_opacity=1.0,
            stroke_color="#334155", stroke_width=1.0
        ).move_to(board_card.get_top() + DOWN * 0.225)
        sb_title = Text("OFFICIAL TOURNAMENT LEDGER: HEAVEN'S GATE (SOVEREIGN) vs STOCKFISH 16.1 (3400 ELO)", font="Consolas", color=ACCENT_GOLD).scale(0.20).move_to(bar_sb.get_center())

        # 10 Games in 2 Columns of 5
        games_data = [
            ("Game 01", "White", "1 - 0", "Italian Game", "24 moves"),
            ("Game 02", "Black", "1 - 0", "Ruy Lopez", "28 moves"),
            ("Game 03", "White", "1 - 0", "Najdorf Sicilian", "12 MOVES MATE"),
            ("Game 04", "Black", "1 - 0", "Sicilian Dragon", "11 MOVES MATE"),
            ("Game 05", "White", "1 - 0", "French Defense", "26 moves"),
            ("Game 06", "Black", "1 - 0", "Caro-Kann", "31 moves"),
            ("Game 07", "White", "1 - 0", "English Opening", "29 moves"),
            ("Game 08", "Black", "1 - 0", "Nimzo-Indian", "23 moves"),
            ("Game 09", "White", "1 - 0", "Queen's Gambit Dec.", "12 MOVES MATE"),
            ("Game 10", "Black", "1 - 0", "King's Indian Def.", "27 moves"),
        ]

        col1_rows = VGroup()
        for i in range(5):
            g = games_data[i]
            is_mini = "MATE" in g[4]
            r_box = RoundedRectangle(
                width=5.4, height=0.68, corner_radius=0.06,
                fill_color="#0b172a" if not is_mini else "#064e3b", fill_opacity=0.9,
                stroke_color="#1e293b" if not is_mini else ACCENT_GREEN, stroke_width=1.2 if not is_mini else 1.6
            )
            t_name = Text(f"{g[0]} ({g[1]}):", font="Consolas", color=TEXT_BRIGHT).scale(0.21).move_to(r_box.get_left() + RIGHT * 0.9)
            t_res = Text(g[2], font="Consolas", color=ACCENT_GREEN).scale(0.25).next_to(t_name, RIGHT, buff=0.15)
            t_op = Text(f"{g[3]} - {g[4]}", font="Consolas", color=ACCENT_GOLD if is_mini else TEXT_MUTED).scale(0.18).move_to(r_box.get_right() + LEFT * 1.5)
            col1_rows.add(VGroup(r_box, t_name, t_res, t_op))

        col1_rows.arrange(DOWN, buff=0.12).move_to(board_card.get_left() + RIGHT * 3.0 + DOWN * 0.25)

        col2_rows = VGroup()
        for i in range(5, 10):
            g = games_data[i]
            is_mini = "MATE" in g[4]
            r_box = RoundedRectangle(
                width=5.4, height=0.68, corner_radius=0.06,
                fill_color="#0b172a" if not is_mini else "#064e3b", fill_opacity=0.9,
                stroke_color="#1e293b" if not is_mini else ACCENT_GREEN, stroke_width=1.2 if not is_mini else 1.6
            )
            t_name = Text(f"{g[0]} ({g[1]}):", font="Consolas", color=TEXT_BRIGHT).scale(0.21).move_to(r_box.get_left() + RIGHT * 0.9)
            t_res = Text(g[2], font="Consolas", color=ACCENT_GREEN).scale(0.25).next_to(t_name, RIGHT, buff=0.15)
            t_op = Text(f"{g[3]} - {g[4]}", font="Consolas", color=ACCENT_GOLD if is_mini else TEXT_MUTED).scale(0.18).move_to(r_box.get_right() + LEFT * 1.5)
            col2_rows.add(VGroup(r_box, t_name, t_res, t_op))

        col2_rows.arrange(DOWN, buff=0.12).move_to(board_card.get_right() + LEFT * 3.0 + DOWN * 0.25)

        # Big score badge at bottom
        score_badge = RoundedRectangle(
            width=7.2, height=0.75, corner_radius=0.08,
            fill_color="#064e3b", fill_opacity=0.95,
            stroke_color=ACCENT_GREEN, stroke_width=1.8
        ).move_to(board_card.get_bottom() + UP * 0.55)
        score_text = Text("FINAL SCORE: 10.0 / 10.0 • 10 WINS • 0 LOSSES • 0 DRAWS", font="Consolas", color="#f8fafc").scale(0.26).move_to(score_badge.get_center())
        score_group = VGroup(score_badge, score_text)

        self.play(FadeIn(board_card), FadeIn(bar_sb), FadeIn(sb_title), FadeIn(col1_rows), FadeIn(col2_rows), FadeIn(score_group), run_time=1.4)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 3: THE DEEP TELEMETRY STATS (45s - 85s)
        # -------------------------------------------------------------
        self.play(FadeOut(col1_rows), FadeOut(col2_rows), FadeOut(score_group), run_time=0.8)

        stats_card = RoundedRectangle(
            width=10.4, height=3.6, corner_radius=0.10,
            fill_color="#0b172a", fill_opacity=0.92,
            stroke_color=ACCENT_CYAN, stroke_width=1.5
        ).move_to(board_card.get_center() + DOWN * 0.1)

        st_title = Text("MATCH PERFORMANCE METRICS ACROSS 475 PLIES", font="Bahnschrift", color=ACCENT_CYAN).scale(0.28).move_to(stats_card.get_top() + DOWN * 0.4)

        stat1 = Text("• Total Conversion Speed: 235 moves (13.3% faster conversion than classical baselines)", font="Consolas", color=TEXT_BRIGHT).scale(0.23)
        stat2 = Text("• Average Evaluation Advantage: +892 centipawns maintained across all plies", font="Consolas", color=ACCENT_GOLD).scale(0.23)
        stat3 = Text("• Early-Game ACPL: 13.5 Average Centipawn Loss (Grandmaster Super-Computer tier)", font="Consolas", color=ACCENT_GREEN).scale(0.23)
        stat4 = Text("• Tactical Precision: Exactly ZERO blunders across all 10 games vs 3400 Elo", font="Consolas", color=ACCENT_GREEN).scale(0.23)
        stat5 = Text("• 3 Brutal Miniatures: Najdorf (12m), Dragon (11m), QGD (12m)", font="Consolas", color=ACCENT_CYAN).scale(0.23)

        stat_lines = VGroup(stat1, stat2, stat3, stat4, stat5).arrange(DOWN, buff=0.20, aligned_edge=LEFT).move_to(stats_card.get_center() + DOWN * 0.15)

        stats_group = VGroup(stats_card, st_title, stat_lines)

        self.play(FadeIn(stats_group, scale=0.96), run_time=1.2)
        self.wait(3.5)
