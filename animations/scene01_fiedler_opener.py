"""
Heaven's Gate Documentary - Scene 01: The Fiedler Opener
Script Beat:
"About 3 months ago, while going through math research written by people
 much more talented than me, I stumbled upon this write up on spectral
 graph theory. For some reason, my initial instinct after reading it
 was that it would make for a great chess engine, due to how well it
 captured and interpreted 2D spaces. So... that's what I did :)"

Resolution: 1920x1080 | 60 fps | 16:9 Landscape
Aesthetic: Editorial Architectural / Swiss Print (No neon, no slop)
"""

import sys
from pathlib import Path

from manimlib import *

# 1. Palette: Matte Architectural Print
BG_BASALT        = "#121418"     # Deep matte charcoal
TEXT_BONE        = "#f1f3f7"     # Crisp chalk white
TEXT_MUTED       = "#7d8594"     # Editorial slate
GRID_HAIRLINE    = "#2d3340"     # Technical divider/border
ACCENT_OCHRE     = "#d97706"     # Archival highlighter (warm, aged ink)
ACCENT_OCHRE_BG  = "#92400e"
ACCENT_COBALT    = "#4d8cf5"     # Blueprint vector line
ACCENT_CYAN      = "#38bdf8"     # Mathematical spectral cyan
BOARD_DARK       = "#181b22"     # Architectural dark square
BOARD_LIGHT      = "#242834"     # Architectural light square

class Scene01FiedlerOpener(Scene):
    def construct(self):
        # -------------------------------------------------------------
        # 0. Solid Matte Basalt Canvas
        # -------------------------------------------------------------
        bg = Rectangle(width=16, height=9, fill_color=BG_BASALT, fill_opacity=1.0)
        bg.set_stroke(width=0)
        self.add(bg)

        # -------------------------------------------------------------
        # BEAT 1 (T=0.0s – 3.0s): The Authentic 1973 Fiedler Paper
        # "About 3 months ago, while going through math research written
        #  by people much more talented than me..."
        # -------------------------------------------------------------
        img_path = Path("assets/fiedler_user_paper.png")
        if not img_path.exists():
            img_path = Path("c:/Users/abhin/heavensgate/assets/fiedler_user_paper.png")

        paper_img = ImageMobject(str(img_path))
        paper_img.set_height(6.5)
        paper_img.move_to(ORIGIN)

        # Crisp framing border around the real document
        paper_border = SurroundingRectangle(paper_img, color=GRID_HAIRLINE, buff=0.01, stroke_width=1.2)

        # Editorial archival citation tag
        citation = Text(
            "MIROSLAV FIEDLER (1973) • CZECHOSLOVAK MATHEMATICAL JOURNAL, VOL. 23, NO. 2",
            font="Consolas", color=TEXT_MUTED
        ).scale(0.28)
        citation.to_edge(UP, buff=0.42)

        # Rock-solid presentation: no spinning, no unearned bounce
        self.play(
            FadeIn(paper_img, UP * 0.25),
            ShowCreation(paper_border),
            FadeIn(citation, UP * 0.15),
            run_time=1.2
        )
        self.wait(1.4)

        # -------------------------------------------------------------
        # BEAT 2 (T=3.0s – 5.5s): Highlighting the Core Discovery
        # "...I stumbled upon this write up on spectral graph theory."
        # -------------------------------------------------------------
        hl_box = RoundedRectangle(
            width=5.9, height=0.44, corner_radius=0.05,
            fill_color=ACCENT_OCHRE, fill_opacity=0.35
        )
        hl_box.set_stroke(ACCENT_OCHRE, width=1.5)
        hl_box.move_to(paper_img.get_center() + UP * 1.54)

        tag_title = Text("SPECTRAL GRAPH THEORY", font="Consolas", color=ACCENT_OCHRE).scale(0.32)
        tag_title.next_to(hl_box, UP, aligned_edge=LEFT, buff=0.12)

        self.play(
            ShowCreation(hl_box),
            FadeIn(tag_title, UP * 0.1),
            run_time=0.9
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 3 (T=5.5s – 8.0s): Mathematical Lift-off
        # The paper recedes to a background watermark; the Laplacian surfaces.
        # -------------------------------------------------------------
        laplacian_formula = Text("L  =  D  −  A", font="Consolas", color=TEXT_BONE).scale(0.95)
        laplacian_formula.move_to(ORIGIN)

        laplacian_sub = Text("The Graph Laplacian: Discrete Spatial Geometry", font="Segoe UI", color=TEXT_MUTED).scale(0.35)
        laplacian_sub.next_to(laplacian_formula, DOWN, buff=0.3)

        self.play(
            paper_img.animate.scale(1.12).set_opacity(0.10),
            paper_border.animate.scale(1.12).set_stroke(opacity=0.08),
            FadeOut(hl_box),
            FadeOut(tag_title),
            FadeOut(citation),
            FadeIn(laplacian_formula, UP * 0.3),
            FadeIn(laplacian_sub, UP * 0.2),
            run_time=1.3
        )
        self.wait(1.5)

        # -------------------------------------------------------------
        # BEAT 4 (T=8.0s – 11.5s): The Continuous 2D Spatial Graph
        # "For some reason, my initial instinct after reading it was that
        #  it would make for a great chess engine, due to how well it
        #  captured and interpreted 2D spaces..."
        # -------------------------------------------------------------
        raw_coords = [
            [-2.7,  1.3, 0], [-0.9,  1.7, 0], [ 0.9,  1.5, 0], [ 2.7,  1.2, 0],
            [-2.5,  0.3, 0], [-0.8,  0.5, 0], [ 0.8,  0.4, 0], [ 2.5,  0.2, 0],
            [-2.6, -0.7, 0], [-0.9, -0.5, 0], [ 0.9, -0.6, 0], [ 2.6, -0.8, 0],
            [-2.4, -1.7, 0], [-0.8, -1.6, 0], [ 0.8, -1.5, 0], [ 2.4, -1.8, 0]
        ]

        graph_nodes = VGroup()
        for pt in raw_coords:
            dot = Dot(point=pt, radius=0.075, color=ACCENT_COBALT)
            graph_nodes.add(dot)

        connections = [
            (0, 1), (1, 2), (2, 3),
            (4, 5), (5, 6), (6, 7),
            (8, 9), (9, 10), (10, 11),
            (12, 13), (13, 14), (14, 15),
            (0, 4), (4, 8), (8, 12),
            (1, 5), (5, 9), (9, 13),
            (2, 6), (6, 10), (10, 14),
            (3, 7), (7, 11), (11, 15),
            (0, 5), (1, 6), (2, 7),
            (4, 9), (5, 10), (6, 11),
            (8, 13), (9, 14), (10, 15)
        ]

        graph_edges = VGroup()
        for i, j in connections:
            edge = Line(raw_coords[i], raw_coords[j], color=GRID_HAIRLINE, stroke_width=1.0)
            graph_edges.add(edge)

        graph_header = Text("2D TOPOLOGICAL MANIFOLD", font="Consolas", color=ACCENT_CYAN).scale(0.32)
        graph_header.to_edge(UP, buff=0.8)

        self.play(
            FadeOut(paper_img),
            FadeOut(paper_border),
            ReplacementTransform(laplacian_formula, graph_header),
            FadeOut(laplacian_sub),
            ShowCreation(graph_edges),
            ShowCreation(graph_nodes),
            run_time=1.3
        )
        self.wait(1.0)

        # -------------------------------------------------------------
        # BEAT 5 (T=11.5s – 14.5s): The Snap to the 8x8 Chessboard
        # -------------------------------------------------------------
        sq_size = 0.85
        target_coords = []
        board_squares = VGroup()

        for r in range(4):
            for c in range(4):
                x = (c - 1.5) * sq_size
                y = (r - 1.5) * sq_size
                target_coords.append(np.array([x, y, 0]))

                sq = Square(side_length=sq_size)
                sq.move_to(np.array([x, y, 0]))
                is_light = (r + c) % 2 == 0
                sq.set_fill(BOARD_LIGHT if is_light else BOARD_DARK, opacity=1.0)
                sq.set_stroke(GRID_HAIRLINE, width=0.8)
                board_squares.add(sq)

        node_snaps = [
            graph_nodes[i].animate.move_to(target_coords[i])
            for i in range(16)
        ]

        board_header = Text("THE CHESSBOARD AS A SPECTRAL GRAPH", font="Consolas", color=TEXT_BONE).scale(0.34)
        board_header.to_edge(UP, buff=0.8)

        self.play(
            FadeOut(graph_edges),
            ReplacementTransform(graph_header, board_header),
            *node_snaps,
            run_time=1.2
        )
        self.play(
            FadeIn(board_squares),
            run_time=0.7
        )

        ray_diag = Line(target_coords[0], target_coords[15], color=ACCENT_COBALT, stroke_width=2.5)
        ray_rank = Line(target_coords[5], target_coords[7], color=ACCENT_COBALT, stroke_width=2.5)
        pulse_source = Dot(target_coords[0], radius=0.11, color=ACCENT_COBALT)

        edge_caption = Text("Adjacency Matrix A  ≡  Tactical Move & Influence Graph", font="Consolas", color=ACCENT_COBALT).scale(0.28)
        edge_caption.next_to(board_squares, DOWN, buff=0.45)

        self.play(
            FadeIn(pulse_source),
            ShowCreation(ray_diag),
            ShowCreation(ray_rank),
            FadeIn(edge_caption, UP * 0.2),
            run_time=1.0
        )
        self.wait(1.2)

        # -------------------------------------------------------------
        # BEAT 6 (T=14.5s – 18.0s): "...so... that's what I did :)"
        # The Knight Drops & The Engine Title Appears
        # -------------------------------------------------------------
        knight_path = Path("web/pieces/cburnett/wN.svg")
        if not knight_path.exists():
            knight_path = Path("c:/Users/abhin/heavensgate/web/pieces/cburnett/wN.svg")

        knight = SVGMobject(str(knight_path))
        knight.set_height(0.68)
        knight_target_sq = target_coords[9]

        title_tag = Text("HEAVEN'S GATE", font="Segoe UI", color=TEXT_BONE).scale(0.65)
        title_sub = Text("A Classical Spectral Chess Engine", font="Consolas", color=TEXT_MUTED).scale(0.32)
        title_sub.next_to(title_tag, DOWN, buff=0.15)
        title_group = VGroup(title_tag, title_sub)
        title_group.to_edge(RIGHT, buff=1.4).shift(UP * 0.2)

        target_highlight = board_squares[9].copy().set_fill(ACCENT_COBALT, opacity=0.28).set_stroke(ACCENT_COBALT, width=1.8)

        self.play(
            FadeOut(ray_diag),
            FadeOut(ray_rank),
            FadeOut(pulse_source),
            FadeOut(edge_caption),
            board_squares.animate.shift(LEFT * 2.2),
            graph_nodes.animate.shift(LEFT * 2.2),
            run_time=1.0
        )

        knight.move_to(target_coords[9] + LEFT * 2.2)
        target_highlight.move_to(target_coords[9] + LEFT * 2.2)

        self.play(
            FadeIn(knight, DOWN * 0.4),
            FadeIn(target_highlight),
            FadeIn(title_tag, LEFT * 0.3),
            FadeIn(title_sub, LEFT * 0.3),
            run_time=0.9
        )
        self.wait(2.5)
