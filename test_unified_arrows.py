"""
Refined Broadcast Tactical Arrows Test
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path("animations")))
from chessboard_widget import BroadcastChessBoard

class TestUnifiedArrows(Scene):
    def construct(self):
        bg = Rectangle(width=16, height=9, fill_color="#080c14", fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.1, sq_size=0.62)
        self.add(board)

        p_data = [
            ("wR", 3, 0), ("wQ", 3, 3), ("wN", 5, 2), ("wP", 4, 3), ("wK", 6, 0),
            ("bB", 2, 4), ("bR", 4, 7), ("bK", 6, 7)
        ]
        pieces = VGroup()
        for name, col, row in p_data:
            pieces.add(board.create_piece(name, col, row))

        def create_broadcast_arrow(start_pt, end_pt, color, path_arc=0, opacity=0.76):
            if path_arc == 0:
                v = end_pt - start_pt
                dist = np.linalg.norm(v)
                if dist == 0:
                    return VMobject()
                u = v / dist
                n = np.array([-u[1], u[0], 0])

                width = 0.13
                head_len = 0.22
                head_w = 0.30
                buff_start = 0.24
                buff_end = 0.24

                p_start = start_pt + u * buff_start
                p_end = end_pt - u * buff_end
                
                eff_len = np.linalg.norm(p_end - p_start)
                if eff_len <= head_len:
                    head_len = eff_len * 0.6

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
                poly.set_stroke(color="#ffffff", width=0.6, opacity=0.4)
                return poly
            else:
                arr = Arrow(
                    start_pt, end_pt,
                    path_arc=path_arc,
                    buff=0.25,
                    fill_color=color,
                    fill_opacity=opacity,
                    stroke_color="#ffffff",
                    stroke_width=0.6,
                    stroke_opacity=0.4,
                    thickness=4.8,
                    tip_width_ratio=2.4,
                    max_tip_length_to_length_ratio=0.30
                )
                return arr

        # Tactical Arrows matching the Scene 02 connections:
        # 1. White Rook on d1 defending White Queen on d4 (battery along d-file)
        arr_rq = create_broadcast_arrow(board.get_square_pos(3, 0), board.get_square_pos(3, 3), "#0ea5e9")

        # 2. White Knight on f3 defending White Pawn on e4 (curved knight guard)
        arr_np = create_broadcast_arrow(board.get_square_pos(5, 2), board.get_square_pos(4, 3), "#0ea5e9", path_arc=-TAU/12)

        # 3. White Queen on d4 attacking Black Bishop on c5
        arr_qb = create_broadcast_arrow(board.get_square_pos(3, 3), board.get_square_pos(2, 4), "#f43f5e")

        # 4. Black Bishop on c5 counter-attacking White Queen on d4 (curved return attack arc)
        arr_bq = create_broadcast_arrow(board.get_square_pos(2, 4), board.get_square_pos(3, 3), "#fb7185", path_arc=TAU/10)

        # 5. Black Rook on e8 attacking White Pawn on e4
        arr_rp = create_broadcast_arrow(board.get_square_pos(4, 7), board.get_square_pos(4, 3), "#f43f5e")

        # 6. Bishop on c5 diagonal pin pressure toward King on g1 (sleek dashed laser ray)
        v_pin = board.get_square_pos(6, 0) - board.get_square_pos(2, 4)
        u_pin = v_pin / np.linalg.norm(v_pin)
        pin_start = board.get_square_pos(2, 4) + u_pin * 0.32
        pin_end = board.get_square_pos(6, 0) - u_pin * 0.32
        pin_ray = DashedLine(pin_start, pin_end, color="#f43f5e", stroke_width=2.0, stroke_opacity=0.55, dash_length=0.10)

        arrows = VGroup(arr_rq, arr_np, arr_qb, arr_bq, arr_rp, pin_ray)

        self.add(arrows, pieces)
        self.wait(0.1)
