"""
Heaven's Gate Documentary - Scene 08: The Hall of Shame (Deluxe 3B1B Edition)
Runtime: ~90 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure High-Density 3Blue1Brown (ManimGL)
Features:
- Layer 0: Grounding tech blueprint grid (NumberPlane)
- Beat 1: The Hall of Shame Header & Git Commit Ledger
- Beat 2: Disaster #1: The Inverted Board (Commit 74c118c) - King marching forward
- Beat 3: Disaster #2: Target Sign Inversion (Commit 2e979cb) - Diverging loss & Queen blunder
- Beat 4: Disaster #3: Material Blindness Catastrophe (Commit af99ca7) - w_0 = 0
- Beat 5: Disaster #4: Ghost En Passant (Commit 4d064af) - Occupancy bitboard leak
- Beat 6: Disaster #5: The Breaking Point & The Great Revert (Commit 8e240be) - 200 commits wiped
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

class Scene08HallOfShame(Scene):
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
        # BEAT 1: THE HALL OF SHAME TITLE (0s - 8s)
        # -------------------------------------------------------------
        head_card = RoundedRectangle(
            width=11.4, height=1.1, corner_radius=0.10,
            fill_color="#180a0e", fill_opacity=0.94,
            stroke_color="#f43f5e", stroke_width=1.6
        ).move_to(UP * 3.2)

        head_t1 = Text("CHAPTER 06 // LESSONS FROM 432 COMMITS", font="Bahnschrift", color=ACCENT_RED).scale(0.24)
        head_t2 = Text("8. The Hall of Shame: 5 Disasters, Sacrificed Queens & The Great Revert", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.36)
        head_text = VGroup(head_t1, head_t2).arrange(DOWN, buff=0.08).move_to(head_card.get_center())
        header_group = VGroup(head_card, head_text)

        self.play(FadeIn(header_group, UP * 0.3), run_time=1.0)
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 2: DISASTER 1: THE INVERTED BOARD (8s - 25s)
        # -------------------------------------------------------------
        card_d1 = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.95,
            stroke_color="#334155", stroke_width=1.5
        ).move_to(DOWN * 0.4)

        d1_badge = RoundedRectangle(
            width=5.2, height=0.45, corner_radius=0.08,
            fill_color="#991b1b", fill_opacity=0.4,
            stroke_color=ACCENT_RED, stroke_width=1.2
        ).move_to(card_d1.get_top() + DOWN * 0.35)
        d1_badge_t = Text("DISASTER #1 // COMMIT 74c118c: THE INVERTED BOARD", font="Consolas", color="#fca5a5").scale(0.22).move_to(d1_badge.get_center())

        cb1_center = card_d1.get_left() + RIGHT * 2.6 + DOWN * 0.2
        cb1 = BroadcastChessBoard(center=cb1_center, sq_size=3.8 / 8.0)
        wk_piece1 = cb1.create_piece("wK", 4, 3) # King in center

        # Arrow showing King marching from e1 to e4
        arrow_k = cb1.create_tactical_arrow(cb1.get_bottom() + UP * 0.48, cb1.get_center(), color=ACCENT_RED)
        lbl_k = Text("Move 4: Ke1 -> Ke4!", font="Consolas", color=ACCENT_RED).scale(0.24).next_to(arrow_k, RIGHT, buff=0.15)

        # Right side diagnosis
        d1_diag_t1 = Text("Bug Diagnosis:", font="Bahnschrift", color=ACCENT_GOLD).scale(0.32)
        d1_diag_t2 = Text("PST Ranks Vertically Inverted: [Rank 1 == Rank 8]", font="Consolas", color=ACCENT_RED).scale(0.28)
        d1_diag_t3 = Text("• The engine evaluated its own starting Rank 1 as the 8th rank promotion zone.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d1_diag_t4 = Text("• Looked at Ke1, calculated +3.5 cp bonus for an 'advanced king in enemy territory'.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d1_diag_t5 = Text("• Marched king straight into the center on move 4, walking into enemy fire.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d1_diag_box = VGroup(d1_diag_t1, d1_diag_t2, d1_diag_t3, d1_diag_t4, d1_diag_t5).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_d1.get_right() + LEFT * 3.4 + DOWN * 0.2)

        d1_group = VGroup(card_d1, d1_badge, d1_badge_t, cb1, wk_piece1, arrow_k, lbl_k, d1_diag_box)

        self.play(FadeIn(d1_group, scale=0.96), run_time=1.2)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 3: DISASTER 2: THE TARGET SIGN INVERSION (25s - 42s)
        # -------------------------------------------------------------
        card_d2 = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.95,
            stroke_color="#334155", stroke_width=1.5
        ).move_to(DOWN * 0.4)

        d2_badge = RoundedRectangle(
            width=5.8, height=0.45, corner_radius=0.08,
            fill_color="#991b1b", fill_opacity=0.4,
            stroke_color=ACCENT_RED, stroke_width=1.2
        ).move_to(card_d2.get_top() + DOWN * 0.35)
        d2_badge_t = Text("DISASTER #2 // COMMIT 2e979cb: TARGET SIGN INVERSION", font="Consolas", color="#fca5a5").scale(0.22).move_to(d2_badge.get_center())

        # Left side: Diverging Loss Graph
        loss_axes = Axes(
            x_range=[0, 60, 10],
            y_range=[0, 10, 2],
            width=4.4,
            height=3.0,
            axis_config={"stroke_color": "#334155", "stroke_width": 1.0}
        ).move_to(card_d2.get_left() + RIGHT * 2.8 + DOWN * 0.3)

        # White Loss (descends) vs Black Loss (explodes)
        curve_white = loss_axes.get_graph(lambda x: 8.0 * np.exp(-0.06 * x) + 0.8, x_range=[0, 60], color=ACCENT_CYAN).set_stroke(width=2.5)
        curve_black = loss_axes.get_graph(lambda x: 1.0 + 0.12 * x + 0.0018 * (x**2), x_range=[0, 60], color=ACCENT_RED).set_stroke(width=3.0)

        lbl_w_loss = Text("White Loss (Converging)", font="Consolas", color=ACCENT_CYAN).scale(0.20).next_to(curve_white.get_end(), RIGHT, buff=0.1)
        lbl_b_loss = Text("Black Loss (DIVERGING)", font="Consolas", color=ACCENT_RED).scale(0.20).next_to(curve_black.get_end(), UP, buff=0.1)

        graph_group = VGroup(loss_axes, curve_white, curve_black, lbl_w_loss, lbl_b_loss)

        # Right side diagnosis
        d2_diag_t1 = Text("The 60-Round Inversion Bug:", font="Bahnschrift", color=ACCENT_GOLD).scale(0.32)
        d2_diag_t2 = Text("Gradient target sign was inverted for Black side-to-move.", font="Consolas", color=ACCENT_RED).scale(0.26)
        d2_diag_t3 = Text("• For 60 continuous training rounds, Adam SGD minimized White error...", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d2_diag_t4 = Text("• ...while MAXIMIZING Black's blunder probability!", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d2_diag_t5 = Text("• The engine learned to enthusiastically sacrifice Black's Queen on move 5.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d2_diag_box = VGroup(d2_diag_t1, d2_diag_t2, d2_diag_t3, d2_diag_t4, d2_diag_t5).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_d2.get_right() + LEFT * 3.4 + DOWN * 0.2)

        d2_group = VGroup(card_d2, d2_badge, d2_badge_t, graph_group, d2_diag_box)

        self.play(Transform(d1_group, d2_group), run_time=1.2)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 4: DISASTER 3: MATERIAL BLINDNESS CATASTROPHE (42s - 60s)
        # -------------------------------------------------------------
        card_d3 = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#0b1120", fill_opacity=0.95,
            stroke_color="#334155", stroke_width=1.5
        ).move_to(DOWN * 0.4)

        d3_badge = RoundedRectangle(
            width=5.8, height=0.45, corner_radius=0.08,
            fill_color="#991b1b", fill_opacity=0.4,
            stroke_color=ACCENT_RED, stroke_width=1.2
        ).move_to(card_d3.get_top() + DOWN * 0.35)
        d3_badge_t = Text("DISASTER #3 // COMMIT af99ca7: MATERIAL BLINDNESS", font="Consolas", color="#fca5a5").scale(0.22).move_to(d3_badge.get_center())

        # Left side: Optimizer equation & zero weight
        loss_box = RoundedRectangle(
            width=4.6, height=3.4, corner_radius=0.10,
            fill_color="#070c18", fill_opacity=0.9,
            stroke_color="#334155", stroke_width=1.2
        ).move_to(card_d3.get_left() + RIGHT * 2.8 + DOWN * 0.2)

        eq_title = Text("Optimizer Mathematical Loophole:", font="Bahnschrift", color=TEXT_MUTED).scale(0.22).move_to(loss_box.get_top() + DOWN * 0.35)
        eq_text = Text("w_material -> 0.000", font="Consolas", color=ACCENT_RED).scale(0.48).move_to(loss_box.get_center() + UP * 0.3)
        eq_sub = Text("In equal/drawn positions, zeroing material weight\nreduced MSE error across the training dataset!", font="Consolas", color=TEXT_MUTED).scale(0.20).move_to(loss_box.get_bottom() + UP * 0.7)

        w_box = VGroup(loss_box, eq_title, eq_text, eq_sub)

        # Right side diagnosis
        d3_diag_t1 = Text("The 'Pure Topological Harmony' Trap:", font="Bahnschrift", color=ACCENT_GOLD).scale(0.32)
        d3_diag_t2 = Text("Evaluation: 'Queen lost? Material = 0. Harmony = 100%!'", font="Consolas", color=ACCENT_RED).scale(0.26)
        d3_diag_t3 = Text("• The engine genuinely stopped caring about pawns, knights, rooks, or queens.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d3_diag_t4 = Text("• Sacrificed its queen on move 7 for a knight because 'topology was cleaner'.", font="Bahnschrift", color=TEXT_MUTED).scale(0.24)
        d3_diag_t5 = Text("• The Fix: Enforced a hardcoded mathematical floor (w0 >= 10.0 cp).", font="Consolas", color=ACCENT_GREEN).scale(0.24)
        d3_diag_box = VGroup(d3_diag_t1, d3_diag_t2, d3_diag_t3, d3_diag_t4, d3_diag_t5).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_d3.get_right() + LEFT * 3.4 + DOWN * 0.2)

        d3_group = VGroup(card_d3, d3_badge, d3_badge_t, w_box, d3_diag_box)

        self.play(Transform(d1_group, d3_group), run_time=1.2)
        self.wait(2.5)

        # -------------------------------------------------------------
        # BEAT 5: DISASTER 5: THE GREAT REVERT (COMMIT 8e240be) (60s - 85s)
        # -------------------------------------------------------------
        card_d5 = RoundedRectangle(
            width=11.6, height=5.2, corner_radius=0.12,
            fill_color="#18070d", fill_opacity=0.96,
            stroke_color=ACCENT_RED, stroke_width=2.0
        ).move_to(DOWN * 0.4)

        d5_badge = RoundedRectangle(
            width=6.6, height=0.5, corner_radius=0.08,
            fill_color="#991b1b", fill_opacity=0.6,
            stroke_color=ACCENT_RED, stroke_width=1.6
        ).move_to(card_d5.get_top() + DOWN * 0.4)
        d5_badge_t = Text("COMMIT 8e240be // AUGUST 11, 2:00 AM // THE BREAKING POINT", font="Consolas", color="#ffffff").scale(0.23).move_to(d5_badge.get_center())

        # Giant REVERT Stamp
        stamp_box = RoundedRectangle(
            width=4.8, height=2.2, corner_radius=0.16,
            fill_color="#450a0a", fill_opacity=0.92,
            stroke_color=ACCENT_RED, stroke_width=3.0
        ).move_to(card_d5.get_left() + RIGHT * 3.0 + DOWN * 0.3).rotate(10 * DEGREES)

        stamp_t1 = Text("CRITICAL REVERT", font="Bahnschrift", color=ACCENT_RED).scale(0.48).move_to(stamp_box.get_center() + UP * 0.3)
        stamp_t2 = Text("WIPED 200+ COMMITS", font="Consolas", color="#fca5a5").scale(0.32).move_to(stamp_box.get_center() + DOWN * 0.3)
        stamp_group = VGroup(stamp_box, stamp_t1, stamp_t2)

        # Right side: What was rolled back
        roll_t1 = Text("The Great Restoration:", font="Bahnschrift", color=TEXT_BRIGHT).scale(0.34)
        roll_t2 = Text("Tore down Phase 4 Tropical Rational Functions [T1 - T2]", font="Consolas", color="#f87171").scale(0.24)
        roll_t3 = Text("Tore down Phase 5 Chebyshev Spectral Filters [T2(L)]", font="Consolas", color="#f87171").scale(0.24)
        roll_t4 = Text("Tore down 25-dimensional feature vectors & memory leaks", font="Consolas", color="#f87171").scale(0.24)
        roll_t5 = Text("Rolled the entire codebase back to Phase 2 Round 17", font="Consolas", color=ACCENT_GOLD).scale(0.25)
        roll_t6 = Text("Result: Zero crashes. Clean slate for the biggest twist.", font="Bahnschrift", color=ACCENT_GREEN).scale(0.26)

        rollback_box = VGroup(roll_t1, roll_t2, roll_t3, roll_t4, roll_t5, roll_t6).arrange(DOWN, buff=0.18, aligned_edge=LEFT).move_to(card_d5.get_right() + LEFT * 3.2 + DOWN * 0.2)

        d5_group = VGroup(card_d5, d5_badge, d5_badge_t, stamp_group, rollback_box)

        self.play(Transform(d1_group, d5_group), run_time=1.4)
        self.wait(3.0)
