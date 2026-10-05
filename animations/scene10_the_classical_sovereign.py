"""
Heaven's Gate Documentary - Scene 10: The Classical Sovereign
Broadcast-Grade Craft Standard (3Blue1Brown / vcubingx quality)
Runtime: ~36 seconds | Resolution: 1920x1080 | 60 fps
Aesthetic: Pure Obsidian Canvas (#000000) + Luminous Jewel Palette

Frame Continuity:
- Frame 0 inherits the exact terminal frame of Scene 09: Dismantling bullet points and classical speed quote.
- Beat 1: The Four High-Performance Pillars (Rayleigh Quotient, 64-Byte Lockless TT, 4D Cont History, Root PVS).
- Beat 2: Lockless Lazy SMP - 16-Thread Orbital Architecture.
- Beat 3: The 35,000,000 NPS Tachometer Surge & Climax.
- Terminal frame held cleanly for Scene 11 handoff.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path(__file__).parent))
from theme import *
from cm_math import CMTex


class Scene10TheClassicalSovereign(Scene):
    def construct(self):
        # Canvas: Pure Obsidian Black
        canvas_bg = Rectangle(width=16, height=10, fill_color="#000000", fill_opacity=1.0, stroke_width=0)
        self.add(canvas_bg)

        # =============================================================
        # FRAME 0 CONTINUITY: 100% INHERITANCE FROM SCENE 09 TERMINAL
        # =============================================================
        title_scene09_prev = CMTex(
            r"\text{9. The Split-Brain Discovery: CPU Profiler Audit}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        title_epi_prev = CMTex(
            r"\text{Commit 6653436: Severing the Complexity Ballast}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GREEN
        ).move_to(UP * 2.50)
        sub_epi_prev = CMTex(
            r"\text{Permanently locking the engine to pure, uncompromised classical evaluation}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_epi_prev, DOWN, buff=0.12)
        epi_header_prev = VGroup(title_epi_prev, sub_epi_prev)

        c_dec = DOWN * 0.20
        card_dec_prev = RoundedRectangle(
            width=9.8, height=2.8, corner_radius=0.14,
            fill_color="#07120c", fill_opacity=0.92,
            stroke_color=JEWEL_GREEN, stroke_width=1.5
        ).move_to(c_dec)

        p1_prev = CMTex(r"\bullet \text{ The 46,920-parameter model was pure drag throttling our search depth.}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(c_dec + UP * 0.85)
        p2_prev = CMTex(r"\bullet \text{ Heaven's Gate was winning purely because of } \mathtt{evaluate\_fast()}.", fontsize=15, height=0.20, color=TEXT_WHITE).next_to(p1_prev, DOWN, buff=0.14)
        p3_prev = CMTex(r"\bullet \text{ Dismantling the heavy model unlocked full } 40{,}000{,}000 \text{ NPS throughput!}", fontsize=15, height=0.20, color=JEWEL_GREEN).next_to(p2_prev, DOWN, buff=0.14)
        p4_prev = CMTex(r"\bullet \mathtt{EvalMode::MasterPositional} \ \Rightarrow \ \text{Locked as permanent default architecture.}", fontsize=14, height=0.18, color=JEWEL_CYAN).next_to(p3_prev, DOWN, buff=0.14)
        points_grp_prev = VGroup(card_dec_prev, p1_prev, p2_prev, p3_prev, p4_prev)

        climax_quote_prev = CMTex(
            r"\text{``We didn't need 46,000 parameters. We needed ruthless classical speed.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        inherited_state = VGroup(title_scene09_prev, epi_header_prev, points_grp_prev, climax_quote_prev)
        self.add(inherited_state)
        self.wait(0.6)

        # =============================================================
        # BEAT 1: THE FOUR HIGH-PERFORMANCE PILLARS (0s - 12s)
        # =============================================================
        title_cs = CMTex(
            r"\text{10. The Classical Sovereign: Architecture of 35 Million NPS}",
            fontsize=28,
            height=0.42,
            color=JEWEL_GOLD
        ).to_corner(UL, buff=0.6)

        sub_cs = CMTex(
            r"\text{Four systems engineering breakthroughs that unlocked sovereign computational speed}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).move_to(UP * 2.35)
        cs_header = VGroup(title_cs, sub_cs)

        self.play(
            FadeOut(inherited_state),
            FadeIn(cs_header, UP * 0.1),
            run_time=1.2,
            rate_func=smooth
        )
        self.wait(0.5)

        # Pillar 1: Top-Left
        c_p1 = LEFT * 3.2 + UP * 0.85
        card_p1 = RoundedRectangle(width=5.8, height=1.7, corner_radius=0.10, fill_color="#070c16", fill_opacity=0.92, stroke_color=JEWEL_CYAN, stroke_width=1.3).move_to(c_p1)
        t_p1 = CMTex(r"\text{1. 28-Loop Rayleigh Quotient}", fontsize=15, height=0.20, color=JEWEL_CYAN).move_to(c_p1 + UP * 0.50)
        d_p1a = CMTex(r"\bullet \text{ Power iterations replace } \mathcal{O}(N^3) \text{ QR eigensolve}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(t_p1, DOWN, buff=0.10, aligned_edge=LEFT)
        d_p1b = CMTex(r"\bullet \ 2{,}048\text{-entry Zobrist spectral cache (2 ns probe)}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(d_p1a, DOWN, buff=0.08, aligned_edge=LEFT)
        grp_p1 = VGroup(card_p1, t_p1, d_p1a, d_p1b)

        # Pillar 2: Top-Right
        c_p2 = RIGHT * 3.2 + UP * 0.85
        card_p2 = RoundedRectangle(width=5.8, height=1.7, corner_radius=0.10, fill_color="#070c16", fill_opacity=0.92, stroke_color=JEWEL_GOLD, stroke_width=1.3).move_to(c_p2)
        t_p2 = CMTex(r"\text{2. 64-Byte TT and Lockless XOR}", fontsize=15, height=0.20, color=JEWEL_GOLD).move_to(c_p2 + UP * 0.50)
        d_p2a = CMTex(r"\bullet \ 4\text{-entry clusters match exact CPU cache lines (64 B)}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(t_p2, DOWN, buff=0.10, aligned_edge=LEFT)
        d_p2b = CMTex(r"\bullet \text{Lockless atomic verification: } \mathtt{key} \oplus \mathtt{data\_word}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(d_p2a, DOWN, buff=0.08, aligned_edge=LEFT)
        grp_p2 = VGroup(card_p2, t_p2, d_p2a, d_p2b)

        # Pillar 3: Bottom-Left
        c_p3 = LEFT * 3.2 + DOWN * 1.15
        card_p3 = RoundedRectangle(width=5.8, height=1.7, corner_radius=0.10, fill_color="#070c16", fill_opacity=0.92, stroke_color=JEWEL_VIOLET, stroke_width=1.3).move_to(c_p3)
        t_p3 = CMTex(r"\text{3. 4D Continuation History Tensor}", fontsize=15, height=0.20, color=JEWEL_VIOLET).move_to(c_p3 + UP * 0.50)
        d_p3a = CMTex(r"\bullet \ 6\text{-ply continuation history tensor + History Malus}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(t_p3, DOWN, buff=0.10, aligned_edge=LEFT)
        d_p3b = CMTex(r"\bullet \ 87.2\% \text{ first-move cutoff rate in move picker}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(d_p3a, DOWN, buff=0.08, aligned_edge=LEFT)
        grp_p3 = VGroup(card_p3, t_p3, d_p3a, d_p3b)

        # Pillar 4: Bottom-Right
        c_p4 = RIGHT * 3.2 + DOWN * 1.15
        card_p4 = RoundedRectangle(width=5.8, height=1.7, corner_radius=0.10, fill_color="#070c16", fill_opacity=0.92, stroke_color=JEWEL_GREEN, stroke_width=1.3).move_to(c_p4)
        t_p4 = CMTex(r"\text{4. Root PVS and Singular Extensions}", fontsize=15, height=0.20, color=JEWEL_GREEN).move_to(c_p4 + UP * 0.50)
        d_p4a = CMTex(r"\bullet \text{Zero-window null probes on all non-PV root moves}", fontsize=13, height=0.16, color=TEXT_WHITE).next_to(t_p4, DOWN, buff=0.10, aligned_edge=LEFT)
        d_p4b = CMTex(r"\bullet \text{Quadratic Safe Check defense matrix}", fontsize=12, height=0.15, color=TEXT_MUTED).next_to(d_p4a, DOWN, buff=0.08, aligned_edge=LEFT)
        grp_p4 = VGroup(card_p4, t_p4, d_p4a, d_p4b)

        self.play(FadeIn(grp_p1, LEFT * 0.2), FadeIn(grp_p2, RIGHT * 0.2), run_time=1.0)
        self.play(FadeIn(grp_p3, LEFT * 0.2), FadeIn(grp_p4, RIGHT * 0.2), run_time=1.0)
        self.wait(2.2)

        # =============================================================
        # BEAT 2: LOCKLESS LAZY SMP PARALLEL SCALING (12s - 22s)
        # =============================================================
        self.play(
            FadeOut(cs_header),
            FadeOut(grp_p1),
            FadeOut(grp_p2),
            FadeOut(grp_p3),
            FadeOut(grp_p4),
            run_time=0.8,
            rate_func=smooth
        )

        title_smp = CMTex(
            r"\text{Lockless Lazy SMP: 16-Thread Tree Traversal}",
            fontsize=22,
            height=0.30,
            color=JEWEL_CYAN
        ).move_to(UP * 2.50)
        sub_smp = CMTex(
            r"\text{Helper threads independently explore alternate branches sharing lockless TT memory}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_smp, DOWN, buff=0.12)
        smp_header = VGroup(title_smp, sub_smp)
        self.play(FadeIn(smp_header, DOWN * 0.1), run_time=0.6)

        c_smp = DOWN * 0.35
        tt_hub = Circle(radius=1.0, fill_color="#0a1220", fill_opacity=0.95, stroke_color=JEWEL_CYAN, stroke_width=2.2).move_to(c_smp)
        tt_lbl1 = CMTex(r"\text{Global TT}", fontsize=15, height=0.20, color=JEWEL_CYAN).move_to(c_smp + UP * 0.15)
        tt_lbl2 = CMTex(r"\text{(Lockless)}", fontsize=13, height=0.16, color=TEXT_MUTED).next_to(tt_lbl1, DOWN, buff=0.08)
        hub_grp = VGroup(tt_hub, tt_lbl1, tt_lbl2)

        thread_lines = VGroup()
        thread_dots = VGroup()
        thread_tags = VGroup()

        for i in range(8):
            angle = i * (TAU / 8.0)
            target_pos = c_smp + np.array([np.cos(angle) * 3.4, np.sin(angle) * 1.9, 0])
            inner_pos = c_smp + np.array([np.cos(angle) * 1.0, np.sin(angle) * 1.0, 0])
            ray = Line(inner_pos, target_pos, color="#253347", stroke_width=1.8)
            dot = Circle(radius=0.14, fill_color=JEWEL_GOLD, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.2).move_to(target_pos)
            tag = CMTex(rf"\mathrm{{T}}_{i}", fontsize=13, height=0.16, color=JEWEL_GOLD).next_to(dot, np.array([np.cos(angle), np.sin(angle), 0]) * 0.6, buff=0.08)
            thread_lines.add(ray)
            thread_dots.add(dot)
            thread_tags.add(tag)

        self.play(FadeIn(hub_grp), FadeIn(thread_lines), FadeIn(thread_dots), FadeIn(thread_tags), run_time=1.2)
        self.wait(2.2)

        # =============================================================
        # BEAT 3: THE 35,000,000 NPS TACHOMETER SURGE (22s - 36s)
        # =============================================================
        self.play(
            FadeOut(smp_header),
            FadeOut(hub_grp),
            FadeOut(thread_lines),
            FadeOut(thread_dots),
            FadeOut(thread_tags),
            run_time=0.8,
            rate_func=smooth
        )

        title_gauge = CMTex(
            r"\text{Throughput Benchmark: The 35 Million NPS Peak}",
            fontsize=22,
            height=0.30,
            color=JEWEL_GREEN
        ).move_to(UP * 2.50)
        sub_gauge = CMTex(
            r"\text{Real-time node counter accelerating from cold start to peak production speed}",
            fontsize=15,
            height=0.20,
            color=TEXT_MUTED
        ).next_to(title_gauge, DOWN, buff=0.12)
        gauge_header = VGroup(title_gauge, sub_gauge)
        self.play(FadeIn(gauge_header, DOWN * 0.1), run_time=0.6)

        g_cen = DOWN * 0.40
        r_gauge = 1.95

        arc_back = Arc(radius=r_gauge, start_angle=210 * DEGREES, angle=-240 * DEGREES, color="#1e293b", stroke_width=4.0, arc_center=g_cen)
        arc_norm = Arc(radius=r_gauge, start_angle=210 * DEGREES, angle=-160 * DEGREES, color=JEWEL_CYAN, stroke_width=4.0, arc_center=g_cen)
        arc_peak = Arc(radius=r_gauge, start_angle=50 * DEGREES, angle=-80 * DEGREES, color=JEWEL_GREEN, stroke_width=6.0, arc_center=g_cen)

        lbl_0 = CMTex(r"0", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([-2.25, -0.9, 0]))
        lbl_10m = CMTex(r"10\mathrm{M}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([-1.75, 1.65, 0]))
        lbl_20m = CMTex(r"20\mathrm{M}", fontsize=14, height=0.18, color=TEXT_MUTED).move_to(g_cen + np.array([0.0, 2.18, 0]))
        lbl_35m = CMTex(r"35\mathrm{M}+", fontsize=16, height=0.22, color=JEWEL_GREEN).move_to(g_cen + np.array([2.40, -0.7, 0]))
        scale_lbls = VGroup(lbl_0, lbl_10m, lbl_20m, lbl_35m)

        hub_pt = Circle(radius=0.16, fill_color=JEWEL_GOLD, fill_opacity=1.0, stroke_color="#ffffff", stroke_width=1.2).move_to(g_cen)

        def make_needle(angle_deg, color=JEWEL_GOLD):
            rad = angle_deg * DEGREES
            tip = g_cen + np.array([np.cos(rad) * (r_gauge - 0.1), np.sin(rad) * (r_gauge - 0.1), 0])
            n_perp = np.array([-np.sin(rad) * 0.08, np.cos(rad) * 0.08, 0])
            poly = Polygon(g_cen - n_perp * 1.2, tip, g_cen + n_perp * 1.2)
            poly.set_fill(color, opacity=1.0)
            poly.set_stroke(color="#ffffff", width=0.8, opacity=0.8)
            return poly

        needle = make_needle(210)

        c_readout = DOWN * 2.35
        nps_digits = CMTex(r"\text{35,420,119 NODES / SEC}", fontsize=24, height=0.34, color=JEWEL_GREEN).move_to(c_readout + UP * 0.15)
        depth_sub = CMTex(
            r"\text{Depth: 14 reached in 0.82s  } \bullet \text{  TT Hit Rate: 74.8\%  } \bullet \text{  First Move Cutoff: 87.2\%}",
            fontsize=13, height=0.17, color=TEXT_MUTED
        ).next_to(nps_digits, DOWN, buff=0.12)
        readout_grp = VGroup(nps_digits, depth_sub)
        gauge_grp = VGroup(arc_back, arc_norm, arc_peak, scale_lbls, hub_pt)

        climax_cs = CMTex(
            r"\text{``Depth beats subtlety. And 35 million positions per second beats everything.''}",
            fontsize=17,
            height=0.24,
            color=TEXT_WHITE
        ).move_to(DOWN * 3.12)

        self.play(FadeIn(gauge_grp), FadeIn(needle), FadeIn(readout_grp), FadeIn(climax_cs, UP * 0.08), run_time=1.2)
        self.wait(0.8)

        needle_surge = make_needle(-25, color=JEWEL_GREEN)

        self.play(Transform(needle, needle_surge), run_time=2.2, rate_func=rush_into)

        # Shudder vibration
        self.play(needle.animate.rotate(-3 * DEGREES, about_point=g_cen), run_time=0.15)
        self.play(needle.animate.rotate(3 * DEGREES, about_point=g_cen), run_time=0.15)
        self.wait(1.5)

        # =============================================================
        # TERMINAL FRAME PRESERVATION (Seamless Scene 11 Handoff)
        # =============================================================
        self.wait(2.5)
