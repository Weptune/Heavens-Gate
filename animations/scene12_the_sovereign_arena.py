"""
Heaven's Gate Documentary - Scene 12: The Sovereign Arena & Master Outro
Broadcast-Grade Craft Standard (3Blue1Brown / vcubingx quality)
Runtime: ~38 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Obsidian Canvas (#000000) + Luminous Jewel Palette

Frame Continuity:
- Frame 0 inherits the exact terminal frame of Scene 11: Sicilian Dragon checkmated board, metrics card, and quote.
- Beat 1: The Sovereign Arena with live evaluation bar and Oracle predictions.
- Beat 2: Post-Match Topological Autopsy & Flank Severance Laser.
- Beat 3: Cosmic Pull-Back & Luminous Spectral Tactical Network.
- Beat 4: Master Open-Source Outro ("HEAVEN'S GATE: THE SOVEREIGN CLASSICAL CHESS ENGINE").
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from theme import *
from cm_math import CMTex
from chessboard_widget import BroadcastChessBoard


class Scene12TheSovereignArena(Scene):
    def construct(self):
        # Canvas: Pure Obsidian Black
        canvas_bg = Rectangle(width=16, height=10, fill_color="#000000", fill_opacity=1.0, stroke_width=0)
        self.add(canvas_bg)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% INHERITANCE FROM SCENE 11 TERMINAL
        # =============================================================
        title_mini_prev = CMTex(
            r"\text{Game 04: The 16-Move Sicilian Dragon Refutation}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GOLD
        ).move_to(UP * 2.50)
        sub_mini_prev = CMTex(
            r"\text{Heaven's Gate (Black) vs Stockfish 16.1  } \bullet \text{  Forced Mate in 17 } (M17) \text{ sequence}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_mini_prev, DOWN, buff=0.12)
        mini_header_prev = VGroup(title_mini_prev, sub_mini_prev)

        cb_mini_prev = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.18, sq_size=0.40, show_coords=True)
        wk_p  = cb_mini_prev.create_piece("wK", 7, 0)
        wra_p = cb_mini_prev.create_piece("wR", 0, 0)
        wrf_p = cb_mini_prev.create_piece("wR", 5, 0)
        wpa_p = cb_mini_prev.create_piece("wP", 0, 1)
        wpb_p = cb_mini_prev.create_piece("wP", 1, 1)
        wpc_p = cb_mini_prev.create_piece("wP", 2, 2)
        wpg_p = cb_mini_prev.create_piece("wP", 6, 1)
        wph_p = cb_mini_prev.create_piece("wP", 7, 3)

        bk_p  = cb_mini_prev.create_piece("bK", 6, 7)
        bra_p = cb_mini_prev.create_piece("bR", 0, 7)
        brf_p = cb_mini_prev.create_piece("bR", 5, 6)
        bq_p  = cb_mini_prev.create_piece("bQ", 3, 7)
        bbc_p = cb_mini_prev.create_piece("bB", 2, 7)
        bbf_p = cb_mini_prev.create_piece("bB", 5, 1)
        bpa_p = cb_mini_prev.create_piece("bP", 0, 6)
        bpb_p = cb_mini_prev.create_piece("bP", 1, 6)
        bpc_p = cb_mini_prev.create_piece("bP", 2, 3)
        bpe_p = cb_mini_prev.create_piece("bP", 4, 4)  # 16... e5 played!
        bpg_p = cb_mini_prev.create_piece("bP", 6, 5)
        bph_p = cb_mini_prev.create_piece("bP", 7, 6)

        pieces_mini_prev = VGroup(
            wk_p, wra_p, wrf_p, wpa_p, wpb_p, wpc_p, wpg_p, wph_p,
            bk_p, bra_p, brf_p, bq_p, bbc_p, bbf_p, bpa_p, bpb_p, bpc_p, bpe_p, bpg_p, bph_p
        )

        r_met = RIGHT * 3.0 + DOWN * 0.18
        card_met_prev = RoundedRectangle(
            width=5.8, height=3.8, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_met)

        met_title_prev = CMTex(r"\text{MATCH PERFORMANCE METRICS}", fontsize=16, height=0.22, color=JEWEL_CYAN).move_to(r_met + UP * 1.45)
        m1_prev = CMTex(r"\bullet \text{ Average Centipawn Loss: } 13.5 \ \mathrm{ACPL}", fontsize=13, height=0.16, color=JEWEL_GREEN).next_to(met_title_prev, DOWN, buff=0.18, aligned_edge=LEFT)
        m2_prev = CMTex(r"\bullet \text{ Terminal Advantage: } +249.83 \ \mathrm{cp} \ (M17)", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(m1_prev, DOWN, buff=0.14, aligned_edge=LEFT)
        m3_prev = CMTex(r"\bullet \text{ Blunder Count: Exactly } 0 \text{ across 475 plies}", fontsize=13, height=0.16, color=JEWEL_GREEN).next_to(m2_prev, DOWN, buff=0.14, aligned_edge=LEFT)
        m4_prev = CMTex(r"\bullet \text{ Tournament Sweep: } 10 \text{ Wins / } 0 \text{ Losses (10.0 / 10)}", fontsize=13, height=0.16, color=JEWEL_GOLD).next_to(m3_prev, DOWN, buff=0.14, aligned_edge=LEFT)
        m5_prev = CMTex(r"\bullet \text{ First-Move Cutoff Rate: } 87.2\% \text{ in search}", fontsize=13, height=0.16, color=JEWEL_CYAN).next_to(m4_prev, DOWN, buff=0.14, aligned_edge=LEFT)
        met_grp_prev = VGroup(card_met_prev, met_title_prev, m1_prev, m2_prev, m3_prev, m4_prev, m5_prev)

        mate_badge_prev = RoundedRectangle(width=4.8, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).move_to(LEFT * 3.4 + DOWN * 2.45)
        mate_txt_prev = CMTex(r"\mathbf{16\dots e5! \quad FORCED \ MATE \ IN \ 17} \quad (\text{0-1 Resignation})", fontsize=12, height=0.16, color=JEWEL_CORAL).move_to(mate_badge_prev.get_center())
        mate_grp_prev = VGroup(mate_badge_prev, mate_txt_prev)

        climax_quote_prev = CMTex(
            r"\text{``Stockfish didn't make a mistake. It was outcalculated from move one.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(mini_header_prev, cb_mini_prev, pieces_mini_prev, met_grp_prev, mate_grp_prev, climax_quote_prev)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE SOVEREIGN ARENA & EVAL BAR (0s - 12s)
        # =============================================================
        title_sa = CMTex(
            r"\text{12. The Sovereign Arena: Live Interactive Engine}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sub_sa = CMTex(
            r"\text{Real-time browser arena powered by Heaven's Gate C++20 Sovereign Core}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).move_to(UP * 2.35)
        sa_header = VGroup(title_sa, sub_sa)

        cb_cen = LEFT * 3.2 + DOWN * 0.18
        sq_size = 0.40
        cb_arena = BroadcastChessBoard(center=cb_cen, sq_size=sq_size, show_coords=True)

        # Arena position pieces
        wk_a = cb_arena.create_piece("wK", 6, 0)
        wq_a = cb_arena.create_piece("wQ", 3, 2)
        wb_a = cb_arena.create_piece("wB", 4, 3)
        bk_a = cb_arena.create_piece("bK", 6, 7)
        br_a = cb_arena.create_piece("bR", 0, 7)
        bp_a = cb_arena.create_piece("bP", 4, 4)

        bar_x = cb_cen[0] - (4.0 * sq_size + 0.55)
        bar_bg = Rectangle(
            width=0.22, height=8 * sq_size,
            fill_color="#09101f", fill_opacity=1.0,
            stroke_color="#223048", stroke_width=1.4
        ).move_to([bar_x, cb_cen[1], 0])
        bar_fill = Rectangle(
            width=0.20, height=8 * sq_size * 0.68,
            fill_color=TEXT_WHITE, fill_opacity=0.95
        ).set_stroke(width=0).move_to(bar_bg.get_center()).align_to(bar_bg, DOWN)
        eval_tag = CMTex(r"+2.45", fontsize=13, height=0.17, color=JEWEL_GOLD).next_to(bar_bg, UP, buff=0.12)
        eval_bar_grp = VGroup(bar_bg, bar_fill, eval_tag)

        r_ora = RIGHT * 3.0 + DOWN * 0.18
        card_ora = RoundedRectangle(
            width=5.8, height=3.8, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(r_ora)

        ora_title = CMTex(r"\text{THE SOVEREIGN ORACLE}", fontsize=16, height=0.22, color=JEWEL_CYAN).move_to(r_ora + UP * 1.45)
        o1 = CMTex(r"\bullet \text{ Brilliant Move Detected: } 14\dots \mathrm{e5!}", fontsize=13, height=0.16, color=JEWEL_GREEN).next_to(ora_title, DOWN, buff=0.18, aligned_edge=LEFT)
        o2 = CMTex(r"\bullet \text{ Win Probability: } 78.4\% \ (\mathrm{WPL \ Model})", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(o1, DOWN, buff=0.14, aligned_edge=LEFT)
        o3 = CMTex(r"\bullet \text{ Search: Depth 16 } \bullet \text{ 35.2M NPS}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(o2, DOWN, buff=0.14, aligned_edge=LEFT)
        o4 = CMTex(r"\bullet \text{ Tactical Refutation: } 15.\ \mathrm{dxe5 \ Nxe5 \ 16.\ Nxe5 \ Qxe5}", fontsize=12, height=0.15, color=JEWEL_GOLD).next_to(o3, DOWN, buff=0.14, aligned_edge=LEFT)
        ora_grp = VGroup(card_ora, ora_title, o1, o2, o3, o4)

        self.play(
            FadeOut(inherited_state),
            FadeIn(sa_header, UP * 0.1),
            FadeIn(cb_arena),
            FadeIn(wk_a), FadeIn(wq_a), FadeIn(wb_a), FadeIn(bk_a), FadeIn(br_a), FadeIn(bp_a),
            FadeIn(eval_bar_grp),
            FadeIn(ora_grp, RIGHT * 0.15),
            run_time=1.4,
            rate_func=smooth
        )
        self.wait(2.2)

        # =============================================================
        # BEAT 2: TOPOLOGICAL MATCH AUTOPSY (12s - 22s)
        # =============================================================
        card_auto = RoundedRectangle(
            width=5.8, height=3.8, corner_radius=0.14,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#38222a", stroke_width=1.5
        ).move_to(r_ora)

        auto_title = CMTex(r"\text{POST-MATCH TOPOLOGICAL AUTOPSY}", fontsize=16, height=0.22, color=JEWEL_CORAL).move_to(r_ora + UP * 1.45)
        a1 = CMTex(r"\bullet \text{ Match Concluded: 44 Plies (Move 22 Resignation)}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(auto_title, DOWN, buff=0.16, aligned_edge=LEFT)
        a2 = CMTex(r"\bullet \text{ Peak Evaluation Swing: } +8.42 \ \text{pawns}", fontsize=13, height=0.16, color=JEWEL_GOLD).next_to(a1, DOWN, buff=0.13, aligned_edge=LEFT)
        a3 = CMTex(r"\bullet \text{ Fatal Blunder: Move } 8\dots \mathrm{Nd7?} \ (-4.80 \ \mathrm{cp})", fontsize=13, height=0.16, color=JEWEL_CORAL).next_to(a2, DOWN, buff=0.13, aligned_edge=LEFT)
        a4 = CMTex(r"\bullet \text{ Topological Cause: Queenside Pawn Disconnect}", fontsize=13, height=0.16, color=JEWEL_CYAN).next_to(a3, DOWN, buff=0.13, aligned_edge=LEFT)
        a5 = CMTex(r"\text{``Pawn chain severed at c4; a8 rook completely isolated.''}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(a4, DOWN, buff=0.14, aligned_edge=LEFT)
        auto_grp = VGroup(card_auto, auto_title, a1, a2, a3, a4, a5)

        pos_a8 = cb_arena.get_square_pos(0, 7)
        pos_g8 = cb_arena.get_square_pos(6, 7)
        sever_glow = Line(pos_a8, pos_g8, color=JEWEL_CORAL, stroke_width=4.8, stroke_opacity=0.30)
        sever_core = Line(pos_a8, pos_g8, color=JEWEL_CORAL, stroke_width=1.8, stroke_opacity=0.95)
        sever_laser = VGroup(sever_glow, sever_core)
        sever_badge = RoundedRectangle(width=4.0, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).move_to(cb_cen + DOWN * 2.25)
        sever_txt = CMTex(r"\mathbf{DEFENSE \ CHAIN \ SEVERED}", fontsize=13, height=0.16, color=JEWEL_CORAL).move_to(sever_badge.get_center())
        sever_grp = VGroup(sever_laser, sever_badge, sever_txt)

        self.play(
            FadeOut(ora_grp),
            FadeIn(auto_grp, RIGHT * 0.15),
            ShowCreation(sever_laser),
            FadeIn(sever_badge), FadeIn(sever_txt),
            run_time=1.3
        )
        self.wait(2.5)

        # =============================================================
        # BEAT 3: COSMIC PULL-BACK & SPECTRAL NETWORK (22s - 31s)
        # =============================================================
        self.play(
            FadeOut(sa_header),
            FadeOut(cb_arena),
            FadeOut(wk_a), FadeOut(wq_a), FadeOut(wb_a), FadeOut(bk_a), FadeOut(br_a), FadeOut(bp_a),
            FadeOut(eval_bar_grp),
            FadeOut(auto_grp),
            FadeOut(sever_grp),
            run_time=1.0,
            rate_func=smooth
        )

        title_cosmic = CMTex(
            r"\text{First Principles Over Statistical Guesswork}",
            fontsize=26,
            height=0.38,
            color=JEWEL_GOLD
        ).move_to(UP * 2.50)
        sub_cosmic = CMTex(
            r"\text{``Sometimes, you can just let the geometry of the network speak for itself.''}",
            fontsize=16,
            height=0.22,
            color=TEXT_WHITE
        ).next_to(title_cosmic, DOWN, buff=0.14)
        cosmic_header = VGroup(title_cosmic, sub_cosmic)

        self.play(FadeIn(cosmic_header, UP * 0.08), run_time=0.9)

        node_pts = [
            np.array([-2.8, 0.9, 0]),
            np.array([-1.2, 1.6, 0]),
            np.array([1.0, 1.4, 0]),
            np.array([2.6, 0.5, 0]),
            np.array([2.0, -0.9, 0]),
            np.array([0.4, -1.5, 0]),
            np.array([-1.4, -1.1, 0]),
            np.array([-2.6, -0.4, 0]),
            np.array([0.0, 0.2, 0]),
        ]

        edges = VGroup()
        for i in range(len(node_pts)):
            for j in range(i + 1, len(node_pts)):
                if (i + j) % 2 == 0 or abs(i - j) <= 2 or i == 8 or j == 8:
                    opacity = 0.85 if (i == 8 or j == 8) else 0.45
                    col = JEWEL_GOLD if (i == 8 or j == 8) else JEWEL_CYAN
                    e = Line(node_pts[i], node_pts[j], stroke_color=col, stroke_width=1.6, stroke_opacity=opacity)
                    edges.add(e)

        nodes = VGroup()
        for idx, pt in enumerate(node_pts):
            fill_c = JEWEL_GOLD if idx == 8 else (JEWEL_CYAN if idx % 2 == 0 else JEWEL_GREEN)
            rad = 0.16 if idx == 8 else 0.12
            d = Circle(radius=rad, fill_color=fill_c, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.2).move_to(pt)
            nodes.add(d)

        tactical_graph = VGroup(edges, nodes).move_to(DOWN * 0.35)

        self.play(ShowCreation(edges), FadeIn(nodes), run_time=1.6)
        self.wait(2.2)

        # =============================================================
        # BEAT 4: THE MASTER OPEN-SOURCE OUTRO (31s - 42s)
        # =============================================================
        self.play(
            FadeOut(cosmic_header),
            tactical_graph.animate.scale(0.70).move_to(UP * 1.15),
            run_time=1.1,
            rate_func=smooth
        )

        outro_name = CMTex(r"\mathbf{HEAVEN'S \ GATE}", fontsize=36, height=0.55, color=JEWEL_GOLD).move_to(DOWN * 0.65)
        outro_sub = CMTex(r"\text{THE SOVEREIGN CLASSICAL CHESS ENGINE}", fontsize=17, height=0.22, color=TEXT_WHITE).next_to(outro_name, DOWN, buff=0.14)

        specs_card = RoundedRectangle(
            width=9.6, height=0.58, corner_radius=0.10,
            fill_color="#0b1424", fill_opacity=0.95,
            stroke_color="#2b4060", stroke_width=1.5
        ).move_to(DOWN * 1.65)
        specs_txt = CMTex(
            r"\text{Pure C++20} \quad \bullet \quad \text{Zero Neural Weights} \quad \bullet \quad \text{35M+ NPS} \quad \bullet \quad \text{100\% Open Source}",
            fontsize=15, height=0.22, color=JEWEL_GREEN
        ).move_to(specs_card.get_center())
        specs_grp = VGroup(specs_card, specs_txt)

        repo_card = RoundedRectangle(
            width=6.8, height=0.50, corner_radius=0.10,
            fill_color="#09182b", fill_opacity=0.95,
            stroke_color=JEWEL_CYAN, stroke_width=1.5
        ).move_to(DOWN * 2.38)
        repo_txt = Text(
            "github.com/Weptune/Heavens-Gate",
            font="Consolas", font_size=18, color=JEWEL_CYAN
        ).move_to(repo_card.get_center())
        repo_grp = VGroup(repo_card, repo_txt)

        climax_final = CMTex(
            r"\text{``From first principles to sovereign mastery.''}",
            fontsize=16,
            height=0.22,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        outro_grp = VGroup(outro_name, outro_sub, specs_grp, repo_grp, climax_final)

        self.play(FadeIn(outro_grp, UP * 0.12), run_time=1.4)
        self.wait(3.5)
