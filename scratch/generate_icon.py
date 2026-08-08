import math
import os
from PIL import Image, ImageDraw, ImageFilter

def generate_heavensgate_icon():
    width = 1024
    height = 1024
    center_x = width // 2
    center_y = height // 2

    # 1. Canvas: Deep Cosmic Midnight Sky
    img = Image.new("RGBA", (width, height), (5, 8, 20, 255))

    # 2. Celestial Volumetric Atmosphere & Starlight Glow
    glow_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)

    # Multi-stage smooth radial heavenly light
    for r in range(480, 0, -4):
        alpha = int(40 * (1.0 - r / 480.0)**2.5)
        # Deep Sapphire aura
        glow_draw.ellipse([center_x - r, center_y - r + 30, center_x + r, center_y + r + 30], fill=(56, 189, 248, alpha))
        # Radiant Golden Celestial Heart
        glow_draw.ellipse([center_x - int(r * 0.65), center_y - int(r * 0.65) - 20, center_x + int(r * 0.65), center_y + int(r * 0.65) - 20], fill=(254, 240, 138, alpha // 2))
        # Pure White Inner Core
        glow_draw.ellipse([center_x - int(r * 0.35), center_y - int(r * 0.35) - 40, center_x + int(r * 0.35), center_y + int(r * 0.35) - 40], fill=(255, 255, 255, alpha // 2))

    img = Image.alpha_composite(img, glow_layer)

    # 3. Silky Smooth Golden & Pearl Archway Threshold ("Heaven's Gate")
    gate_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    gate_draw = ImageDraw.Draw(gate_layer)

    gate_w = 260
    gate_h = 330
    arch_cx = center_x
    arch_cy = center_y + 40
    bot_y = arch_cy + 220

    # Smooth Outer Celestial Arch (Pearl Gold)
    gate_draw.arc([arch_cx - gate_w, arch_cy - gate_h, arch_cx + gate_w, arch_cy + gate_h], start=180, end=360, fill=(254, 240, 138, 230), width=16)
    gate_draw.line([arch_cx - gate_w + 8, arch_cy, arch_cx - gate_w + 8, bot_y], fill=(254, 240, 138, 230), width=16)
    gate_draw.line([arch_cx + gate_w - 8, arch_cy, arch_cx + gate_w - 8, bot_y], fill=(254, 240, 138, 230), width=16)

    # Smooth Inner Ethereal Sky Accent Line
    in_w = gate_w - 20
    in_h = gate_h - 20
    gate_draw.arc([arch_cx - in_w, arch_cy - in_h, arch_cx + in_w, arch_cy + in_h], start=180, end=360, fill=(248, 250, 252, 240), width=6)
    gate_draw.line([arch_cx - in_w + 3, arch_cy, arch_cx - in_w + 3, bot_y], fill=(248, 250, 252, 240), width=6)
    gate_draw.line([arch_cx + in_w - 3, arch_cy, arch_cx + in_w - 3, bot_y], fill=(248, 250, 252, 240), width=6)

    # Smooth Rounded Base Threshold
    gate_draw.line([arch_cx - gate_w - 40, bot_y, arch_cx + gate_w + 40, bot_y], fill=(254, 240, 138, 255), width=10)
    gate_draw.ellipse([arch_cx - gate_w - 45, bot_y - 5, arch_cx - gate_w - 35, bot_y + 5], fill=(254, 240, 138, 255))
    gate_draw.ellipse([arch_cx + gate_w + 35, bot_y - 5, arch_cx + gate_w + 45, bot_y + 5], fill=(254, 240, 138, 255))

    # Apply Heavy Volumetric Soft Glow to Archway
    gate_glow = gate_layer.filter(ImageFilter.GaussianBlur(16))
    img = Image.alpha_composite(img, gate_glow)
    img = Image.alpha_composite(img, gate_layer)

    # 4. Smooth Organic Fluid Vector Silhouette of Chess King Standing in Portal Light
    king_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    king_draw = ImageDraw.Draw(king_layer)

    # Smooth Rounded Cross & Crown Top
    cross_cy = arch_cy - 190
    # Cross Horizontal Bar (Rounded Capsule)
    king_draw.ellipse([center_x - 22, cross_cy - 12, center_x - 10, cross_cy + 2], fill=(255, 255, 255, 255))
    king_draw.ellipse([center_x + 10, cross_cy - 12, center_x + 22, cross_cy + 2], fill=(255, 255, 255, 255))
    king_draw.rectangle([center_x - 16, cross_cy - 12, center_x + 16, cross_cy + 2], fill=(255, 255, 255, 255))
    # Cross Vertical Bar (Rounded Capsule)
    king_draw.rectangle([center_x - 7, cross_cy - 28, center_x + 7, cross_cy + 16], fill=(255, 255, 255, 255))
    king_draw.ellipse([center_x - 7, cross_cy - 34, center_x + 7, cross_cy - 22], fill=(254, 240, 138, 255))

    # Smooth Curved Crown Spheres & Arcs
    crown_y = cross_cy + 40
    # Central Orb
    king_draw.ellipse([center_x - 28, crown_y - 28, center_x + 28, crown_y + 28], fill=(254, 240, 138, 240))
    king_draw.ellipse([center_x - 18, crown_y - 18, center_x + 18, crown_y + 18], fill=(255, 255, 255, 255))

    # Left & Right Wing Spheres (Smooth Spherical Crown)
    king_draw.ellipse([center_x - 110, crown_y - 15, center_x - 70, crown_y + 25], fill=(56, 189, 248, 220))
    king_draw.ellipse([center_x + 70, crown_y - 15, center_x + 110, crown_y + 25], fill=(56, 189, 248, 220))
    king_draw.ellipse([center_x - 65, crown_y + 15, center_x - 35, crown_y + 45], fill=(125, 211, 252, 200))
    king_draw.ellipse([center_x + 35, crown_y + 15, center_x + 65, crown_y + 45], fill=(125, 211, 252, 200))

    # Smooth Flowing Robe Body (Curved Hourglass Contour sampled smoothly)
    body_points = []
    num_pts = 60
    for i in range(num_pts + 1):
        t = i / float(num_pts)
        y = (crown_y + 35) + t * (bot_y - 10 - (crown_y + 35))
        # Smooth wavering waist curve equation
        w = 40 + 75 * (t**1.4) + 20 * math.sin(t * math.pi)
        body_points.append((center_x - w, y))
    
    for i in range(num_pts, -1, -1):
        t = i / float(num_pts)
        y = (crown_y + 35) + t * (bot_y - 10 - (crown_y + 35))
        w = 40 + 75 * (t**1.4) + 20 * math.sin(t * math.pi)
        body_points.append((center_x + w, y))

    king_draw.polygon(body_points, fill=(15, 23, 42, 245), outline=(248, 250, 252, 230))

    # Smooth Concentric Halo Rings (Graph Physics Coordination Invariant)
    halo_draw = ImageDraw.Draw(king_layer)
    halo_draw.ellipse([center_x - 140, arch_cy - 100, center_x + 140, arch_cy + 180], fill=None, outline=(56, 189, 248, 140), width=2)
    halo_draw.ellipse([center_x - 90, arch_cy - 50, center_x + 90, arch_cy + 130], fill=None, outline=(254, 240, 138, 160), width=2)

    # Soft Volumetric Blur for King Silhouette
    king_glow = king_layer.filter(ImageFilter.GaussianBlur(10))
    img = Image.alpha_composite(img, king_glow)
    img = Image.alpha_composite(img, king_layer)

    # Save PNG
    os.makedirs("assets", exist_ok=True)
    png_path = "assets/heavensgate_icon.png"
    img.save(png_path, "PNG")
    print(f"[SUCCESS] Generated Ethereal Smooth 1024x1024 PNG Icon at {png_path}")

    # Generate Smooth Scalable SVG
    svg_path = "assets/heavensgate_icon.svg"
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">
  <defs>
    <radialGradient id="bgGlow" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="#FEF08A" stop-opacity="0.45"/>
      <stop offset="45%" stop-color="#38BDF8" stop-opacity="0.25"/>
      <stop offset="100%" stop-color="#050814" stop-opacity="1.0"/>
    </radialGradient>
    <linearGradient id="goldPearl" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FFFFFF"/>
      <stop offset="50%" stop-color="#FEF08A"/>
      <stop offset="100%" stop-color="#EAB308"/>
    </linearGradient>
    <linearGradient id="skyGrad" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#38BDF8"/>
      <stop offset="100%" stop-color="#0284C7"/>
    </linearGradient>
    <filter id="etherealGlow">
      <feGaussianBlur stdDeviation="12" result="coloredBlur"/>
      <feMerge>
        <feMergeNode in="coloredBlur"/>
        <feMergeNode in="SourceGraphic"/>
      </feMerge>
    </filter>
  </defs>

  <!-- Deep Midnight Sky -->
  <rect width="1024" height="1024" fill="#050814"/>
  <circle cx="512" cy="540" r="480" fill="url(#bgGlow)"/>

  <!-- Celestial Archway Threshold -->
  <path d="M {arch_cx - gate_w + 8} {bot_y} L {arch_cx - gate_w + 8} {arch_cy} A {gate_w} {gate_h} 0 0 1 {arch_cx + gate_w - 8} {arch_cy} L {arch_cx + gate_w - 8} {bot_y}" fill="none" stroke="url(#goldPearl)" stroke-width="16" stroke-linecap="round" filter="url(#etherealGlow)"/>
  <path d="M {arch_cx - in_w + 3} {bot_y} L {arch_cx - in_w + 3} {arch_cy} A {in_w} {in_h} 0 0 1 {arch_cx + in_w - 3} {arch_cy} L {arch_cx + in_w - 3} {bot_y}" fill="none" stroke="#F8FAFC" stroke-width="6" stroke-linecap="round" opacity="0.9"/>
  <line x1="{arch_cx - gate_w - 40}" y1="{bot_y}" x2="{arch_cx + gate_w + 40}" y2="{bot_y}" stroke="url(#goldPearl)" stroke-width="10" stroke-linecap="round"/>

  <!-- Smooth Flowing Ethereal Chess King -->
  <g filter="url(#etherealGlow)">
    <!-- Cross Top -->
    <rect x="{center_x - 7}" y="{cross_cy - 28}" width="14" height="44" rx="7" fill="#FFFFFF"/>
    <rect x="{center_x - 20}" y="{cross_cy - 12}" width="40" height="14" rx="7" fill="#FFFFFF"/>
    <circle cx="{center_x}" cy="{cross_cy - 28}" r="7" fill="#FEF08A"/>

    <!-- Smooth Spherical Crown -->
    <circle cx="{center_x}" cy="{crown_y}" r="28" fill="#FEF08A"/>
    <circle cx="{center_x}" cy="{crown_y}" r="18" fill="#FFFFFF"/>
    <circle cx="{center_x - 90}" cy="{crown_y + 5}" r="20" fill="url(#skyGrad)"/>
    <circle cx="{center_x + 90}" cy="{crown_y + 5}" r="20" fill="url(#skyGrad)"/>
    <circle cx="{center_x - 50}" cy="{crown_y + 30}" r="15" fill="#7DD3FC"/>
    <circle cx="{center_x + 50}" cy="{crown_y + 30}" r="15" fill="#7DD3FC"/>

    <!-- Concentric Halo Rings -->
    <circle cx="{center_x}" cy="{arch_cy + 40}" r="140" fill="none" stroke="#38BDF8" stroke-width="2" stroke-opacity="0.6"/>
    <circle cx="{center_x}" cy="{arch_cy + 40}" r="90" fill="none" stroke="#FEF08A" stroke-width="2" stroke-opacity="0.7"/>
  </g>
</svg>
'''
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"[SUCCESS] Generated Ethereal Smooth SVG Icon at {svg_path}")

if __name__ == "__main__":
    generate_heavensgate_icon()
