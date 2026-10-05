"""
Heaven's Gate Documentary - Scene 09: The Split-Brain Discovery
Broadcast-Grade Craft Standard (3Blue1Brown / vcubingx quality)
Runtime: ~36 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Obsidian Canvas (#000000) + Luminous Jewel Palette

Frame Continuity:
- Frame 0 inherits the exact terminal frame of Scene 08: Git cut laser, restored branch, and Revert quote.
- Beat 1: The Two Hemispheres Side-by-Side (Heavy 46K Spectral vs Fast 0-Weight Classical).
- Beat 2: The 1,000,000-Node Profiler Waterfall (0.2% vs 99.8% reality).
- Beat 3: The Source Code Illusion vs CPU Reality.
- Beat 4: Commit 6653436 - Severing the Ballast, Unleashing 40M NPS.
- Terminal frame held cleanly for Scene 10 handoff.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from theme import *
from cm_math import CMTex


class Scene09TheSplitBrain(Scene):
    def construct(self):
        # Canvas: Pure Obsidian Black
        canvas_bg = Rectangle(width=16, height=10, fill_color="#000000", fill_opacity=1.0, stroke_width=0)
        self.add(canvas_bg)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% INHERITANCE FROM SCENE 08 TERMINAL
        # =============================================================
        title_scene08_prev = CMTex(
            r"\text{8. The Hall of Shame: When Mathematics Goes Horribly Wrong}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        d5_title_prev = CMTex(r"\text{Commit 8e240be // August 11, 2:00 AM: The Breaking Point}", fontsize=22, height=0.30, color=JEWEL_CORAL).move_to(UP * 2.50)
        d5_sub_prev = CMTex(r"\text{Two months of experimental code: purged in a single stroke}", fontsize=15, height=0.20, color=TEXT_MUTED).move_to(UP * 2.15)
        d5_header_prev = VGroup(d5_title_prev, d5_sub_prev)

        git_track_prev = Line(LEFT * 5.2, RIGHT * 5.2, color="#253347", stroke_width=2.5).move_to(UP * 0.70)
        clean_branch_prev = Line(LEFT * 5.2, LEFT * 0.5, color=JEWEL_GREEN, stroke_width=4.0).move_to(UP * 0.70)

        cut_laser_prev = Line(UP * 1.45 + LEFT * 0.5, DOWN * 0.05 + LEFT * 0.5, color="#ffffff", stroke_width=4.5)
        cut_badge_prev = RoundedRectangle(width=5.2, height=0.38, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.4).next_to(cut_laser_prev, UP, buff=0.08)
        cut_lbl_prev = CMTex(r"\text{GIT RESET --HARD (200 COMMITS SEVERED)}", fontsize=14, height=0.18, color=JEWEL_CORAL).move_to(cut_badge_prev.get_center())
        cut_grp_prev = VGroup(cut_badge_prev, cut_lbl_prev)

        c_rest = DOWN * 1.35
        card_rest_prev = RoundedRectangle(
            width=9.8, height=2.1, corner_radius=0.12,
            fill_color="#07120c", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.4
        ).move_to(c_rest)

        rest_t1_prev = CMTex(r"\bullet \text{ Tore down Phase 4 Tropical Rational Functions [T1 - T2]}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(c_rest + UP * 0.55)
        rest_t2_prev = CMTex(r"\bullet \text{ Tore down Phase 5 Chebyshev Spectral Filters [T2(L)]}", fontsize=14, height=0.18, color=TEXT_MUTED).next_to(rest_t1_prev, DOWN, buff=0.12)
        rest_t3_prev = CMTex(r"\bullet \text{ Restored codebase to Phase 2 Round 17: Pure Stability}", fontsize=15, height=0.20, color=JEWEL_GREEN).next_to(rest_t2_prev, DOWN, buff=0.14)
        rest_grp_prev = VGroup(card_rest_prev, rest_t1_prev, rest_t2_prev, rest_t3_prev)

        revert_quote_prev = CMTex(
            r"\text{``Sometimes the most important commit you make is deleting two months of work.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(title_scene08_prev, d5_header_prev, git_track_prev, clean_branch_prev, cut_laser_prev, cut_grp_prev, rest_grp_prev, revert_quote_prev)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE TWO HEMISPHERES (0s - 10s)
        # =============================================================
        title_sb = CMTex(
            r"\text{9. The Split-Brain Discovery: CPU Profiler Audit}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sub_sb = CMTex(
            r"\text{When high-resolution CPU profiling exposed what the engine was actually executing}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).move_to(UP * 2.35)
        sb_header = VGroup(title_sb, sub_sb)

        self.play(
            FadeOut(inherited_state),
            FadeIn(sb_header, UP * 0.1),
            run_time=1.2,
            rate_func=smooth
        )
        self.wait(0.5)

        # Left Hemisphere Card: Heavy Spectral-Tropical Model
        c_left = LEFT * 3.2 + DOWN * 0.35
        card_left = RoundedRectangle(
            width=5.8, height=4.2, corner_radius=0.14,
            fill_color="#090a16", fill_opacity=0.94,
            stroke_color=JEWEL_VIOLET, stroke_width=1.5
        ).move_to(c_left)

        l_hdr = CMTex(r"\text{Heavy Spectral-Tropical Model}", fontsize=17, height=0.24, color=JEWEL_VIOLET).move_to(c_left + UP * 1.55)
        l_p1 = CMTex(r"\bullet \ 46{,}920 \text{ Trained Floating-Point Parameters}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(l_hdr, DOWN, buff=0.20, aligned_edge=LEFT)
        l_p2 = CMTex(r"\bullet \ 10 \text{ Spatial King Buckets (Piece Clustered)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(l_p1, DOWN, buff=0.14, aligned_edge=LEFT)
        l_p3 = CMTex(r"\bullet \text{Log-Sum-Exp Temperature Smoothing } (\tau = 0.5)", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(l_p2, DOWN, buff=0.14, aligned_edge=LEFT)
        l_p4 = CMTex(r"\bullet \text{Latency: } \sim 45 \ \mu\mathrm{s} \text{ / evaluation call}", fontsize=14, height=0.18, color=JEWEL_CORAL).next_to(l_p3, DOWN, buff=0.16, aligned_edge=LEFT)
        l_badge = RoundedRectangle(width=4.6, height=0.36, corner_radius=0.08, fill_color="#180a0e", fill_opacity=0.95, stroke_color=JEWEL_CORAL, stroke_width=1.2).next_to(l_p4, DOWN, buff=0.18)
        l_badge_txt = CMTex(r"\text{HIGH COMPLEXITY BALLAST}", fontsize=12, height=0.15, color=JEWEL_CORAL).move_to(l_badge.get_center())
        l_grp = VGroup(card_left, l_hdr, l_p1, l_p2, l_p3, l_p4, l_badge, l_badge_txt)

        # Right Hemisphere Card: evaluate_fast() Classical Core
        c_right = RIGHT * 3.2 + DOWN * 0.35
        card_right = RoundedRectangle(
            width=5.8, height=4.2, corner_radius=0.14,
            fill_color="#07140e", fill_opacity=0.94,
            stroke_color=JEWEL_GREEN, stroke_width=1.5
        ).move_to(c_right)

        r_hdr = CMTex(r"\mathtt{evaluate\_fast()} \text{ Classical Core}", fontsize=17, height=0.24, color=JEWEL_GREEN).move_to(c_right + UP * 1.55)
        r_p1 = CMTex(r"\bullet \ 0 \text{ Weights // Handcrafted Chess Heuristics}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(r_hdr, DOWN, buff=0.20, aligned_edge=LEFT)
        r_p2 = CMTex(r"\bullet \ 64\text{-bit Bitboard Material \& Centered PST}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(r_p1, DOWN, buff=0.14, aligned_edge=LEFT)
        r_p3 = CMTex(r"\bullet \text{Tapered Game Phase Interpolation } (\mathrm{MG} \to \mathrm{EG})", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(r_p2, DOWN, buff=0.14, aligned_edge=LEFT)
        r_p4 = CMTex(r"\bullet \text{Latency: } \sim 3 \ \mathrm{ns} \text{ / evaluation call}", fontsize=14, height=0.18, color=JEWEL_CYAN).next_to(r_p3, DOWN, buff=0.16, aligned_edge=LEFT)
        r_badge = RoundedRectangle(width=4.6, height=0.36, corner_radius=0.08, fill_color="#071810", fill_opacity=0.95, stroke_color=JEWEL_GREEN, stroke_width=1.2).next_to(r_p4, DOWN, buff=0.18)
        r_badge_txt = CMTex(r"\mathbf{15{,}000\times \ FASTER \ THROUGHPUT}", fontsize=12, height=0.15, color=JEWEL_GREEN).move_to(r_badge.get_center())
        r_grp = VGroup(card_right, r_hdr, r_p1, r_p2, r_p3, r_p4, r_badge, r_badge_txt)

        self.play(FadeIn(l_grp, LEFT * 0.3), FadeIn(r_grp, RIGHT * 0.3), run_time=1.2)
        self.wait(2.2)

        # =============================================================
        # BEAT 2: THE 1,000,000 NODE PROFILER WATERFALL (10s - 20s)
        # =============================================================
        self.play(
            FadeOut(sb_header),
            FadeOut(l_grp),
            FadeOut(r_grp),
            run_time=0.8,
            rate_func=smooth
        )

        title_prof = CMTex(
            r"\text{The } 1{,}000{,}000 \text{ Node Profiler Audit: The Great Dispatch Shock}",
            fontsize=22,
            height=0.30,
            color=JEWEL_CYAN
        ).move_to(UP * 2.50)
        sub_prof = CMTex(
            r"\text{Sampling evaluation function call distribution during tournament search}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_prof, DOWN, buff=0.12)
        prof_header = VGroup(title_prof, sub_prof)
        self.play(FadeIn(prof_header, DOWN * 0.1), run_time=0.6)

        root_card = RoundedRectangle(
            width=8.6, height=0.75, corner_radius=0.10,
            fill_color="#0a101d", fill_opacity=0.95,
            stroke_color=JEWEL_GOLD, stroke_width=1.5
        ).move_to(UP * 1.45)
        root_txt = CMTex(r"\mathbf{Alpha\text{-}Beta \ Search \ Tree: \ 1{,}000{,}000 \ Evaluation \ Calls}", fontsize=16, height=0.22, color=TEXT_WHITE).move_to(root_card.get_center())
        root_node = VGroup(root_card, root_txt)

        self.play(FadeIn(root_node, DOWN * 0.1), run_time=0.8)

        # Diverging Stream Paths
        p_root_l = root_card.get_bottom() + LEFT * 1.5
        p_root_r = root_card.get_bottom() + RIGHT * 1.5

        p_dest_l = LEFT * 3.5 + DOWN * 0.1
        p_dest_r = RIGHT * 3.5 + DOWN * 0.1

        stream_l = Line(p_root_l, p_dest_l, color=JEWEL_CORAL, stroke_width=2.0)
        stream_r = Line(p_root_r, p_dest_r, color=JEWEL_GREEN, stroke_width=8.5)

        self.play(ShowCreation(stream_l), ShowCreation(stream_r), run_time=1.0)

        # Left Result Card: 0.2%
        card_res_l = RoundedRectangle(
            width=4.8, height=2.2, corner_radius=0.12,
            fill_color="#12080c", fill_opacity=0.92,
            stroke_color=JEWEL_CORAL, stroke_width=1.4
        ).move_to(LEFT * 3.5 + DOWN * 1.35)

        pct_l = CMTex(r"0.2 \%", fontsize=38, height=0.52, color=JEWEL_CORAL).move_to(card_res_l.get_center() + UP * 0.50)
        cnt_l = CMTex(r"2{,}104 \text{ calls (PV Root Only)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(pct_l, DOWN, buff=0.12)
        tag_l = CMTex(r"\text{Spectral-Tropical Model (Ballast)}", fontsize=14, height=0.18, color=JEWEL_CORAL).next_to(cnt_l, DOWN, buff=0.10)
        grp_res_l = VGroup(card_res_l, pct_l, cnt_l, tag_l)

        # Right Result Card: 99.8%
        card_res_r = RoundedRectangle(
            width=4.8, height=2.2, corner_radius=0.12,
            fill_color="#07180e", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.5
        ).move_to(RIGHT * 3.5 + DOWN * 1.35)

        pct_r = CMTex(r"99.8 \%", fontsize=38, height=0.52, color=JEWEL_GREEN).move_to(card_res_r.get_center() + UP * 0.50)
        cnt_r = CMTex(r"997{,}896 \text{ calls (QSearch + Cutoffs)}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(pct_r, DOWN, buff=0.12)
        tag_r = CMTex(r"\mathtt{evaluate\_fast()} \text{ (The Real Engine)}", fontsize=14, height=0.18, color=JEWEL_GREEN).next_to(cnt_r, DOWN, buff=0.10)
        grp_res_r = VGroup(card_res_r, pct_r, cnt_r, tag_r)

        self.play(FadeIn(grp_res_l, UP * 0.1), FadeIn(grp_res_r, UP * 0.1), run_time=1.1)

        fact_banner = CMTex(
            r"\text{Fact: } 99.8\% \text{ of all chess moves were decided by 0 weights.}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.0)

        self.play(FadeIn(fact_banner, UP * 0.08), run_time=0.9)
        self.wait(2.2)

        # =============================================================
        # BEAT 3: THE CODE ILLUSION VS REALITY (20s - 28s)
        # =============================================================
        self.play(
            FadeOut(prof_header),
            FadeOut(root_node),
            FadeOut(stream_l),
            FadeOut(stream_r),
            FadeOut(grp_res_l),
            FadeOut(grp_res_r),
            FadeOut(fact_banner),
            run_time=0.8,
            rate_func=smooth
        )

        title_code = CMTex(
            r"\text{The Architectural Illusion: What We Believed vs What Ran}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GOLD
        ).move_to(UP * 2.50)
        sub_code = CMTex(
            r"\text{The startup banner promised advanced AI; the inner search loop begged for speed}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_code, DOWN, buff=0.12)
        code_header = VGroup(title_code, sub_code)
        self.play(FadeIn(code_header, DOWN * 0.1), run_time=0.6)

        c_top = UP * 1.05
        card_ill = RoundedRectangle(
            width=9.8, height=1.7, corner_radius=0.12,
            fill_color="#070c16", fill_opacity=0.92,
            stroke_color="#223048", stroke_width=1.5
        ).move_to(c_top)

        lbl_ill = CMTex(r"\text{STARTUP BANNER ILLUSION // console output:}", fontsize=13, height=0.16, color=JEWEL_VIOLET).move_to(c_top + UP * 0.50)
        txt_ill_1 = CMTex(r"\mathtt{[INFO] \ Loaded \ 46{,}920 \ weights \ across \ 10 \ spatial \ king \ zones}", fontsize=14, height=0.18, color=TEXT_MUTED).next_to(lbl_ill, DOWN, buff=0.12)
        txt_ill_2 = CMTex(r"\mathtt{Evaluator::set\_mode(EvalMode::SpectralTropical);}", fontsize=14, height=0.18, color=JEWEL_GOLD).next_to(txt_ill_1, DOWN, buff=0.08)
        grp_ill = VGroup(card_ill, lbl_ill, txt_ill_1, txt_ill_2)

        c_bot = DOWN * 0.85
        card_real = RoundedRectangle(
            width=9.8, height=1.9, corner_radius=0.12,
            fill_color="#07180e", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.5
        ).move_to(c_bot)

        lbl_real = CMTex(r"\text{INNER SEARCH ENGINE REALITY // search.cpp line 412:}", fontsize=13, height=0.16, color=JEWEL_GREEN).move_to(c_bot + UP * 0.58)
        txt_real_1 = CMTex(r"\mathtt{if \ (in\_check \ || \ qsearch \ || \ !is\_pv\_node \ || \ delta\_pruned)}", fontsize=14, height=0.18, color=TEXT_WHITE).next_to(lbl_real, DOWN, buff=0.12)
        txt_real_2 = CMTex(r"\mathtt{\quad return \ evaluate\_fast(board); \quad // \ 997{,}896 \ of \ 1{,}000{,}000 \ calls!}", fontsize=14, height=0.18, color=JEWEL_CYAN).next_to(txt_real_1, DOWN, buff=0.08)
        grp_real = VGroup(card_real, lbl_real, txt_real_1, txt_real_2)

        self.play(FadeIn(grp_ill, UP * 0.1), run_time=0.9)
        self.play(FadeIn(grp_real, UP * 0.1), run_time=1.0)

        stamp_box = RoundedRectangle(
            width=5.8, height=0.48, corner_radius=0.08,
            fill_color="#1a080d", fill_opacity=0.95,
            stroke_color=JEWEL_CORAL, stroke_width=1.6
        ).move_to(DOWN * 2.30)
        stamp_txt = CMTex(r"\mathbf{THE \ COMPLEXITY \ WAS \ A \ COSMETIC \ FACADE}", fontsize=13, height=0.16, color=JEWEL_CORAL).move_to(stamp_box.get_center())
        stamp_grp = VGroup(stamp_box, stamp_txt)

        self.play(FadeIn(stamp_grp, scale=1.15), run_time=0.8)
        self.wait(2.0)

        # =============================================================
        # BEAT 4: COMMIT 6653436 - PERMANENT LIBERATION (28s - 36s)
        # =============================================================
        self.play(
            FadeOut(code_header),
            FadeOut(grp_ill),
            FadeOut(grp_real),
            FadeOut(stamp_grp),
            run_time=0.8,
            rate_func=smooth
        )

        title_epi = CMTex(
            r"\text{Commit 6653436: Severing the Complexity Ballast}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GREEN
        ).move_to(UP * 2.50)
        sub_epi = CMTex(
            r"\text{Permanently locking the engine to pure, uncompromised classical evaluation}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_epi, DOWN, buff=0.12)
        epi_header = VGroup(title_epi, sub_epi)
        self.play(FadeIn(epi_header, DOWN * 0.1), run_time=0.6)

        c_dec = DOWN * 0.20
        card_dec = RoundedRectangle(
            width=9.8, height=2.8, corner_radius=0.14,
            fill_color="#07120c", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.5
        ).move_to(c_dec)

        p1 = CMTex(r"\bullet \text{ The 46,920-parameter model was pure drag throttling our search depth.}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(c_dec + UP * 0.85)
        p2 = CMTex(r"\bullet \text{ Heaven's Gate was winning purely because of } \mathtt{evaluate\_fast()}.", fontsize=15, height=0.20, color=TEXT_WHITE).next_to(p1, DOWN, buff=0.14)
        p3 = CMTex(r"\bullet \text{ Dismantling the heavy model unlocked full } 40{,}000{,}000 \text{ NPS throughput!}", fontsize=15, height=0.20, color=JEWEL_GREEN).next_to(p2, DOWN, buff=0.14)
        p4 = CMTex(r"\bullet \mathtt{EvalMode::MasterPositional} \ \Rightarrow \ \text{Locked as permanent default architecture.}", fontsize=14, height=0.18, color=JEWEL_CYAN).next_to(p3, DOWN, buff=0.14)
        points_grp = VGroup(card_dec, p1, p2, p3, p4)

        climax_quote = CMTex(
            r"\text{``We didn't need 46,000 parameters. We needed ruthless classical speed.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        self.play(FadeIn(points_grp), FadeIn(climax_quote, UP * 0.08), run_time=1.2)
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 10 Handoff)
        # =============================================================
        self.wait(2.5)
