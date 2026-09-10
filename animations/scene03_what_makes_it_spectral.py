"""
Heaven's Gate Documentary - Scene 03: What Makes it "Spectral"?
Script Beat:
- "2. What makes it 'Spectral'?"
- "Normally, when you multiply a matrix by a vector, it stretches the vector and rotates it..." (L v = w)
- "Except for a few very special directions... the matrix doesn't rotate them at all—it only scales them by a single constant factor, λ: L v = λ v"
- "Those special vectors are eigenvectors, and the scaling factors are eigenvalues."
- "The entire set of these eigenvalues is called the spectrum... like an optical prism splitting light into constituent wavelengths, eigenvalues split the chessboard into spatial frequencies."

Resolution: 1920x1080 | 16:9 Landscape
Aesthetic: Editorial Architectural / Swiss Print (No neon, no slop)
"""

import sys
from pathlib import Path
import numpy as np

from manimlib import *

# Palette: Matte Architectural Print
BG_BASALT        = "#121418"     # Deep matte charcoal
TEXT_BONE        = "#f1f3f7"     # Crisp chalk white
TEXT_MUTED       = "#7d8594"     # Editorial slate
GRID_HAIRLINE    = "#2d3340"     # Technical divider/border
ACCENT_COBALT    = "#4d8cf5"     # Blueprint vector / eigenvector
ACCENT_OCHRE     = "#d97706"     # General vector w
ACCENT_CYAN      = "#38bdf8"     # Mathematical eigenvalue λ
ACCENT_CRITICAL  = "#e04343"     # High frequency mode

class Scene03WhatMakesItSpectral(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # 0. Solid Matte Basalt Canvas
        # -------------------------------------------------------------
        bg = Rectangle(width=16, height=9, fill_color=BG_BASALT, fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # -------------------------------------------------------------
        # BEAT 1 (T=0.0s – 3.0s): Section Header
        # "2. What makes it 'Spectral'?"
        # -------------------------------------------------------------
        ch_tag = Text("CHAPTER 01 // THE MATHEMATICAL FOUNDATION", font="Consolas", color=TEXT_MUTED).scale(0.28)
        ch_tag.to_corner(UL, buff=0.7)

        q2_title = Text("2. What Makes it \"Spectral\"?", font="Segoe UI", color=TEXT_BONE).scale(0.55)
        q2_title.next_to(ch_tag, DOWN, aligned_edge=LEFT, buff=0.15)

        divider = Line(LEFT * 7.0, RIGHT * 7.0, color=GRID_HAIRLINE, stroke_width=0.8)
        divider.next_to(q2_title, DOWN, aligned_edge=LEFT, buff=0.25)

        self.play(
            FadeIn(ch_tag, UP * 0.15),
            FadeIn(q2_title, UP * 0.2),
            ShowCreation(divider),
            run_time=1.0
        )
        self.wait(1.0)

        # -------------------------------------------------------------
        # BEAT 2 (T=3.0s – 8.5s): General Linear Transformation (L v = w)
        # "Normally, when you multiply a matrix by a vector,
        #  it stretches the vector and rotates it into a completely new direction."
        # -------------------------------------------------------------
        axes_origin = LEFT * 3.6 + DOWN * 0.6
        axes = Axes(
            x_range=[-3, 3, 1],
            y_range=[-3, 3, 1],
            width=4.8,
            height=4.8,
            axis_config={
                "stroke_color": GRID_HAIRLINE,
                "stroke_width": 1.2,
                "include_ticks": True,
                "tick_size": 0.08
            }
        )
        axes.move_to(axes_origin)

        # Vector v (initial input vector)
        v_coords = np.array([1.6, 0.8, 0])
        arrow_v = Vector(v_coords, color=TEXT_BONE, stroke_width=3.0)
        arrow_v.shift(axes_origin)

        label_v = Text("v", font="Segoe UI", color=TEXT_BONE).scale(0.38)
        label_v.next_to(arrow_v.get_end(), UR, buff=0.12)

        formula_gen = Text("L · v  =  w", font="Consolas", color=TEXT_BONE).scale(0.60)
        formula_gen_desc = Text("General Matrix Action: Both Stretches and Rotates", font="Segoe UI", color=TEXT_MUTED).scale(0.32)
        formula_gen_desc.next_to(formula_gen, DOWN, aligned_edge=LEFT, buff=0.15)
        formula_group = VGroup(formula_gen, formula_gen_desc)
        formula_group.move_to(RIGHT * 3.0 + UP * 1.8)

        self.play(
            ShowCreation(axes),
            ShowCreation(arrow_v),
            FadeIn(label_v),
            FadeIn(formula_group, UP * 0.2),
            run_time=1.2
        )
        self.wait(0.6)

        # Transformed vector w = L * v (stretched and rotated!)
        w_coords = np.array([-0.7, 2.2, 0])
        arrow_w = Vector(w_coords, color=ACCENT_OCHRE, stroke_width=3.0)
        arrow_w.shift(axes_origin)

        label_w = Text("w (Rotated & Stretched)", font="Consolas", color=ACCENT_OCHRE).scale(0.32)
        label_w.next_to(arrow_w.get_end(), UL, buff=0.12)

        # Arc showing rotation
        rot_arc = CurvedArrow(
            axes_origin + v_coords * 0.55,
            axes_origin + w_coords * 0.45,
            color=ACCENT_OCHRE,
            stroke_width=1.5
        )

        self.play(
            ShowCreation(rot_arc),
            ShowCreation(arrow_w),
            FadeIn(label_w),
            run_time=1.2
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 3 (T=8.5s – 14.5s): The Special Eigenvector Direction (L v = λ v)
        # "Except for a few very special directions...
        #  the matrix doesn't rotate them at all—it only scales them by λ."
        # -------------------------------------------------------------
        formula_eigen = Text("L · v  =  λ · v", font="Consolas", color=ACCENT_CYAN).scale(0.68)
        formula_eigen_desc = Text("Eigenvector Condition: Zero Rotation • Pure Scaling by λ", font="Segoe UI", color=TEXT_BONE).scale(0.32)
        formula_eigen_desc.next_to(formula_eigen, DOWN, aligned_edge=LEFT, buff=0.15)
        eigen_box = VGroup(formula_eigen, formula_eigen_desc)
        eigen_box.move_to(RIGHT * 3.0 + UP * 1.8)

        # Target eigenvector coords along line y = x (pure 45 degree axis)
        eigen_line = DashedLine(
            axes_origin + np.array([-2.2, -2.2, 0]),
            axes_origin + np.array([2.2, 2.2, 0]),
            color=GRID_HAIRLINE,
            stroke_width=1.2
        )

        eigen_v_coords = np.array([1.1, 1.1, 0])
        scaled_v_coords = np.array([2.0, 2.0, 0])

        arrow_eigen_v = Vector(eigen_v_coords, color=ACCENT_COBALT, stroke_width=3.2)
        arrow_eigen_v.shift(axes_origin)
        label_eigen_v = Text("Eigenvector (v)", font="Consolas", color=ACCENT_COBALT).scale(0.30)
        label_eigen_v.next_to(arrow_eigen_v.get_end(), DR, buff=0.12)

        arrow_scaled_v = Vector(scaled_v_coords, color=ACCENT_CYAN, stroke_width=2.6)
        arrow_scaled_v.shift(axes_origin)
        label_scaled_v = Text("λ · v", font="Consolas", color=ACCENT_CYAN).scale(0.34)
        label_scaled_v.next_to(arrow_scaled_v.get_end(), UR, buff=0.15)

        self.play(
            FadeOut(formula_group),
            FadeOut(arrow_w),
            FadeOut(label_w),
            FadeOut(rot_arc),
            FadeOut(arrow_v),
            FadeOut(label_v),
            ShowCreation(eigen_line),
            FadeIn(eigen_box, UP * 0.2),
            ShowCreation(arrow_eigen_v),
            FadeIn(label_eigen_v),
            run_time=1.3
        )
        self.wait(0.8)

        # Scale along the same line (no rotation!)
        self.play(
            TransformFromCopy(arrow_eigen_v, arrow_scaled_v),
            FadeIn(label_scaled_v, UP * 0.1),
            run_time=1.2
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 4 (T=14.5s – 21.0s): The Spectrum & The Prism Analogy
        # "The entire set of these eigenvalues is called the spectrum...
        #  Just like an optical prism splits light into wavelengths,
        #  the eigenvalues split the chessboard into spatial frequencies."
        # -------------------------------------------------------------
        self.play(
            FadeOut(axes),
            FadeOut(eigen_line),
            FadeOut(arrow_eigen_v),
            FadeOut(label_eigen_v),
            FadeOut(arrow_scaled_v),
            FadeOut(label_scaled_v),
            eigen_box.animate.to_edge(UP, buff=1.3).shift(LEFT * 0.2),
            run_time=1.0
        )

        # Triangular Prism in Center Left
        prism = Polygon(
            np.array([-4.2, -1.2, 0]),
            np.array([-2.2, -1.2, 0]),
            np.array([-3.2, 0.8, 0]),
            color=GRID_HAIRLINE,
            stroke_width=1.5,
            fill_color="#182030",
            fill_opacity=0.6
        )

        prism_label = Text("GRAPH LAPLACIAN", font="Consolas", color=TEXT_MUTED).scale(0.24)
        prism_label.next_to(prism, DOWN, buff=0.2)

        incoming_beam = Line(np.array([-6.8, -0.2, 0]), np.array([-3.5, -0.2, 0]), color=TEXT_BONE, stroke_width=3.5)
        beam_label = Text("Messy Tactical Geometry", font="Segoe UI", color=TEXT_BONE).scale(0.30)
        beam_label.next_to(incoming_beam, UP, buff=0.15)

        self.play(
            ShowCreation(prism),
            FadeIn(prism_label),
            ShowCreation(incoming_beam),
            FadeIn(beam_label, UP * 0.1),
            run_time=1.1
        )
        self.wait(0.5)

        ray_targets = [
            (np.array([1.2, 1.0, 0]), ACCENT_CYAN, "λ₁ = 0.00  (Whole Board Baseline)"),
            (np.array([1.2, 0.3, 0]), ACCENT_COBALT, "λ₂ = 0.42  (Fiedler Algebraic Connectivity)"),
            (np.array([1.2, -0.4, 0]), "#60a5fa", "λ₃ = 1.18  (Flank Sub-modes)"),
            (np.array([1.2, -1.1, 0]), "#93c5fd", "λ₄ = 2.45  (Local Piece Tension)"),
            (np.array([1.2, -1.8, 0]), ACCENT_CRITICAL, "λ_N = 8.60 (Maximum Attack Shock)")
        ]

        dispersed_rays = VGroup()
        spectral_bars = VGroup()

        for end_pt, color_val, text_lbl in ray_targets:
            ray = Line(np.array([-2.9, -0.2, 0]), end_pt, color=color_val, stroke_width=2.0)
            dispersed_rays.add(ray)

            lbl = Text(text_lbl, font="Consolas", color=color_val if "λ₂" in text_lbl else TEXT_BONE).scale(0.26)
            lbl.next_to(end_pt, RIGHT, buff=0.25)
            spectral_bars.add(lbl)

        fiedler_highlight = SurroundingRectangle(spectral_bars[1], color=ACCENT_COBALT, buff=0.08, stroke_width=1.2)

        self.play(
            LaggedStart(*[ShowCreation(r) for r in dispersed_rays], lag_ratio=0.1),
            LaggedStart(*[FadeIn(b, RIGHT) for b in spectral_bars], lag_ratio=0.1),
            run_time=1.5
        )
        self.play(
            ShowCreation(fiedler_highlight),
            run_time=0.8
        )
        self.wait(3.0)
