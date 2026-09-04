"""
Heaven's Gate - 3Blue1Brown Style Animation Script
Built using ManimGL / Manim

To render this scene:
    manimgl chess_scenes.py BitboardToBinaryScene
    (or for high-quality MP4 export):
    manimgl chess_scenes.py BitboardToBinaryScene -w
"""

from manimlib import *

class BitboardToBinaryScene(Scene):
    def construct(self):
        # 1. 3Blue1Brown Title
        title = Text("Heaven's Gate: Bitboard Architecture", font="Cinzel", color=YELLOW)
        subtitle = Text("How a 64-Square Board Becomes a Single 64-Bit Integer", font="Inter", color=GREY_A)
        subtitle.scale(0.5).next_to(title, DOWN)
        
        self.play(FadeIn(title, UP), FadeIn(subtitle, UP))
        self.wait(1.5)
        self.play(FadeOut(title), FadeOut(subtitle))

        # 2. Draw 8x8 Chessboard Grid
        board_grid = VGroup()
        sq_size = 0.5
        for r in range(8):
            for c in range(8):
                sq = Square(side_length=sq_size)
                sq.move_to(np.array([(c - 3.5) * sq_size, (r - 3.5) * sq_size, 0]))
                if (r + c) % 2 == 0:
                    sq.set_fill(GREY_D, opacity=0.8)
                else:
                    sq.set_fill(GREY_B, opacity=0.8)
                sq.set_stroke(GREY_E, width=0.5)
                board_grid.add(sq)
        
        board_grid.to_edge(LEFT, buff=1.0)
        board_label = Text("Occupancy Bitboard (8x8)", color=WHITE).scale(0.5).next_to(board_grid, UP)

        self.play(ShowCreation(board_grid), Write(board_label))
        self.wait(1)

        # 3. Highlight Rank 1 (Pawns) or Pieces with Golden Glow
        highlight_squares = VGroup()
        for idx in range(8, 16): # Rank 2 White Pawns
            sq = board_grid[idx].copy()
            sq.set_fill(YELLOW, opacity=0.7)
            sq.set_stroke(YELLOW_A, width=2)
            highlight_squares.add(sq)

        self.play(FadeIn(highlight_squares))
        self.wait(1)

        # 4. Transform into 64-Bit Binary Stream
        binary_title = Text("64-Bit Unsigned Integer (uint64_t)", color=BLUE_B).scale(0.55)
        binary_title.to_edge(RIGHT, buff=1.5).shift(UP * 2)

        bits_text = Text("0x000000000000FF00", font="Consolas", color=YELLOW).scale(0.7)
        bits_text.next_to(binary_title, DOWN, buff=0.4)

        binary_desc = Text(
            "Bit 8 to Bit 15 are set to 1.\nEvery chess operation (attacks, moves, pins)\nis a single 1-cycle CPU bitwise instruction.",
            font="Inter", color=GREY_A
        ).scale(0.45).next_to(bits_text, DOWN, buff=0.5)

        self.play(Write(binary_title), TransformFromCopy(highlight_squares, bits_text))
        self.play(FadeIn(binary_desc, UP))
        self.wait(2)

        # 5. Bitwise Operation Showcase (AND, NOT, SHIFT)
        formula = Text(
            "Pawn Attacks = (Pawns << 7) & ~FileH",
            font="Consolas", color=WHITE
        ).scale(0.6).next_to(binary_desc, DOWN, buff=0.6)

        self.play(Write(formula))
        self.wait(2.5)

        self.play(FadeOut(Group(board_grid, board_label, highlight_squares, binary_title, bits_text, binary_desc, formula)))


class AlphaBetaPruningScene(Scene):
    def construct(self):
        # 3B1B Minimax Tree Pruning Visualization
        title = Text("Alpha-Beta Pruning & The 99.3% Reduction", font="Cinzel", color=YELLOW).scale(0.8)
        self.play(FadeIn(title, UP))
        self.wait(1)
        self.play(title.animate.to_edge(UP))

        # Root node
        root = Dot(point=UP * 1.5, color=WHITE, radius=0.15)
        root_label = Text("Root (d=0)", color=GREY_B).scale(0.4).next_to(root, UP)

        # Child nodes (depth 1)
        c1 = Dot(point=LEFT * 3 + UP * 0.2, color=BLUE, radius=0.12)
        c2 = Dot(point=ORIGIN + UP * 0.2, color=GREEN, radius=0.14)
        c3 = Dot(point=RIGHT * 3 + UP * 0.2, color=RED, radius=0.12)

        e1 = Line(root.get_center(), c1.get_center(), color=GREY)
        e2 = Line(root.get_center(), c2.get_center(), color=GREEN)
        e3 = Line(root.get_center(), c3.get_center(), color=GREY)

        self.play(FadeIn(root), Write(root_label))
        self.play(ShowCreation(e1), ShowCreation(e2), ShowCreation(e3), FadeIn(c1), FadeIn(c2), FadeIn(c3))
        self.wait(1)

        # Best Move Cutoff (Beta Cutoff)
        beta_label = Text("Best Move Searched First (TT Move)", color=GREEN).scale(0.45).next_to(c2, DOWN, buff=0.4)
        cutoff_cross = Text("β-Cutoff (Pruned!)", color=RED, font="Inter").scale(0.6).next_to(c3, RIGHT, buff=0.3)
        cut_line = Line(c3.get_center() + LEFT*0.5 + UP*0.5, c3.get_center() + RIGHT*1.5 + DOWN*0.8, color=RED, stroke_width=4)

        self.play(Write(beta_label), c2.animate.scale(1.2))
        self.wait(1)
        self.play(ShowCreation(cut_line), Write(cutoff_cross))
        self.wait(2)

        # Conclusion Stat
        stat = Text("1,000,000 Nodes  ──▶  7,000 Nodes Searched", font="Fira Code", color=YELLOW).scale(0.7)
        stat.to_edge(DOWN, buff=1.0)
        self.play(FadeIn(stat, UP))
        self.wait(2)
