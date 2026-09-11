from PIL import Image, ImageDraw, ImageFilter

SS = 4
SIZE = 1024 * SS
GOLD_LIGHT = (240, 208, 96, 255)
GOLD_DARK = (168, 130, 30, 255)
BG_DARK = (8, 20, 13, 255)
BG_DARK2 = (14, 32, 21, 255)

cx, cy = SIZE / 2, SIZE / 2 - 30 * SS

# ---- build silhouette mask using simple, reliable primitives ----
mask = Image.new("L", (SIZE, SIZE), 0)
mdraw = ImageDraw.Draw(mask)

# top spike: sharp triangle pointing up
spike_top = (cx, cy - 300 * SS)
spike_base_y = cy - 60 * SS
spike_half_w = 60 * SS
mdraw.polygon([
    spike_top,
    (cx - spike_half_w, spike_base_y),
    (cx + spike_half_w, spike_base_y),
], fill=255)

# two round shoulders, overlapping each other and the spike base
r = 190 * SS
offset_x = 150 * SS
shoulder_y = cy - 40 * SS
mdraw.ellipse([cx - offset_x - r, shoulder_y - r, cx - offset_x + r, shoulder_y + r], fill=255)
mdraw.ellipse([cx + offset_x - r, shoulder_y - r, cx + offset_x + r, shoulder_y + r], fill=255)

# bridge triangle so the two shoulders + spike read as one continuous body
mdraw.polygon([
    (cx - offset_x, shoulder_y - r * 0.3),
    (cx + offset_x, shoulder_y - r * 0.3),
    (cx + offset_x, shoulder_y + r * 0.6),
    (cx - offset_x, shoulder_y + r * 0.6),
], fill=255)

# taper from the shoulders down to a bottom point
taper_top_y = shoulder_y + r * 0.55
taper_bottom = (cx, cy + 260 * SS)
mdraw.polygon([
    (cx - offset_x - r * 0.55, taper_top_y),
    (cx + offset_x + r * 0.55, taper_top_y),
    taper_bottom,
], fill=255)

# stem below the point
stem_w = 55 * SS
mdraw.polygon([
    (cx - stem_w, cy + 340 * SS),
    (cx + stem_w, cy + 340 * SS),
    (cx + 14 * SS, cy + 255 * SS),
    (cx - 14 * SS, cy + 255 * SS),
], fill=255)

mask.save("imgs/_debug_spade_mask.png")

# ---- compose final icon ----
img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
margin = 24 * SS
draw.rounded_rectangle([margin, margin, SIZE - margin, SIZE - margin], radius=200 * SS, fill=BG_DARK)
inset = margin + 40 * SS
draw.rounded_rectangle([inset, inset, SIZE - inset, SIZE - inset], radius=170 * SS, fill=BG_DARK2)

grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
gdraw = ImageDraw.Draw(grad)
top_y, bot_y = int(cy - 300*SS), int(cy + 340*SS)
for y in range(max(top_y, 0), min(bot_y, SIZE)):
    t = (y - top_y) / max(bot_y - top_y, 1)
    r_ = int(GOLD_LIGHT[0] + (GOLD_DARK[0] - GOLD_LIGHT[0]) * t)
    g_ = int(GOLD_LIGHT[1] + (GOLD_DARK[1] - GOLD_LIGHT[1]) * t)
    b_ = int(GOLD_LIGHT[2] + (GOLD_DARK[2] - GOLD_LIGHT[2]) * t)
    gdraw.line([(0, y), (SIZE, y)], fill=(r_, g_, b_, 255))
img.paste(grad, (0, 0), mask)

shine = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
sdraw = ImageDraw.Draw(shine)
sdraw.ellipse([cx - offset_x - r*0.8, shoulder_y - r*0.8, cx - offset_x + r*0.5, shoulder_y + r*0.1], fill=(255, 255, 255, 70))
shine = shine.filter(ImageFilter.GaussianBlur(26 * SS))
img = Image.alpha_composite(img, shine)

img = img.resize((1024, 1024), Image.LANCZOS)
img.save("imgs/icon_v4_spade.png")
print("done")
