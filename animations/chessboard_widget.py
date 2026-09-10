"""
Heaven's Gate Animation System - Broadcast ChessBoard Component
Provides a high-contrast, broadcast-grade 8x8 chessboard with:
- Crisp alternating slate/tournament squares (Lichess/3B1B quality)
- Vector-enhanced piece styling (Black pieces perfectly visible with bright silver contour)
- Semi-transparent mathematical overlays (Fiedler vector clusters, heatmaps)
- Broadcast tactical ribbon arrows (Chess.com / Lichess standard, zero piece piercing)
- Architectural beveled frame and clean coordinates
"""

from pathlib import Path
import numpy as np
from manimlib import *

# Muted Editorial Tournament Palette
BOARD_LIGHT_SQ = "#c8d2de"  # Soft matte silver-slate
BOARD_DARK_SQ  = "#516075"  # Architectural muted slate-blue
BOARD_BORDER   = "#253142"  # Subtle tile boundary
FRAME_BG       = "#101622"  # Deep matte charcoal frame
FRAME_STROKE   = "#253142"  # Outer frame border
COORD_TEXT     = "#738499"  # Quiet notation text

# Fiedler Vector Accents (Muted Tones)
FIEDLER_POS_COLOR = "#6889b5"  # Muted Steel Blue (+0.74 Kingside)
FIEDLER_NEG_COLOR = "#c89b58"  # Warm Muted Ochre (-0.82 Queenside)
FIEDLER_CUT_COLOR = "#b55a58"  # Dusty Terracotta zero-crossing line

# Tactical Colors (Muted Tones)
TACTICAL_FRIENDLY = "#6889b5"  # Muted Steel Blue / Coordination
TACTICAL_HOSTILE  = "#b55a58"  # Dusty Terracotta / Tension & Attacks

class BroadcastChessBoard(VGroup):
    def __init__(
        self,
        center=LEFT * 3.4 + DOWN * 0.1,
        sq_size=0.62,
        light_color=BOARD_LIGHT_SQ,
        dark_color=BOARD_DARK_SQ,
        show_coords=True,
        **kwargs
    ):
        super().__init__(**kwargs)
        self.center_pt = center
        self.sq_size = sq_size
        self.light_color = light_color
        self.dark_color = dark_color
        self.squares = {}
        
        # 1. Squares
        self.squares_group = VGroup()
        for r in range(8):
            for c in range(8):
                sq = Square(side_length=sq_size)
                sq.move_to(self.center_pt + np.array([(c - 3.5) * sq_size, (r - 3.5) * sq_size, 0]))
                is_light = (r + c) % 2 != 0
                sq.set_fill(light_color if is_light else dark_color, opacity=1.0)
                sq.set_stroke(color=BOARD_BORDER, width=0.5)
                self.squares[(c, r)] = sq
                self.squares_group.add(sq)

        # 2. Architectural Frame
        self.inner_border = SurroundingRectangle(self.squares_group, color="#1e293b", buff=0.0, stroke_width=2.0)
        self.outer_frame = RoundedRectangle(
            width=8 * sq_size + 0.68,
            height=8 * sq_size + 0.68,
            corner_radius=0.14,
            fill_color=FRAME_BG,
            fill_opacity=1.0,
            stroke_color="#334155",
            stroke_width=2.5
        )
        self.outer_frame.move_to(self.center_pt)

        # 3. Coordinates
        self.coords_group = VGroup()
        if show_coords:
            files = "abcdefgh"
            for c in range(8):
                pos_x = self.center_pt[0] + (c - 3.5) * sq_size
                lbl_bot = Text(files[c], font="Consolas", color=COORD_TEXT).scale(0.24)
                lbl_bot.move_to(np.array([pos_x, self.center_pt[1] - 4.0 * sq_size - 0.17, 0]))
                self.coords_group.add(lbl_bot)
                
                lbl_top = Text(files[c], font="Consolas", color="#64748b").scale(0.22)
                lbl_top.move_to(np.array([pos_x, self.center_pt[1] + 4.0 * sq_size + 0.17, 0]))
                self.coords_group.add(lbl_top)

            for r in range(8):
                pos_y = self.center_pt[1] + (r - 3.5) * sq_size
                lbl_l = Text(str(r + 1), font="Consolas", color=COORD_TEXT).scale(0.24)
                lbl_l.move_to(np.array([self.center_pt[0] - 4.0 * sq_size - 0.17, pos_y, 0]))
                self.coords_group.add(lbl_l)
                
                lbl_r = Text(str(r + 1), font="Consolas", color="#64748b").scale(0.22)
                lbl_r.move_to(np.array([self.center_pt[0] + 4.0 * sq_size + 0.17, pos_y, 0]))
                self.coords_group.add(lbl_r)

        self.add(self.outer_frame, self.squares_group, self.inner_border, self.coords_group)

    def get_square_pos(self, col, row):
        return self.squares[(col, row)].get_center()

    def create_piece(self, piece_name, col, row):
        """
        Loads and styles SVG pieces so Black pieces have high-contrast contour
        and White pieces have crisp vector clarity.
        """
        p_path = Path(f"web/pieces/cburnett/{piece_name}.svg")
        if not p_path.exists():
            p_path = Path("c:/Users/abhin/heavensgate/web/pieces/cburnett") / f"{piece_name}.svg"

        p_svg = SVGMobject(str(p_path))
        
        # Style pieces for maximum contrast
        if piece_name.startswith("b"):
            for sub in p_svg.family_members_with_points():
                sub.set_stroke(color="#f8fafc", width=1.8)
        else:
            for sub in p_svg.family_members_with_points():
                sub.set_stroke(color="#0f172a", width=1.4)

        p_svg.set_height(self.sq_size * 0.72)
        p_svg.move_to(self.get_square_pos(col, row))
        return p_svg

    def create_tactical_arrow(
        self,
        start_pt,
        end_pt,
        color=TACTICAL_FRIENDLY,
        path_arc=0,
        opacity=0.80,
        buff=0.26
    ):
        """
        Creates a broadcast-quality tactical arrow (Chess.com / Lichess standard).
        - Clean polygon ribbon shaft with sleek triangular arrowhead.
        - Buffered endpoints so it never pierces piece icons.
        - Supports curved arcs for Knight maneuvers.
        """
        if path_arc == 0:
            v = end_pt - start_pt
            dist = np.linalg.norm(v)
            if dist == 0:
                return VMobject()
            u = v / dist
            n = np.array([-u[1], u[0], 0])

            width = 0.12
            head_len = 0.20
            head_w = 0.28

            p_start = start_pt + u * buff
            p_end = end_pt - u * buff
            
            eff_len = np.linalg.norm(p_end - p_start)
            if eff_len <= head_len:
                head_len = eff_len * 0.55

            shaft_end = p_end - u * head_len

            pts = [
                p_start - n * (width / 2),
                shaft_end - n * (width / 2),
                shaft_end - n * (head_w / 2),
                p_end,
                shaft_end + n * (head_w / 2),
                shaft_end + n * (width / 2),
                p_start + n * (width / 2)
            ]
            poly = Polygon(*pts)
            poly.set_fill(color, opacity=opacity)
            poly.set_stroke(color="#ffffff", width=0.6, opacity=0.45)
            return poly
        else:
            arr = Arrow(
                start_pt, end_pt,
                path_arc=path_arc,
                buff=buff,
                fill_color=color,
                fill_opacity=opacity,
                stroke_color="#ffffff",
                stroke_width=0.6,
                stroke_opacity=0.45,
                thickness=4.5,
                tip_width_ratio=2.4,
                max_tip_length_to_length_ratio=0.30
            )
            return arr

    def create_cluster_overlays(self, neg_cols=(0, 1, 2), pos_cols=(4, 5, 6, 7)):
        """
        Creates semi-transparent color washes for Fiedler clusters
        that preserve the underlying checkerboard texture.
        """
        overlays = VGroup()
        for r in range(8):
            for c in range(8):
                sq = self.squares[(c, r)]
                if c in neg_cols:
                    ov = Square(side_length=self.sq_size).move_to(sq.get_center())
                    ov.set_fill(FIEDLER_NEG_COLOR, opacity=0.22)
                    ov.set_stroke(width=0)
                    overlays.add(ov)
                elif c in pos_cols:
                    ov = Square(side_length=self.sq_size).move_to(sq.get_center())
                    ov.set_fill(FIEDLER_POS_COLOR, opacity=0.25)
                    ov.set_stroke(width=0)
                    overlays.add(ov)
        return overlays

    def create_fault_line(self, split_col=3.5):
        """
        Creates a sleek glowing zero-crossing boundary line along split_col.
        """
        x_pos = self.center_pt[0] + (split_col - 3.5) * self.sq_size
        top_pt = np.array([x_pos, self.center_pt[1] + 4.0 * self.sq_size, 0])
        bot_pt = np.array([x_pos, self.center_pt[1] - 4.0 * self.sq_size, 0])

        glow = Line(top_pt, bot_pt, color=FIEDLER_CUT_COLOR, stroke_width=6.0, stroke_opacity=0.35)
        dash = DashedLine(top_pt, bot_pt, color="#fb7185", stroke_width=2.5, dash_length=0.12)
        
        badge = RoundedRectangle(
            width=2.8, height=0.42, corner_radius=0.1,
            fill_color="#1e1124", fill_opacity=0.95,
            stroke_color=FIEDLER_CUT_COLOR, stroke_width=1.5
        )
        badge.move_to(top_pt + UP * 0.40)
        badge_txt = Text("FIEDLER CUT:  v₂ = 0", font="Consolas", color=FIEDLER_CUT_COLOR).scale(0.24)
        badge_txt.move_to(badge.get_center())
        
        badge_group = VGroup(badge, badge_txt)
        return VGroup(glow, dash, badge_group)
