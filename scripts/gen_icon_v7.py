from svg.path import parse_path
from PIL import Image, ImageDraw, ImageFilter

SS = 4
SIZE = 1024 * SS
GOLD_LIGHT = (240, 208, 96, 255)
GOLD_DARK = (168, 130, 30, 255)
BG_DARK = (8, 20, 13, 255)
BG_DARK2 = (14, 32, 21, 255)

# Real spade path from Bootstrap Icons (bi-suit-spade-fill, MIT licensed),
# in its native 16x16 viewBox coordinate space.
D = ("M7.184 11.246A3.5 3.5 0 0 1 1 9c0-1.602 1.14-2.633 2.66-4.008"
     "C4.986 3.792 6.602 2.33 8 0c1.398 2.33 3.014 3.792 4.34 4.992"
     "C13.86 6.367 15 7.398 15 9a3.5 3.5 0 0 1-6.184 2.246"
     " 20 20 0 0 0 1.582 2.907c.231.35-.02.847-.438.847H6.04"
     "c-.419 0-.67-.497-.438-.847a20 20 0 0 0 1.582-2.907z")

path = parse_path(D)
samples = 900
pts = []
for i in range(samples + 1):
    t = i / samples
    p = path.point(t)
    pts.append((p.real, p.imag))

xs = [p[0] for p in pts]
ys = [p[1] for p in pts]
minx, maxx = min(xs), max(xs)
miny, maxy = min(ys), max(ys)
w16, h16 = maxx - minx, maxy - miny

target_h = 620 * SS
scale = target_h / h16
cx, cy = SIZE / 2, SIZE / 2 - 20 * SS

def to_canvas(p):
    x, y = p
    return (cx + (x - (minx + maxx) / 2) * scale, cy + (y - (miny + maxy) / 2) * scale)

canvas_pts = [to_canvas(p) for p in pts]

mask = Image.new("L", (SIZE, SIZE), 0)
mdraw = ImageDraw.Draw(mask)
mdraw.polygon(canvas_pts, fill=255)

img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
margin = 24 * SS
draw.rounded_rectangle([margin, margin, SIZE - margin, SIZE - margin], radius=200 * SS, fill=BG_DARK)
inset = margin + 40 * SS
draw.rounded_rectangle([inset, inset, SIZE - inset, SIZE - inset], radius=170 * SS, fill=BG_DARK2)

grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
gdraw = ImageDraw.Draw(grad)
top_y = int(cy - target_h/2)
bot_y = int(cy + target_h/2)
for y in range(max(top_y, 0), min(bot_y, SIZE)):
    t = (y - top_y) / max(bot_y - top_y, 1)
    r_ = int(GOLD_LIGHT[0] + (GOLD_DARK[0] - GOLD_LIGHT[0]) * t)
    g_ = int(GOLD_LIGHT[1] + (GOLD_DARK[1] - GOLD_LIGHT[1]) * t)
    b_ = int(GOLD_LIGHT[2] + (GOLD_DARK[2] - GOLD_LIGHT[2]) * t)
    gdraw.line([(0, y), (SIZE, y)], fill=(r_, g_, b_, 255))
img.paste(grad, (0, 0), mask)

shine = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
sdraw = ImageDraw.Draw(shine)
sdraw.ellipse([cx - 230*SS, cy - 60*SS, cx - 20*SS, cy + 150*SS], fill=(255, 255, 255, 70))
shine = shine.filter(ImageFilter.GaussianBlur(24 * SS))
img = Image.alpha_composite(img, shine)

img = img.resize((1024, 1024), Image.LANCZOS)
img.save("imgs/icon_v7_spade.png")

for s in (16, 32, 48):
    img.resize((s, s), Image.LANCZOS).resize((256, 256), Image.NEAREST).save(f"imgs/_check7_{s}px.png")

print("done")
