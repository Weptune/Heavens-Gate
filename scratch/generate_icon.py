import math
import os
from PIL import Image, ImageDraw, ImageFilter

def generate_heavensgate_icon():
    width = 1024
    height = 1024
    center_x = width // 2
    center_y = height // 2

    # 1. Base Image Canvas (Deep Obsidian Space)
    img = Image.new("RGBA", (width, height), (9, 11, 16, 255))

    # 2. Draw Subtle Background Radial Glow (Celestial Atmosphere)
    glow_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)
    for r in range(480, 0, -6):
        alpha = int(35 * (1.0 - r / 480.0)**2.0)
        glow_draw.ellipse([center_x - r, center_y - r + 30, center_x + r, center_y + r + 30], fill=(0, 240, 255, alpha))
        glow_draw.ellipse([center_x - r * 0.7, center_y - r * 0.7, center_x + r * 0.7, center_y + r * 0.7], fill=(255, 215, 0, alpha // 3))
    img = Image.alpha_composite(img, glow_layer)

    # 3. Draw Sleek Golden Archway ("Heaven's Gate")
    gate_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    gate_draw = ImageDraw.Draw(gate_layer)

    # Outer Archway Geometry
    arch_cx = center_x
    arch_cy = center_y + 40
    arch_rx = 260
    arch_ry = 320
    top_y = arch_cy - arch_ry
    bot_y = arch_cy + 220

    # Main Outer Golden Arch (Smooth Tapered Arc)
    gate_draw.arc([arch_cx - arch_rx, arch_cy - arch_ry, arch_cx + arch_rx, arch_cy + arch_ry], start=180, end=360, fill=(255, 215, 0, 240), width=18)
    gate_draw.line([arch_cx - arch_rx + 9, arch_cy, arch_cx - arch_rx + 9, bot_y], fill=(255, 215, 0, 240), width=18)
    gate_draw.line([arch_cx + arch_rx - 9, arch_cy, arch_cx + arch_rx - 9, bot_y], fill=(255, 215, 0, 240), width=18)

    # Inner Cyan Accent Line (Spectral Boundary)
    in_rx = arch_rx - 24
    in_ry = arch_ry - 24
    gate_draw.arc([arch_cx - in_rx, arch_cy - in_ry, arch_cx + in_rx, arch_cy + in_ry], start=180, end=360, fill=(0, 240, 255, 220), width=5)
    gate_draw.line([arch_cx - in_rx + 2, arch_cy, arch_cx - in_rx + 2, bot_y], fill=(0, 240, 255, 220), width=5)
    gate_draw.line([arch_cx + in_rx - 2, arch_cy, arch_cx + in_rx - 2, bot_y], fill=(0, 240, 255, 220), width=5)

    # Base Golden Pedestal Threshold
    gate_draw.line([arch_cx - arch_rx - 30, bot_y, arch_cx + arch_rx + 30, bot_y], fill=(255, 215, 0, 255), width=12)
    gate_draw.line([arch_cx - arch_rx - 10, bot_y + 16, arch_cx + arch_rx + 10, bot_y + 16], fill=(0, 240, 255, 200), width=6)

    # Soft Arch Ambient Glow
    gate_glow = gate_layer.filter(ImageFilter.GaussianBlur(14))
    img = Image.alpha_composite(img, gate_glow)
    img = Image.alpha_composite(img, gate_layer)

    # 4. Draw Geometric Faceted Vector Chess King Silhouette inside Gate
    king_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    king_draw = ImageDraw.Draw(king_layer)

    # Cross Top
    cross_y = arch_cy - 210
    king_draw.rectangle([center_x - 6, cross_y - 30, center_x + 6, cross_y + 20], fill=(255, 215, 0, 255))
    king_draw.rectangle([center_x - 22, cross_y - 14, center_x + 22, cross_y - 2], fill=(255, 215, 0, 255))
    king_draw.ellipse([center_x - 6, cross_y - 36, center_x + 6, cross_y - 24], fill=(0, 240, 255, 255))

    # Crown Crown Points (Faceted Polygons)
    # Left Wing
    king_draw.polygon([(center_x, cross_y + 25), (center_x - 130, cross_y + 70), (center_x - 70, cross_y + 120), (center_x, cross_y + 90)], fill=(0, 240, 255, 140), outline=(255, 215, 0, 220))
    # Right Wing
    king_draw.polygon([(center_x, cross_y + 25), (center_x + 130, cross_y + 70), (center_x + 70, cross_y + 120), (center_x, cross_y + 90)], fill=(0, 200, 255, 160), outline=(255, 215, 0, 220))
    # Center Gem
    king_draw.polygon([(center_x, cross_y + 20), (center_x - 45, cross_y + 75), (center_x, cross_y + 130), (center_x + 45, cross_y + 75)], fill=(255, 215, 0, 230), outline=(255, 255, 255, 255))

    # King Body / Robe Facets
    body_y = cross_y + 130
    king_draw.polygon([(center_x - 70, body_y), (center_x + 70, body_y), (center_x + 95, body_y + 90), (center_x - 95, body_y + 90)], fill=(15, 25, 45, 240), outline=(0, 240, 255, 220))
    king_draw.polygon([(center_x - 95, body_y + 90), (center_x + 95, body_y + 90), (center_x + 135, body_y + 190), (center_x - 135, body_y + 190)], fill=(12, 20, 36, 240), outline=(255, 215, 0, 240))

    # Inner Core Radiant Lattice Lines (Spectral Graph Interactions)
    lattice_nodes = [
        (center_x, cross_y + 50),
        (center_x - 60, cross_y + 90), (center_x + 60, cross_y + 90),
        (center_x - 45, body_y + 45), (center_x + 45, body_y + 45),
        (center_x - 80, body_y + 140), (center_x, body_y + 130), (center_x + 80, body_y + 140)
    ]
    for n1 in lattice_nodes:
        for n2 in lattice_nodes:
            d = math.hypot(n1[0] - n2[0], n1[1] - n2[1])
            if 30 < d < 110:
                king_draw.line([n1, n2], fill=(0, 240, 255, 120), width=1)

    for nx, ny in lattice_nodes:
        king_draw.ellipse([nx - 5, ny - 5, nx + 5, ny + 5], fill=(255, 215, 0, 255))

    # Soft Glow for King Silhouette
    king_glow = king_layer.filter(ImageFilter.GaussianBlur(8))
    img = Image.alpha_composite(img, king_glow)
    img = Image.alpha_composite(img, king_layer)

    # Save High-Res PNG
    os.makedirs("assets", exist_ok=True)
    png_path = "assets/heavensgate_icon.png"
    img.save(png_path, "PNG")
    print(f"[SUCCESS] Generated Minimalist Luxury 1024x1024 PNG Icon at {png_path}")

    # Generate Crisp Vector SVG
    svg_path = "assets/heavensgate_icon.svg"
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">
  <defs>
    <radialGradient id="bgGlow" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="#00F0FF" stop-opacity="0.30"/>
      <stop offset="60%" stop-color="#9D4EDD" stop-opacity="0.10"/>
      <stop offset="100%" stop-color="#090B10" stop-opacity="1.0"/>
    </radialGradient>
    <linearGradient id="goldArch" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FFF3B0"/>
      <stop offset="50%" stop-color="#FFD700"/>
      <stop offset="100%" stop-color="#DAA520"/>
    </linearGradient>
    <linearGradient id="cyanAccent" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#00F0FF"/>
      <stop offset="100%" stop-color="#7000FF"/>
    </linearGradient>
    <filter id="glow">
      <feGaussianBlur stdDeviation="8" result="coloredBlur"/>
      <feMerge>
        <feMergeNode in="coloredBlur"/>
        <feMergeNode in="SourceGraphic"/>
      </feMerge>
    </filter>
  </defs>

  <!-- Deep Obsidian Space Canvas -->
  <rect width="1024" height="1024" fill="#090B10"/>
  <circle cx="512" cy="550" r="480" fill="url(#bgGlow)"/>

  <!-- Celestial Golden Archway -->
  <path d="M {arch_cx - arch_rx + 9} {bot_y} L {arch_cx - arch_rx + 9} {arch_cy} A {arch_rx} {arch_ry} 0 0 1 {arch_cx + arch_rx - 9} {arch_cy} L {arch_cx + arch_rx - 9} {bot_y}" fill="none" stroke="url(#goldArch)" stroke-width="18" stroke-linecap="round" filter="url(#glow)"/>
  <path d="M {arch_cx - in_rx + 2} {bot_y} L {arch_cx - in_rx + 2} {arch_cy} A {in_rx} {in_ry} 0 0 1 {arch_cx + in_rx - 2} {arch_cy} L {arch_cx + in_rx - 2} {bot_y}" fill="none" stroke="url(#cyanAccent)" stroke-width="5" stroke-linecap="round" opacity="0.85"/>
  <line x1="{arch_cx - arch_rx - 30}" y1="{bot_y}" x2="{arch_cx + arch_rx + 30}" y2="{bot_y}" stroke="url(#goldArch)" stroke-width="12" stroke-linecap="round"/>

  <!-- Vector Faceted Chess King Silhouette -->
  <g filter="url(#glow)">
    <!-- Cross Top -->
    <rect x="{center_x - 6}" y="{cross_y - 30}" width="12" height="50" fill="#FFD700"/>
    <rect x="{center_x - 22}" y="{cross_y - 14}" width="44" height="12" fill="#FFD700"/>
    <circle cx="{center_x}" cy="{cross_y - 30}" r="6" fill="#00F0FF"/>

    <!-- Crown Wings -->
    <polygon points="{center_x},{cross_y + 25} {center_x - 130},{cross_y + 70} {center_x - 70},{cross_y + 120} {center_x},{cross_y + 90}" fill="#00F0FF" fill-opacity="0.5" stroke="#FFD700" stroke-width="2"/>
    <polygon points="{center_x},{cross_y + 25} {center_x + 130},{cross_y + 70} {center_x + 70},{cross_y + 120} {center_x},{cross_y + 90}" fill="#00C8FF" fill-opacity="0.6" stroke="#FFD700" stroke-width="2"/>
    <polygon points="{center_x},{cross_y + 20} {center_x - 45},{cross_y + 75} {center_x},{cross_y + 130} {center_x + 45},{cross_y + 75}" fill="#FFD700" fill-opacity="0.9" stroke="#FFFFFF" stroke-width="2"/>

    <!-- King Body -->
    <polygon points="{center_x - 70},{body_y} {center_x + 70},{body_y} {center_x + 95},{body_y + 90} {center_x - 95},{body_y + 90}" fill="#0F192D" stroke="#00F0FF" stroke-width="3"/>
    <polygon points="{center_x - 95},{body_y + 90} {center_x + 95},{body_y + 90} {center_x + 135},{body_y + 190} {center_x - 135},{body_y + 190}" fill="#0C1424" stroke="#FFD700" stroke-width="3"/>
  </g>
</svg>
'''
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"[SUCCESS] Generated Minimalist Luxury SVG Icon at {svg_path}")

if __name__ == "__main__":
    generate_heavensgate_icon()
