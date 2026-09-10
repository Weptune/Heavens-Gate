"""
Prototype for Broadcast-Quality Chess Board Visuals
Testing:
1. High-contrast piece rendering (Black pieces visible on any background)
2. Elegant alternating checkerboard (Slate/Nordic and Classic Tournament)
3. Semi-transparent Fiedler vector highlight tiles (preserving checkerboard)
4. Sleek glowing zero-crossing fault line
5. Crisp piece scaling and positioning (no edge clipping)
"""

import sys
from pathlib import Path
import numpy as np
from manimlib import *

class TestBoardVisual(Scene):
    def construct(self):
        bg = Rectangle(width=16, height=9, fill_color="#080c14", fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # Board specifications
        sq_size = 0.62
        b_center = LEFT * 3.4 + DOWN * 0.1
        
        # Color Palette - Elegant Tournament Slate/Ice
        # Light square: Warm ivory-ice
        # Dark square: Deep slate-blue
        LIGHT_SQ = "#cbd5e1"  # Crisp light slate
        DARK_SQ  = "#475569"  # Rich dark slate
        
        board_group = VGroup()
        squares_dict = {}
        
        for r in range(8):
            for c in range(8):
                sq = Square(side_length=sq_size)
                sq.move_to(b_center + np.array([(c - 3.5) * sq_size, (r - 3.5) * sq_size, 0]))
                is_light = (r + c) % 2 != 0
                sq.set_fill(LIGHT_SQ if is_light else DARK_SQ, opacity=1.0)
                sq.set_stroke(color="#334155", width=0.5)
                board_group.add(sq)
                squares_dict[(c, r)] = sq

        # Architectural Bevel Frame
        inner_border = SurroundingRectangle(board_group, color="#1e293b", buff=0.0, stroke_width=2.0)
        outer_frame = RoundedRectangle(
            width=8 * sq_size + 0.70,
            height=8 * sq_size + 0.70,
            corner_radius=0.15,
            fill_color="#0f172a",
            fill_opacity=1.0,
            stroke_color="#334155",
            stroke_width=2.5
        )
        outer_frame.move_to(b_center)
        
        # Coordinates (a-h, 1-8)
        files = "abcdefgh"
        coords = VGroup()
        for c in range(8):
            lbl = Text(files[c], font="Consolas", color="#94a3b8").scale(0.24)
            lbl.move_to(np.array([b_center[0] + (c - 3.5) * sq_size, b_center[1] - 4.0 * sq_size - 0.18, 0]))
            coords.add(lbl)
            lbl_top = Text(files[c], font="Consolas", color="#64748b").scale(0.22)
            lbl_top.move_to(np.array([b_center[0] + (c - 3.5) * sq_size, b_center[1] + 4.0 * sq_size + 0.18, 0]))
            coords.add(lbl_top)

        for r in range(8):
            lbl = Text(str(r + 1), font="Consolas", color="#94a3b8").scale(0.24)
            lbl.move_to(np.array([b_center[0] - 4.0 * sq_size - 0.18, b_center[1] + (r - 3.5) * sq_size, 0]))
            coords.add(lbl)
            lbl_r = Text(str(r + 1), font="Consolas", color="#64748b").scale(0.22)
            lbl_r.move_to(np.array([b_center[0] + 4.0 * sq_size + 0.18, b_center[1] + (r - 3.5) * sq_size, 0]))
            coords.add(lbl_r)

        self.add(outer_frame, board_group, inner_border, coords)

        # Fiedler Vector Tint Overlays (SEMI-TRANSPARENT, checkerboard stays visible!)
        fiedler_overlays = VGroup()
        for r in range(8):
            for c in range(8):
                sq_orig = squares_dict[(c, r)]
                if c <= 2: # Queenside negative cluster (-0.8)
                    ov = Square(side_length=sq_size).move_to(sq_orig.get_center())
                    ov.set_fill("#f59e0b", opacity=0.28) # Amber gold
                    ov.set_stroke(width=0)
                    fiedler_overlays.add(ov)
                elif c >= 4: # Kingside positive cluster (+0.74)
                    ov = Square(side_length=sq_size).move_to(sq_orig.get_center())
                    ov.set_fill("#0284c7", opacity=0.32) # Electric Cyan / Azure
                    ov.set_stroke(width=0)
                    fiedler_overlays.add(ov)
        self.add(fiedler_overlays)

        # Pieces placement
        demo_pieces = [
            ("wP", 3, 3), ("wP", 4, 4), ("bP", 3, 4), ("bP", 4, 5),
            ("wQ", 6, 3), ("wR", 5, 0), ("wN", 5, 2), ("wK", 6, 0),
            ("bK", 6, 7),
            ("bR", 0, 7), ("bN", 1, 7)
        ]

        pieces_group = VGroup()
        for p_name, col, row in demo_pieces:
            sq = squares_dict[(col, row)]
            p_path = Path(f"web/pieces/cburnett/{p_name}.svg")
            p_svg = SVGMobject(str(p_path))
            
            # Black pieces contrast fix:
            # If black piece, ensure strokes and paths have high-contrast styling!
            if p_name.startswith("b"):
                # Style black pieces with crisp white outline and deep charcoal fill
                for sub in p_svg.family_members_with_points():
                    sub.set_stroke(color="#f8fafc", width=1.8)
            else:
                for sub in p_svg.family_members_with_points():
                    sub.set_stroke(color="#0f172a", width=1.5)

            p_svg.set_height(sq_size * 0.72)
            p_svg.move_to(sq.get_center())
            pieces_group.add(p_svg)

        self.add(pieces_group)

        # Sleek Glowing Zero-Crossing Fault Line
        top_pt = b_center + np.array([(4.0 - 3.5) * sq_size - sq_size/2, 4.0 * sq_size, 0])
        bot_pt = b_center + np.array([(4.0 - 3.5) * sq_size - sq_size/2, -4.0 * sq_size, 0])
        
        fault_glow = Line(top_pt, bot_pt, color="#f43f5e", stroke_width=6.0, stroke_opacity=0.35)
        fault_line = DashedLine(top_pt, bot_pt, color="#fb7185", stroke_width=2.5, dash_length=0.12)
        
        badge = RoundedRectangle(width=2.8, height=0.42, corner_radius=0.1, fill_color="#1e1124", fill_opacity=0.9, stroke_color="#f43f5e", stroke_width=1.5)
        badge.next_to(top_pt, UP, buff=0.15)
        badge_txt = Text("FIEDLER CUT:  v₂ = 0", font="Consolas", color="#f43f5e").scale(0.24)
        badge_txt.move_to(badge.get_center())
        fault_badge = VGroup(badge, badge_txt)

        # Telemetry panel on the right
        panel_title = Text("DEPTH 0 TOPOLOGICAL DIAGNOSIS", font="Consolas", color="#f8fafc").scale(0.35)
        panel_sub = Text("M. Fiedler (1973) Eigenvector Bisection", font="Consolas", color="#64748b").scale(0.24)
        panel_sub.next_to(panel_title, DOWN, aligned_edge=LEFT, buff=0.08)

        findings = [
            ("TOPOLOGY", "Board severed into 2 independent subgraphs"),
            ("WHITE KINGSIDE", "v₂ = +0.74 (Coordinated Attack Fist)"),
            ("BLACK DEFENDER (a8)", "v₂ = −0.82 (Stranded • 0 Communication)"),
            ("TACTICAL RESULT", "Black Rook cannot cross bottleneck in time"),
            ("SEARCH COST", "0 Moves Searched • Evaluated in 1 Eigensolve")
        ]

        card_group = VGroup()
        for head, body in findings:
            h = Text(head, font="Consolas", color="#38bdf8" if "RESULT" not in head else "#f43f5e").scale(0.24)
            b = Text(body, font="Segoe UI", color="#f8fafc").scale(0.30)
            b.next_to(h, DOWN, aligned_edge=LEFT, buff=0.06)
            row = VGroup(h, b)
            card_group.add(row)

        card_group.arrange(DOWN, aligned_edge=LEFT, buff=0.28)
        telemetry_panel = VGroup(panel_title, panel_sub, card_group)
        card_group.next_to(panel_sub, DOWN, aligned_edge=LEFT, buff=0.35)
        telemetry_panel.move_to(RIGHT * 3.0 + DOWN * 0.1)

        self.add(fault_glow, fault_line, fault_badge, telemetry_panel)
        self.wait(0.1)
