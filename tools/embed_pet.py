#!/usr/bin/env python3
"""Embed cute-rem sprites into Buddy pet_*.c (RGB565A8 LVGL).

cute-rem v1 sheet (1536x1872, 8x9, cell 192x208). Map by Codex official
row→pose semantics (1-based row N → 0-based index N-1).
"""
from __future__ import annotations
import os
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SHEET = os.path.expanduser("~/.codex/pets/cute-rem/spritesheet.webp")
OUT_PET = os.path.join(ROOT, "main", "pet")
OUT_PREV = os.path.join(ROOT, "assets", "preview")
OUT_MAP = os.path.join(ROOT, "assets", "PET_MAPPING.md")

CELL_W, CELL_H = 192, 208
CROP = (35, 3, 157, 205)  # -> 122x202
OUT_W, OUT_H = 128, 212
STRIDE = OUT_W * 2  # RGB565
DATA_SIZE = OUT_W * OUT_H * 2 + OUT_W * OUT_H  # 81408

# scene -> (sheet_row 0-based, frame_count, official_name, note)
# Official 1-based rows from Codex cute-rem:
# 1 idle, 2 run-right, 3 run-left, 4 waving, 5 jumping,
# 6 failed, 7 waiting, 8 running(task), 9 review
MAPPING = {
    "sleep":    (6, 6, "waiting", "行7 waiting：双手胸前，期待/安静 — 不用 idle(行1)，idle 像在干活"),
    "work":     (7, 6, "running", "行8 running：坐着敲笔记本（任务执行）— 不用 idle(行1)"),
    "alert":    (3, 4, "waving",  "行4 waving：举手招呼 — 需批准"),
    "question": (8, 6, "review",  "行9 review：托腮审阅 — 需回答（不用 waiting，避免与 SLEEP 同循环）"),
}

def encode_rgb565a8(im: Image.Image) -> bytes:
    rgb = bytearray()
    alpha = bytearray()
    for r, g, b, a in im.convert("RGBA").getdata():
        v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        rgb.append(v & 0xFF)
        rgb.append((v >> 8) & 0xFF)
        alpha.append(a)
    return bytes(rgb) + bytes(alpha)

def c_bytes(data: bytes, per_line: int = 32) -> str:
    lines = []
    for i in range(0, len(data), per_line):
        chunk = data[i : i + per_line]
        lines.append("  " + ",".join(f"0x{b:02x}" for b in chunk) + ",")
    return "\n".join(lines)

def write_header(name: str, count: int) -> None:
    frames = "\n".join(f"extern const lv_image_dsc_t buddy_pet_{name}_{i};" for i in range(count))
    path = os.path.join(OUT_PET, f"pet_{name}.h")
    with open(path, "w") as f:
        f.write(
            f"""#pragma once
#include "lvgl.h"
#include <stdint.h>

{frames}
extern const lv_image_dsc_t * const buddy_pet_{name}_frames[{count}];
extern const uint8_t buddy_pet_{name}_count;
"""
        )

def write_c(name: str, images: list[Image.Image], row: int, official: str) -> None:
    path = os.path.join(OUT_PET, f"pet_{name}.c")
    parts = [
        '#include "lvgl.h"',
        f'#include "pet_{name}.h"',
        "",
        "#ifndef LV_ATTRIBUTE_MEM_ALIGN",
        "#define LV_ATTRIBUTE_MEM_ALIGN",
        "#endif",
        "",
        f"/* cute-rem sheet row {row} ({official}), crop {CROP} -> {OUT_W}x{OUT_H} RGB565A8 */",
        "",
    ]
    for i, im in enumerate(images):
        data = encode_rgb565a8(im)
        assert len(data) == DATA_SIZE
        parts.append(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t buddy_pet_{name}_{i}_map[{DATA_SIZE}] = {{")
        parts.append(c_bytes(data))
        parts.append("};")
        parts.append("")
        parts.append(f"const lv_image_dsc_t buddy_pet_{name}_{i} = {{")
        parts.append("  .header = {")
        parts.append("    .magic = LV_IMAGE_HEADER_MAGIC,")
        parts.append("    .cf = LV_COLOR_FORMAT_RGB565A8,")
        parts.append(f"    .w = {OUT_W},")
        parts.append(f"    .h = {OUT_H},")
        parts.append(f"    .stride = {STRIDE},")
        parts.append("  },")
        parts.append(f"  .data_size = sizeof(buddy_pet_{name}_{i}_map),")
        parts.append(f"  .data = buddy_pet_{name}_{i}_map,")
        parts.append("};")
        parts.append("")
    count = len(images)
    parts.append(f"const lv_image_dsc_t * const buddy_pet_{name}_frames[{count}] = {{")
    for i in range(count):
        parts.append(f"  &buddy_pet_{name}_{i},")
    parts.append("};")
    parts.append(f"const uint8_t buddy_pet_{name}_count = {count};")
    parts.append("")
    with open(path, "w") as f:
        f.write("\n".join(parts))

def main() -> None:
    sheet = Image.open(SHEET).convert("RGBA")
    assert sheet.size == (1536, 1872), sheet.size
    os.makedirs(OUT_PREV, exist_ok=True)

    scene_thumbs = []
    for name, (row, nframes, official, note) in MAPPING.items():
        images = []
        for c in range(nframes):
            cell = sheet.crop((c * CELL_W, row * CELL_H, (c + 1) * CELL_W, (row + 1) * CELL_H))
            cropped = cell.crop(CROP).resize((OUT_W, OUT_H), Image.Resampling.LANCZOS)
            images.append(cropped)
            prev = os.path.join(OUT_PREV, f"embed_{name}_{c}.png")
            cropped.save(prev)
        write_header(name, nframes)
        write_c(name, images, row, official)
        scene_thumbs.append((name, images[0], row, official, note, nframes))
        print(f"wrote pet_{name}: row{row}/{official} x{nframes}")

    # Contact sheet for assets/
    gap = 16
    label_h = 36
    canvas = Image.new("RGBA", (OUT_W * 4 + gap * 5, OUT_H + label_h + gap * 2), (24, 24, 32, 255))
    try:
        from PIL import ImageDraw, ImageFont
        draw = ImageDraw.Draw(canvas)
        font = ImageFont.load_default()
    except Exception:
        draw = None
        font = None
    for i, (name, im, row, official, note, nframes) in enumerate(scene_thumbs):
        x = gap + i * (OUT_W + gap)
        y = gap + label_h
        canvas.paste(im, (x, y), im)
        if draw:
            draw.text((x, 8), f"{name.upper()}\nr{row} {official} x{nframes}", fill=(255, 255, 255, 255), font=font)
    map_path = os.path.join(OUT_PREV, "buddy_scene_map.png")
    canvas.save(map_path)
    print("wrote", map_path)

    md = f"""# cute-rem → Buddy scene mapping

Spritesheet: 1536×1872, **8×9**, cell **192×208** (Codex Pet V1).

按 Codex 官方行→姿势语义绑定（1-based 行号；sheet row = 行号-1）。
`idle`(行1) 与 `running`(行8) 画面都像坐着笔记本，但合约不同：空闲场景用
**waiting**，工作场景用 **running**，不要用 idle。

| Buddy scene | Status | Sheet row (0-based) | Official (1-based) | Frames | Note |
|---|---|---|---|---|---|
| SLEEP | 0 | 6 | 7 waiting | 6 | {MAPPING['sleep'][3]} |
| WORK | 1/2 | 7 | 8 running | 6 | {MAPPING['work'][3]} |
| ALERT | 3 | 3 | 4 waving | 4 | {MAPPING['alert'][3]} |
| QUESTION | 4 | 8 | 9 review | 6 | {MAPPING['question'][3]} |

未嵌入：failed(行6) — 固件有 `last_event==2` (error)，后续可短暂切 failed。

Embed: {OUT_W}×{OUT_H} RGB565A8, crop {CROP}, LANCZOS resize. Regenerated by `tools/embed_pet.py`.
"""
    with open(OUT_MAP, "w") as f:
        f.write(md)
    print("wrote", OUT_MAP)

if __name__ == "__main__":
    main()
