"""
Heaven's Gate Documentary - Scene 09: The Split-Brain Discovery (Deluxe 3B1B Edition)
Runtime: ~85 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: Title Header (Chapter 07: The Plot Twist / Commit 6653436)
- Beat 2: The Two Hemispheres: Spectral-Tropical (46k Params) vs evaluate_fast() (Bitboards)
- Beat 3: The Profiler Audit: 99.8% traffic waterfall flowing into evaluate_fast()
- Beat 4: The CLI Deception: Console stdout vs Code Reality
- Beat 5: The Dismantling & The Awakening: Locking EvalMode::MasterPositional
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

class Scene09TheSplitBrain(Scene):
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

        head_t1 = Text("CHAPTER 07 // THE TURNING POINT", font="Bahnschrift", color=ACCENT_GOLD).scale(0.24)
        head_t2 = Text("9. The 'Split-Brain' Discovery: The Master Code Audit", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.40)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: THE SPLIT BRAIN ARCHITECTURE (8s - 32s)
        # -------------------------------------------------------------
        # Left Hemisphere: The Heavy Tropical Monster
        card_left = RoundedRectangle(
            width=5.6, height=4.6, corner_radius=0.12,
            fill_color="#0d111d", fill_opacity=0.94,
            stroke_color="#7c3aed", stroke_width=1.6
        ).move_to(LEFT * 3.3 + DOWN * 0.2)

        badge_l = RoundedRectangle(
            width=4.4, height=0.45, corner_radius=0.08,
            fill_color="#5b21b6", fill_opacity=0.35,
            stroke_color=ACCENT_VIOLET, stroke_width=1.2
        ).move_to(card_left.get_top() + DOWN * 0.35)
        badge_l_t = Text("SPECTRAL-TROPICAL MODEL", font="Consolas", color=ACCENT_VIOLET).scale(0.24).move_to(badge_l.get_center())

        l_spec1 = Text("• 46,920 Trained Parameters", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.26)
        l_spec2 = Text("• 10 Spatial King Hyperplanes", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        l_spec3 = Text("• Log-Sum-Exp Temperature Blend", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        l_spec4 = Text("• Cost: ~45 microseconds / node", font="Consolas", color=ACCENT_RED).scale(0.24)
        l_specs = VGroup(l_spec1, l_spec2, l_spec3, l_spec4).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_left.get_center() + UP * 0.2)

        # Bottom Traffic Share
        l_traffic_box = RoundedRectangle(
            width=4.8, height=0.85, corner_radius=0.08,
            fill_color="#180a14", fill_opacity=0.9,
            stroke_color="#7c3aed", stroke_width=1.2
        ).move_to(card_left.get_bottom() + UP * 0.65)
        l_traffic_lbl = Text("ACTUAL SEARCH TRAFFIC:", font="Bahnschrift", color=TEXT_MUTED).scale(0.20).move_to(l_traffic_box.get_top() + DOWN * 0.22)
        l_traffic_pct = Text("0.2 % of Search Queries", font="Consolas", color=ACCENT_RED).scale(0.32).move_to(l_traffic_box.get_center() + DOWN * 0.12)
        l_traffic_grp = VGroup(l_traffic_box, l_traffic_lbl, l_traffic_pct)

        group_left = VGroup(card_left, badge_l, badge_l_t, l_specs, l_traffic_grp)

        # Right Hemisphere: The Razor-Sharp evaluate_fast()
        card_right = RoundedRectangle(
            width=5.6, height=4.6, corner_radius=0.12,
            fill_color="#07151e", fill_opacity=0.94,
            stroke_color=ACCENT_CYAN, stroke_width=1.8
        ).move_to(RIGHT * 3.3 + DOWN * 0.2)

        badge_r = RoundedRectangle(
            width=4.4, height=0.45, corner_radius=0.08,
            fill_color="#0284c7", fill_opacity=0.35,
            stroke_color=ACCENT_CYAN, stroke_width=1.2
        ).move_to(card_right.get_top() + DOWN * 0.35)
        badge_r_t = Text("EVALUATE_FAST() BITBOARDS", font="Consolas", color=ACCENT_CYAN).scale(0.24).move_to(badge_r.get_center())

        r_spec1 = Text("• Pure 64-bit Bitboard Material", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.26)
        r_spec2 = Text("• Tapered Piece-Square Tables", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        r_spec3 = Text("• Handcrafted Pawn Structure", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        r_spec4 = Text("• Cost: ~3 nanoseconds / node", font="Consolas", color=ACCENT_GREEN).scale(0.24)
        r_specs = VGroup(r_spec1, r_spec2, r_spec3, r_spec4).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_right.get_center() + UP * 0.2)

        # Bottom Traffic Share
        r_traffic_box = RoundedRectangle(
            width=4.8, height=0.85, corner_radius=0.08,
            fill_color="#082f49", fill_opacity=0.9,
            stroke_color=ACCENT_CYAN, stroke_width=1.4
        ).move_to(card_right.get_bottom() + UP * 0.65)
        r_traffic_lbl = Text("ACTUAL SEARCH TRAFFIC:", font="Bahnschrift", color=TEXT_MUTED).scale(0.20).move_to(r_traffic_box.get_top() + DOWN * 0.22)
        r_traffic_pct = Text("99.8 % of Search Queries", font="Consolas", color=ACCENT_GREEN).scale(0.32).move_to(r_traffic_box.get_center() + DOWN * 0.12)
        r_traffic_grp = VGroup(r_traffic_box, r_traffic_lbl, r_traffic_pct)

        group_right = VGroup(card_right, badge_r, badge_r_t, r_specs, r_traffic_grp)

        self.play(FadeIn(group_left, LEFT * 0.4), FadeIn(group_right, RIGHT * 0.4), run_time=1.4)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 3: THE CODE DECEPTION VS REALITY (32s - 55s)
        # -------------------------------------------------------------
        self.play(FadeOut(group_left, LEFT * 0.3), FadeOut(group_right, RIGHT * 0.3), run_time=0.8)

        # Dual Code Card
        audit_card = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#070c18", fill_opacity=0.96,
            stroke_color="#334155", stroke_width=1.6
        ).move_to(DOWN * 0.4)

        audit_bar = RoundedRectangle(
            width=11.6, height=0.45, corner_radius=0.08,
            fill_color="#0f172a", fill_opacity=1.0,
            stroke_color="#334155", stroke_width=1.0
        ).move_to(audit_card.get_top() + DOWN * 0.225)
        audit_title = Text("COMMIT 6653436: THE MASTER PROFILER DISCOVERY", font="Consolas", color=ACCENT_GOLD).scale(0.24).move_to(audit_bar.get_center())

        # Top Section: What the CLI thought
        cli_box = RoundedRectangle(
            width=10.6, height=1.6, corner_radius=0.08,
            fill_color="#0b1120", fill_opacity=0.9,
            stroke_color="#a78bfa", stroke_width=1.2
        ).move_to(audit_card.get_top() + DOWN * 1.3)

        cli_tag = Text("WHAT THE CONSOLE & LOGS WERE TELLING US:", font="Bahnschrift", color=ACCENT_VIOLET).scale(0.20).move_to(cli_box.get_top() + DOWN * 0.25)
        cli_code = Text(
            "// uci.cpp & main.cpp:\nEvaluator::set_mode(EvalMode::SpectralTropical); // 46,000 parameter model loaded",
            font="Consolas", color=TEXT_MUTED
        ).scale(0.24).move_to(cli_box.get_center() + DOWN * 0.15)
        cli_group = VGroup(cli_box, cli_tag, cli_code)

        # Bottom Section: What the CPU was actually executing
        cpu_box = RoundedRectangle(
            width=10.6, height=2.0, corner_radius=0.08,
            fill_color="#081f2e", fill_opacity=0.95,
            stroke_color=ACCENT_GREEN, stroke_width=1.4
        ).move_to(audit_card.get_bottom() + UP * 1.3)

        cpu_tag = Text("WHAT THE CPU WAS ACTUALLY EXECUTING ON 99.8% OF NODES:", font="Bahnschrift", color=ACCENT_GREEN).scale(0.20).move_to(cpu_box.get_top() + DOWN * 0.25)
        cpu_code = Text(
            "// search.cpp - negamax_alphabeta & quiescence_search:\nif (unbalanced || qsearch || in_check || !is_pv_node)\n    return evaluate_fast(board);  // <--- 99.8% of ALL calls hit this shortcut!",
            font="Consolas", color=ACCENT_CYAN
        ).scale(0.25).move_to(cpu_box.get_center() + DOWN * 0.15)
        cpu_group = VGroup(cpu_box, cpu_tag, cpu_code)

        self.play(FadeIn(audit_card), FadeIn(audit_bar), FadeIn(audit_title), FadeIn(cli_group), FadeIn(cpu_group), run_time=1.2)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 4: THE EPIPHANY & AWAKENING (55s - 85s)
        # -------------------------------------------------------------
        awakening_card = RoundedRectangle(
            width=10.8, height=2.0, corner_radius=0.12,
            fill_color="#064e3b", fill_opacity=0.92,
            stroke_color=ACCENT_GREEN, stroke_width=2.2
        ).move_to(DOWN * 0.4)

        aw_t1 = Text("THE EPIPHANY: DISMANTLING THE COMPLEXITY BALLAST", font="Bahnschrift", color=ACCENT_GREEN).scale(0.28)
        aw_t2 = Text("Locked Engine Permanently to:  EvalMode::MasterPositional", font="Consolas", color=TEXT_BRIGHT).scale(0.38)
        aw_t3 = Text("\"We didn't need 46,000 parameters. We needed ruthless classical speed.\"", font="Bahnschrift", color="#a7f3d0").scale(0.24)
        aw_box = VGroup(aw_t1, aw_t2, aw_t3).arrange(DOWN, buff=0.16).move_to(awakening_card.get_center())

        awakening_group = VGroup(awakening_card, aw_box)

        self.play(
            FadeOut(cli_group),
            FadeOut(cpu_group),
            FadeIn(awakening_group, scale=0.95),
            audit_card.animate.set_stroke(color=ACCENT_GREEN, width=2.0),
            run_time=1.4
        )
        self.wait(3.5)
