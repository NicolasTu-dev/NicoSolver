from PIL import Image, ImageDraw, ImageFilter
import math

SS = 4
SIZE = 1024 * SS
GOLD_LIGHT = (240, 208, 96, 255)
GOLD = (212, 175, 55, 255)
GOLD_DARK = (168, 130, 30, 255)
BG_DARK = (8, 20, 13, 255)
BG_DARK2 = (14, 32, 21, 255)


def base_canvas():
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    margin = 24 * SS
    # radial-ish background via two overlaid rounded rects for subtle depth
    draw.rounded_rectangle([margin, margin, SIZE - margin, SIZE - margin], radius=200 * SS, fill=BG_DARK)
    inset = margin + 40 * SS
    draw.rounded_rectangle([inset, inset, SIZE - inset, SIZE - inset], radius=170 * SS, fill=BG_DARK2)
    return img, draw


def spade_path(cx, cy, scale):
    pts = []
    steps = 300
    for i in range(steps + 1):
        t = i / steps * 2 * math.pi
        x = 16 * (math.sin(t) ** 3)
        y = 13 * math.cos(t) - 5 * math.cos(2*t) - 2 * math.cos(3*t) - math.cos(4*t)
        pts.append((cx + x * scale, cy - y * scale))
    return pts


def gradient_fill_polygon(img, pts, top_color, bottom_color):
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    x0, x1 = min(xs), max(xs)
    y0, y1 = min(ys), max(ys)
    w, h = int(x1 - x0) + 1, int(y1 - y0) + 1
    grad = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    gdraw = ImageDraw.Draw(grad)
    for y in range(h):
        t = y / max(h - 1, 1)
        r = int(top_color[0] + (bottom_color[0] - top_color[0]) * t)
        g = int(top_color[1] + (bottom_color[1] - top_color[1]) * t)
        b = int(top_color[2] + (bottom_color[2] - top_color[2]) * t)
        gdraw.line([(0, y), (w, y)], fill=(r, g, b, 255))
    mask = Image.new("L", (w, h), 0)
    mdraw = ImageDraw.Draw(mask)
    local_pts = [(x - x0, y - y0) for x, y in pts]
    mdraw.polygon(local_pts, fill=255)
    img.paste(grad, (int(x0), int(y0)), mask)


# ---- Option 1: refined gradient spade with shine ----
img1, draw1 = base_canvas()
cx, cy = SIZE / 2, SIZE / 2 - 30 * SS
scale = 16.5 * SS
pts = spade_path(cx, cy - 65 * SS, scale)
gradient_fill_polygon(img1, pts, GOLD_LIGHT, GOLD_DARK)
stem_w = 65 * SS
stem_pts = [
    (cx - stem_w, cy + 260 * SS),
    (cx + stem_w, cy + 260 * SS),
    (cx + 18 * SS, cy + 155 * SS),
    (cx - 18 * SS, cy + 155 * SS),
]
gradient_fill_polygon(img1, stem_pts, GOLD, GOLD_DARK)
# shine highlight ellipse (soft)
shine = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
sdraw = ImageDraw.Draw(shine)
sdraw.ellipse([cx - 280 * SS, cy - 320 * SS, cx - 40 * SS, cy - 120 * SS], fill=(255, 255, 255, 70))
shine = shine.filter(ImageFilter.GaussianBlur(30 * SS))
img1 = Image.alpha_composite(img1, shine)
img1 = img1.resize((1024, 1024), Image.LANCZOS)
img1.save("imgs/icon_option1_gradient_spade.png")

# ---- Option 2: "S" monogram built from a spade silhouette ----
img2, draw2 = base_canvas()
cx, cy = SIZE / 2, SIZE / 2
# Draw a bold serif-like "S" using two overlapping circles (classic monogram technique)
r = 260 * SS
draw2.ellipse([cx - r, cy - 340 * SS - r/2.4, cx + r*0.15, cy - 340*SS + r/2.4], fill=None)
# Simpler: render text "S" using a thick synthetic stroke via multiple offset ellipses is complex;
# instead draw a clean serif S using PIL's font rendering with the bold static Cinzel font for consistency.
from PIL import ImageFont
try:
    font = ImageFont.truetype("resources/fonts/Cinzel-Bold.ttf", int(620 * SS))
except Exception:
    font = ImageFont.load_default()
bbox = draw2.textbbox((0, 0), "S", font=font)
tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
tx, ty = cx - tw/2 - bbox[0], cy - th/2 - bbox[1] - 20*SS
# gradient text: render mask then paste gradient
mask2 = Image.new("L", (SIZE, SIZE), 0)
mdraw2 = ImageDraw.Draw(mask2)
mdraw2.text((tx, ty), "S", font=font, fill=255)
grad2 = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
gdraw2 = ImageDraw.Draw(grad2)
for y in range(SIZE):
    t = y / SIZE
    r_ = int(GOLD_LIGHT[0] + (GOLD_DARK[0] - GOLD_LIGHT[0]) * t)
    g_ = int(GOLD_LIGHT[1] + (GOLD_DARK[1] - GOLD_LIGHT[1]) * t)
    b_ = int(GOLD_LIGHT[2] + (GOLD_DARK[2] - GOLD_LIGHT[2]) * t)
    gdraw2.line([(0, y), (SIZE, y)], fill=(r_, g_, b_, 255))
img2.paste(grad2, (0, 0), mask2)
# small spade accent bottom-right
mini = spade_path(cx + 220*SS, cy + 260*SS, 4.2*SS)
draw2.polygon(mini, fill=GOLD_LIGHT)
img2 = img2.resize((1024, 1024), Image.LANCZOS)
img2.save("imgs/icon_option2_S_monogram.png")

# ---- Option 3: playing-card outline with a checkmark (solved) ----
img3, draw3 = base_canvas()
card_w, card_h = 480 * SS, 640 * SS
cx3, cy3 = SIZE / 2, SIZE / 2 + 10*SS
card_box = [cx3 - card_w/2, cy3 - card_h/2, cx3 + card_w/2, cy3 + card_h/2]
draw3.rounded_rectangle(card_box, radius=48*SS, outline=GOLD, width=14*SS)
# small spade pip top-left
pip = spade_path(card_box[0] + 70*SS, card_box[1] + 90*SS, 3.6*SS)
draw3.polygon(pip, fill=GOLD)
# checkmark in the middle, green accent (matches app's "checked" green)
check_pts = [
    (cx3 - 110*SS, cy3 + 20*SS),
    (cx3 - 30*SS, cy3 + 100*SS),
    (cx3 + 140*SS, cy3 - 110*SS),
]
draw3.line(check_pts, fill=(34, 197, 94, 255), width=42*SS, joint="curve")
img3 = img3.resize((1024, 1024), Image.LANCZOS)
img3.save("imgs/icon_option3_card_check.png")

print("done")
