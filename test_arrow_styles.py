"""
Testing different broadcast arrow styles in ManimGL
Style 1: Chess.com Broadcast Ribbon Arrow (wide flat shaft, clean triangle head)
Style 2: Lichess Translucent Neon Glow Arrow
Style 3: Elegant Curved Arcs with Soft Terminal Dots (Network Graph style)
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path("animations")))
from chessboard_widget import BroadcastChessBoard

class TestArrowStyles(Scene):
    def construct(self):
        bg = Rectangle(width=16, height=9, fill_color="#080c14", fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.1, sq_size=0.62)
        self.add(board)

        # Let's place a couple pieces to test arrows connecting them
        p_data = [
            ("wR", 3, 0), ("wQ", 3, 3), ("wN", 5, 2), ("wP", 4, 3),
            ("bB", 2, 4), ("bR", 4, 7)
        ]
        pieces = VGroup()
        for name, col, row in p_data:
            pieces.add(board.create_piece(name, col, row))

        # Function to generate authentic Chess.com style broadcast ribbon arrow
        def create_broadcast_arrow(start_pt, end_pt, color="#0ea5e9", opacity=0.75, width=0.14, head_len=0.24, head_w=0.34, buff_start=0.22, buff_end=0.22):
            """
            Builds a flat 2D polygon ribbon arrow exactly like Chess.com / Lichess:
            Wide rectangular shaft + triangular arrowhead with clean rounded geometry.
            """
            v = end_pt - start_pt
            dist = np.linalg.norm(v)
            if dist == 0:
                return VMobject()
            u = v / dist
            # Normal perpendicular vector
            n = np.array([-u[1], u[0], 0])

            # Effective start and end with buffers
            p_start = start_pt + u * buff_start
            p_end = end_pt - u * buff_end
            
            eff_len = np.linalg.norm(p_end - p_start)
            if eff_len <= head_len:
                head_len = eff_len * 0.6

            shaft_end = p_end - u * head_len
            
            # Vertices of the ribbon arrow:
            # 1. Shaft bottom left
            # 2. Shaft top left (at arrowhead base)
            # 3. Arrowhead left wing
            # 4. Arrowhead tip (p_end)
            # 5. Arrowhead right wing
            # 6. Shaft top right
            # 7. Shaft bottom right
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
            poly.set_stroke(color="#ffffff", width=0.8, opacity=0.5)
            return poly

        # Function to generate authentic Knight L-shape arrow
        def create_knight_arrow(start_pt, end_pt, color="#0ea5e9", opacity=0.75, width=0.14, head_len=0.24, head_w=0.34):
            # For knight move (e.g. from f3 to e4: dx=-1, dy=+1 or 2 then 1)
            # Two segments: corner point then to destination
            dx = end_pt[0] - start_pt[0]
            dy = end_pt[1] - start_pt[1]
            # Turn at corner: (start_x, end_y) or (end_x, start_y)
            corner = np.array([end_pt[0], start_pt[1], 0])
            
            # Or an elegant curved ribbon arrow!
            curve = Arrow(
                start_pt, end_pt,
                path_arc=-TAU/12,
                buff=0.25,
                fill_color=color,
                fill_opacity=opacity,
                stroke_color="#ffffff",
                stroke_width=0.8,
                stroke_opacity=0.5,
                thickness=4.5,
                tip_width_ratio=2.6,
                max_tip_length_to_length_ratio=0.32
            )
            return curve

        # Let's test ribbon arrows:
        # 1. White Rook on d1 defending Queen on d4 (Cyan ribbon arrow)
        arr_rq = create_broadcast_arrow(board.get_square_pos(3, 0), board.get_square_pos(3, 3), color="#0ea5e9")
        
        # 2. Black Rook on e8 attacking Pawn on e4 (Rose/Red ribbon arrow)
        arr_rp = create_broadcast_arrow(board.get_square_pos(4, 7), board.get_square_pos(4, 3), color="#f43f5e")

        # 3. Queen on d4 attacking Bishop on c5 (Rose ribbon arrow)
        arr_qb = create_broadcast_arrow(board.get_square_pos(3, 3), board.get_square_pos(2, 4), color="#f43f5e")

        # 4. Knight on f3 guarding Pawn on e4 (Curved Cyan arrow)
        arr_np = create_knight_arrow(board.get_square_pos(5, 2), board.get_square_pos(4, 3), color="#0ea5e9")

        arrows = VGroup(arr_rq, arr_rp, arr_qb, arr_np)

        self.add(arrows, pieces)
        self.wait(0.1)
