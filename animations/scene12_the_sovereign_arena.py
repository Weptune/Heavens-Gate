"""
Heaven's Gate Documentary - Scene 12: The Sovereign Arena & Geometry Speaks
Runtime: ~95 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: Title Header (Chapter 10: Geometry Speaks / The Sovereign Arena)
- Beat 2: The Sovereign Arena Web UI in Deep Obsidian & Celestial Gold
          Oracle Live Commentary ("⚡ BRILLIANT"), Accuracy HUD
- Beat 3: The Post-Match Autopsy Card (Survival plies, Sovereign Verdict)
- Beat 4: Cosmic Pull-Back: The Spectral Tactical Graph & Vector Crest
- Beat 5: The First Principles Manifesto & Open-Source GitHub Outro
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

class Scene12TheSovereignArena(Scene):
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

        head_t1 = Text("CHAPTER 10 // THE SOVEREIGN EXPERIENCE", font="Bahnschrift", color=ACCENT_GOLD).scale(0.24)
        head_t2 = Text("12. The Sovereign Arena & Conclusion: Geometry Speaks", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.38)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: THE SOVEREIGN ARENA BROWSER UI (8s - 38s)
        # -------------------------------------------------------------
        # Browser Window Container
        browser_card = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#060913", fill_opacity=0.97,
            stroke_color="#334155", stroke_width=1.6
        ).move_to(DOWN * 0.4)

        # Top Bar
        browser_bar = RoundedRectangle(
            width=11.6, height=0.45, corner_radius=0.08,
            fill_color="#0f172a", fill_opacity=1.0,
            stroke_color="#334155", stroke_width=1.0
        ).move_to(browser_card.get_top() + DOWN * 0.225)

        dot_r = Circle(radius=0.08, fill_color="#ef4444", fill_opacity=1.0).set_stroke(width=0).move_to(browser_bar.get_left() + RIGHT * 0.3)
        dot_y = Circle(radius=0.08, fill_color="#f59e0b", fill_opacity=1.0).set_stroke(width=0).next_to(dot_r, RIGHT, buff=0.12)
        dot_g = Circle(radius=0.08, fill_color="#10b981", fill_opacity=1.0).set_stroke(width=0).next_to(dot_y, RIGHT, buff=0.12)
        url_text = Text("https://heavensgate.dev • THE SOVEREIGN ARENA [NO NEURAL NETWORKS • PURE HCE]", font="Consolas", color=ACCENT_GOLD).scale(0.22).move_to(browser_bar.get_center())
        b_header = VGroup(browser_bar, dot_r, dot_y, dot_g, url_text)

        # Challenge Subheader
        chal_text = Text("\"try your hand against heaven's gate :)\"", font="Bahnschrift", color=TEXT_MUTED).scale(0.24).move_to(browser_card.get_top() + DOWN * 0.7)

        # Left Side: Chessboard in Arena
        cb_center = browser_card.get_left() + RIGHT * 2.8 + DOWN * 0.35
        cb = BroadcastChessBoard(center=cb_center, sq_size=3.8 / 8.0)
        p_wk = cb.create_piece("wK", 6, 0)
        p_wq = cb.create_piece("wQ", 3, 2)
        p_wb = cb.create_piece("wB", 4, 3)
        p_bk = cb.create_piece("bK", 6, 7)
        pieces = VGroup(p_wk, p_wq, p_wb, p_bk)

        # Right Side: The Oracle Commentary HUD
        oracle_box = RoundedRectangle(
            width=5.2, height=3.8, corner_radius=0.10,
            fill_color="#0c1427", fill_opacity=0.92,
            stroke_color=ACCENT_GOLD, stroke_width=1.4
        ).move_to(browser_card.get_right() + LEFT * 3.1 + DOWN * 0.35)

        ora_title = Text("THE ORACLE // REAL-TIME EVALUATION", font="Bahnschrift", color=ACCENT_GOLD).scale(0.24).move_to(oracle_box.get_top() + DOWN * 0.3)

        # Move classification badge: Brilliant
        brill_badge = RoundedRectangle(
            width=4.4, height=0.75, corner_radius=0.08,
            fill_color="#064e3b", fill_opacity=0.9,
            stroke_color=ACCENT_GREEN, stroke_width=1.5
        ).move_to(oracle_box.get_center() + UP * 0.7)
        brill_text = Text("⚡ BRILLIANT MOVE  [ 14... e5! ]", font="Consolas", color="#f8fafc").scale(0.28).move_to(brill_badge.get_center())
        brill_grp = VGroup(brill_badge, brill_text)

        ora_desc1 = Text("• Win-Probability-Loss (WPL) Model Active", font="Bahnschrift", color=TEXT_MUTED).scale(0.22)
        ora_desc2 = Text("• Evaluation: +2.45 cp | Sovereign Depth 16", font="Consolas", color=ACCENT_CYAN).scale(0.22)
        ora_desc3 = Text("• Complete Tactical Refutation Loaded", font="Bahnschrift", color=TEXT_MUTED).scale(0.22)
        ora_descs = VGroup(ora_desc1, ora_desc2, ora_desc3).arrange(DOWN, buff=0.16, aligned_edge=LEFT).move_to(oracle_box.get_center() + DOWN * 0.35)

        oracle_group = VGroup(oracle_box, ora_title, brill_grp, ora_descs)

        arena_group = VGroup(browser_card, b_header, chal_text, cb, pieces, oracle_group)

        self.play(FadeIn(arena_group, scale=0.96), run_time=1.4)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 3: POST-MATCH AUTOPSY CARD (38s - 60s)
        # -------------------------------------------------------------
        autopsy_card = RoundedRectangle(
            width=5.2, height=3.8, corner_radius=0.10,
            fill_color="#18070d", fill_opacity=0.95,
            stroke_color=ACCENT_RED, stroke_width=1.6
        ).move_to(oracle_box.get_center())

        auto_title = Text("MATCH AUTOPSY // DIAGNOSTIC VERDICT", font="Bahnschrift", color=ACCENT_RED).scale(0.23).move_to(autopsy_card.get_top() + DOWN * 0.3)

        auto_stat1 = Text("SURVIVED: 22 PLIES (11 MOVES)", font="Consolas", color=TEXT_BRIGHT).scale(0.26)
        auto_stat2 = Text("Peak Evaluation Swing: +8.42 pawns", font="Consolas", color=ACCENT_GOLD).scale(0.22)
        auto_stat3 = Text("Fatal Blunder: Move 8... Nd7? (-4.80 cp)", font="Consolas", color=ACCENT_RED).scale(0.22)

        verdict_box = RoundedRectangle(
            width=4.6, height=1.1, corner_radius=0.06,
            fill_color="#0b1120", fill_opacity=0.9,
            stroke_color="#334155", stroke_width=1.0
        ).move_to(autopsy_card.get_bottom() + UP * 0.8)

        v_lbl = Text("SOVEREIGN VERDICT:", font="Bahnschrift", color=TEXT_DIM).scale(0.18).move_to(verdict_box.get_top() + DOWN * 0.18)
        v_txt = Text("\"Tactical communication fractured on the\nQueenside file. Defending rook isolated.\"", font="Consolas", color="#fca5a5").scale(0.19).move_to(verdict_box.get_center() + DOWN * 0.1)
        verdict_grp = VGroup(verdict_box, v_lbl, v_txt)

        autopsy_stats = VGroup(auto_stat1, auto_stat2, auto_stat3).arrange(DOWN, buff=0.14, aligned_edge=LEFT).move_to(autopsy_card.get_center() + UP * 0.4)

        autopsy_group = VGroup(autopsy_card, auto_title, autopsy_stats, verdict_grp)

        self.play(Transform(oracle_group, autopsy_group), run_time=1.2)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 4: COSMIC PULL-BACK & SPECTRAL GRAPH (60s - 80s)
        # -------------------------------------------------------------
        self.play(FadeOut(arena_group), FadeOut(header_group), run_time=1.0)

        # Tactical Spectral Network returns in center
        node_pts = [
            np.array([-2.2, 1.2, 0]),
            np.array([-0.8, 1.8, 0]),
            np.array([1.2, 1.5, 0]),
            np.array([2.4, 0.4, 0]),
            np.array([1.8, -1.2, 0]),
            np.array([0.2, -1.8, 0]),
            np.array([-1.5, -1.2, 0]),
            np.array([-2.6, -0.2, 0]),
        ]

        net_edges = VGroup()
        for i in range(len(node_pts)):
            for j in range(i + 1, len(node_pts)):
                if (i + j) % 2 == 0 or abs(i - j) == 1:
                    e = Line(node_pts[i], node_pts[j], stroke_color=ACCENT_CYAN, stroke_width=1.4, stroke_opacity=0.6)
                    net_edges.add(e)

        net_nodes = VGroup()
        for pt in node_pts:
            dot = Circle(radius=0.14, fill_color=ACCENT_GOLD, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.0).move_to(pt)
            net_nodes.add(dot)

        crest_path = Path("assets/heavensgate_icon.svg")
        if crest_path.exists():
            crest = SVGMobject(str(crest_path)).set_height(2.0).move_to(ORIGIN)
        else:
            crest = Circle(radius=1.0, fill_color=ACCENT_GOLD, fill_opacity=0.3, stroke_color=ACCENT_GOLD, stroke_width=2.0)

        manifesto_t1 = Text("FIRST PRINCIPLES OVER STATISTICAL GUESSWORK", font="Bahnschrift", color=ACCENT_GOLD).scale(0.32).move_to(UP * 2.8)
        manifesto_t2 = Text("\"Sometimes, you can just let the geometry of the network speak for itself.\"", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.34).move_to(DOWN * 2.6)

        final_group = VGroup(net_edges, net_nodes, crest, manifesto_t1, manifesto_t2)

        self.play(FadeIn(final_group, scale=0.9), run_time=1.6)
        self.wait(3.0)

        # -------------------------------------------------------------
        # BEAT 5: OUTRO & GITHUB BANNER (80s - 95s)
        # -------------------------------------------------------------
        github_card = RoundedRectangle(
            width=8.4, height=1.2, corner_radius=0.10,
            fill_color="#0f172a", fill_opacity=0.95,
            stroke_color=ACCENT_CYAN, stroke_width=1.6
        ).move_to(DOWN * 2.6)

        gh_t1 = Text("100% OPEN SOURCE ON GITHUB", font="Bahnschrift", color=ACCENT_GREEN).scale(0.24).move_to(github_card.get_top() + DOWN * 0.3)
        gh_t2 = Text("github.com / weeping-angel / heavensgate", font="Consolas", color=TEXT_BRIGHT).scale(0.36).move_to(github_card.get_center() + DOWN * 0.15)
        github_banner = VGroup(github_card, gh_t1, gh_t2)

        self.play(
            Transform(manifesto_t2, github_banner),
            crest.animate.scale(1.1),
            run_time=1.4
        )
        self.wait(4.0)
