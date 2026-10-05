"""
Heaven's Gate Documentary - Scene 11: The Stockfish Gauntlet
Broadcast-Grade Craft Standard (3Blue1Brown / vcubingx quality)
Runtime: ~36 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Obsidian Canvas (#000000) + Luminous Jewel Palette

Frame Continuity:
- Frame 0 inherits the exact terminal frame of Scene 10: 35M NPS tachometer surge, gauge, digital readout, and quote.
- Beat 1: The Matchup & Elo Ascent (3400 Stockfish baseline vs 3520 Heaven's Gate).
- Beat 2: The 10-0 Tournament Ledger Matrix & Final Score Banner.
- Beat 3: The Sicilian Dragon 11-Move Mating Miniature (Qa1#) & Match Analytics.
- Terminal frame held cleanly for Scene 12 handoff.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from theme import *
from cm_math import CMTex
from chessboard_widget import BroadcastChessBoard


class Scene11TheStockfishGauntlet(Scene):
    def construct(self):
        # Canvas: Pure Obsidian Black
        canvas_bg = Rectangle(width=16, height=10, fill_color="#000000", fill_opacity=1.0, stroke_width=0)
        self.add(canvas_bg)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% INHERITANCE FROM SCENE 10 TERMINAL
        # =============================================================
        title_gauge_prev = CMTex(
            r"\text{Throughput Benchmark: The 35 Million NPS Peak}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GREEN
        ).move_to(UP * 2.50)
        sub_gauge_prev = CMTex(
            r"\text{Real-time node counter accelerating from cold start to peak production speed}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_gauge_prev, DOWN, buff=0.12)
        gauge_header_prev = VGroup(title_gauge_prev, sub_gauge_prev)

        g_cen = DOWN * 0.40
        r_gauge = 1.95

        arc_back_prev = Arc(radius=r_gauge, start_angle=210 * DEGREES, angle=-240 * DEGREES, color="#1e293b", stroke_width=4.0, arc_center=g_cen)
        arc_norm_prev = Arc(radius=r_gauge, start_angle=210 * DEGREES, angle=-160 * DEGREES, color=JEWEL_CYAN, stroke_width=4.0, arc_center=g_cen)
        arc_peak_prev = Arc(radius=r_gauge, start_angle=50 * DEGREES, angle=-80 * DEGREES, color=JEWEL_GREEN, stroke_width=6.0, arc_center=g_cen)

        lbl_0_prev = CMTex(r"0", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([-2.25, -0.9, 0]))
        lbl_10m_prev = CMTex(r"10\mathrm{M}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([-1.75, 1.65, 0]))
        lbl_20m_prev = CMTex(r"20\mathrm{M}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([0.0, 2.18, 0]))
        lbl_35m_prev = CMTex(r"35\mathrm{M}+", fontsize=16, height=0.22, color=JEWEL_GREEN).move_to(g_cen + np.array([2.40, -0.7, 0]))
        scale_lbls_prev = VGroup(lbl_0_prev, lbl_10m_prev, lbl_20m_prev, lbl_35m_prev)

        hub_pt_prev = Circle(radius=0.16, fill_color=JEWEL_GOLD, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.2).move_to(g_cen)

        rad = -25 * DEGREES
        tip = g_cen + np.array([np.cos(rad) * (r_gauge - 0.1), np.sin(rad) * (r_gauge - 0.1), 0])
        n_perp = np.array([-np.sin(rad) * 0.08, np.cos(rad) * 0.08, 0])
        needle_prev = Polygon(
            g_cen - n_perp * 1.2, tip, g_cen + n_perp * 1.2,
            fill_color=JEWEL_GREEN, fill_opacity=1.0,
            stroke_color="#ffffff", stroke_width=0.8
        )

        c_readout = DOWN * 2.35
        nps_digits_prev = CMTex(r"\text{35,420,119 NODES / SEC}", fontsize=24, height=0.34, color=JEWEL_GREEN).move_to(c_readout + UP * 0.15)
        depth_sub_prev = CMTex(
            r"\text{Depth: 14 reached in 0.82s  } \bullet \text{  TT Hit Rate: 74.8\%  } \bullet \text{  First Move Cutoff: 87.2\%}",
            fontsize=13, height=0.17, color=TEXT_MUTED
        ).next_to(nps_digits_prev, DOWN, buff=0.12)
        readout_grp_prev = VGroup(nps_digits_prev, depth_sub_prev)
        gauge_grp_prev = VGroup(arc_back_prev, arc_norm_prev, arc_peak_prev, scale_lbls_prev, hub_pt_prev)

        climax_cs_prev = CMTex(
            r"\text{``Depth beats subtlety. And 35 million positions per second beats everything.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(gauge_header_prev, gauge_grp_prev, needle_prev, readout_grp_prev, climax_cs_prev)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE MATCHUP & ELO ASCENT (0s - 11s)
        # =============================================================
        title_sg = CMTex(
            r"\text{11. The Stockfish Gauntlet: 10 - 0 Clean Sweep}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sub_sg = CMTex(
            r"\text{Official 10-Game Benchmark Match vs Stockfish 16.1 (3400+ Elo Standard)}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).move_to(UP * 2.35)
        sg_header = VGroup(title_sg, sub_sg)

        self.play(
            FadeOut(inherited_state),
            FadeIn(sg_header, UP * 0.1),
            run_time=1.2,
            rate_func=smooth
        )
        self.wait(0.5)

        elo_axes = Axes(
            x_range=[0, 10, 2],
            y_range=[0, 8, 2],
            width=7.6,
            height=3.4,
            axis_config={"stroke_color": "#2a364f", "stroke_width": 1.4}
        ).move_to(DOWN * 0.35)

        # Elo Ticks
        y_labels = VGroup(
            CMTex(r"2800", fontsize=11, height=0.14, color=TEXT_MUTED).next_to(elo_axes.c2p(0, 0), LEFT, buff=0.08),
            CMTex(r"3000", fontsize=11, height=0.14, color=TEXT_MUTED).next_to(elo_axes.c2p(0, 2), LEFT, buff=0.08),
            CMTex(r"3200", fontsize=11, height=0.14, color=TEXT_MUTED).next_to(elo_axes.c2p(0, 4), LEFT, buff=0.08),
            CMTex(r"3400", fontsize=11, height=0.14, color=JEWEL_CORAL).next_to(elo_axes.c2p(0, 6), LEFT, buff=0.08),
            CMTex(r"3600", fontsize=11, height=0.14, color=JEWEL_GREEN).next_to(elo_axes.c2p(0, 8), LEFT, buff=0.08),
        )

        elo_x = CMTex(r"\text{Tournament Game}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(elo_axes.x_axis, DOWN, buff=0.12)
        elo_y = CMTex(r"\text{Performance Rating (Elo)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(y_labels, LEFT, buff=0.25)

        sf_line = elo_axes.get_graph(lambda x: 6.0, x_range=[0, 10], color=JEWEL_CORAL).set_stroke(width=2.2)
        sf_lbl = CMTex(r"\text{Stockfish 16.1 Baseline (3400 Elo)}", fontsize=13, height=0.17, color=JEWEL_CORAL).move_to(elo_axes.c2p(6.8, 5.55))

        hg_curve = elo_axes.get_graph(lambda x: 5.8 + 0.12 * x + 0.15 * np.sin(x), x_range=[0, 10], color=JEWEL_GREEN).set_stroke(width=3.4)
        hg_lbl = CMTex(r"\text{Heaven's Gate Performance: } \sim 3520 \ \text{Elo}", fontsize=14, height=0.18, color=JEWEL_GREEN).move_to(elo_axes.c2p(6.6, 7.55))

        self.play(ShowCreation(elo_axes), FadeIn(elo_x), FadeIn(elo_y), FadeIn(y_labels), run_time=0.9)
        self.play(ShowCreation(sf_line), FadeIn(sf_lbl), run_time=0.8)
        self.play(ShowCreation(hg_curve), FadeIn(hg_lbl), run_time=1.4)
        self.wait(2.8)

        # =============================================================
        # BEAT 2: THE 10-0 TOURNAMENT SWEEP MATRIX (11s - 22s)
        # =============================================================
        self.play(
            FadeOut(sg_header),
            FadeOut(elo_axes),
            FadeOut(elo_x),
            FadeOut(elo_y),
            FadeOut(y_labels),
            FadeOut(sf_line),
            FadeOut(sf_lbl),
            FadeOut(hg_curve),
            FadeOut(hg_lbl),
            run_time=0.8,
            rate_func=smooth
        )

        title_sweep = CMTex(
            r"\text{Official Tournament Ledger: 10 Consecutive Wins}",
            fontsize=22,
            height=0.30,
            color=JEWEL_CYAN
        ).move_to(UP * 2.50)
        sub_sweep = CMTex(
            r"\text{Heaven's Gate (Sovereign) vs Stockfish 16.1  } \bullet \text{  Time Control: 40 moves / 15 mins}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_sweep, DOWN, buff=0.12)
        sweep_header = VGroup(title_sweep, sub_sweep)
        self.play(FadeIn(sweep_header, DOWN * 0.1), run_time=0.6)

        games = [
            ("Game 01", "White", r"\text{Italian Game}", r"24 \ \text{moves (M12)}", r"\mathbf{1 \ - \ 0}"),
            ("Game 02", "Black", r"\text{Sicilian Defense}", r"46 \ \text{moves (M19)}", r"\mathbf{0 \ - \ 1}"),
            ("Game 03", "White", r"\text{Sicilian Classical}", r"12 \ \text{moves (M16)}", r"\mathbf{1 \ - \ 0}"),
            ("Game 04", "Black", r"\text{Sicilian Dragon}", r"16 \ \text{moves (M17)}", r"\mathbf{0 \ - \ 1}"),
            ("Game 05", "White", r"\text{Ruy Lopez Steinitz}", r"27 \ \text{moves (M11)}", r"\mathbf{1 \ - \ 0}"),
            ("Game 06", "Black", r"\text{Ruy Lopez Open}", r"37 \ \text{moves (M38)}", r"\mathbf{0 \ - \ 1}"),
            ("Game 07", "White", r"\text{French Defense}", r"20 \ \text{moves (M7)}", r"\mathbf{1 \ - \ 0}"),
            ("Game 08", "Black", r"\text{Nimzo-French}", r"25 \ \text{moves (M11)}", r"\mathbf{0 \ - \ 1}"),
            ("Game 09", "White", r"\text{Queen's Gambit Decl.}", r"12 \ \text{moves (M5)}", r"\mathbf{1 \ - \ 0}"),
            ("Game 10", "Black", r"\text{Queen's Gambit Acc.}", r"25 \ \text{moves (M9)}", r"\mathbf{0 \ - \ 1}"),
        ]

        c1_group = VGroup()
        for i in range(5):
            g = games[i]
            t_id = CMTex(rf"\mathtt{{{g[0]}}} \ ({g[1]}):", fontsize=13, height=0.16, color=TEXT_WHITE)
            t_sc = CMTex(g[4], fontsize=14, height=0.18, color=JEWEL_GREEN)
            t_op = CMTex(g[2] + rf" \ \bullet \ {g[3]}", fontsize=13, height=0.16, color=TEXT_MUTED)
            row = VGroup(t_id, t_sc, t_op).arrange(RIGHT, buff=0.25)
            c1_group.add(row)

        c1_group.arrange(DOWN, buff=0.20, aligned_edge=LEFT).move_to(LEFT * 3.4 + DOWN * 0.20)

        c2_group = VGroup()
        for i in range(5, 10):
            g = games[i]
            t_id = CMTex(rf"\mathtt{{{g[0]}}} \ ({g[1]}):", fontsize=13, height=0.16, color=TEXT_WHITE)
            t_sc = CMTex(g[4], fontsize=14, height=0.18, color=JEWEL_GREEN)
            t_op = CMTex(g[2] + rf" \ \bullet \ {g[3]}", fontsize=13, height=0.16, color=TEXT_MUTED)
            row = VGroup(t_id, t_sc, t_op).arrange(RIGHT, buff=0.25)
            c2_group.add(row)

        c2_group.arrange(DOWN, buff=0.20, aligned_edge=LEFT).move_to(RIGHT * 3.4 + DOWN * 0.20)

        self.play(FadeIn(c1_group, LEFT * 0.2), FadeIn(c2_group, RIGHT * 0.2), run_time=1.2)

        score_card = RoundedRectangle(
            width=9.8, height=0.55, corner_radius=0.10,
            fill_color="#18140a", fill_opacity=0.95,
            stroke_color=JEWEL_GOLD, stroke_width=1.5
        ).move_to(DOWN * 2.25)
        score_tally = CMTex(
            r"\mathbf{FINAL \ SCORE: \ 10.0 \ / \ 10.0 \quad \bullet \quad 10 \ WINS \quad \bullet \quad 0 \ LOSSES \quad \bullet \quad 0 \ DRAWS}",
            fontsize=15, height=0.20, color=JEWEL_GOLD
        ).move_to(score_card.get_center())
        score_grp = VGroup(score_card, score_tally)

        self.play(FadeIn(score_grp, UP * 0.08), run_time=0.9)
        self.wait(2.2)

        # =============================================================
        # BEAT 3: THE DRAGON MINIATURE & ANALYTICS (22s - 36s)
        # =============================================================
        self.play(
            FadeOut(sweep_header),
            FadeOut(c1_group),
            FadeOut(c2_group),
            FadeOut(score_grp),
            run_time=0.8,
            rate_func=smooth
        )

        title_mini = CMTex(
            r"\text{Game 04: The 16-Move Sicilian Dragon Refutation}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GOLD
        ).move_to(UP * 2.50)
        sub_mini = CMTex(
            r"\text{Heaven's Gate (Black) vs Stockfish 16.1  } \bullet \text{  Forced Mate in 17 } (M17) \text{ sequence}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_mini, DOWN, buff=0.12)
        mini_header = VGroup(title_mini, sub_mini)
        self.play(FadeIn(mini_header, DOWN * 0.1), run_time=0.6)

        cb_mini = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.18, sq_size=0.40, show_coords=True)
        # Authentic Sicilian Dragon position after 10. Kh1 Rxf7 11. h4:
        # White: King h1, Rooks a1, f1, Pawns a2, b2, c3, g2, h4
        # Black: King g8, Rooks a8, f7, Queen d8, Bishops c8, f2, Pawns a7, b7, c4, e7, g6, h7
        wk  = cb_mini.create_piece("wK", 7, 0)
        wra = cb_mini.create_piece("wR", 0, 0)
        wrf = cb_mini.create_piece("wR", 5, 0)
        wpa = cb_mini.create_piece("wP", 0, 1)
        wpb = cb_mini.create_piece("wP", 1, 1)
        wpc = cb_mini.create_piece("wP", 2, 2)
        wpg = cb_mini.create_piece("wP", 6, 1)
        wph = cb_mini.create_piece("wP", 7, 3)

        bk  = cb_mini.create_piece("bK", 6, 7)
        bra = cb_mini.create_piece("bR", 0, 7)
        brf = cb_mini.create_piece("bR", 5, 6)
        bq  = cb_mini.create_piece("bQ", 3, 7)
        bbc = cb_mini.create_piece("bB", 2, 7)
        bbf = cb_mini.create_piece("bB", 5, 1)
        bpa = cb_mini.create_piece("bP", 0, 6)
        bpb = cb_mini.create_piece("bP", 1, 6)
        bpc = cb_mini.create_piece("bP", 2, 3)
        bpe = cb_mini.create_piece("bP", 4, 6)
        bpg = cb_mini.create_piece("bP", 6, 5)
        bph = cb_mini.create_piece("bP", 7, 6)

        pieces_mini = VGroup(
            wk, wra, wrf, wpa, wpb, wpc, wpg, wph,
            bk, bra, brf, bq, bbc, bbf, bpa, bpb, bpc, bpe, bpg, bph
        )

        self.play(FadeIn(cb_mini), FadeIn(pieces_mini), run_time=0.9)

        r_met = RIGHT * 3.0 + DOWN * 0.18
        card_met = RoundedRectangle(
            width=5.8, height=3.8, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_met)

        met_title = CMTex(r"\text{MATCH PERFORMANCE METRICS}", fontsize=16, height=0.22, color=JEWEL_CYAN).move_to(r_met + UP * 1.45)
        m1 = CMTex(r"\bullet \text{ Average Centipawn Loss: } 13.5 \ \mathrm{ACPL}", fontsize=13, height=0.16, color=JEWEL_GREEN).next_to(met_title, DOWN, buff=0.18, aligned_edge=LEFT)
        m2 = CMTex(r"\bullet \text{ Terminal Advantage: } +249.83 \ \mathrm{cp} \ (M17)", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(m1, DOWN, buff=0.14, aligned_edge=LEFT)
        m3 = CMTex(r"\bullet \text{ Blunder Count: Exactly } 0 \text{ across 475 plies}", fontsize=13, height=0.16, color=JEWEL_GREEN).next_to(m2, DOWN, buff=0.14, aligned_edge=LEFT)
        m4 = CMTex(r"\bullet \text{ Tournament Sweep: } 10 \text{ Wins / } 0 \text{ Losses (10.0 / 10)}", fontsize=13, height=0.16, color=JEWEL_GOLD).next_to(m3, DOWN, buff=0.14, aligned_edge=LEFT)
        m5 = CMTex(r"\bullet \text{ First-Move Cutoff Rate: } 87.2\% \text{ in search}", fontsize=13, height=0.16, color=JEWEL_CYAN).next_to(m4, DOWN, buff=0.14, aligned_edge=LEFT)

        met_grp = VGroup(card_met, met_title, m1, m2, m3, m4, m5)
        self.play(FadeIn(met_grp, RIGHT * 0.2), run_time=1.1)

        pos_e5 = cb_mini.get_square_pos(4, 4)
        mate_badge = RoundedRectangle(width=4.8, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).move_to(LEFT * 3.4 + DOWN * 2.45)
        mate_txt = CMTex(r"\mathbf{16\dots e5! \quad FORCED \ MATE \ IN \ 17} \quad (\text{0-1 Resignation})", fontsize=12, height=0.16, color=JEWEL_CORAL).move_to(mate_badge.get_center())
        mate_grp = VGroup(mate_badge, mate_txt)

        self.play(
            bpe.animate.move_to(pos_e5),
            FadeIn(mate_grp, UP * 0.08),
            run_time=1.4
        )

        climax_quote = CMTex(
            r"\text{``Stockfish didn't make a mistake. It was outcalculated from move one.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)
        self.play(FadeIn(climax_quote, UP * 0.08), run_time=1.2)
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 12 Handoff)
        # =============================================================
        self.wait(2.5)
