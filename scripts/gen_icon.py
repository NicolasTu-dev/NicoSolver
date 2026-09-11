from PIL import Image, ImageDraw
import math

SS = 4
SIZE = 1024 * SS

img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)

bg_color = (10, 24, 16, 255)      # matches poker room theme background
accent = (212, 175, 55, 255)      # matches poker room gold accent

margin = 24 * SS
draw.rounded_rectangle([margin, margin, SIZE - margin, SIZE - margin], radius=180 * SS, fill=bg_color)

cx, cy = SIZE / 2, SIZE / 2 - 30 * SS

def spade_path(cx, cy, scale):
    pts = []
    steps = 300
    for i in range(steps + 1):
        t = i / steps * 2 * math.pi
        x = 16 * (math.sin(t) ** 3)
        y = 13 * math.cos(t) - 5 * math.cos(2*t) - 2 * math.cos(3*t) - math.cos(4*t)
        pts.append((cx + x * scale, cy - y * scale))
    return pts

scale = 16.5 * SS
pts = spade_path(cx, cy - 65 * SS, scale)
draw.polygon(pts, fill=accent)

stem_w = 65 * SS
draw.polygon([
    (cx - stem_w, cy + 260 * SS),
    (cx + stem_w, cy + 260 * SS),
    (cx + 18 * SS, cy + 155 * SS),
    (cx - 18 * SS, cy + 155 * SS),
], fill=accent)

img = img.resize((1024, 1024), Image.LANCZOS)
img.save("imgs/nicosolver_logo.png")

sizes = [16, 24, 32, 48, 64, 128, 256]
imgs = [img.resize((s, s), Image.LANCZOS) for s in sizes]
imgs[0].save("imgs/nicosolver_logo.ico", format="ICO", sizes=[(s, s) for s in sizes], append_images=imgs[1:])
print("done")
