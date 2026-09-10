"""
Heaven's Gate Documentary — Scene 03: The 1973 Fiedler Breakthrough
Runtime: ~85 seconds | 1920×1080 | 60 fps

Flow:
  0-20s   Ascending eigenvalue spectrum rail with pulsing Fiedler dot
 20-48s   Tug-of-war: number line with spring-connected piece clusters
          sliding into +/- territory under ∑v[i]=0 constraint
 48-85s   Real locked chess position with Fiedler charge overlays
          and animated zero-crossing fault line
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import (
    BroadcastChessBoard,
    FIEDLER_POS_COLOR, FIEDLER_NEG_COLOR, FIEDLER_CUT_COLOR
)

# ── palette ──────────────────────────────────────────────────
BG       = "#070b12"
GRID_C   = "#1e293b"
T_BRI    = "#f8fafc"
T_MUT    = "#94a3b8"
T_DIM    = "#475569"

CYAN     = "#38bdf8"
GOLD     = "#fbbf24"
RED      = "#f43f5e"
GREEN    = "#34d399"
VIOLET   = "#a78bfa"
ORANGE   = "#fb923c"
AMBER    = "#d97706"


class Scene03TheFiedlerBreakthrough(Scene):
    def construct(self):
        # ── background ───────────────────────────────────────────
        bg = FullScreenRectangle(fill_color=BG, fill_opacity=1).set_stroke(width=0)
        self.add(bg)
        grid = NumberPlane(
            x_range=[-14, 14, 1], y_range=[-9, 9, 1],
            width=28, height=18,
            axis_config={"stroke_color": GRID_C, "stroke_width": 0.7, "stroke_opacity": 0.3},
            background_line_style={"stroke_color": GRID_C, "stroke_width": 0.5, "stroke_opacity": 0.2},
            faded_line_style={"stroke_color": "#121b2a", "stroke_width": 0.3, "stroke_opacity": 0.12},
        )
        self.add(grid)

        # ============================================================
        # BEAT 1 — ASCENDING EIGENVALUE SPECTRUM  (0 – 20 s)
        # ============================================================
        # header
        h_text = Text("The 1973 Fiedler Breakthrough", font="Bahnschrift", color=T_BRI).scale(0.48)
        h_sub = Text("Miroslav Fiedler  ·  Algebraic Connectivity of Graphs",
                      font="Bahnschrift", color=GOLD).scale(0.24)
        h_grp = VGroup(h_text, h_sub).arrange(DOWN, buff=0.10).move_to(UP * 3.0)

        self.play(FadeIn(h_grp, scale=0.92), run_time=1.5)
        self.wait(1.0)

        # spectrum rail
        rail = Line(LEFT * 5.0, RIGHT * 5.0, color=T_DIM, stroke_width=2.5).move_to(DOWN * 0.3)

        # eigenvalue dots  (positions along rail)
        spec_data = [
            (-4.2, "0 = λ₁",   RED,    0.13, "Trivial\nBaseline"),
            (-1.5, "λ₂",       GOLD,   0.18, "ALGEBRAIC\nCONNECTIVITY"),
            ( 1.2, "λ₃",       CYAN,   0.12, "Sub-flank"),
            ( 4.0, "λ_N",      VIOLET, 0.12, "Micro-\ntactical"),
        ]

        dots = VGroup()
        top_lbls = VGroup()
        bot_lbls = VGroup()
        for x, sym, col, r, desc in spec_data:
            d = Dot(np.array([x, -0.3, 0]), radius=r, color=col)
            tl = Text(sym, font="Consolas", color=col).scale(0.34 if sym == "λ₂" else 0.28)
            tl.next_to(d, UP, buff=0.18)
            bl = Text(desc, font="Bahnschrift", color=T_DIM if sym != "λ₂" else GOLD).scale(0.18)
            bl.next_to(d, DOWN, buff=0.18)
            dots.add(d)
            top_lbls.add(tl)
            bot_lbls.add(bl)

        # ≤ arrows between dots
        leq_arrows = VGroup()
        for i in range(len(spec_data) - 1):
            x1 = spec_data[i][0] + 0.4
            x2 = spec_data[i+1][0] - 0.4
            arr = Arrow(
                np.array([x1, -0.3, 0]), np.array([x2, -0.3, 0]),
                buff=0, color=T_DIM, stroke_width=1.5
            )
            leq = Text("≤", font="Consolas", color=T_DIM).scale(0.22)
            leq.move_to(np.array([(x1+x2)/2, -0.08, 0]))
            leq_arrows.add(VGroup(arr, leq))

        self.play(
            ShowCreation(rail),
            LaggedStart(*[GrowFromCenter(d) for d in dots], lag_ratio=0.15),
            LaggedStart(*[FadeIn(t, UP*0.08) for t in top_lbls], lag_ratio=0.15),
            LaggedStart(*[FadeIn(b, DOWN*0.05) for b in bot_lbls], lag_ratio=0.15),
            LaggedStart(*[FadeIn(a) for a in leq_arrows], lag_ratio=0.15),
            run_time=2.8
        )
        self.wait(1.0)

        # pulse Fiedler dot λ₂
        fiedler_dot = dots[1]
        pulse1 = Circle(radius=0.25, color=GOLD, stroke_width=3.0).move_to(fiedler_dot.get_center())
        pulse2 = Circle(radius=0.35, color=GOLD, stroke_width=2.0).move_to(fiedler_dot.get_center())

        self.play(
            fiedler_dot.animate.scale(1.4),
            pulse1.animate.scale(3.5).set_opacity(0),
            pulse2.animate.scale(3.0).set_opacity(0),
            run_time=1.5
        )
        self.play(fiedler_dot.animate.scale(1/1.4), run_time=0.5)
        self.wait(2.0)

        # ============================================================
        # BEAT 2 — THE MATHEMATICAL TUG-OF-WAR  (20 – 48 s)
        # ============================================================
        spectrum_stuff = VGroup(h_grp, rail, dots, top_lbls, bot_lbls, leq_arrows, pulse1, pulse2)
        self.play(FadeOut(spectrum_stuff), run_time=1.0)

        # equation at top
        eq_card = RoundedRectangle(
            width=10.2, height=0.8, corner_radius=0.08,
            fill_color="#0c1222", fill_opacity=0.94,
            stroke_color=GOLD, stroke_width=1.2
        ).move_to(UP * 3.0)
        eq_txt = Text("min  ∑ A[i,j] · (v[i] − v[j])²     subject to    ∑ v[i] = 0",
                       font="Consolas", color=GOLD).scale(0.30)
        eq_txt.move_to(eq_card.get_center())

        self.play(FadeIn(eq_card, UP*0.1), FadeIn(eq_txt, UP*0.1), run_time=1.2)

        # number line from -1.0 to +1.0
        nline = Line(LEFT * 5.0 + DOWN * 0.4, RIGHT * 5.0 + DOWN * 0.4,
                      color="#4a5568", stroke_width=2.0)

        # tick at zero
        zero_tick = Line(DOWN * 0.15 + DOWN * 0.4, DOWN * 0.65 + DOWN * 0.4,
                          color=GOLD, stroke_width=2.5)
        zero_lbl = Text("0.0", font="Consolas", color=GOLD).scale(0.24)
        zero_lbl.next_to(zero_tick, DOWN, buff=0.08)

        # endpoint labels
        neg_lbl = Text("-1.0", font="Consolas", color=ORANGE).scale(0.22).move_to(LEFT * 5.0 + DOWN * 1.0)
        pos_lbl = Text("+1.0", font="Consolas", color=CYAN).scale(0.22).move_to(RIGHT * 5.0 + DOWN * 1.0)

        self.play(
            ShowCreation(nline), ShowCreation(zero_tick),
            FadeIn(zero_lbl), FadeIn(neg_lbl), FadeIn(pos_lbl),
            run_time=1.2
        )

        # piece dots starting at center, then pulled into clusters
        # Positive cluster (White Kingside Attack): Queen, Knight, Bishop
        pos_pieces = VGroup()
        pos_labels_text = ["Q", "N", "B"]
        for i, lbl in enumerate(pos_labels_text):
            d = Dot(np.array([0.1 * (i - 1), -0.4, 0]), radius=0.14, color=CYAN)
            t = Text(lbl, font="Consolas", color=T_BRI).scale(0.18).move_to(d.get_center())
            pos_pieces.add(VGroup(d, t))

        # Negative cluster (Black stranded): Rook, Pawn
        neg_pieces = VGroup()
        neg_labels_text = ["R", "P"]
        for i, lbl in enumerate(neg_labels_text):
            d = Dot(np.array([-0.1 * (i + 1), -0.4, 0]), radius=0.14, color=ORANGE)
            t = Text(lbl, font="Consolas", color=T_BRI).scale(0.18).move_to(d.get_center())
            neg_pieces.add(VGroup(d, t))

        self.play(
            LaggedStart(*[FadeIn(p, scale=0.8) for p in pos_pieces], lag_ratio=0.1),
            LaggedStart(*[FadeIn(p, scale=0.8) for p in neg_pieces], lag_ratio=0.1),
            run_time=1.0
        )
        self.wait(0.5)

        # spring connections within clusters (to show they "stick together")
        spring_pos = Line(
            pos_pieces[0][0].get_center(), pos_pieces[2][0].get_center(),
            color=CYAN, stroke_width=3.0, stroke_opacity=0.5
        )
        spring_neg = Line(
            neg_pieces[0][0].get_center(), neg_pieces[1][0].get_center(),
            color=ORANGE, stroke_width=3.0, stroke_opacity=0.5
        )
        self.play(ShowCreation(spring_pos), ShowCreation(spring_neg), run_time=0.6)

        # animate clusters pulling apart — coordinated pieces refuse to split
        pos_targets = [RIGHT * 3.0, RIGHT * 3.5, RIGHT * 4.0]
        neg_targets = [LEFT * 3.5, LEFT * 4.0]

        pos_anims = []
        for piece, tgt in zip(pos_pieces, pos_targets):
            pos_anims.append(piece.animate.move_to(np.array([tgt[0], -0.4, 0])))
        neg_anims = []
        for piece, tgt in zip(neg_pieces, neg_targets):
            neg_anims.append(piece.animate.move_to(np.array([tgt[0], -0.4, 0])))

        # update springs
        spring_pos_new = Line(
            np.array([3.0, -0.4, 0]), np.array([4.0, -0.4, 0]),
            color=CYAN, stroke_width=3.0, stroke_opacity=0.5
        )
        spring_neg_new = Line(
            np.array([-3.5, -0.4, 0]), np.array([-4.0, -0.4, 0]),
            color=ORANGE, stroke_width=3.0, stroke_opacity=0.5
        )

        self.play(
            *pos_anims, *neg_anims,
            Transform(spring_pos, spring_pos_new),
            Transform(spring_neg, spring_neg_new),
            run_time=2.5, rate_func=smooth
        )

        # cluster labels
        pos_clabel = Text("Coordinated Attack\n(+0.6 to +0.8)", font="Bahnschrift", color=CYAN).scale(0.22)
        pos_clabel.next_to(pos_pieces, UP, buff=0.25)
        neg_clabel = Text("Isolated Queenside\n(-0.6 to -0.8)", font="Bahnschrift", color=ORANGE).scale(0.22)
        neg_clabel.next_to(neg_pieces, UP, buff=0.25)

        self.play(FadeIn(pos_clabel), FadeIn(neg_clabel), run_time=0.8)

        # explanation
        exp1 = Text("(v[i] − v[j])²  penalizes separating defended pieces",
                     font="Bahnschrift", color=T_BRI).scale(0.22)
        exp2 = Text("∑ v[i] = 0  forces the board to split into +/− flanks",
                     font="Bahnschrift", color=GOLD).scale(0.22)
        exp_grp = VGroup(exp1, exp2).arrange(DOWN, buff=0.10).move_to(DOWN * 2.2)
        self.play(FadeIn(exp_grp, UP*0.08), run_time=0.8)
        self.wait(3.0)

        # ============================================================
        # BEAT 3 — REAL CHESS POSITION + FIEDLER OVERLAYS  (48 – 85 s)
        # ============================================================
        tow_stuff = VGroup(
            eq_card, eq_txt, nline, zero_tick, zero_lbl, neg_lbl, pos_lbl,
            pos_pieces, neg_pieces, spring_pos, spring_neg,
            pos_clabel, neg_clabel, exp_grp
        )
        self.play(FadeOut(tow_stuff), run_time=1.0)

        # chessboard on left
        board = BroadcastChessBoard(
            center=LEFT * 3.4 + DOWN * 0.15,
            sq_size=0.56, show_coords=True
        )

        # locked center position
        piece_data = [
            ("wP", 3, 3),  # d4
            ("wP", 4, 4),  # e5
            ("bP", 3, 4),  # d5
            ("bP", 4, 5),  # e6
            ("wQ", 7, 4),  # h5 — kingside attack
            ("wN", 5, 2),  # f3
            ("bR", 0, 7),  # a8 — stranded
            ("bK", 6, 7),  # g8
        ]
        pieces = VGroup()
        for name, c, r in piece_data:
            pieces.add(board.create_piece(name, c, r))

        # animate board and pieces in
        board.save_state()
        board.scale(0.88).set_opacity(0)
        self.play(board.animate.restore(), run_time=1.5)
        self.play(
            LaggedStart(*[FadeIn(p, scale=0.85) for p in pieces], lag_ratio=0.10),
            run_time=1.5
        )
        self.wait(0.5)

        # telemetry HUD on right
        hud = RoundedRectangle(
            width=5.8, height=5.4, corner_radius=0.10,
            fill_color="#0c1222", fill_opacity=0.94,
            stroke_color="#1e293b", stroke_width=1.2
        ).move_to(RIGHT * 3.6 + DOWN * 0.15)

        hud_title = Text("DEPTH 0 TOPOLOGICAL MAP", font="Bahnschrift", color=CYAN).scale(0.26)
        hud_title.move_to(hud.get_top() + DOWN * 0.42)

        entries = [
            ("Kingside Attack:  +0.74", CYAN),
            ("Black King Wing:  +0.62", GOLD),
            ("Stranded a8 Rook: -0.82", RED),
            ("Zero-cut:  v₂ = 0.00",    GREEN),
        ]
        hud_items = VGroup()
        for txt, col in entries:
            t = Text(txt, font="Consolas", color=col).scale(0.22)
            hud_items.add(t)
        hud_items.arrange(DOWN, aligned_edge=LEFT, buff=0.18)
        hud_items.move_to(hud.get_center() + UP * 0.2)

        concl = Text("Analytical math diagnoses the board\ninstantly — no search required.",
                      font="Bahnschrift", color=T_MUT).scale(0.20)
        concl.move_to(hud.get_center() + DOWN * 1.2)

        self.play(
            FadeIn(hud, RIGHT*0.12),
            FadeIn(hud_title, UP*0.06),
            LaggedStart(*[FadeIn(h, LEFT*0.08) for h in hud_items], lag_ratio=0.15),
            FadeIn(concl, UP*0.06),
            run_time=2.0
        )
        self.wait(1.0)

        # Fiedler cluster overlays
        clusters = board.create_cluster_overlays(neg_cols=(0, 1, 2), pos_cols=(4, 5, 6, 7))
        fault = board.create_fault_line(split_col=3.5)

        self.play(
            FadeIn(clusters, lag_ratio=0.02),
            run_time=1.5
        )
        self.play(
            ShowCreation(fault[0]),  # glow
            ShowCreation(fault[1]),  # dash
            FadeIn(fault[2], UP*0.08),  # badge
            run_time=1.8
        )

        # pulse the fault line
        self.play(
            fault[0].animate.set_stroke(width=10, opacity=0.6),
            run_time=0.6
        )
        self.play(
            fault[0].animate.set_stroke(width=6, opacity=0.35),
            run_time=0.6
        )
        self.wait(3.5)

        # ── outro ─────────────────────────────────────────────
        self.play(
            FadeOut(board), FadeOut(pieces), FadeOut(clusters), FadeOut(fault),
            FadeOut(hud), FadeOut(hud_title), FadeOut(hud_items), FadeOut(concl),
            run_time=1.5
        )
        self.wait(0.5)
