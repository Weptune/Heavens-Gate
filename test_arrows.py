"""
Test Chess Tactical Arrows:
Testing broadcast-grade arrows vs network arcs.
Comparing Lichess/Chess.com style translucent ribbon arrows with clean tips.
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

sys.path.append(str(Path("animations")))
from chessboard_widget import BroadcastChessBoard

class TestArrowsVisual(Scene):
    def construct(self):
        bg = Rectangle(width=16, height=9, fill_color="#080c14", fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        board = BroadcastChessBoard(center=LEFT * 3.4 + DOWN * 0.1, sq_size=0.62)
        self.add(board)

        pieces_data = [
            ("wR", 3, 0), ("wQ", 3, 3), ("wN", 5, 2), ("wP", 4, 3), ("wK", 6, 0),
            ("bB", 2, 4), ("bR", 4, 7), ("bK", 6, 7)
        ]

        pieces_group = VGroup()
        coords_map = {}

        for p_name, col, row in pieces_data:
            p_mob = board.create_piece(p_name, col, row)
            pieces_group.add(p_mob)
            coords_map[p_name] = board.get_square_pos(col, row)

        # Tactical Connections
        # 1. White Rook (d1) defends White Queen (d4) -> Cyan Support Arrow
        # 2. White Knight (f3) guards White Pawn (e4) -> Cyan Curved Guard Arrow
        # 3. White Queen (d4) attacks Black Bishop (c5) -> Rose/Red Attack Arrow
        # 4. Black Bishop (c5) pins towards White King (g1) -> Elegant dashed ray or slender tactical arrow
        # 5. Black Rook (e8) pressures White Pawn (e4) -> Red Pressure Arrow

        # Let's test custom broadcast tactical arrows:
        def make_chess_arrow(start_pt, end_pt, color, path_arc=0, opacity=0.78):
            arrow = Arrow(
                start_pt,
                end_pt,
                buff=0.28,                # Clean buffer so it never pierces piece icons
                path_arc=path_arc,
                fill_color=color,
                fill_opacity=opacity,
                stroke_color="#ffffff",
                stroke_width=0.8,         # Crisp white micro-border like Chess.com/Lichess
                stroke_opacity=0.6,
                thickness=2.2,            # Balanced shaft thickness
                tip_width_ratio=3.8,      # Elegant aerodynamic tip
                max_tip_length_to_length_ratio=0.35,
            )
            return arrow

        # 1. Mutual support (Rook to Queen on d-file)
        arr_rq = make_chess_arrow(coords_map["wR"], coords_map["wQ"], "#0284c7")
        
        # 2. Knight defends pawn (f3 to e4 curved)
        arr_np = make_chess_arrow(coords_map["wN"], coords_map["wP"], "#0284c7", path_arc=-TAU/14)
        
        # 3. Queen attacks Bishop (d4 to c5)
        arr_qb = make_chess_arrow(coords_map["wQ"], coords_map["bB"], "#e11d48")
        
        # 4. Bishop pin toward King (c5 to g1)
        arr_bk = make_chess_arrow(coords_map["bB"], coords_map["wK"], "#f43f5e", path_arc=-TAU/18)
        
        # 5. Rook attacks pawn (e8 to e4 down e-file)
        arr_rp = make_chess_arrow(coords_map["bR"], coords_map["wP"], "#e11d48")

        tactical_arrows = VGroup(arr_rq, arr_np, arr_qb, arr_bk, arr_rp)

        # Subtle glowing tile highlights under key pieces instead of ugly circles over heads!
        tile_highlights = VGroup()
        for p_key, col_rgb in [("wQ", "#38bdf8"), ("bB", "#f43f5e"), ("wP", "#38bdf8")]:
            pos = coords_map[p_key]
            tile = Square(side_length=0.62).move_to(pos)
            tile.set_fill(col_rgb, opacity=0.32)
            tile.set_stroke(color=col_rgb, width=1.5, opacity=0.7)
            tile_highlights.add(tile)

        # Add in proper visual depth order:
        # 1. Board
        # 2. Tile highlights
        # 3. Tactical arrows (behind pieces or nicely buffered)
        # 4. Pieces on top!
        self.add(tile_highlights, tactical_arrows, pieces_group)
        self.wait(0.1)
