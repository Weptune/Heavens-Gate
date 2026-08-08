import math
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

def generate_heavensgate_icon():
    width = 1024
    height = 1024
    center_x = width // 2
    center_y = height // 2

    # 1. Base Image Canvas (RGBA)
    img = Image.new("RGBA", (width, height), (8, 12, 22, 255))
    draw = ImageDraw.Draw(img)

    # 2. Draw Background Radial Glow
    glow_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)
    for r in range(450, 0, -5):
        alpha = int(45 * (1.0 - r / 450.0)**1.8)
        glow_draw.ellipse([center_x - r, center_y - r + 40, center_x + r, center_y + r + 40], fill=(0, 240, 255, alpha))
        glow_draw.ellipse([center_x - r * 0.8, center_y - r * 0.8, center_x + r * 0.8, center_y + r * 0.8], fill=(157, 78, 221, alpha // 2))
    img = Image.alpha_composite(img, glow_layer)

    # 3. Draw 32 Tropical Sector Rays (Tropical Geometry Metaphor)
    rays_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    rays_draw = ImageDraw.Draw(rays_layer)
    num_rays = 32
    for i in range(num_rays):
        angle = (2 * math.pi / num_rays) * i
        dx = math.cos(angle) * 460
        dy = math.sin(angle) * 460
        alpha = 70 if i % 2 == 0 else 35
        color = (0, 240, 255, alpha) if i % 4 == 0 else (255, 215, 0, alpha // 2)
        rays_draw.line([center_x, center_y - 20, center_x + dx, center_y - 20 + dy], fill=color, width=2 if i % 2 == 0 else 1)
    img = Image.alpha_composite(img, rays_layer)

    # 4. Draw Heaven's Gate Archway (Golden Arch)
    arch_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    arch_draw = ImageDraw.Draw(arch_layer)

    # Outer Arch Ring
    gate_left = center_x - 260
    gate_right = center_x + 260
    gate_top = center_y - 320
    gate_bottom = center_y + 360

    # Draw Outer Celestial Golden Arch Body
    arch_draw.arc([gate_left, gate_top, gate_right, gate_top + 520], start=180, end=360, fill=(255, 215, 0, 240), width=28)
    arch_draw.line([gate_left + 14, gate_top + 260, gate_left + 14, gate_bottom], fill=(255, 215, 0, 240), width=28)
    arch_draw.line([gate_right - 14, gate_top + 260, gate_right - 14, gate_bottom], fill=(255, 215, 0, 240), width=28)

    # Inner Arch Ring Accent (Cyan Spectral Highlight)
    arch_draw.arc([gate_left + 24, gate_top + 24, gate_right - 24, gate_top + 500], start=180, end=360, fill=(0, 240, 255, 200), width=8)
    arch_draw.line([gate_left + 36, gate_top + 260, gate_left + 36, gate_bottom], fill=(0, 240, 255, 200), width=8)
    arch_draw.line([gate_right - 36, gate_top + 260, gate_right - 36, gate_bottom], fill=(0, 240, 255, 200), width=8)

    # Base Pedestal / Threshold Line
    arch_draw.line([gate_left - 40, gate_bottom, gate_right + 40, gate_bottom], fill=(255, 215, 0, 255), width=16)
    arch_draw.line([gate_left - 20, gate_bottom + 16, gate_right + 20, gate_bottom + 16], fill=(0, 240, 255, 220), width=8)

    # Apply soft glow to archway
    arch_glow = arch_layer.filter(ImageFilter.GaussianBlur(12))
    img = Image.alpha_composite(img, arch_glow)
    img = Image.alpha_composite(img, arch_layer)

    # 5. Draw Spectral Graph Physics Nodes & Edges (Chess King Silhouette inside Gate)
    graph_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    graph_draw = ImageDraw.Draw(graph_layer)

    # Graph Nodes defining a Majestic Crown / King Silhouette
    nodes = [
        # Cross Top
        (center_x, center_y - 240),
        (center_x - 30, center_y - 210), (center_x + 30, center_y - 210),
        (center_x, center_y - 180),
        # Crown Points
        (center_x - 140, center_y - 130), (center_x - 70, center_y - 150),
        (center_x + 70, center_y - 150), (center_x + 140, center_y - 130),
        # Crown Body / Head
        (center_x - 100, center_y - 60), (center_x, center_y - 80), (center_x + 100, center_y - 60),
        # Neck / Collar
        (center_x - 70, center_y + 40), (center_x + 70, center_y + 40),
        # Body / Base
        (center_x - 150, center_y + 160), (center_x - 60, center_y + 170),
        (center_x + 60, center_y + 170), (center_x + 150, center_y + 160),
        (center_x - 170, center_y + 260), (center_x, center_y + 270), (center_x + 170, center_y + 260),
    ]

    # Graph Edges (Laplacian adjacency matrix representation)
    edges = [
        (0,1), (0,2), (1,3), (2,3), (3,9),
        (4,5), (5,9), (6,9), (6,7),
        (4,8), (5,8), (6,10), (7,10),
        (8,9), (9,10), (8,11), (10,12), (9,11), (9,12),
        (11,12), (11,13), (11,14), (12,15), (12,16),
        (13,14), (14,15), (15,16),
        (13,17), (14,18), (15,18), (16,19),
        (17,18), (18,19)
    ]

    # Draw Weighted Graph Edges
    for idx1, idx2 in edges:
        p1 = nodes[idx1]
        p2 = nodes[idx2]
        dist = math.hypot(p1[0] - p2[0], p1[1] - p2[1])
        alpha = int(max(80, 230 - dist * 0.5))
        color = (0, 240, 255, alpha) if (idx1 + idx2) % 2 == 0 else (157, 78, 221, alpha)
        w = 3 if dist < 120 else 2
        graph_draw.line([p1, p2], fill=color, width=w)

    # Draw Spectral Graph Nodes (Glowing Spheres)
    for i, (nx, ny) in enumerate(nodes):
        r = 12 if i == 0 else (9 if i in [4, 7, 17, 19] else 7)
        # Outer glow ring
        graph_draw.ellipse([nx - r - 4, ny - r - 4, nx + r + 4, ny + r + 4], fill=(0, 240, 255, 60))
        # Inner core
        color = (255, 215, 0, 255) if i == 0 else ((0, 240, 255, 255) if i % 2 == 0 else (255, 255, 255, 255))
        graph_draw.ellipse([nx - r, ny - r, nx + r, ny + r], fill=color)

    graph_glow = graph_layer.filter(ImageFilter.GaussianBlur(6))
    img = Image.alpha_composite(img, graph_glow)
    img = Image.alpha_composite(img, graph_layer)

    # 6. Celestial Title / Emblem Text: "HEAVEN'S GATE"
    text_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text_layer)

    # Draw stylized central emblem text
    try:
        font = ImageFont.truetype("arial.ttf", 46)
        font_sub = ImageFont.truetype("arial.ttf", 22)
    except Exception:
        font = ImageFont.load_default()
        font_sub = ImageFont.load_default()

    title_text = "HEAVEN'S GATE"
    sub_text = "SPECTRAL-TROPICAL GRAPH PHYSICS ENGINE"

    # Draw Title
    tb = text_draw.textbbox((0, 0), title_text, font=font)
    tw = tb[2] - tb[0]
    text_draw.text((center_x - tw // 2, center_y + 380), title_text, fill=(255, 215, 0, 255), font=font)

    # Draw Subtitle
    sb = text_draw.textbbox((0, 0), sub_text, font=font_sub)
    sw = sb[2] - sb[0]
    text_draw.text((center_x - sw // 2, center_y + 440), sub_text, fill=(0, 240, 255, 220), font=font_sub)

    text_glow = text_layer.filter(ImageFilter.GaussianBlur(4))
    img = Image.alpha_composite(img, text_glow)
    img = Image.alpha_composite(img, text_layer)

    # 7. Ensure output directory exists and save PNG
    os.makedirs("assets", exist_ok=True)
    png_path = "assets/heavensgate_icon.png"
    img.save(png_path, "PNG")
    print(f"[SUCCESS] Generated 1024x1024 PNG Icon at {png_path}")

    # 8. Generate Scalable Vector Graphics (SVG) File
    svg_path = "assets/heavensgate_icon.svg"
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">
  <defs>
    <radialGradient id="bgGlow" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="#00F0FF" stop-opacity="0.35"/>
      <stop offset="50%" stop-color="#9D4EDD" stop-opacity="0.15"/>
      <stop offset="100%" stop-color="#080C16" stop-opacity="1.0"/>
    </radialGradient>
    <linearGradient id="goldArch" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FFF0AA"/>
      <stop offset="50%" stop-color="#FFD700"/>
      <stop offset="100%" stop-color="#FF8C00"/>
    </linearGradient>
    <linearGradient id="cyanAccent" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#00F0FF"/>
      <stop offset="100%" stop-color="#7000FF"/>
    </linearGradient>
    <filter id="glow">
      <feGaussianBlur stdDeviation="6" result="coloredBlur"/>
      <feMerge>
        <feMergeNode in="coloredBlur"/>
        <feMergeNode in="SourceGraphic"/>
      </feMerge>
    </filter>
  </defs>

  <!-- Background Canvas -->
  <rect width="1024" height="1024" fill="#080C16"/>
  <circle cx="512" cy="550" r="450" fill="url(#bgGlow)"/>

  <!-- Tropical Rays -->
'''
    for i in range(num_rays):
        angle = (2 * math.pi / num_rays) * i
        dx = math.cos(angle) * 460
        dy = math.sin(angle) * 460
        stroke_c = "#00F0FF" if i % 4 == 0 else "#FFD700"
        opacity = "0.3" if i % 2 == 0 else "0.15"
        svg_content += f'  <line x1="512" y1="492" x2="{512+dx:.1f}" y2="{492+dy:.1f}" stroke="{stroke_c}" stroke-width="2" stroke-opacity="{opacity}"/>\n'

    svg_content += f'''
  <!-- Heaven's Gate Golden Archway -->
  <path d="M {gate_left + 14} {gate_bottom} L {gate_left + 14} {gate_top + 260} A 246 246 0 0 1 {gate_right - 14} {gate_top + 260} L {gate_right - 14} {gate_bottom}" fill="none" stroke="url(#goldArch)" stroke-width="28" stroke-linecap="round" filter="url(#glow)"/>
  <path d="M {gate_left + 36} {gate_bottom} L {gate_left + 36} {gate_top + 260} A 224 224 0 0 1 {gate_right - 36} {gate_top + 260} L {gate_right - 36} {gate_bottom}" fill="none" stroke="url(#cyanAccent)" stroke-width="8" stroke-linecap="round" opacity="0.85"/>
  <line x1="{gate_left - 40}" y1="{gate_bottom}" x2="{gate_right + 40}" y2="{gate_bottom}" stroke="url(#goldArch)" stroke-width="16" stroke-linecap="round"/>

  <!-- Spectral Graph Edges -->
  <g filter="url(#glow)">
'''
    for idx1, idx2 in edges:
        p1 = nodes[idx1]
        p2 = nodes[idx2]
        stroke = "#00F0FF" if (idx1 + idx2) % 2 == 0 else "#9D4EDD"
        svg_content += f'    <line x1="{p1[0]}" y1="{p1[1]}" x2="{p2[0]}" y2="{p2[1]}" stroke="{stroke}" stroke-width="2.5" stroke-opacity="0.8"/>\n'

    svg_content += f'''  </g>

  <!-- Spectral Graph Nodes -->
  <g filter="url(#glow)">
'''
    for i, (nx, ny) in enumerate(nodes):
        r = 12 if i == 0 else (9 if i in [4, 7, 17, 19] else 7)
        fill = "#FFD700" if i == 0 else ("#00F0FF" if i % 2 == 0 else "#FFFFFF")
        svg_content += f'    <circle cx="{nx}" cy="{ny}" r="{r}" fill="{fill}"/>\n'

    svg_content += f'''  </g>

  <!-- Typography Emblem -->
  <text x="512" y="892" font-family="Arial, sans-serif" font-size="44" font-weight="bold" fill="#FFD700" text-anchor="middle" letter-spacing="4" filter="url(#glow)">HEAVEN'S GATE</text>
  <text x="512" y="952" font-family="Arial, sans-serif" font-size="20" font-weight="600" fill="#00F0FF" text-anchor="middle" letter-spacing="2" opacity="0.9">SPECTRAL-TROPICAL GRAPH PHYSICS ENGINE</text>
</svg>
'''
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"[SUCCESS] Generated Vector SVG Icon at {svg_path}")

if __name__ == "__main__":
    generate_heavensgate_icon()
