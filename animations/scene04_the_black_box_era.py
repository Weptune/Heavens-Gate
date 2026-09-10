import sys
from pathlib import Path
sys.path.append(str(Path(__file__).parent))

from manimlib import *
import numpy as np

try:
    from chessboard_widget import BroadcastChessBoard
except ImportError:
    pass

class Scene04TheBlackBoxEra(Scene):
    def construct(self):
        self.camera.frame.set_width(14)
        
        # Color palette
        BG_COLOR = "#080c14"
        GOLD = "#f59e0b"
        CYAN = "#38bdf8"
        RED = "#ef4444"
        GREEN = "#10b981"
        VIOLET = "#8b5cf6"
        TEXT_LIGHT = "#f8fafc"
        TEXT_MUTED = "#94a3b8"
        
        self.camera.background_color = BG_COLOR
        
        # Beat 1: Classical Era
        board_center = LEFT * 3
        try:
            board = BroadcastChessBoard(center=board_center, sq_size=0.7, show_coords=True)
            self.play(FadeIn(board), run_time=1)
            
            wK = board.create_piece("wK", 4, 0)
            bK = board.create_piece("bK", 4, 7)
            wP1 = board.create_piece("wP", 4, 1)
            wP2 = board.create_piece("wP", 3, 1)
            wB = board.create_piece("wB", 2, 0)
            pieces = VGroup(wK, bK, wP1, wP2, wB)
            self.play(FadeIn(pieces))
            
            rule1 = Text("RULE: Bishop Pair = +0.5", font="Bahnschrift", font_size=24, color=CYAN).to_corner(UR).shift(DOWN*1)
            rule2 = Text("RULE: Pawns shield King", font="Bahnschrift", font_size=24, color=GREEN).next_to(rule1, DOWN, aligned_edge=LEFT)
            
            self.play(Write(rule1), Write(rule2))
            
            shield_rect = Rectangle(width=1.4, height=0.7, color=GREEN).move_to(board.get_square_pos(3.5, 1))
            shield_rect.set_stroke(opacity=0.8, width=3)
            shield_rect.set_fill(GREEN, opacity=0.2)
            
            self.play(ShowCreation(shield_rect))
            
            ceiling_line = Line(LEFT*6, RIGHT*6, color=RED, stroke_width=5).move_to(UP*3)
            ceiling_text = Text("HEURISTIC CEILING", font="Consolas", font_size=30, color=RED).next_to(ceiling_line, DOWN, aligned_edge=RIGHT).shift(LEFT*1)
            
            self.play(ShowCreation(ceiling_line), Write(ceiling_text))
            
            test_arrow = Arrow(DOWN*2, UP*2.9, color=GOLD, stroke_width=6)
            self.play(GrowArrow(test_arrow))
            self.play(test_arrow.animate.shift(DOWN*0.5).set_color(TEXT_MUTED), run_time=0.5, rate_func=there_and_back)
            
            self.wait(1)
            
            self.play(FadeOut(VGroup(board, pieces, rule1, rule2, shield_rect, ceiling_line, ceiling_text, test_arrow)))
            
        except Exception:
            text = Text("Classical Era - Fallback").scale(2)
            self.play(Write(text))
            self.play(FadeOut(text))

        # Beat 2: Neural Revolution
        layers = [4, 6, 6, 2]
        nodes = VGroup()
        edges = VGroup()
        
        layer_spacing = 2
        node_spacing = 0.8
        
        for i, num_nodes in enumerate(layers):
            layer_nodes = VGroup()
            for j in range(num_nodes):
                node = Circle(radius=0.2, color=CYAN, stroke_width=2, fill_opacity=0.1)
                node.move_to(RIGHT * (i - len(layers)/2 + 0.5) * layer_spacing + UP * (j - num_nodes/2 + 0.5) * node_spacing)
                layer_nodes.add(node)
            nodes.add(layer_nodes)
            
        for i in range(len(layers) - 1):
            for node1 in nodes[i]:
                for node2 in nodes[i+1]:
                    edge = Line(node1.get_center(), node2.get_center(), stroke_width=1, stroke_opacity=0.2, color=CYAN)
                    edges.add(edge)
                    
        nn_group = VGroup(edges, nodes)
        self.play(FadeIn(nn_group, lag_ratio=0.1))
        
        # Animate signal propagation
        for layer_idx in range(len(layers) - 1):
            pulses = VGroup()
            for edge in edges:
                if abs(edge.get_start()[0] - nodes[layer_idx][0].get_center()[0]) < 0.01:
                    pulse = Dot(color=GOLD, radius=0.05).move_to(edge.get_start())
                    pulses.add(pulse)
                    
            self.add(pulses)
            animations = []
            for pulse, edge in zip(pulses, [e for e in edges if abs(e.get_start()[0] - nodes[layer_idx][0].get_center()[0]) < 0.01]):
                animations.append(pulse.animate.move_to(edge.get_end()))
            
            if animations:
                self.play(*animations, run_time=0.5, rate_func=linear)
                self.play(nodes[layer_idx+1].animate.set_color(GOLD).set_fill(GOLD, opacity=0.5), run_time=0.2)
            self.remove(pulses)
            
        self.wait(0.5)
        self.play(nodes.animate.set_color(CYAN).set_fill(opacity=0.1))
        
        chart_axes = Axes(
            x_range=[0, 3, 1],
            y_range=[3000, 3500, 100],
            width=4,
            height=3,
            axis_config={"color": TEXT_MUTED}
        ).to_corner(DL).shift(UP*1 + RIGHT*1)
        
        bar_classical = Rectangle(width=0.8, height=1, color=CYAN, fill_opacity=0.7).next_to(chart_axes.c2p(1, 3000), UP, buff=0)
        bar_nnue = Rectangle(width=0.8, height=2.5, color=GOLD, fill_opacity=0.7).next_to(chart_axes.c2p(2, 3000), UP, buff=0)
        
        label_az = Text("AlphaZero\n2017", font="Bahnschrift", font_size=16).next_to(bar_classical, DOWN)
        label_nnue = Text("NNUE\n2020", font="Bahnschrift", font_size=16).next_to(bar_nnue, DOWN)
        elo_text = Text("+150 ELO", font="Consolas", font_size=24, color=GOLD).next_to(bar_nnue, UP)
        
        self.play(ShowCreation(chart_axes))
        self.play(GrowFromEdge(bar_classical, DOWN), Write(label_az))
        self.play(GrowFromEdge(bar_nnue, DOWN), Write(label_nnue))
        self.play(Write(elo_text))
        
        self.wait(1)
        
        self.play(FadeOut(VGroup(chart_axes, bar_classical, bar_nnue, label_az, label_nnue, elo_text)))

        # Beat 3: Black Box & Horizon Drift
        black_box = Square(side_length=4, color=VIOLET, fill_color=BLACK, fill_opacity=0.9, stroke_width=4)
        black_box.move_to(nn_group.get_center())
        
        self.play(
            nn_group.animate.set_color(TEXT_MUTED).set_stroke(opacity=0.1),
            FadeIn(black_box),
            run_time=1.5
        )
        
        box_label = Text("BLACK BOX", font="Consolas", font_size=32, color=VIOLET).move_to(black_box.get_center())
        self.play(Write(box_label))
        
        self.play(
            nn_group.animate.scale(0.5).shift(UP*2 + LEFT*4),
            black_box.animate.scale(0.5).shift(UP*2 + LEFT*4),
            box_label.animate.scale(0.5).shift(UP*2 + LEFT*4)
        )
        
        eval_axes = Axes(
            x_range=[0, 40, 10],
            y_range=[-5, 5, 2],
            width=8,
            height=4,
            axis_config={"color": TEXT_MUTED}
        ).shift(DOWN*1 + RIGHT*1)
        
        x_label = Text("Depth", font="Bahnschrift", font_size=20, color=TEXT_MUTED).next_to(eval_axes.x_axis, RIGHT)
        y_label = Text("Eval", font="Bahnschrift", font_size=20, color=TEXT_MUTED).next_to(eval_axes.y_axis, UP)
        
        self.play(ShowCreation(eval_axes), Write(x_label), Write(y_label))
        
        flat_line = eval_axes.get_graph(lambda x: 0, x_range=[0, 30], color=CYAN)
        crash_line = eval_axes.get_graph(lambda x: -4.5 * (x - 30)/6, x_range=[30, 36], color=RED)
        
        self.play(ShowCreation(flat_line), run_time=2, rate_func=linear)
        self.play(ShowCreation(crash_line), run_time=0.5, rate_func=rush_into)
        
        crash_point = eval_axes.c2p(36, -4.5)
        
        warning_dot = Dot(crash_point, color=RED, radius=0.1)
        warning_ring = Circle(radius=0.1, color=RED).move_to(crash_point)
        
        self.play(FadeIn(warning_dot))
        self.play(warning_ring.animate.scale(5).set_opacity(0), run_time=1)
        
        warning_text = Text("BLIND SPOT DETECTED", font="Consolas", font_size=36, color=RED).move_to(crash_point + UP*1.5 + LEFT*2)
        
        self.play(
            self.camera.frame.animate.scale(0.5).move_to(crash_point + UP*0.5 + LEFT*1),
            Write(warning_text),
            run_time=2
        )
        
        self.wait(2)
