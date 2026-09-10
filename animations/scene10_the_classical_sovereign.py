"""
Heaven's Gate Documentary - Scene 10: The Classical Sovereign (Engineering 35 Million NPS)
Runtime: ~85 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: Title Header (Chapter 08: The Master Craft / Engineering 35M NPS)
- Beat 2: The 4 Performance Pillars:
          1. 28-Loop Rayleigh Quotient & 2048 Zobrist Spectral Cache
          2. 64-byte Cache-Line TT Clusters & Lockless XOR Verification
          3. 4D Continuation History (6-ply tensor) & History Malus
          4. Root PVS, Singular Double Extensions & Safe Check Matrix
- Beat 3: Deluxe Speedometer Surge from 89,000 NPS all the way to 35,000,000 NPS!
- Beat 4: Digital Cockpit Telemetry HUD: 12 Lazy SMP Threads, Depth 14+ in 0.8s
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

class Scene10TheClassicalSovereign(Scene):
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

        head_t1 = Text("CHAPTER 08 // THE MASTER CRAFT", font="Bahnschrift", color=ACCENT_GOLD).scale(0.24)
        head_t2 = Text("10. The Classical Sovereign: Engineering 35 Million NPS", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.38)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: THE FOUR ARCHITECTURAL UPGRADES (8s - 45s)
        # -------------------------------------------------------------
        # 2x2 Grid of Engineering Upgrades
        # Top-Left: Rayleigh Quotient & Zobrist Cache
        card1 = RoundedRectangle(
            width=5.5, height=2.4, corner_radius=0.10,
            fill_color="#0b1120", fill_opacity=0.94,
            stroke_color=ACCENT_CYAN, stroke_width=1.4
        ).move_to(LEFT * 3.1 + UP * 1.3)

        c1_t1 = Text("1. 28-LOOP RAYLEIGH QUOTIENT", font="Bahnschrift", color=ACCENT_CYAN).scale(0.24).move_to(card1.get_top() + DOWN * 0.3)
        c1_t2 = Text("• Replaced QR decomposition with 28 power iterations", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        c1_t3 = Text("• 2048-entry thread-local Zobrist Spectral Cache", font="Consolas", color=ACCENT_GOLD).scale(0.21)
        c1_t4 = Text("• Eigenvalue probe in 2 nanoseconds (+300% eval speed)", font="Consolas", color=ACCENT_GREEN).scale(0.21)
        c1_box = VGroup(c1_t2, c1_t3, c1_t4).arrange(DOWN, buff=0.12, aligned_edge=LEFT).move_to(card1.get_center() + DOWN * 0.15)
        p1 = VGroup(card1, c1_t1, c1_box)

        # Top-Right: 64-byte TT & Lockless XOR
        card2 = RoundedRectangle(
            width=5.5, height=2.4, corner_radius=0.10,
            fill_color="#0b1120", fill_opacity=0.94,
            stroke_color=ACCENT_GOLD, stroke_width=1.4
        ).move_to(RIGHT * 3.1 + UP * 1.3)

        c2_t1 = Text("2. 64-BYTE TT & LOCKLESS XOR VERIFICATION", font="Bahnschrift", color=ACCENT_GOLD).scale(0.23).move_to(card2.get_top() + DOWN * 0.3)
        c2_t2 = Text("• 4-entry clusters aligned to 64-byte CPU cache lines", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        c2_t3 = Text("• Lockless verification: stored_key = key ^ data_word", font="Consolas", color=ACCENT_CYAN).scale(0.20)
        c2_t4 = Text("• Hardware __builtin_prefetch masks memory latency (+9.7%)", font="Consolas", color=ACCENT_GREEN).scale(0.20)
        c2_box = VGroup(c2_t2, c2_t3, c2_t4).arrange(DOWN, buff=0.12, aligned_edge=LEFT).move_to(card2.get_center() + DOWN * 0.15)
        p2 = VGroup(card2, c2_t1, c2_box)

        # Bottom-Left: 4D Continuation History
        card3 = RoundedRectangle(
            width=5.5, height=2.4, corner_radius=0.10,
            fill_color="#0b1120", fill_opacity=0.94,
            stroke_color=ACCENT_VIOLET, stroke_width=1.4
        ).move_to(LEFT * 3.1 + DOWN * 1.4)

        c3_t1 = Text("3. 4D CONTINUATION HISTORY (6-PLY TENSOR)", font="Bahnschrift", color=ACCENT_VIOLET).scale(0.23).move_to(card3.get_top() + DOWN * 0.3)
        c3_t2 = Text("• Scores: hist + 2*cont1 + cont2 + cont4 + (cont6/2)", font="Consolas", color=TEXT_BRIGHT).scale(0.20)
        c3_t3 = Text("• History Malus penalizes quiet non-cutoff moves", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        c3_t4 = Text("• 85%+ First-move cutoff accuracy in move picker", font="Consolas", color=ACCENT_GREEN).scale(0.21)
        c3_box = VGroup(c3_t2, c3_t3, c3_t4).arrange(DOWN, buff=0.12, aligned_edge=LEFT).move_to(card3.get_center() + DOWN * 0.15)
        p3 = VGroup(card3, c3_t1, c3_box)

        # Bottom-Right: Advanced Pruning & Safe Checks
        card4 = RoundedRectangle(
            width=5.5, height=2.4, corner_radius=0.10,
            fill_color="#0b1120", fill_opacity=0.94,
            stroke_color=ACCENT_GREEN, stroke_width=1.4
        ).move_to(RIGHT * 3.1 + DOWN * 1.4)

        c4_t1 = Text("4. ROOT PVS & SAFE CHECK DANGER MATRIX", font="Bahnschrift", color=ACCENT_GREEN).scale(0.23).move_to(card4.get_top() + DOWN * 0.3)
        c4_t2 = Text("• Root PVS: move 0 searched full; rest with [-alpha-1, -alpha]", font="Consolas", color=TEXT_MUTED).scale(0.20)
        c4_t3 = Text("• Singular Double Extensions for forced tactical replies", font="Bahnschrift", color=TEXT_MUTED).scale(0.21)
        c4_t4 = Text("• Quadratic Safe Check matrix eliminates king blindspots", font="Consolas", color=ACCENT_GOLD).scale(0.21)
        c4_box = VGroup(c4_t2, c4_t3, c4_t4).arrange(DOWN, buff=0.12, aligned_edge=LEFT).move_to(card4.get_center() + DOWN * 0.15)
        p4 = VGroup(card4, c4_t1, c4_box)

        quad_grid = VGroup(p1, p2, p3, p4)

        self.play(FadeIn(p1, LEFT * 0.3), FadeIn(p2, RIGHT * 0.3), run_time=1.2)
        self.play(FadeIn(p3, LEFT * 0.3), FadeIn(p4, RIGHT * 0.3), run_time=1.2)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 3: THE 35 MILLION NPS SPEEDOMETER SURGE (45s - 85s)
        # -------------------------------------------------------------
        self.play(FadeOut(quad_grid, scale=0.95), run_time=0.8)

        # Speedometer Gauge Assembly
        g_center = DOWN * 0.4
        r = 2.4

        outer_ring = Arc(radius=r + 0.38, start_angle=210 * DEGREES, angle=-240 * DEGREES, stroke_color="#334155", stroke_width=2.5, arc_center=g_center)
        inner_ring = Arc(radius=r - 0.28, start_angle=210 * DEGREES, angle=-240 * DEGREES, stroke_color="#1e293b", stroke_width=1.5, arc_center=g_center)

        # Colored zones along arc
        arc_red    = Arc(radius=r, start_angle=210 * DEGREES, angle=-40 * DEGREES, stroke_color=ACCENT_RED, stroke_width=8.0, arc_center=g_center)
        arc_yellow = Arc(radius=r, start_angle=170 * DEGREES, angle=-70 * DEGREES, stroke_color=ACCENT_GOLD, stroke_width=8.0, arc_center=g_center)
        arc_green  = Arc(radius=r, start_angle=100 * DEGREES, angle=-70 * DEGREES, stroke_color=ACCENT_CYAN, stroke_width=8.0, arc_center=g_center)
        arc_purple = Arc(radius=r, start_angle=30 * DEGREES,  angle=-60 * DEGREES, stroke_color=ACCENT_GREEN, stroke_width=10.0, arc_center=g_center)

        # Central Hub
        hub_outer = Circle(radius=0.35, fill_color="#0f172a", fill_opacity=1.0, stroke_color=ACCENT_GOLD, stroke_width=2.0).move_to(g_center)
        hub_inner = Circle(radius=0.15, fill_color=ACCENT_GOLD, fill_opacity=1.0).set_stroke(width=0).move_to(g_center)

        # Labels around dial
        lbl_0   = Text("0", font="Consolas", color=TEXT_MUTED).scale(0.24).move_to(g_center + np.array([-2.1, -1.1, 0]))
        lbl_100k = Text("100K", font="Consolas", color=TEXT_MUTED).scale(0.22).move_to(g_center + np.array([-2.4, 0.4, 0]))
        lbl_1m  = Text("1M", font="Consolas", color=TEXT_MUTED).scale(0.24).move_to(g_center + np.array([-1.5, 1.9, 0]))
        lbl_10m = Text("10M", font="Consolas", color=TEXT_MUTED).scale(0.24).move_to(g_center + np.array([0.0, 2.5, 0]))
        lbl_20m = Text("20M", font="Consolas", color=TEXT_MUTED).scale(0.24).move_to(g_center + np.array([1.6, 1.8, 0]))
        lbl_35m = Text("35M+", font="Consolas", color=ACCENT_GREEN).scale(0.30).move_to(g_center + np.array([2.3, -0.9, 0]))

        dial_labels = VGroup(lbl_0, lbl_100k, lbl_1m, lbl_10m, lbl_20m, lbl_35m)

        gauge_group = VGroup(
            outer_ring, inner_ring,
            arc_red, arc_yellow, arc_green, arc_purple,
            hub_outer, hub_inner, dial_labels
        )

        # Dynamic Needle pointing from center
        def make_needle(angle_deg, color=ACCENT_GOLD):
            rad = angle_deg * DEGREES
            tip = g_center + np.array([np.cos(rad) * (r + 0.15), np.sin(rad) * (r + 0.15), 0])
            n_perp = np.array([-np.sin(rad) * 0.08, np.cos(rad) * 0.08, 0])
            poly = Polygon(
                g_center - n_perp * 1.5,
                tip,
                g_center + n_perp * 1.5
            )
            poly.set_fill(color, opacity=1.0)
            poly.set_stroke(color="#ffffff", width=0.8, opacity=0.8)
            return poly

        needle = make_needle(210)

        # Digital HUD readout below dial
        hud_box = RoundedRectangle(
            width=8.4, height=1.5, corner_radius=0.10,
            fill_color="#07131e", fill_opacity=0.96,
            stroke_color=ACCENT_CYAN, stroke_width=1.6
        ).move_to(DOWN * 2.3)

        hud_title = Text("BENCHMARK TELEMETRY // LOCKLESS LAZY SMP (12 THREADS)", font="Bahnschrift", color=TEXT_MUTED).scale(0.22).move_to(hud_box.get_top() + DOWN * 0.25)
        hud_nps = Text("CURRENT SEARCH:  35,420,119 NODES / SEC", font="Consolas", color=ACCENT_GREEN).scale(0.38).move_to(hud_box.get_center() + DOWN * 0.05)
        hud_depth = Text("Depth: 14 reached in 0.82 seconds  |  TT Hits: 74.8%  |  First Move Cutoff: 87.2%", font="Consolas", color=ACCENT_CYAN).scale(0.20).move_to(hud_box.get_bottom() + UP * 0.22)
        hud_group = VGroup(hud_box, hud_title, hud_nps, hud_depth)

        self.play(FadeIn(gauge_group), FadeIn(needle), FadeIn(hud_group), run_time=1.2)
        self.wait(1.0)

        # Animate needle sweeping triumphantly from 210 degrees (0) to -30 degrees (35M NPS)!
        needle_surge = make_needle(-25, color=ACCENT_GREEN)
        self.play(
            Transform(needle, needle_surge),
            hud_box.animate.set_stroke(color=ACCENT_GREEN, width=2.2),
            run_time=2.2,
            rate_func=rush_into
        )

        # Small triumphant bounce at 35M+
        self.play(
            needle.animate.rotate(-4 * DEGREES, about_point=g_center),
            run_time=0.15
        )
        self.play(
            needle.animate.rotate(4 * DEGREES, about_point=g_center),
            run_time=0.15
        )
        self.wait(3.5)
