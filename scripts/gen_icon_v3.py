from PIL import Image, ImageDraw, ImageFilter
import math

SS = 4
SIZE = 1024 * SS
GOLD_LIGHT = (240, 208, 96, 255)
GOLD_DARK = (168, 130, 30, 255)
BG_DARK = (8, 20, 13, 255)
BG_DARK2 = (14, 32, 21, 255)

img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
margin = 24 * SS
draw.rounded_rectangle([margin, margin, SIZE - margin, SIZE - margin], radius=200 * SS, fill=BG_DARK)
inset = margin + 40 * SS
draw.rounded_rectangle([inset, inset, SIZE - inset, SIZE - inset], radius=170 * SS, fill=BG_DARK2)

cx, cy = SIZE / 2, SIZE / 2 - 40 * SS

# Real spade silhouette: a smooth pointed dome on top (no heart-style notch),
# flaring into two rounded shoulders, tapering back in toward the stem.
def spade_outline(cx, cy, w, h):
    pts = []
    steps = 240
    for i in range(steps + 1):
        t = i / steps * 2 * math.pi
        # superellipse-ish top dome blended with waisted sides:
        # use a heart curve rotated 180 (point up) then bias to remove bottom notch
        x = 16 * (math.sin(t) ** 3)
        y = 13 * math.cos(t) - 5 * math.cos(2*t) - 2 * math.cos(3*t) - math.cos(4*t)
        # flip so the single point is at the TOP, lobes/notch at bottom
        y = -y
        pts.append((cx + x * w, cy + y * h))
    return pts

w = 15.5 * SS
h = 15.5 * SS
outline = spade_outline(cx, cy - 40 * SS, w, h)

# Fill the flipped-heart shape (point up, notch down)
mask = Image.new("L", (SIZE, SIZE), 0)
mdraw = ImageDraw.Draw(mask)
mdraw.polygon(outline, fill=255)

# Cover the bottom notch with a smooth rounded wedge so the silhouette reads
# as a single continuous spade body instead of two separate lobes.
notch_y = cy - 40*SS + h * 0.62
wedge = [
    (cx - w * 0.92, notch_y - 20*SS),
    (cx + w * 0.92, notch_y - 20*SS),
    (cx + w * 0.55, notch_y + 90*SS),
    (cx - w * 0.55, notch_y + 90*SS),
]
mdraw.polygon(wedge, fill=255)
# round the wedge's bottom corners by adding two small circles
mdraw.ellipse([cx - w*0.92 - 20*SS, notch_y - 40*SS, cx - w*0.92 + 60*SS, notch_y + 40*SS], fill=255)
mdraw.ellipse([cx + w*0.92 - 60*SS, notch_y - 40*SS, cx + w*0.92 + 20*SS, notch_y + 40*SS], fill=255)

# stem
stem_w = 60 * SS
stem_top_y = notch_y + 70*SS
stem_bottom_y = cy + 300 * SS
mdraw.polygon([
    (cx - stem_w, stem_bottom_y),
    (cx + stem_w, stem_bottom_y),
    (cx + 16*SS, stem_top_y),
    (cx - 16*SS, stem_top_y),
], fill=255)

# gradient fill through the mask
grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
gdraw = ImageDraw.Draw(grad)
top_y = int(cy - 40*SS - h)
bot_y = int(stem_bottom_y)
for y in range(max(top_y, 0), min(bot_y, SIZE)):
    t = (y - top_y) / max(bot_y - top_y, 1)
    r = int(GOLD_LIGHT[0] + (GOLD_DARK[0] - GOLD_LIGHT[0]) * t)
    g = int(GOLD_LIGHT[1] + (GOLD_DARK[1] - GOLD_LIGHT[1]) * t)
    b = int(GOLD_LIGHT[2] + (GOLD_DARK[2] - GOLD_LIGHT[2]) * t)
    gdraw.line([(0, y), (SIZE, y)], fill=(r, g, b, 255))
img.paste(grad, (0, 0), mask)

# soft shine highlight, upper-left of the dome
shine = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
sdraw = ImageDraw.Draw(shine)
sdraw.ellipse([cx - 260*SS, cy - 40*SS - h - 40*SS, cx - 20*SS, cy - 40*SS - h + 220*SS], fill=(255, 255, 255, 70))
shine = shine.filter(ImageFilter.GaussianBlur(28 * SS))
img = Image.alpha_composite(img, shine)

img = img.resize((1024, 1024), Image.LANCZOS)
img.save("imgs/icon_v3_spade.png")
print("done")
