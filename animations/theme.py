"""
Heaven's Gate Manim Animation Theme — Luminous Technical Jewel Palette
Inspired by the broadcast craft standards of 3Blue1Brown, vcubingx, and Reducible.
Pure obsidian canvas with high-contrast, translucent glowing jewel accents.
"""

from manimlib import *

# 1. Canvas & Structural Grids
BG_COLOR       = "#000000"     # Pure pitch black canvas (3b1b / vcubingx standard)
SURFACE_COLOR  = "#0a0e18"     # Translucent subpanel obsidian
CARD_BORDER    = "#1c2538"     # Subtle technical grid lines
GRID_LINE      = "#141c2c"     # Primary coordinate grid
GRID_FADED     = "#0d1320"     # Faded micro grid

# 2. Luminous Jewel Accents (High Luminescence & Backlit Glow)
# Reducible / 3b1b Jewel Standards:
JEWEL_VIOLET   = "#8c4dfb"     # Electric amethyst (Eigenvectors, spectral operators)
JEWEL_LAVENDER = "#d7b5fe"     # Soft glowing violet (Secondary harmonics, citations)
JEWEL_CYAN     = "#08B6CE"     # Cyber teal (Tactical rays, matrix entries, bitboards)
JEWEL_BLUE     = "#4d88ff"     # Deep royal electric blue
JEWEL_GREEN    = "#00cc70"     # Luminous neon sage (Row sum verification, Elo leaps)
JEWEL_GOLD     = "#ffff5c"     # Electric laser gold (Fiedler highlight, degree diagonals)
JEWEL_ORANGE   = "#ff9933"     # Warm amber
JEWEL_CORAL    = "#FF5752"     # Coral crimson (Pruning, horizon cliff, negative weights)

# 3. Typography & Text Hierarchy
TEXT_WHITE     = "#ffffff"     # Pure radiant white for key formulas
TEXT_BRIGHT    = "#e8edf5"     # Soft ivory white for body labels
TEXT_MUTED     = "#8fa0b5"     # Technical annotation slate
TEXT_DIM       = "#4a5a70"     # Minor dimensional coordinates

# 4. Chessboard Architectural Neutrals
BOARD_LIGHT_SQ = "#c8d4e4"     # Luminous silver-slate
BOARD_DARK_SQ  = "#344256"     # Deep architectural slate-blue
BOARD_FRAME_BG = "#0c111a"     # Recessed matte frame
BOARD_FRAME_STRK = "#1e283a"   # Chamfered hairline border

# 5. Legacy & Harmonization Aliases
COLOR_GOLD       = JEWEL_GOLD
COLOR_GOLD_LIGHT = "#ffff8d"
COLOR_CYAN       = JEWEL_CYAN
COLOR_CYAN_LIGHT = "#40d9f0"
COLOR_RED        = JEWEL_CORAL
COLOR_RED_LIGHT  = "#ff8a86"
COLOR_GREEN      = JEWEL_GREEN
COLOR_GREEN_LIGHT= "#40e699"
COLOR_VIOLET     = JEWEL_VIOLET
COLOR_VIOLET_LIGHT = JEWEL_LAVENDER

AQUA         = JEWEL_CYAN
LAVENDER     = JEWEL_LAVENDER
SALMON       = JEWEL_CORAL
SLATE_BLUE   = JEWEL_BLUE
SOFT_YELLOW  = JEWEL_GOLD
SOFT_ORANGE  = JEWEL_ORANGE
MUTED_GREEN  = JEWEL_GREEN
WARM_OCHRE   = JEWEL_GOLD

BG_DARK       = BG_COLOR
GRID_COLOR    = CARD_BORDER


def create_drafting_mat():
    """Create a high-craft pitch-black drafting canvas with hairline technical grid."""
    bg = FullScreenRectangle(fill_color=BG_COLOR, fill_opacity=1.0).set_stroke(width=0)
    grid = NumberPlane(
        x_range=[-16, 16, 1],
        y_range=[-10, 10, 1],
        width=32,
        height=20,
        axis_config={
            "stroke_color": CARD_BORDER,
            "stroke_width": 0.7,
            "stroke_opacity": 0.40,
        },
        background_line_style={
            "stroke_color": GRID_LINE,
            "stroke_width": 0.5,
            "stroke_opacity": 0.25,
        },
        faded_line_style={
            "stroke_color": GRID_FADED,
            "stroke_width": 0.3,
            "stroke_opacity": 0.15,
        }
    )
    return VGroup(bg, grid)
