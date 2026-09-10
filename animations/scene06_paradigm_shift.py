"""
Heaven's Gate Documentary - Scene 06: The Paradigm Shift (Analytical Math vs Statistical Guesswork)
Script Beat:
- "This is where Spectral Graph Theory turns classical engine architecture on its head."
- "When you replace an opaque neural network with the Graph Laplacian, three fundamental things happen:"
- "First: Zero Training Weights. No GPUs, no self-play database, no statistical bias. Derived purely from linear algebra."
- "Second: Complete Interpretability (The White Box). Output is not a mysterious float; it is a full topological map."
- "Third: Depth 0 Topological Vision. Closed-form eigensolve diagnoses the severed board on move 0 without 30-ply search."
- "Instead of trying to approximate human intuition through statistical guessing... we let the geometry of the network speak for itself."

Resolution: 1920x1080 | 16:9 Landscape
Aesthetic: Editorial Tech Documentary (Split-screen comparative cards, broadcast chessboard, Fiedler vector heatmap & fault line, telemetry HUD)
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

# Import our dedicated broadcast chessboard component
sys.path.append(str(Path(__file__).parent))
from chessboard_widget import (
    BroadcastChessBoard,
    FIEDLER_POS_COLOR,
    FIEDLER_NEG_COLOR,
    FIEDLER_CUT_COLOR,
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
ACCENT_GOLD   = "#fbbf24"
ACCENT_RED    = "#f43f5e"
ACCENT_GREEN  = "#34d399"

class Scene06TheParadigmShift(Scene):
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
        header_tag = Text("THE PARADIGM SHIFT // FIRST PRINCIPLES VS BRUTE FORCE", font="Consolas", color=ACCENT_CYAN).scale(0.24)
        header_tag.to_edge(UP, buff=0.55).to_edge(LEFT, buff=0.85)

        header_title = Text("Analytical Math vs. Statistical Guesswork", font="Segoe UI", color=TEXT_BRIGHT).scale(0.48)
        header_title.next_to(header_tag, DOWN, aligned_edge=LEFT, buff=0.10)

        self.play(
            FadeIn(header_tag, UP * 0.1),
            FadeIn(header_title, UP * 0.15),
            run_time=0.9
        )
        self.wait(0.5)

        # -------------------------------------------------------------
        # 3. Stage 1: The Split Screen Comparison
        # -------------------------------------------------------------
        card_w, card_h = 6.4, 5.3

        # LEFT CARD: The Statistical Black Box (NNUE)
        nn_card_bg = RoundedRectangle(
            width=card_w, height=card_h, corner_radius=0.16,
            fill_color="#0c101a", fill_opacity=0.95,
            stroke_color="#334155", stroke_width=1.5
        )
        nn_card_bg.move_to(LEFT * 3.55 + DOWN * 0.50)

        nn_badge = RoundedRectangle(width=4.2, height=0.44, corner_radius=0.08, fill_color="#261217", fill_opacity=0.95, stroke_color=ACCENT_RED, stroke_width=1.2)
        nn_badge_txt = Text("STATISTICAL APPROXIMATOR (NNUE)", font="Consolas", color=ACCENT_RED).scale(0.22)
        nn_badge_txt.move_to(nn_badge.get_center())
        nn_badge_group = VGroup(nn_badge, nn_badge_txt)
        nn_badge_group.move_to(nn_card_bg.get_top() + DOWN * 0.45)

        nn_sub = Text("4,300,000 Floating-Point Weights", font="Segoe UI", color=TEXT_MUTED).scale(0.28)
        nn_sub.next_to(nn_badge_group, DOWN, buff=0.12)

        # Neural Net Synapse Diagram
        nn_diagram = VGroup()
        layer_sizes = [5, 5, 4, 1]
        layer_xs = np.linspace(-1.9, 1.9, 4)
        node_map = {}
        for l_idx, (sz, lx) in enumerate(zip(layer_sizes, layer_xs)):
            ys = np.linspace(0.65, -0.65, sz)
            for n_idx, ly in enumerate(ys):
                is_out = (l_idx == 3)
                dot = Circle(
                    radius=0.09 if is_out else 0.075,
                    fill_color="#220b10" if is_out else "#1e293b",
                    fill_opacity=1.0,
                    stroke_color=ACCENT_RED if is_out else "#475569",
                    stroke_width=1.5 if is_out else 1.0
                )
                dot.move_to(nn_card_bg.get_center() + UP * 0.70 + np.array([lx, ly, 0]))
                nn_diagram.add(dot)
                node_map[(l_idx, n_idx)] = dot

        synapses = VGroup()
        for l_idx in range(len(layer_sizes) - 1):
            for i in range(layer_sizes[l_idx]):
                for j in range(layer_sizes[l_idx + 1]):
                    line = Line(
                        node_map[(l_idx, i)].get_center(),
                        node_map[(l_idx + 1, j)].get_center(),
                        color="#334155", stroke_width=0.6, stroke_opacity=0.45
                    )
                    synapses.add(line)

        out_lbl = Text("Output: +0.42", font="Consolas", color=ACCENT_RED).scale(0.22)
        out_lbl.next_to(node_map[(3, 0)], UP, buff=0.12)
        nn_vis = VGroup(synapses, nn_diagram, out_lbl)

        nn_bullet1 = Text("• Training: Billions of self-play games & GPU hours", font="Segoe UI", color="#cbd5e1").scale(0.24)
        nn_bullet2 = Text("• Mechanism: Correlation & pattern interpolation", font="Segoe UI", color="#cbd5e1").scale(0.24)
        nn_bullet3 = Text("• Interpretability: 0% — Single opaque scalar output", font="Segoe UI", color="#f87171").scale(0.24)
        nn_bullet4 = Text("• Blind Spot: Horizon drift in locked pawn structures", font="Segoe UI", color="#f87171").scale(0.24)
        nn_bullets = VGroup(nn_bullet1, nn_bullet2, nn_bullet3, nn_bullet4).arrange(DOWN, aligned_edge=LEFT, buff=0.16)
        nn_bullets.move_to(nn_card_bg.get_center() + DOWN * 1.35)

        nn_group = VGroup(nn_card_bg, nn_badge_group, nn_sub, nn_vis, nn_bullets)

        # RIGHT CARD: The Analytical White Box (Spectral Graph)
        sp_card_bg = RoundedRectangle(
            width=card_w, height=card_h, corner_radius=0.16,
            fill_color="#0a111e", fill_opacity=0.95,
            stroke_color=BORDER_COL, stroke_width=1.5
        )
        sp_card_bg.move_to(RIGHT * 3.55 + DOWN * 0.50)

        sp_badge = RoundedRectangle(width=4.2, height=0.44, corner_radius=0.08, fill_color="#0b2238", fill_opacity=0.95, stroke_color=ACCENT_CYAN, stroke_width=1.2)
        sp_badge_txt = Text("ANALYTICAL FIRST PRINCIPLES", font="Consolas", color=ACCENT_CYAN).scale(0.22)
        sp_badge_txt.move_to(sp_badge.get_center())
        sp_badge_group = VGroup(sp_badge, sp_badge_txt)
        sp_badge_group.move_to(sp_card_bg.get_top() + DOWN * 0.45)

        sp_sub = Text("0 Weights • Closed-Form Linear Algebra", font="Segoe UI", color=TEXT_MUTED).scale(0.28)
        sp_sub.next_to(sp_badge_group, DOWN, buff=0.12)

        # Spectral Math Box
        math_box = RoundedRectangle(width=4.8, height=1.45, corner_radius=0.12, fill_color="#0f1b30", fill_opacity=0.9, stroke_color=ACCENT_CYAN, stroke_width=1.2)
        math_box.move_to(sp_card_bg.get_center() + UP * 0.70)
        eq1 = Text("L = D - A   (Discrete Laplacian)", font="Consolas", color=ACCENT_CYAN).scale(0.25)
        eq2 = Text("L v₂ = λ₂ v₂   (Fiedler Eigenvector)", font="Consolas", color=TEXT_BRIGHT).scale(0.28)
        eq_sub = Text("Zero Weights • Pure First-Principles Physics", font="Segoe UI", color=ACCENT_GOLD).scale(0.20)
        eq_group = VGroup(eq1, eq2, eq_sub).arrange(DOWN, buff=0.10).move_to(math_box.get_center())
        sp_vis = VGroup(math_box, eq_group)

        sp_bullet1 = Text("• Training: Exactly 0 weights (No GPUs, zero training data)", font="Segoe UI", color="#cbd5e1").scale(0.24)
        sp_bullet2 = Text("• Mechanism: Graph Laplacian & algebraic connectivity", font="Segoe UI", color="#cbd5e1").scale(0.24)
        sp_bullet3 = Text("• Interpretability: 100% — Full 2D spatial topological map", font="Segoe UI", color=ACCENT_CYAN).scale(0.24)
        sp_bullet4 = Text("• Superpower: Instant Depth 0 bottleneck diagnosis", font="Segoe UI", color=ACCENT_GREEN).scale(0.24)
        sp_bullets = VGroup(sp_bullet1, sp_bullet2, sp_bullet3, sp_bullet4).arrange(DOWN, aligned_edge=LEFT, buff=0.16)
        sp_bullets.move_to(sp_card_bg.get_center() + DOWN * 1.35)

        sp_group = VGroup(sp_card_bg, sp_badge_group, sp_sub, sp_vis, sp_bullets)

        # Animate Stage 1 Split Screen
        self.play(
            FadeIn(nn_group, LEFT * 0.3),
            FadeIn(sp_group, RIGHT * 0.3),
            run_time=1.2
        )
        self.wait(2.5)

        # -------------------------------------------------------------
        # 4. Stage 2: Deep Dive into the 3 Pillars
        # -------------------------------------------------------------
        self.play(
            FadeOut(nn_group, LEFT * 0.4),
            FadeOut(sp_group, RIGHT * 0.4),
            run_time=0.8
        )

        # Update Subheader for Pillars
        header_tag_p = Text("THE THREE PILLARS // WHY SPECTRAL GRAPHS CHANGE CHESS ENGINES", font="Consolas", color=ACCENT_GOLD).scale(0.24)
        header_tag_p.to_edge(UP, buff=0.55).to_edge(LEFT, buff=0.85)

        header_title_p = Text("Zero Weights, Total Interpretability, and Depth 0 Vision", font="Segoe UI", color=TEXT_BRIGHT).scale(0.46)
        header_title_p.next_to(header_tag_p, DOWN, aligned_edge=LEFT, buff=0.10)

        self.play(
            Transform(header_tag, header_tag_p),
            Transform(header_title, header_title_p),
            run_time=0.7
        )

        # Broadcast ChessBoard on the Left
        board = BroadcastChessBoard(
            center=LEFT * 3.7 + DOWN * 0.30,
            sq_size=0.54,
            show_coords=True
        )

        # Real locked center position
        # Locked pawns: White c4, d5, e4 vs Black c5, d6, e5
        # White attack: Nf5, Qh5, Rf1, Kg1
        # Black king: Kg8, pawns g7, h7
        # Black stranded: Ra8, Nb8, pawns a7, b7
        pieces = VGroup(
            # White locked pawns
            board.create_piece("wP", 2, 3), # c4
            board.create_piece("wP", 3, 4), # d5
            board.create_piece("wP", 4, 3), # e4
            # Black locked pawns
            board.create_piece("bP", 2, 4), # c5
            board.create_piece("bP", 3, 5), # d6
            board.create_piece("bP", 4, 4), # e5
            # White kingside attack fist
            board.create_piece("wN", 5, 4), # Nf5
            board.create_piece("wQ", 7, 4), # Qh5
            board.create_piece("wR", 5, 0), # Rf1
            board.create_piece("wK", 6, 0), # Kg1
            # Black kingside defense
            board.create_piece("bK", 6, 7), # Kg8
            board.create_piece("bP", 6, 6), # g7
            board.create_piece("bP", 7, 6), # h7
            # Black stranded queenside pieces
            board.create_piece("bR", 0, 7), # Ra8 (trapped!)
            board.create_piece("bN", 1, 7), # Nb8
            board.create_piece("bP", 0, 6), # a7
            board.create_piece("bP", 1, 6), # b7
        )

        self.play(
            FadeIn(board, UP * 0.2),
            FadeIn(pieces, scale=0.9),
            run_time=1.0
        )
        self.wait(0.6)

        # Apply Fiedler Cluster Overlays & Fault Line
        clusters = board.create_cluster_overlays(neg_cols=(0, 1, 2), pos_cols=(4, 5, 6, 7))
        fault_line = board.create_fault_line(split_col=3.5)

        # Tactical attack arrow from Qh5 to h7 and Nf5 to g7
        arr_qh5 = board.create_tactical_arrow(
            board.get_square_pos(7, 4), board.get_square_pos(7, 6),
            color=TACTICAL_HOSTILE, buff=0.22
        )
        arr_nf5 = board.create_tactical_arrow(
            board.get_square_pos(5, 4), board.get_square_pos(6, 6),
            color=TACTICAL_HOSTILE, buff=0.22
        )
        tactical_arrows = VGroup(arr_qh5, arr_nf5)

        # Flank Legend Pills Below the Board
        pill_w, pill_h = 2.4, 0.52
        tag_neg_bg = RoundedRectangle(width=pill_w, height=pill_h, corner_radius=0.10, fill_color="#2b1806", fill_opacity=0.95, stroke_color=ACCENT_GOLD, stroke_width=1.2)
        tag_neg_txt = Text("QUEENSIDE: -0.82", font="Consolas", color=ACCENT_GOLD).scale(0.21)
        tag_neg_sub = Text("Stranded Defender Flank", font="Segoe UI", color="#d97706").scale(0.18)
        tag_neg_content = VGroup(tag_neg_txt, tag_neg_sub).arrange(DOWN, buff=0.04).move_to(tag_neg_bg.get_center())
        tag_neg = VGroup(tag_neg_bg, tag_neg_content)
        tag_neg.move_to(board.get_center() + LEFT * 1.30 + DOWN * (4 * 0.54 + 0.65))

        tag_pos_bg = RoundedRectangle(width=pill_w, height=pill_h, corner_radius=0.10, fill_color="#0b2440", fill_opacity=0.95, stroke_color=ACCENT_CYAN, stroke_width=1.2)
        tag_pos_txt = Text("KINGSIDE: +0.74", font="Consolas", color=ACCENT_CYAN).scale(0.21)
        tag_pos_sub = Text("Active Attacking Battery", font="Segoe UI", color="#38bdf8").scale(0.18)
        tag_pos_content = VGroup(tag_pos_txt, tag_pos_sub).arrange(DOWN, buff=0.04).move_to(tag_pos_bg.get_center())
        tag_pos = VGroup(tag_pos_bg, tag_pos_content)
        tag_pos.move_to(board.get_center() + RIGHT * 1.30 + DOWN * (4 * 0.54 + 0.65))

        flank_tags = VGroup(tag_neg, tag_pos)

        # Telemetry Card on the Right (Pillars 1, 2, 3)
        hud_bg = RoundedRectangle(
            width=6.5, height=5.3, corner_radius=0.16,
            fill_color="#0a101d", fill_opacity=0.95,
            stroke_color=BORDER_COL, stroke_width=1.5
        )
        hud_bg.move_to(RIGHT * 3.55 + DOWN * 0.30)

        hud_title = Text("THE THREE STRUCTURAL PILLARS", font="Consolas", color=ACCENT_GOLD).scale(0.28)
        hud_title.move_to(hud_bg.get_top() + DOWN * 0.40)

        # Helper to construct well-spaced pillar cards with generous typography
        def make_pillar_card(title_txt, sub_txt, body_txt, title_color, sub_color):
            p_box = RoundedRectangle(width=6.0, height=1.26, corner_radius=0.10, fill_color=SURFACE_BG, fill_opacity=0.92, stroke_color="#1e293b", stroke_width=1.2)
            t = Text(title_txt, font="Consolas", color=title_color).scale(0.26)
            s = Text(sub_txt, font="Consolas", color=sub_color).scale(0.22)
            b = Text(body_txt, font="Segoe UI", color=TEXT_BRIGHT).scale(0.24)
            content = VGroup(t, s, b).arrange(DOWN, aligned_edge=LEFT, buff=0.07)
            content.move_to(p_box.get_center() + LEFT * 0.10)
            return VGroup(p_box, content)

        p1_group = make_pillar_card(
            "1. ZERO TRAINING WEIGHTS (FIRST PRINCIPLES)",
            "• 0 Weights | 0 Training Games | 0 GPU Hours",
            "Pure discrete physics derived dynamically from the graph.",
            ACCENT_CYAN, TEXT_MUTED
        )
        p2_group = make_pillar_card(
            "2. THE \"WHITE BOX\" (TOTAL INTERPRETABILITY)",
            "• Kingside (+0.74) vs. Queenside (-0.82)",
            "Linear algebra proves the board is severed into two games.",
            ACCENT_BLUE, ACCENT_CYAN
        )
        p3_group = make_pillar_card(
            "3. DEPTH 0 TOPOLOGICAL VISION",
            "• Standard Search: 30-ply search (40M nodes to notice)",
            "Graph Laplacian diagnoses structural severance in 1.2ms.",
            ACCENT_GREEN, TEXT_DIM
        )

        pillars_group = VGroup(p1_group, p2_group, p3_group).arrange(DOWN, buff=0.13)
        pillars_group.next_to(hud_title, DOWN, buff=0.20)
        pillars_group.move_to(np.array([hud_bg.get_center()[0], pillars_group.get_center()[1], 0]))

        hud_total = VGroup(hud_bg, hud_title, pillars_group)

        # Animate Board Transformations, Arrows & HUD
        self.play(
            FadeIn(clusters, run_time=1.0),
            FadeIn(fault_line, run_time=1.0),
            FadeIn(flank_tags, UP * 0.15),
            FadeIn(tactical_arrows, run_time=0.8),
            FadeIn(hud_total, RIGHT * 0.3),
            run_time=1.4
        )
        self.wait(3.0)

        # -------------------------------------------------------------
        # 5. Finale: Transform HUD into the Master Takeaway Card
        # -------------------------------------------------------------
        verdict_card_bg = RoundedRectangle(
            width=6.5, height=5.3, corner_radius=0.16,
            fill_color="#070c17", fill_opacity=0.98,
            stroke_color=ACCENT_CYAN, stroke_width=1.8
        )
        verdict_card_bg.move_to(hud_bg.get_center())

        quote_sym = Text("“", font="Georgia", color=ACCENT_CYAN).scale(2.2)

        verdict_l1 = Text("Instead of trying to approximate", font="Segoe UI", color=TEXT_MUTED).scale(0.35)
        verdict_l2 = Text("human chess intuition through", font="Segoe UI", color=TEXT_MUTED).scale(0.35)
        verdict_l3 = Text("brute statistical training...", font="Segoe UI", color=TEXT_MUTED).scale(0.35)

        verdict_l4 = Text("...we let the geometry", font="Segoe UI", color=TEXT_BRIGHT).scale(0.44)
        verdict_l5 = Text("of the network speak for itself.", font="Segoe UI", color=ACCENT_CYAN).scale(0.44)

        verdict_lines = VGroup(verdict_l1, verdict_l2, verdict_l3, verdict_l4, verdict_l5).arrange(DOWN, buff=0.16)
        
        quote_sym.next_to(verdict_lines, UP, buff=0.20)
        quote_block = VGroup(quote_sym, verdict_lines)
        quote_block.move_to(verdict_card_bg.get_center() + UP * 0.15)

        div_line = Line(LEFT * 2.2, RIGHT * 2.2, color="#1e293b", stroke_width=1.2)
        div_line.next_to(quote_block, DOWN, buff=0.25)

        auth_tag = Text("HEAVEN'S GATE • SPECTRAL ARCHITECTURE", font="Consolas", color=ACCENT_GOLD).scale(0.22)
        auth_tag.next_to(div_line, DOWN, buff=0.14)

        verdict_elements = VGroup(quote_sym, verdict_lines, div_line, auth_tag)

        self.play(
            FadeOut(pillars_group, scale=0.95),
            FadeOut(hud_title, UP * 0.1),
            Transform(hud_bg, verdict_card_bg),
            FadeIn(verdict_elements, UP * 0.15),
            run_time=1.4
        )
        self.wait(3.5)
