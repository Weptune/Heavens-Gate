import sys
from pathlib import Path
from manimlib import *
import numpy as np
import random

sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard
from theme import *

class Scene06ComplexityTrap(Scene):
    def construct(self):
        # Setup colors
        COLOR_BG = "#080c14"
        COLOR_GOLD = "#fbbf24"
        COLOR_CYAN = "#0ea5e9"
        COLOR_RED = "#ef4444"
        COLOR_GREEN = "#10b981"
        COLOR_VIOLET = "#a78bfa"
        COLOR_TEXT_BRIGHT = "#f8fafc"
        COLOR_TEXT_MUTED = "#94a3b8"

        self.camera.background_color = COLOR_BG

        # BEAT 1: The Iron Law (0-12s)
        title = Text("The Iron Law of Chess Programming", font="Bahnschrift", font_size=48, color=COLOR_GOLD)
        title.to_edge(UP)
        self.play(Write(title))

        # Split screen
        vline = Line(UP * 3, DOWN * 3, color=COLOR_TEXT_MUTED, stroke_width=2)
        self.play(ShowCreation(vline))

        # Left: Stick figure (Fast, 15 PLY)
        left_group = VGroup()
        stick_figure = VGroup(
            Circle(radius=0.5, color=COLOR_CYAN).shift(UP*1.5),
            Line(UP*1, DOWN*1, color=COLOR_CYAN),
            Line(UP*0.5, LEFT*0.5 + UP*0, color=COLOR_CYAN),
            Line(UP*0.5, RIGHT*0.5 + UP*0, color=COLOR_CYAN),
            Line(DOWN*1, LEFT*0.5 + DOWN*2, color=COLOR_CYAN),
            Line(DOWN*1, RIGHT*0.5 + DOWN*2, color=COLOR_CYAN)
        ).scale(0.5).shift(LEFT * 4 + UP * 0.5)
        
        badge_left = Text("15 PLY", font="Consolas", font_size=36, color=COLOR_GREEN)
        badge_left.next_to(stick_figure, DOWN, buff=0.5)
        
        speed_lines = VGroup(*[
            Line(LEFT*0.2, RIGHT*0.2, color=COLOR_GREEN, stroke_width=2).shift(LEFT*5 + UP*(i*0.5 - 0.5))
            for i in range(3)
        ])

        left_group.add(stick_figure, badge_left, speed_lines)

        # Right: Galaxy brain (Slow, 5 PLY)
        right_group = VGroup()
        brain = VGroup(
            Ellipse(width=2, height=1.5, color=COLOR_VIOLET),
            *[
                Arc(arc_center=RIGHT*4 + UP*0.5, radius=0.2+i*0.1, start_angle=0, angle=PI, color=COLOR_VIOLET)
                for i in range(5)
            ]
        ).shift(RIGHT * 4 + UP * 0.5)
        
        badge_right = Text("5 PLY", font="Consolas", font_size=36, color=COLOR_RED)
        badge_right.next_to(brain, DOWN, buff=0.5)

        right_group.add(brain, badge_right)

        self.play(
            FadeIn(left_group, shift=RIGHT),
            FadeIn(right_group, shift=LEFT)
        )

        # Fast vs Slow animation
        self.play(
            speed_lines.animate.shift(RIGHT * 1).set_color(COLOR_GREEN),
            brain.animate.set_color(COLOR_RED).scale(1.1),
            run_time=2,
            rate_func=there_and_back
        )

        conclusion = Text("Search Depth beats evaluation subtlety every single time.", font="Bahnschrift", font_size=36, color=COLOR_TEXT_BRIGHT)
        conclusion.to_edge(DOWN)
        
        self.play(Write(conclusion))
        self.wait(2)

        self.play(
            FadeOut(left_group),
            FadeOut(right_group),
            FadeOut(vline),
            FadeOut(title),
            FadeOut(conclusion)
        )

        # BEAT 2: O(1) vs O(N³) (12-35s)
        # Left: Magic Bitboards
        bb_title = Text("Magic Bitboards ( O(1) )", font="Bahnschrift", font_size=36, color=COLOR_GREEN)
        bb_title.move_to(LEFT * 3.5 + UP * 3)
        
        binary_str = Text("01001010" * 3, font="Consolas", font_size=16, color=COLOR_CYAN)
        binary_str.move_to(LEFT * 3.5 + UP * 2)

        board = BroadcastChessBoard(center=np.array([-3.5, -0.5, 0]), sq_size=0.4, show_coords=False)
        rook = board.create_piece("wR", 3, 3)
        board.add(rook)

        rays = VGroup(
            Line(board.get_square_pos(3,3), board.get_square_pos(3,7), color=COLOR_GREEN),
            Line(board.get_square_pos(3,3), board.get_square_pos(3,0), color=COLOR_GREEN),
            Line(board.get_square_pos(3,3), board.get_square_pos(0,3), color=COLOR_GREEN),
            Line(board.get_square_pos(3,3), board.get_square_pos(7,3), color=COLOR_GREEN)
        )

        self.play(
            Write(bb_title),
            FadeIn(binary_str),
            FadeIn(board)
        )
        self.play(
            binary_str.animate.set_color(COLOR_GREEN).scale(1.2),
            ShowCreation(rays),
            run_time=1.5
        )

        # Right: O(N³) eigensolver
        eigen_title = Text("Graph Laplacian ( O(N³) )", font="Bahnschrift", font_size=36, color=COLOR_RED)
        eigen_title.move_to(RIGHT * 3.5 + UP * 3)

        matrix = VGroup(*[
            Square(side_length=0.4, stroke_color=COLOR_TEXT_MUTED).move_to(RIGHT * 3.5 + np.array([(i-1.5)*0.4, (j-1.5)*0.4, 0]))
            for i in range(4) for j in range(4)
        ])

        ops_text = Text("64³ = 262,144 ops/node", font="Consolas", font_size=24, color=COLOR_RED)
        ops_text.move_to(RIGHT * 3.5 + DOWN * 2)

        self.play(
            Write(eigen_title),
            ShowCreation(matrix)
        )

        matrix_glow = matrix.copy().set_color(COLOR_RED).set_opacity(0.5)
        self.play(
            matrix.animate.set_color(COLOR_RED),
            FadeIn(matrix_glow),
            Write(ops_text),
            run_time=1.5
        )

        self.wait(2)

        self.play(
            FadeOut(bb_title), FadeOut(binary_str), FadeOut(board), FadeOut(rays),
            FadeOut(eigen_title), FadeOut(matrix), FadeOut(matrix_glow), FadeOut(ops_text)
        )

        # BEAT 3: The Speedometer Crash (35-65s)
        gauge_center = ORIGIN
        gauge_arc = Arc(arc_center=gauge_center, radius=2.5, start_angle=PI, angle=-PI, color=COLOR_TEXT_MUTED, stroke_width=10)
        
        green_zone = Arc(arc_center=gauge_center, radius=2.5, start_angle=PI, angle=-PI/2, color=COLOR_GREEN, stroke_width=10)
        red_zone = Arc(arc_center=gauge_center, radius=2.5, start_angle=PI/2, angle=-PI/2, color=COLOR_RED, stroke_width=10)

        needle = Line(gauge_center, gauge_center + LEFT * 2, color=COLOR_CYAN, stroke_width=5)
        pivot = Dot(gauge_center, radius=0.15, color=COLOR_TEXT_BRIGHT)

        speed_label = Text("40,120,000 NPS", font="Consolas", font_size=48, color=COLOR_GREEN)
        speed_label.next_to(gauge_center, DOWN, buff=1)

        depth_label = Text("Depth 14", font="Bahnschrift", font_size=36, color=COLOR_GREEN)
        depth_label.next_to(speed_label, DOWN, buff=0.5)

        self.play(
            ShowCreation(gauge_arc),
            ShowCreation(green_zone),
            ShowCreation(red_zone),
            FadeIn(needle),
            FadeIn(pivot),
            Write(speed_label),
            Write(depth_label)
        )
        self.wait(1)

        engaging_text = Text("ENGAGING EIGENSOLVER...", font="Consolas", font_size=36, color=COLOR_RED)
        engaging_text.to_edge(UP)
        self.play(FadeIn(engaging_text, shift=DOWN))

        # Crash animation
        self.play(
            Rotate(needle, angle=-PI + 0.2, about_point=gauge_center),
            speed_label.animate.become(Text("30,412 NPS", font="Consolas", font_size=48, color=COLOR_RED).move_to(speed_label.get_center())),
            depth_label.animate.become(Text("Depth 5 (TIMEOUT)", font="Bahnschrift", font_size=36, color=COLOR_RED).move_to(depth_label.get_center())),
            needle.animate.set_color(COLOR_RED),
            run_time=2,
            rate_func=rush_into
        )

        # Shudder
        for _ in range(2):
            self.play(
                Rotate(needle, angle=0.1, about_point=gauge_center),
                run_time=0.1, rate_func=there_and_back
            )
            self.play(
                Rotate(needle, angle=-0.1, about_point=gauge_center),
                run_time=0.1, rate_func=there_and_back
            )

        self.wait(1)

        quote = Text('"Our algorithm was a genius...\nand it was playing chess like a complete idiot."', 
                     font="Bahnschrift", font_size=32, color=COLOR_TEXT_BRIGHT)
        quote.to_edge(DOWN)
        
        self.play(Write(quote))
        self.wait(3)
