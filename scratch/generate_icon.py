import math
import os
from PIL import Image, ImageDraw, ImageFilter

def generate_heavensgate_icon():
    width = 1024
    height = 1024
    center_x = width // 2
    center_y = height // 2

    # 1. Canvas: Deep Midnight Slate
    img = Image.new("RGBA", (width, height), (10, 13, 20, 255))

    # 2. Celestial Light Atmosphere (Soft Ambient Glow Behind Gate)
    glow_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)

    for r in range(450, 0, -5):
        alpha = int(35 * (1.0 - r / 450.0)**2.2)
        # Soft Golden Champagne Center Aura
        glow_draw.ellipse([center_x - r, center_y - r - 20, center_x + r, center_y + r - 20], fill=(253, 230, 138, alpha))
        # Deep Oceanic Background Ring
        glow_draw.ellipse([center_x - int(r * 0.7), center_y - int(r * 0.7), center_x + int(r * 0.7), center_y + int(r * 0.7)], fill=(30, 58, 138, alpha // 2))

    img = Image.alpha_composite(img, glow_layer)

    # 3. The Celestial Archway ("Heaven's Gate") — Pure Gothic Arch Symmetry
    arch_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    arch_draw = ImageDraw.Draw(arch_layer)

    arch_w = 230
    arch_h = 360
    base_y = center_y + 260
    top_y  = center_y - 280

    # Smooth Gothic Arch Top (Intersection of two smooth arcs)
    # Left Arc
    left_cx = center_x + arch_w // 2
    left_cy = top_y + arch_h // 2 + 40
    # Right Arc
    right_cx = center_x - arch_w // 2
    right_cy = top_y + arch_h // 2 + 40

    # Draw Outer Champagne Gold Gate Arch
    # Pillars
    arch_draw.line([center_x - arch_w, top_y + 120, center_x - arch_w, base_y], fill=(253, 230, 138, 240), width=16)
    arch_draw.line([center_x + arch_w, top_y + 120, center_x + arch_w, base_y], fill=(253, 230, 138, 240), width=16)
    
    # Curved Pointed Gothic Top
    arch_draw.arc([center_x - arch_w * 2, top_y - 60, center_x, top_y + arch_h + 60], start=295, end=360, fill=(253, 230, 138, 240), width=16)
    arch_draw.arc([center_x, top_y - 60, center_x + arch_w * 2, top_y + arch_h + 60], start=180, end=245, fill=(253, 230, 138, 240), width=16)

    # Inner Ivory Accent Frame
    in_w = arch_w - 18
    arch_draw.line([center_x - in_w, top_y + 130, center_x - in_w, base_y], fill=(250, 250, 250, 220), width=4)
    arch_draw.line([center_x + in_w, top_y + 130, center_x + in_w, base_y], fill=(250, 250, 250, 220), width=4)
    arch_draw.arc([center_x - in_w * 2, top_y - 45, center_x, top_y + arch_h + 45], start=295, end=360, fill=(250, 250, 250, 220), width=4)
    arch_draw.arc([center_x, top_y - 45, center_x + in_w * 2, top_y + arch_h + 45], start=180, end=245, fill=(250, 250, 250, 220), width=4)

    # Threshold Base Line
    arch_draw.line([center_x - arch_w - 40, base_y, center_x + arch_w + 40, base_y], fill=(253, 230, 138, 255), width=12)

    # Volumetric Soft Glow for the Gate
    arch_glow = arch_layer.filter(ImageFilter.GaussianBlur(12))
    img = Image.alpha_composite(img, arch_glow)
    img = Image.alpha_composite(img, arch_layer)

    # 4. Iconic Grandmaster Chess Knight Silhouette standing inside the Gate
    knight_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    knight_draw = ImageDraw.Draw(knight_layer)

    # Iconic Knight Contour Points (Smooth Curved Silhouette)
    # Scaled to fit perfectly inside the archway
    ky_off = center_y - 20
    kx_off = center_x

    knight_path = [
        (kx_off - 10, ky_off - 170),  # Ear Top
        (kx_off - 25, ky_off - 140),  # Ear Back
        (kx_off + 25, ky_off - 160),  # Mane Top
        (kx_off + 75, ky_off - 110),  # Mane Crest
        (kx_off + 105, ky_off - 30),  # Mane Slope
        (kx_off + 115, ky_off + 60),  # Back Neck
        (kx_off + 125, ky_off + 170), # Base Right
        (kx_off - 125, ky_off + 170), # Base Left
        (kx_off - 105, ky_off + 100), # Lower Chest
        (kx_off - 85, ky_off + 40),   # Chest Curve
        (kx_off - 115, ky_off - 20),  # Jaw Curve
        (kx_off - 130, ky_off - 60),  # Snout Tip
        (kx_off - 80, ky_off - 90),   # Muzzle Top
        (kx_off - 40, ky_off - 130),  # Head Slope
    ]

    # Draw Knight Body in Deep Titanium Dark Blue
    knight_draw.polygon(knight_path, fill=(15, 23, 42, 250), outline=(253, 230, 138, 230))

    # Inner Facet Line Details (Champagne Gold Vector Accents)
    facet_lines = [
        ((kx_off - 10, ky_off - 170), (kx_off - 40, ky_off - 90)),
        ((kx_off - 40, ky_off - 90), (kx_off - 130, ky_off - 60)),
        ((kx_off - 40, ky_off - 90), (kx_off - 30, ky_off + 10)),
        ((kx_off - 30, ky_off + 10), (kx_off - 105, ky_off + 100)),
        ((kx_off - 30, ky_off + 10), (kx_off + 50, ky_off - 30)),
        ((kx_off + 50, ky_off - 30), (kx_off + 75, ky_off - 110)),
        ((kx_off + 50, ky_off - 30), (kx_off + 115, ky_off + 60)),
        ((kx_off - 30, ky_off + 10), (kx_off + 10, ky_off + 170)),
    ]

    for p1, p2 in facet_lines:
        knight_draw.line([p1, p2], fill=(253, 230, 138, 140), width=2)

    # Knight Eye Spot (Soft Glowing Pearl Dot)
    knight_draw.ellipse([kx_off - 60, ky_off - 80, kx_off - 48, ky_off - 68], fill=(255, 255, 255, 255))
    knight_draw.ellipse([kx_off - 62, ky_off - 82, kx_off - 46, ky_off - 66], fill=(253, 230, 138, 180))

    # Apply Gentle Drop Shadow and Soft Glow
    knight_glow = knight_layer.filter(ImageFilter.GaussianBlur(8))
    img = Image.alpha_composite(img, knight_glow)
    img = Image.alpha_composite(img, knight_layer)

    # 5. Save PNG and Vector SVG
    os.makedirs("assets", exist_ok=True)
    png_path = "assets/heavensgate_icon.png"
    img.save(png_path, "PNG")
    print(f"[SUCCESS] Generated World-Class 1024x1024 PNG Icon at {png_path}")

    # Generate Crisp Scalable Vector SVG
    svg_path = "assets/heavensgate_icon.svg"
    svg_content = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" width="1024" height="1024">
  <defs>
    <radialGradient id="bgGlow" cx="50%" cy="45%" r="55%">
      <stop offset="0%" stop-color="#FDE68A" stop-opacity="0.35"/>
      <stop offset="50%" stop-color="#1E3A8A" stop-opacity="0.15"/>
      <stop offset="100%" stop-color="#0A0D14" stop-opacity="1.0"/>
    </radialGradient>
    <linearGradient id="goldGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FFFFFF"/>
      <stop offset="40%" stop-color="#FDE68A"/>
      <stop offset="100%" stop-color="#D97706"/>
    </linearGradient>
    <filter id="softGlow">
      <feGaussianBlur stdDeviation="10" result="coloredBlur"/>
      <feMerge>
        <feMergeNode in="coloredBlur"/>
        <feMergeNode in="SourceGraphic"/>
      </feMerge>
    </filter>
  </defs>

  <!-- Deep Midnight Slate Background -->
  <rect width="1024" height="1024" fill="#0A0D14"/>
  <circle cx="512" cy="512" r="480" fill="url(#bgGlow)"/>

  <!-- Celestial Gothic Archway ("Heaven's Gate") -->
  <g filter="url(#softGlow)">
    <line x1="{center_x - arch_w}" y1="{top_y + 120}" x2="{center_x - arch_w}" y2="{base_y}" stroke="url(#goldGrad)" stroke-width="16" stroke-linecap="round"/>
    <line x1="{center_x + arch_w}" y1="{top_y + 120}" x2="{center_x + arch_w}" y2="{base_y}" stroke="url(#goldGrad)" stroke-width="16" stroke-linecap="round"/>
    <path d="M {center_x - arch_w} {top_y + 125} A {arch_w * 2} {arch_h + 60} 0 0 1 {center_x} {top_y + 5}" fill="none" stroke="url(#goldGrad)" stroke-width="16" stroke-linecap="round"/>
    <path d="M {center_x + arch_w} {top_y + 125} A {arch_w * 2} {arch_h + 60} 0 0 0 {center_x} {top_y + 5}" fill="none" stroke="url(#goldGrad)" stroke-width="16" stroke-linecap="round"/>

    <!-- Inner Ivory Accent Frame -->
    <line x1="{center_x - in_w}" y1="{top_y + 130}" x2="{center_x - in_w}" y2="{base_y}" stroke="#FAFAFA" stroke-width="4" opacity="0.9"/>
    <line x1="{center_x + in_w}" y1="{top_y + 130}" x2="{center_x + in_w}" y2="{base_y}" stroke="#FAFAFA" stroke-width="4" opacity="0.9"/>
    <path d="M {center_x - in_w} {top_y + 130} A {in_w * 2} {arch_h + 45} 0 0 1 {center_x} {top_y + 15}" fill="none" stroke="#FAFAFA" stroke-width="4" opacity="0.9"/>
    <path d="M {center_x + in_w} {top_y + 130} A {in_w * 2} {arch_h + 45} 0 0 0 {center_x} {top_y + 15}" fill="none" stroke="#FAFAFA" stroke-width="4" opacity="0.9"/>

    <!-- Threshold Base -->
    <line x1="{center_x - arch_w - 40}" y1="{base_y}" x2="{center_x + arch_w + 40}" y2="{base_y}" stroke="url(#goldGrad)" stroke-width="12" stroke-linecap="round"/>
  </g>

  <!-- Iconic Grandmaster Chess Knight Silhouette -->
  <g filter="url(#softGlow)">
    <polygon points="''' + ' '.join([f'{px},{py}' for px, py in knight_path]) + f'''" fill="#0F172A" stroke="url(#goldGrad)" stroke-width="3"/>
'''
    for p1, p2 in facet_lines:
        svg_content += f'    <line x1="{p1[0]}" y1="{p1[1]}" x2="{p2[0]}" y2="{p2[1]}" stroke="#FDE68A" stroke-width="2" stroke-opacity="0.6"/>\n'

    svg_content += f'''    <circle cx="{kx_off - 54}" cy="{ky_off - 74}" r="6" fill="#FFFFFF"/>
  </g>
</svg>
'''
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"[SUCCESS] Generated World-Class Vector SVG Icon at {svg_path}")

if __name__ == "__main__":
    generate_heavensgate_icon()
