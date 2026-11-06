#!/usr/bin/env python3
"""Pack offline-decoded 12 Hz Idle V3 review baseline frames for ESP32."""
import argparse
import io
import struct
from pathlib import Path

from PIL import Image
from build_pet_v11_assets import pack_image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("frames", type=Path)
parser.add_argument("stage", type=Path)
parser.add_argument("preview", type=Path)
parser.add_argument("--ambient", type=Path)
args = parser.parse_args()

box = (97, 67, 369, 387)
width, height = box[2] - box[0], box[3] - box[1]
stage = Image.open(args.stage).convert("RGB")
assert stage.size == (466, 466)
base = stage.crop(box)
ambient = Image.open(args.ambient).convert("RGBA") if args.ambient else None
mask = Image.new("L", (width, height))
mask.putdata([
    min(255, 32 * min(x, y, width - 1 - x, height - 1 - y))
    for y in range(height) for x in range(width)
])
shoe_y = 288
shoe_box = (0, shoe_y, width, height)
blob = bytearray()
offsets = [0]
previews = []
for index in range(96):
    with Image.open(args.frames / f"{index:03}.png") as source:
        assert source.size == (480, 480)
        crop = source.convert("RGB").resize(stage.size, Image.Resampling.LANCZOS).crop(box)
    crop = Image.composite(crop, base, mask)
    if ambient:
        crop = Image.alpha_composite(crop.convert("RGBA"), ambient.crop(box)).convert("RGB")
    if index == 0:
        locked_feet = crop.crop(shoe_box)
        encoded = pack_image(crop, False)
    else:
        crop.paste(locked_feet, (0, shoe_y))
        buffer = io.BytesIO()
        crop.save(buffer, "JPEG", quality=87, subsampling=0)
        encoded = buffer.getvalue()
        with Image.open(io.BytesIO(encoded)) as check:
            assert check.size == (width, height)
    blob.extend(encoded)
    offsets.append(len(blob))
    preview = stage.copy()
    if ambient:
        preview = Image.alpha_composite(preview.convert("RGBA"), ambient)
    preview.paste(crop, box[:2])
    previews.append(preview)

target = Path(__file__).resolve().parents[1] / "main/generated/pet_idle_v3_frames.bin"
target.write_bytes(struct.pack("<97I", *offsets) + blob)
previews[0].save(args.preview / "idle-v3-static.png")
previews[0].save(args.preview / "idle-v3-loop.apng", save_all=True,
                 append_images=previews[1:], duration=[83, 83, 84] * 32, loop=0)
print(f"{target}: {target.stat().st_size:,} bytes")
