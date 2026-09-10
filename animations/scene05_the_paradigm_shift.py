from manimlib import *
import numpy as np
import sys
from pathlib import Path

# Add parent directory for chessboard_widget
sys.path.append(str(Path(__file__).parent))
from chessboard_widget import BroadcastChessBoard

class Scene05TheParadigmShift(Scene):
    def construct(self):
        # BEAT 1: Side-by-Side (0-25s)
        self.beat_1_split_screen()
        
        # BEAT 2: The Three Pillars (25-55s)
        self.beat_2_three_pillars()
        
        # BEAT 3: The Cliffhanger (55-65s)
        self.beat_3_cliffhanger()

    def beat_1_split_screen(self):
        # Dividing line
        divider = Line(UP * 4, DOWN * 4, color=WHITE)
        
        # LEFT: Neural Net
        nn_title = Text("Opaque / 4 Million Weights", font="Bahnschrift", color="#94a3b8").scale(0.6)
        nn_title.move_to(LEFT * 3.5 + UP * 3)
        
        # Create a sprawling NN
        layers = [4, 6, 6, 4]
        nodes = VGroup()
        edges = VGroup()
        for i, count in enumerate(layers):
            layer = VGroup()
            for j in range(count):
                node = Circle(radius=0.15, color="#94a3b8", fill_opacity=1)
                node.move_to(LEFT * (5 - i) + UP * (count / 2.0 - j - 0.5) * 0.8)
                layer.add(node)
            nodes.add(layer)
            
        for i in range(len(layers) - 1):
            for n1 in nodes[i]:
                for n2 in nodes[i+1]:
                    edges.add(Line(n1.get_center(), n2.get_center(), color="#475569", stroke_opacity=0.3, stroke_width=1))
                    
        nn_group = VGroup(edges, nodes)
        
        # RIGHT: Graph Laplacian
        gl_title = Text("Analytical / 0 Weights", font="Bahnschrift", color="#f59e0b").scale(0.6)
        gl_title.move_to(RIGHT * 3.5 + UP * 3)
        
        math_text = Text("L = D - A", font="Cambria Math", color="#38bdf8").scale(1.5)
        math_text.move_to(RIGHT * 3.5 + UP * 1)
        
        # Matrix visualization (just a grid of squares)
        matrix = VGroup()
        for i in range(4):
            for j in range(4):
                val = 4 if i == j else -1
                color = "#38bdf8" if i == j else "#f43f5e"
                sq = Square(side_length=0.4, color=color, fill_opacity=0.2)
                sq.move_to(RIGHT * 3.5 + DOWN * 1 + RIGHT * (j - 1.5) * 0.45 + DOWN * (i - 1.5) * 0.45)
                matrix.add(sq)
                
        self.play(ShowCreation(divider))
        self.play(FadeIn(nn_title), FadeIn(gl_title))
        self.play(ShowCreation(edges), FadeIn(nodes))
        self.play(Write(math_text), ShowCreation(matrix))
        
        # Animate NN dimming
        self.play(nn_group.animate.set_opacity(0.1), run_time=2)
        
        # Animate GL glowing and shooting rays
        rays = VGroup()
        for _ in range(10):
            ray = Line(math_text.get_center(), math_text.get_center() + np.random.uniform(-2, 2, 3), color="#38bdf8", stroke_width=2)
            rays.add(ray)
            
        self.play(ShowCreation(rays), math_text.animate.set_color("#f59e0b"))
        self.play(FadeOut(rays))
        
        self.play(FadeOut(nn_group), FadeOut(nn_title), FadeOut(gl_title), FadeOut(divider), FadeOut(math_text), FadeOut(matrix))

    def beat_2_three_pillars(self):
        # Pillar 1
        p1_title = Text("1. Derived Features", font="Bahnschrift", color="#38bdf8")
        p1_title.to_edge(UP)
        self.play(Write(p1_title))
        
        num_line = NumberLine(x_range=[-3, 3, 1], width=10, color=WHITE)
        num_line.move_to(DOWN * 1)
        self.play(ShowCreation(num_line))
        
        dots = VGroup()
        for val, col in zip([-2.1, -0.5, 1.2, 2.5], ["#f43f5e", "#f59e0b", "#10b981", "#38bdf8"]):
            dot = Circle(radius=0.15, color=col, fill_opacity=1)
            dot.move_to(num_line.n2p(val))
            dots.add(dot)
            
        self.play(LaggedStartMap(FadeIn, dots, scale=0.5))
        self.play(dots.animate.shift(UP * 0.5), rate_func=there_and_back, run_time=1.5)
        
        self.play(FadeOut(dots), FadeOut(num_line), FadeOut(p1_title))
        
        # Pillar 2
        p2_title = Text("2. Complete Interpretability", font="Bahnschrift", color="#10b981")
        p2_title.to_edge(UP)
        self.play(Write(p2_title))
        
        # BroadcastChessBoard
        board = BroadcastChessBoard(center=DOWN * 0.5, sq_size=0.6)
        self.play(FadeIn(board))
        
        # Pieces and values
        labels = VGroup()
        pieces = VGroup()
        
        wK = board.create_piece("wK", 4, 0)
        bK = board.create_piece("bK", 4, 7)
        pieces.add(wK, bK)
        self.play(FadeIn(pieces))
        
        val1 = Text("+0.74", font="Consolas", color="#10b981").scale(0.4)
        val1.next_to(wK, UP, buff=0.1)
        val2 = Text("-0.82", font="Consolas", color="#f43f5e").scale(0.4)
        val2.next_to(bK, UP, buff=0.1)
        labels.add(val1, val2)
        
        self.play(FadeIn(labels, shift=UP))
        self.wait(1)
        self.play(FadeOut(board), FadeOut(pieces), FadeOut(labels), FadeOut(p2_title))

        # Pillar 3
        p3_title = Text("3. Depth 0 Topological Vision", font="Bahnschrift", color="#f59e0b")
        p3_title.to_edge(UP)
        self.play(Write(p3_title))
        
        # Search tree pruning
        root = Circle(radius=0.2, color=WHITE).move_to(UP * 1.5)
        n1 = Circle(radius=0.2, color=WHITE).move_to(LEFT * 2 + DOWN * 0.5)
        n2 = Circle(radius=0.2, color=WHITE).move_to(RIGHT * 2 + DOWN * 0.5)
        
        e1 = Line(root.get_bottom(), n1.get_top())
        e2 = Line(root.get_bottom(), n2.get_top())
        
        n1_1 = Circle(radius=0.2, color=WHITE).move_to(LEFT * 3 + DOWN * 2)
        n1_2 = Circle(radius=0.2, color=WHITE).move_to(LEFT * 1 + DOWN * 2)
        
        e1_1 = Line(n1.get_bottom(), n1_1.get_top())
        e1_2 = Line(n1.get_bottom(), n1_2.get_top())
        
        tree = VGroup(root, n1, n2, e1, e2, n1_1, n1_2, e1_1, e1_2)
        self.play(ShowCreation(tree))
        
        # Pruning n1 and its children
        prune_group = VGroup(n1, e1, n1_1, n1_2, e1_1, e1_2)
        self.play(prune_group.animate.set_color("#f43f5e"))
        
        # Cut lines
        cut1 = Line(LEFT * 1.5 + DOWN * 0.5, LEFT * 0.5 + UP * 0.5, color=RED).move_to(e1.get_center())
        cut2 = Line(LEFT * 0.5 + DOWN * 0.5, LEFT * 1.5 + UP * 0.5, color=RED).move_to(e1.get_center())
        cross = VGroup(cut1, cut2)
        self.play(ShowCreation(cross))
        
        self.play(FadeOut(prune_group), FadeOut(cross))
        self.play(FadeOut(tree), FadeOut(p3_title))

    def beat_3_cliffhanger(self):
        nightmare = Text("2 Month Training Nightmare", font="Bahnschrift", color="#f43f5e")
        self.play(Write(nightmare))
        
        # Ticking counter
        tracker = ValueTracker(0)
        count = Text("0", font="Consolas").scale(1.5).next_to(nightmare, DOWN, buff=1)
        
        def update_count(c):
            c.become(Text(str(int(tracker.get_value())), font="Consolas", color=WHITE).scale(1.5).move_to(count.get_center()))
            
        count.add_updater(update_count)
        self.add(count)
        
        # Progress bar
        bar_bg = Rectangle(width=6, height=0.3, color="#475569")
        bar_bg.next_to(count, DOWN, buff=1)
        bar_fg = Rectangle(width=0.1, height=0.3, color="#f43f5e", fill_opacity=1).align_to(bar_bg, LEFT)
        bar_fg.match_y(bar_bg)
        
        self.play(ShowCreation(bar_bg))
        
        def update_bar(b, alpha):
            b.stretch_to_fit_width(max(0.1, 6 * alpha))
            b.align_to(bar_bg, LEFT)
            
        # Simulate failed rounds
        for _ in range(3):
            self.play(
                tracker.animate.set_value(tracker.get_value() + 45),
                UpdateFromAlphaFunc(bar_fg, update_bar),
                run_time=0.5, rate_func=linear
            )
            bar_fg.stretch_to_fit_width(0.1)
            bar_fg.align_to(bar_bg, LEFT)
            
        count.remove_updater(update_count)
        self.play(
            nightmare.animate.scale(3).set_opacity(0),
            count.animate.scale(3).set_opacity(0),
            bar_bg.animate.scale(3).set_opacity(0),
            bar_fg.animate.scale(3).set_opacity(0),
            run_time=2
        )
