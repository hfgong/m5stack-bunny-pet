import io
import os
from PIL import Image, ImageDraw, ImageFilter
import numpy as np

# Load self-contained master cutout
master_path = os.path.join(os.path.dirname(__file__), 'bunny_master.png')
base_hires = Image.open(master_path).convert('RGBA')
W, H = base_hires.size
TARGET_W, TARGET_H = 52, 62

left_eye = (249, 171)
right_eye = (423, 168)
eye_col = (40, 20, 18, 255)
mouth_col = (95, 38, 42, 255)
yawn_inside = (130, 42, 58, 255)
tongue_col = (250, 140, 160, 255)

def seamless_cover_eye(im):
    result = im.copy()
    for cx, cy in [left_eye, right_eye]:
        fur_patch = im.crop((cx - 38, cy - 85, cx + 38, cy - 9))
        mask = Image.new('L', (76, 76), 0)
        mdraw = ImageDraw.Draw(mask)
        mdraw.ellipse([8, 8, 68, 68], fill=255)
        mask = mask.filter(ImageFilter.GaussianBlur(8))
        result.paste(fur_patch, (cx - 38, cy - 38), mask)
    return result

def add_soft_blush(im, alpha=130):
    blush = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    bdraw = ImageDraw.Draw(blush)
    bdraw.ellipse([left_eye[0]-70, left_eye[1]+30, left_eye[0]+15, left_eye[1]+105], fill=(255, 115, 135, alpha))
    bdraw.ellipse([right_eye[0]-15, right_eye[1]+30, right_eye[0]+70, right_eye[1]+105], fill=(255, 115, 135, alpha))
    blush = blush.filter(ImageFilter.GaussianBlur(18))
    return Image.alpha_composite(im, blush)

# 1. IDLE (Standard sweet alert bunny)
idle_img = base_hires.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 2. HAPPY (Joyful curved squint eyes, soft blushing cheeks, sweet smile)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
for cx, cy in [left_eye, right_eye]:
    pts = []
    for deg in range(195, 346, 5):
        rad = np.radians(deg)
        x = cx + int(36 * np.cos(rad))
        y = cy + int(24 * np.sin(rad)) + 12
        pts.append((x, y))
    draw.line(pts, fill=eye_col, width=16, joint='curve')
im = add_soft_blush(im, 150)
draw = ImageDraw.Draw(im)
mx, my = W // 2, int(left_eye[1] + 90)
draw.pieslice([mx-38, my-10, mx+38, my+52], start=0, end=180, fill=yawn_inside, outline=mouth_col, width=6)
draw.chord([mx-26, my+15, mx+26, my+52], start=0, end=180, fill=tongue_col)
happy_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 3. SLEEP (Downward curved lashes, peaceful smile, soft blush)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
ew, eh = 36, 24
draw.arc([left_eye[0]-ew, left_eye[1]-15, left_eye[0]+ew, left_eye[1]+eh], start=15, end=165, fill=eye_col, width=12)
draw.arc([right_eye[0]-ew, right_eye[1]-15, right_eye[0]+ew, right_eye[1]+eh], start=15, end=165, fill=eye_col, width=12)
draw.line([(left_eye[0]+ew-8, left_eye[1]+5), (left_eye[0]+ew+6, left_eye[1]+15)], fill=eye_col, width=9)
draw.line([(right_eye[0]-ew+8, right_eye[1]+5), (right_eye[0]-ew-6, right_eye[1]+15)], fill=eye_col, width=9)
im = add_soft_blush(im, 70)
sleep_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 4. DIZZY (Spiral swirly eyes & wavy mouth)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
for center in [left_eye, right_eye]:
    cx, cy = center
    draw.ellipse([cx-32, cy-32, cx+32, cy+32], outline=eye_col, width=8)
    draw.ellipse([cx-14, cy-14, cx+14, cy+14], fill=eye_col)
mx, my = W // 2, int(left_eye[1] + 92)
draw.arc([mx-40, my, mx, my+25], start=180, end=360, fill=mouth_col, width=7)
draw.arc([mx, my, mx+40, my+25], start=0, end=180, fill=mouth_col, width=7)
dizzy_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 5. DRAGGED (Stretched vertically with dangling paws, surprised round bead eyes & open mouth)
stretched = base_hires.resize((int(W * 0.94), int(H * 1.05)), Image.Resampling.LANCZOS)
drag_canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
drag_canvas.paste(stretched, ((W - stretched.width)//2, (H - stretched.height)//2))
draw = ImageDraw.Draw(drag_canvas)
mx, my = W // 2, int(left_eye[1] + 95)
draw.ellipse([mx-28, my, mx+28, my+55], fill=yawn_inside, outline=mouth_col, width=6)
draw.ellipse([mx-16, my+25, mx+16, my+50], fill=tongue_col)
drag_img = drag_canvas.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 6. EAT1 (Head down munch with puffed cheeks)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
ew, eh = 36, 24
draw.arc([left_eye[0]-ew, left_eye[1]-eh+10, left_eye[0]+ew, left_eye[1]+eh+10], start=190, end=350, fill=eye_col, width=11)
draw.arc([right_eye[0]-ew, right_eye[1]-eh+10, right_eye[0]+ew, right_eye[1]+eh+10], start=190, end=350, fill=eye_col, width=11)
im = add_soft_blush(im, 180)
draw = ImageDraw.Draw(im)
mx, my = W // 2, int(left_eye[1] + 98)
draw.ellipse([mx-32, my, mx+32, my+38], fill=mouth_col)
eat1_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 7. EAT2 (Head up chomp with crumb)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
ew, eh = 38, 26
draw.arc([left_eye[0]-ew, left_eye[1]-eh-5, left_eye[0]+ew, left_eye[1]+eh-5], start=190, end=350, fill=eye_col, width=11)
draw.arc([right_eye[0]-ew, right_eye[1]-eh-5, right_eye[0]+ew, right_eye[1]+eh-5], start=190, end=350, fill=eye_col, width=11)
im = add_soft_blush(im, 140)
draw = ImageDraw.Draw(im)
mx, my = W // 2, int(left_eye[1] + 85)
draw.arc([mx-36, my-10, mx+36, my+45], start=0, end=180, fill=mouth_col, width=9)
draw.ellipse([mx+15, my+55, mx+28, my+68], fill=(245, 155, 30, 255))
eat2_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 8. CELEBRATE (Joy hop with solid smile arcs, big open laugh, and golden sparkles)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
for cx, cy in [left_eye, right_eye]:
    pts = []
    for deg in range(195, 346, 5):
        rad = np.radians(deg)
        x = cx + int(36 * np.cos(rad))
        y = cy + int(24 * np.sin(rad)) + 10
        pts.append((x, y))
    draw.line(pts, fill=eye_col, width=16, joint='curve')
im = add_soft_blush(im, 160)
draw = ImageDraw.Draw(im)
mx, my = W // 2, int(left_eye[1] + 82)
draw.pieslice([mx-46, my-15, mx+46, my+58], start=0, end=180, fill=yawn_inside, outline=mouth_col, width=7)
draw.chord([mx-32, my+12, mx+32, my+58], start=0, end=180, fill=tongue_col)
for sp in [(left_eye[0]-140, left_eye[1]-90), (right_eye[0]+140, right_eye[1]-90)]:
    draw.polygon([
        (sp[0], sp[1]-30), (sp[0]+8, sp[1]-8), (sp[0]+30, sp[1]),
        (sp[0]+8, sp[1]+8), (sp[0], sp[1]+30), (sp[0]-8, sp[1]+8),
        (sp[0]-30, sp[1]), (sp[0]-8, sp[1]-8)
    ], fill=(255, 215, 60, 255))
celeb_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 9. YAWN (Droopy sleepy stretch)
im = seamless_cover_eye(base_hires)
draw = ImageDraw.Draw(im)
draw.line([(left_eye[0]-35, left_eye[1]+5), (left_eye[0]+35, left_eye[1]-5)], fill=eye_col, width=10)
draw.line([(right_eye[0]-35, right_eye[1]-5), (right_eye[0]+35, right_eye[1]+5)], fill=eye_col, width=10)
mx, my = W // 2, int(left_eye[1] + 90)
draw.ellipse([mx-35, my, mx+35, my+75], fill=yawn_inside, outline=mouth_col, width=6)
draw.chord([mx-25, my+35, mx+25, my+72], start=0, end=180, fill=tongue_col)
yawn_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 10. TIRED (Squashed posture, droopy eyes, pout)
squashed = base_hires.resize((int(W * 1.05), int(H * 0.95)), Image.Resampling.LANCZOS)
tired_canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
tired_canvas.paste(squashed, ((W - squashed.width)//2, H - squashed.height))
im = seamless_cover_eye(tired_canvas)
draw = ImageDraw.Draw(im)
cx_l, cy_l = int(left_eye[0] * 1.02), int(left_eye[1] + (H - squashed.height))
cx_r, cy_r = int(right_eye[0] * 0.98), int(right_eye[1] + (H - squashed.height))
draw.line([(cx_l-35, cy_l-8), (cx_l+35, cy_l+10)], fill=eye_col, width=10)
draw.line([(cx_r-35, cy_r+10), (cx_r+35, cy_r-8)], fill=eye_col, width=10)
mx, my = W // 2, int(cy_l + 90)
draw.arc([mx-36, my+15, mx+36, my+60], start=190, end=350, fill=mouth_col, width=8)
tired_img = im.resize((TARGET_W, TARGET_H), Image.Resampling.LANCZOS)

# 11. WALKING WADDLES (Emblem 'G' remains correctly oriented)
walk1_r = idle_img.rotate(-2.5, resample=Image.Resampling.BICUBIC, center=(TARGET_W//2, TARGET_H - 6))
walk2_r = idle_img.rotate(2.5, resample=Image.Resampling.BICUBIC, center=(TARGET_W//2, TARGET_H - 6))
walk1_l = idle_img.rotate(2.5, resample=Image.Resampling.BICUBIC, center=(TARGET_W//2, TARGET_H - 6))
walk2_l = idle_img.rotate(-2.5, resample=Image.Resampling.BICUBIC, center=(TARGET_W//2, TARGET_H - 6))

def img_to_png_bytes(im):
    buf = io.BytesIO()
    im.save(buf, format='PNG', optimize=True)
    return buf.getvalue()

png_dict = {
    'png_bunny_idle': img_to_png_bytes(idle_img),
    'png_bunny_walk1': img_to_png_bytes(walk1_r),
    'png_bunny_walk2': img_to_png_bytes(walk2_r),
    'png_bunny_walk1_l': img_to_png_bytes(walk1_l),
    'png_bunny_walk2_l': img_to_png_bytes(walk2_l),
    'png_bunny_happy': img_to_png_bytes(happy_img),
    'png_bunny_sleep': img_to_png_bytes(sleep_img),
    'png_bunny_dizzy': img_to_png_bytes(dizzy_img),
    'png_bunny_dragged': img_to_png_bytes(drag_img),
    'png_bunny_eat1': img_to_png_bytes(eat1_img),
    'png_bunny_eat2': img_to_png_bytes(eat2_img),
    'png_bunny_celebrate': img_to_png_bytes(celeb_img),
    'png_bunny_yawn': img_to_png_bytes(yawn_img),
    'png_bunny_tired': img_to_png_bytes(tired_img),
}

# Save preview images
preview_dir = os.path.join(os.path.dirname(__file__), 'previews')
os.makedirs(preview_dir, exist_ok=True)
idle_img.save(os.path.join(preview_dir, 'bunny_idle.png'))
happy_img.save(os.path.join(preview_dir, 'bunny_happy.png'))
sleep_img.save(os.path.join(preview_dir, 'bunny_sleep.png'))
drag_img.save(os.path.join(preview_dir, 'bunny_dragged.png'))
eat1_img.save(os.path.join(preview_dir, 'bunny_eat1.png'))
eat2_img.save(os.path.join(preview_dir, 'bunny_eat2.png'))
celeb_img.save(os.path.join(preview_dir, 'bunny_celebrate.png'))
yawn_img.save(os.path.join(preview_dir, 'bunny_yawn.png'))
tired_img.save(os.path.join(preview_dir, 'bunny_tired.png'))
dizzy_img.save(os.path.join(preview_dir, 'bunny_dizzy.png'))

# Generate AvatarSprites.h
header_path = os.path.join(os.path.dirname(__file__), 'src', 'AvatarSprites.h')
with open(header_path, 'w') as f:
    f.write('// Auto-generated by generate_bunny_sprites.py\n')
    f.write('// High-fidelity bunny plush avatar sprites with embroidered "G" monogram\n')
    f.write('#pragma once\n#include <Arduino.h>\n\n')
    f.write(f'#define AVATAR_W {TARGET_W}\n')
    f.write(f'#define AVATAR_H {TARGET_H}\n\n')
    
    for name, data in png_dict.items():
        f.write(f'// Sprite: {name}, Size: {len(data)} bytes\n')
        f.write(f'const uint8_t {name}[{len(data)}] PROGMEM = {{\n')
        for i in range(0, len(data), 12):
            chunk = data[i:i+12]
            f.write('    ' + ', '.join(f'0x{b:02X}' for b in chunk) + ',\n')
        f.write('};\n\n')
    
    # Backward compatibility aliases for seamless code reuse
    f.write('// Compatibility aliases\n')
    for name in png_dict.keys():
        old_name = name.replace('png_bunny_', 'png_muse_')
        f.write(f'#define {old_name} {name}\n')

print(f"Successfully generated AvatarSprites.h with {len(png_dict)} bunny animations ({sum(len(d) for d in png_dict.values())} bytes)!")
