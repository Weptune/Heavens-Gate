import math
import os
from PIL import Image, ImageDraw, ImageFilter

def generate_heavensgate_icon():
    width = 1024
    height = 1024
    center_x = width // 2
    center_y = height // 2

    # 1. Base Image Canvas (Matte Titanium Charcoal)
    img = Image.new("RGBA", (width, height), (11, 13, 19, 255))

    # 2. Subtle Sapphire Ambient Atmosphere (Radial Glow)
    glow_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)
    for r in range(460, 0, -5):
        alpha = int(28 * (1.0 - r / 460.0)**2.2)
        glow_draw.ellipse([center_x - r, center_y - r + 20, center_x + r, center_y + r + 20], fill=(56, 189, 248, alpha))
        glow_draw.ellipse([center_x - r * 0.6, center_y - r * 0.6, center_x + r * 0.6, center_y + r * 0.6], fill=(2, 132, 199, alpha // 2))
    img = Image.alpha_composite(img, glow_layer)

    # 3. Precision Vector Archway Threshold (The "Heaven's Gate")
    gate_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    gate_draw = ImageDraw.Draw(gate_layer)

    gate_w = 270
    gate_h = 340
    arch_cx = center_x
    arch_cy = center_y + 30
    bot_y = arch_cy + 220
    top_y = arch_cy - gate_h

    # Outer Platinum Arch
    gate_draw.arc([arch_cx - gate_w, arch_cy - gate_h, arch_cx + gate_w, arch_cy + gate_h], start=180, end=360, fill=(226, 232, 240, 230), width=10)
    gate_draw.line([arch_cx - gate_w + 5, arch_cy, arch_cx - gate_w + 5, bot_y], fill=(226, 232, 240, 230), width=10)
    gate_draw.line([arch_cx + gate_w - 5, arch_cy, arch_cx + gate_w - 5, bot_y], fill=(226, 232, 240, 230), width=10)

    # Inner Sapphire Accent Line
    in_w = gate_w - 18
    in_h = gate_h - 18
    gate_draw.arc([arch_cx - in_w, arch_cy - in_h, arch_cx + in_w, arch_cy + in_h], start=180, end=360, fill=(56, 189, 248, 180), width=3)
    gate_draw.line([arch_cx - in_w + 1, arch_cy, arch_cx - in_w + 1, bot_y], fill=(56, 189, 248, 180), width=3)
    gate_draw.line([arch_cx + in_w - 1, arch_cy, arch_cx + in_w - 1, bot_y], fill=(56, 189, 248, 180), width=3)

    # Base Platinum Threshold Line
    gate_draw.line([arch_cx - gate_w - 30, bot_y, arch_cx + gate_w + 30, bot_y], fill=(226, 232, 240, 255), width=8)

    gate_glow = gate_layer.filter(ImageFilter.GaussianBlur(10))
    img = Image.alpha_composite(img, gate_glow)
    img = Image.alpha_composite(img, gate_layer)

    # 4. Stylized Grandmaster Vector Knight / King Chess Emblem Inside Gate
    emblem_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    emblem_draw = ImageDraw.Draw(emblem_layer)

    # Geometric Faceted Knight / King Coordinates
    # Modern luxury polygonal silhouette
    head_top = (center_x - 30, arch_cy - 210)
    crown_cross = (center_x, arch_cy - 240)
    snout = (center_x - 140, arch_cy - 120)
    jaw = (center_x - 110, arch_cy - 30)
    mane_top = (center_x + 90, arch_cy - 180)
    mane_mid = (center_x + 130, arch_cy - 80)
    chest = (center_x - 80, arch_cy + 90)
    back = (center_x + 120, arch_cy + 90)
    base_left = (center_x - 160, bot_y - 10)
    base_right = (center_x + 160, bot_y - 10)

    # Facet Polygons (Translucent Glassmorphic Sapphire)
    # Head & Mane Facets
    emblem_draw.polygon([crown_cross, head_top, snout, (center_x - 40, arch_cy - 110)], fill=(30, 58, 138, 200), outline=(226, 232, 240, 220))
    emblem_draw.polygon([crown_cross, (center_x - 40, arch_cy - 110), (center_x + 20, arch_cy - 100), mane_top], fill=(2, 132, 199, 180), outline=(226, 232, 240, 220))
    emblem_draw.polygon([mane_top, (center_x + 20, arch_cy - 100), (center_x + 30, arch_cy - 10), mane_mid], fill=(14, 116, 144, 170), outline=(226, 232, 240, 220))

    # Snout & Jaw Facets
    emblem_draw.polygon([snout, jaw, (center_x - 30, arch_cy - 40), (center_x - 40, arch_cy - 110)], fill=(56, 189, 248, 140), outline=(226, 232, 240, 220))

    # Neck & Chest Facets
    emblem_draw.polygon([(center_x - 40, arch_cy - 110), (center_x - 30, arch_cy - 40), chest, (center_x, arch_cy + 20)], fill=(30, 58, 138, 220), outline=(226, 232, 240, 220))
    emblem_draw.polygon([(center_x - 30, arch_cy - 40), mane_mid, back, (center_x, arch_cy + 20)], fill=(15, 23, 42, 240), outline=(226, 232, 240, 220))

    # Base Pedestal Facet
    emblem_draw.polygon([chest, back, base_right, base_left], fill=(11, 19, 36, 250), outline=(56, 189, 248, 240))

    # Spectral Graph Nodes & Connection Threads (Subtle Graph Physics Overlay)
    graph_nodes = [
        crown_cross, head_top, snout, jaw, mane_top, mane_mid, chest, back, (center_x, arch_cy - 100), (center_x - 30, arch_cy - 40)
    ]
    for n1 in graph_nodes:
        for n2 in graph_nodes:
            d = math.hypot(n1[0] - n2[0], n1[1] - n2[1])
            if 40 < d < 140:
                emblem_draw.line([n1, n2], fill=(56, 189, 248, 90), width=1)

    for nx, ny in graph_nodes:
        emblem_draw.ellipse([nx - 5, ny - 5, nx + 5, ny + 5], fill=(226, 232, 240, 255), outline=(56, 189, 248, 255))

    # Apply Ambient Soft Glow
    emblem_glow = emblem_layer.filter(ImageFilter.GaussianBlur(6))
    img = Image.alpha_composite(img, emblem_glow)
    img = Image.alpha_composite(img, emblem_layer)

    # Save High-Res PNG
    os.makedirs("assets", exist_ok=True)
    png_path = "assets/heavensgate_icon.png"
    img.save(png_path, "PNG")
    print(f"[SUCCESS] Generated Executive Sapphire Platinum 1024x1024 PNG Icon at {png_path}")

    # Generate Precision SVG Vector
    svg_path = "assets/heavensgate_icon.svg"
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">
  <defs>
    <radialGradient id="bgGlow" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="#38BDF8" stop-opacity="0.25"/>
      <stop offset="60%" stop-color="#0284C7" stop-opacity="0.08"/>
      <stop offset="100%" stop-color="#0B0D13" stop-opacity="1.0"/>
    </radialGradient>
    <linearGradient id="platGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FFFFFF"/>
      <stop offset="50%" stop-color="#E2E8F0"/>
      <stop offset="100%" stop-color="#94A3B8"/>
    </linearGradient>
    <linearGradient id="sapphireGrad" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#38BDF8"/>
      <stop offset="100%" stop-color="#1E3A8A"/>
    </linearGradient>
    <filter id="glow">
      <feGaussianBlur stdDeviation="6" result="coloredBlur"/>
      <feMerge>
        <feMergeNode in="coloredBlur"/>
        <feMergeNode in="SourceGraphic"/>
      </feMerge>
    </filter>
  </defs>

  <!-- Titanium Space Canvas -->
  <rect width="1024" height="1024" fill="#0B0D13"/>
  <circle cx="512" cy="540" r="460" fill="url(#bgGlow)"/>

  <!-- Celestial Platinum Archway Threshold -->
  <path d="M {arch_cx - gate_w + 5} {bot_y} L {arch_cx - gate_w + 5} {arch_cy} A {gate_w} {gate_h} 0 0 1 {arch_cx + gate_w - 5} {arch_cy} L {arch_cx + gate_w - 5} {bot_y}" fill="none" stroke="url(#platGrad)" stroke-width="10" stroke-linecap="round" filter="url(#glow)"/>
  <path d="M {arch_cx - in_w + 1} {bot_y} L {arch_cx - in_w + 1} {arch_cy} A {in_w} {in_h} 0 0 1 {arch_cx + in_w - 1} {arch_cy} L {arch_cx + in_w - 1} {bot_y}" fill="none" stroke="url(#sapphireGrad)" stroke-width="3" stroke-linecap="round" opacity="0.8"/>
  <line x1="{arch_cx - gate_w - 30}" y1="{bot_y}" x2="{arch_cx + gate_w + 30}" y2="{bot_y}" stroke="url(#platGrad)" stroke-width="8" stroke-linecap="round"/>

  <!-- Faceted Vector Grandmaster Emblem -->
  <g filter="url(#glow)">
    <polygon points="{crown_cross[0]},{crown_cross[1]} {head_top[0]},{head_top[1]} {snout[0]},{snout[1]} {center_x - 40},{arch_cy - 110}" fill="#1E3A8A" fill-opacity="0.8" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{crown_cross[0]},{crown_cross[1]} {center_x - 40},{arch_cy - 110} {center_x + 20},{arch_cy - 100} {mane_top[0]},{mane_top[1]}" fill="#0284C7" fill-opacity="0.7" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{mane_top[0]},{mane_top[1]} {center_x + 20},{arch_cy - 100} {center_x + 30},{arch_cy - 10} {mane_mid[0]},{mane_mid[1]}" fill="#0E7490" fill-opacity="0.7" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{snout[0]},{snout[1]} {jaw[0]},{jaw[1]} {center_x - 30},{arch_cy - 40} {center_x - 40},{arch_cy - 110}" fill="#38BDF8" fill-opacity="0.6" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{center_x - 40},{arch_cy - 110} {center_x - 30},{arch_cy - 40} {chest[0]},{chest[1]} {center_x},{arch_cy + 20}" fill="#1E3A8A" fill-opacity="0.85" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{center_x - 30},{arch_cy - 40} {mane_mid[0]},{mane_mid[1]} {back[0]},{back[1]} {center_x},{arch_cy + 20}" fill="#0F172A" fill-opacity="0.9" stroke="#E2E8F0" stroke-width="2"/>
    <polygon points="{chest[0]},{chest[1]} {back[0]},{back[1]} {base_right[0]},{base_right[1]} {base_left[0]},{base_left[1]}" fill="#0B1324" fill-opacity="0.95" stroke="#38BDF8" stroke-width="3"/>
  </g>
</svg>
'''
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"[SUCCESS] Generated Executive Sapphire Platinum SVG Icon at {svg_path}")

if __name__ == "__main__":
    generate_heavensgate_icon()
