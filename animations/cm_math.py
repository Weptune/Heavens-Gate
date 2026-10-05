"""
Computer Modern Vector Math & Technical HeatMatrix Engine for ManimGL
Delivers authentic 3Blue1Brown / Computer Modern LaTeX math typography
and dynamic technical matrices without requiring an external pdflatex binary.
"""

import io
import os
import re
import tempfile
import numpy as np
import matplotlib
matplotlib.rcParams['mathtext.fontset'] = 'cm'
import matplotlib.pyplot as plt
from manimlib import *


class CMTex(SVGMobject):
    """
    Renders authentic Computer Modern LaTeX mathematics as vector bezier curves
    directly via Matplotlib's embedded TeX rendering engine.
    Zero external pdflatex / xelatex installation required.
    """
    _cache = {}

    def __init__(self, tex_str, color=WHITE, fontsize=32, height=0.42, **kwargs):
        clean_key = (tex_str, fontsize)
        if clean_key in CMTex._cache:
            clean_svg = CMTex._cache[clean_key]
        else:
            fig = plt.figure(figsize=(6, 1.4))
            fig.text(
                0.5, 0.5,
                f"${tex_str}$" if not tex_str.startswith("$") else tex_str,
                fontsize=fontsize,
                ha="center",
                va="center",
                color="white"
            )
            buf = io.BytesIO()
            plt.savefig(buf, format="svg", bbox_inches="tight", pad_inches=0.04, transparent=True)
            plt.close(fig)
            svg_text = buf.getvalue().decode("utf-8")

            # Sanitize paths with no d attribute (e.g. empty space glyphs) so svgelements parses without error
            lines = []
            for line in svg_text.splitlines():
                if "<path " in line and " d=" not in line:
                    line = line.replace("<path ", '<path d="" ')
                lines.append(line)
            clean_svg = "\n".join(lines)
            CMTex._cache[clean_key] = clean_svg

        tmp = tempfile.NamedTemporaryFile(suffix=".svg", delete=False, mode="w", encoding="utf-8")
        tmp.write(clean_svg)
        tmp.close()

        super().__init__(tmp.name, **kwargs)
        try:
            os.unlink(tmp.name)
        except OSError:
            pass

        self.set_color(color)
        if height is not None:
            self.set_height(height)


class HeatMatrix(VGroup):
    """
    High-craft technical matrix with glowing backlit cells, formatted numeric values,
    beveled bracket calipers, and scanning row/column highlight mechanisms.
    """
    def __init__(
        self,
        matrix_data,
        cell_size=(0.72, 0.60),
        h_buff=0.10,
        v_buff=0.10,
        font_size=20,
        bracket_color="#8c4dfb",
        bracket_width=2.5,
        default_font="Consolas",
        **kwargs
    ):
        super().__init__(**kwargs)
        self.matrix_data = np.array(matrix_data)
        rows, cols = self.matrix_data.shape
        self.rows_num = rows
        self.cols_num = cols

        self.cell_width, self.cell_height = cell_size
        self.cell_rects = []
        self.cell_texts = []
        self.row_groups = VGroup()

        for r in range(rows):
            row_rects = []
            row_texts = []
            row_mobs = VGroup()
            for c in range(cols):
                val = self.matrix_data[r, c]

                # Backlit subtle cell frame
                cell_bg = RoundedRectangle(
                    width=self.cell_width,
                    height=self.cell_height,
                    corner_radius=0.04,
                    fill_color="#0d1424",
                    fill_opacity=0.30,
                    stroke_color="#1c273c",
                    stroke_width=0.8
                )

                # Format text
                if isinstance(val, (int, np.integer)):
                    val_str = str(val)
                elif isinstance(val, (float, np.floating)):
                    val_str = f"{val:+.1f}" if abs(val) > 0.001 else " 0.0"
                else:
                    val_str = str(val)

                txt = Text(val_str, font=default_font, font_size=font_size, color="#e8edf5")
                txt.move_to(cell_bg.get_center())

                cell_unit = VGroup(cell_bg, txt)
                row_mobs.add(cell_unit)
                row_rects.append(cell_bg)
                row_texts.append(txt)

            row_mobs.arrange(RIGHT, buff=h_buff)
            self.row_groups.add(row_mobs)
            self.cell_rects.append(row_rects)
            self.cell_texts.append(row_texts)

        self.row_groups.arrange(DOWN, buff=v_buff)
        self.add(self.row_groups)

        # Technical caliper brackets
        caliper_arm = 0.18
        tl = self.row_groups.get_corner(UL) + LEFT * 0.12 + UP * 0.08
        dl = self.row_groups.get_corner(DL) + LEFT * 0.12 + DOWN * 0.08
        tr = self.row_groups.get_corner(UR) + RIGHT * 0.12 + UP * 0.08
        dr = self.row_groups.get_corner(DR) + RIGHT * 0.12 + DOWN * 0.08

        self.left_bracket = VGroup(
            Line(tl + RIGHT * caliper_arm, tl, stroke_color=bracket_color, stroke_width=bracket_width),
            Line(tl, dl, stroke_color=bracket_color, stroke_width=bracket_width),
            Line(dl, dl + RIGHT * caliper_arm, stroke_color=bracket_color, stroke_width=bracket_width)
        )
        self.right_bracket = VGroup(
            Line(tr + LEFT * caliper_arm, tr, stroke_color=bracket_color, stroke_width=bracket_width),
            Line(tr, dr, stroke_color=bracket_color, stroke_width=bracket_width),
            Line(dr, dr + LEFT * caliper_arm, stroke_color=bracket_color, stroke_width=bracket_width)
        )
        self.add(self.left_bracket, self.right_bracket)

    def get_cell_bg(self, r, c):
        return self.cell_rects[r][c]

    def get_cell_text(self, r, c):
        return self.cell_texts[r][c]

    def get_row_rects(self, r):
        return VGroup(*self.cell_rects[r])

    def highlight_cell(self, r, c, fill_color="#08B6CE", fill_opacity=0.55, text_color="#ffffff", stroke_color="#08B6CE"):
        """Returns animation to highlight a specific cell."""
        bg = self.get_cell_bg(r, c)
        txt = self.get_cell_text(r, c)
        return AnimationGroup(
            bg.animate.set_fill(fill_color, fill_opacity).set_stroke(stroke_color, 2.0),
            txt.animate.set_color(text_color)
        )

    def unhighlight_cell(self, r, c, fill_color="#0d1424", fill_opacity=0.30, text_color="#e8edf5", stroke_color="#1c273c"):
        bg = self.get_cell_bg(r, c)
        txt = self.get_cell_text(r, c)
        return AnimationGroup(
            bg.animate.set_fill(fill_color, fill_opacity).set_stroke(stroke_color, 0.8),
            txt.animate.set_color(text_color)
        )
