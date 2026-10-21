import sys
from PIL import Image, ImageDraw

# Image dimensions (2084x2084 as referenced in the code)
width, height = 2084, 2084
img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)

# Scale factor (similar to ds in the code)
scale = 3.0

# Character bounding box (centered in the image)
bb_width = 600
bb_height = 1000
bb_x = (width - bb_width) // 2
bb_y = (height - bb_height) // 2

# Colors (from g_style->col(pcol_text, 0.10f) and pcol_border, 0.85f)
fill_color = (255, 255, 255, 25)  # low opacity fill
outline_color = (200, 200, 200, 215)  # higher opacity outline

# Hitbox points (from draw_ped_hitbox_fallback)
head = (bb_x + bb_width // 2, bb_y + int(height * 0.18))
torso_top = (bb_x + bb_width // 2, bb_y + int(height * 0.32))
torso_bottom = (bb_x + bb_width // 2, bb_y + int(height * 0.62))
left_arm = (bb_x + int(bb_width * 0.27), bb_y + int(height * 0.43))
right_arm = (bb_x + bb_width - int(bb_width * 0.27), bb_y + int(height * 0.43))
left_leg = (bb_x + int(bb_width * 0.42), height - int(height * 0.14))
right_leg = (bb_x + bb_width - int(bb_width * 0.42), height - int(height * 0.14))

# Draw head (circle)
head_radius = int(18 * scale)
draw.ellipse(
    [head[0] - head_radius, head[1] - head_radius,
     head[0] + head_radius, head[1] + head_radius],
    fill=fill_color,
    outline=outline_color,
    width=int(2 * scale)
)

# Draw torso (line)
draw.line([torso_top, torso_bottom], fill=outline_color, width=int(16 * scale))
draw.line([torso_top, torso_bottom], fill=fill_color, width=int(12 * scale))

# Draw arms
draw.line([torso_top[0] - int(2 * scale), torso_top[1] + int(8 * scale), left_arm],
          fill=outline_color, width=int(8 * scale))
draw.line([torso_top[0] + int(2 * scale), torso_top[1] + int(8 * scale), right_arm],
          fill=outline_color, width=int(8 * scale))

# Draw legs
draw.line([torso_bottom, left_leg], fill=outline_color, width=int(9 * scale))
draw.line([torso_bottom, right_leg], fill=outline_color, width=int(9 * scale))

# Save the image
output_path = r'C:\Users\tyler\Desktop\Wizard External\pedhitbox_exported.png'
img.save(output_path)
print(f'Exported pedhitbox to: {output_path}')
