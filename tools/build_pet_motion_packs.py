#!/usr/bin/env python3
"""Encode review motion as independently seekable RGB565 JPEG inputs."""
import argparse
import io
import struct
from pathlib import Path
from PIL import Image, ImageStat

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("actions", type=Path)
parser.add_argument("working", type=Path)
parser.add_argument("stage", type=Path)
parser.add_argument("ambient", type=Path)
parser.add_argument("work_ambient", type=Path)
parser.add_argument("idle_frame", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
stage = Image.open(args.stage).convert("RGBA")
ambient = Image.open(args.ambient).convert("RGBA")
box = (97, 67, 369, 387)
base = stage.crop(box)
mask = Image.new("L", base.size)
mask.putdata([min(255, 32 * min(x, y, 271-x, 319-y)) for y in range(320) for x in range(272)])
shoe_y = 288
idle = Image.open(args.idle_frame).convert("RGB").resize(stage.size, Image.Resampling.LANCZOS).crop(box)
idle = Image.composite(idle, base.convert("RGB"), mask).convert("RGBA")
locked_feet = Image.alpha_composite(idle, ambient.crop(box)).crop((0,shoe_y,272,320))


def write_pack(name, frames):
    offsets = [0]
    payload = bytearray()
    for frame in frames:
        stream = io.BytesIO()
        frame.convert("RGB").save(stream, "JPEG", quality=82, subsampling=2)
        encoded = stream.getvalue()
        assert len(encoded) <= 65536
        payload.extend(encoded)
        offsets.append(len(payload))
    count = len(offsets)-1
    assert 1 <= count <= 240
    (args.output / f"{name}.mjp").write_bytes(struct.pack(f"<{count+2}I", count, *offsets) + payload)
    print(name, count, len(payload))


def idle_action(folder):
    started = False
    for file in sorted(folder.glob("*.png")):
        frame = Image.open(file).convert("RGB").resize(stage.size, Image.Resampling.LANCZOS).crop(box)
        frame = Image.composite(frame, base.convert("RGB"), mask).convert("RGBA")
        frame = Image.alpha_composite(frame, ambient.crop(box))
        # A review clip can export a dark startup frame before its artwork.
        # Only trim the leading blank frames, leaving the action itself intact.
        if not started:
            luminance = ImageStat.Stat(frame.crop((60, 25, 212, 270)).convert("L")).mean[0]
            if luminance < 45:
                print(f"Skipping dark startup frame: {folder.name}/{file.name}")
                continue
            started = True
        frame.paste(locked_feet, (0,shoe_y))
        yield frame


def working_frames():
    source = Image.open(args.working)
    # Center the 360px source within the 466px display at its native size.
    assert source.size == (360, 360)
    inset = (466 - source.width) // 2
    target_box = (73, 66, 393, 402)
    source_crop = tuple(value - inset for value in target_box)
    work_ambient = Image.open(args.work_ambient).convert("RGBA")
    work_base = Image.alpha_composite(stage, work_ambient).crop(target_box)
    work_light = work_ambient.crop(target_box)
    fade = Image.new("L", work_base.size)
    fade.putdata([min(255, 24 * min(x, y, 319-x, 335-y)) for y in range(336) for x in range(320)])
    for index in range(source.n_frames):
        source.seek(index)
        frame = source.convert("RGBA").crop(source_crop)
        frame = Image.alpha_composite(frame, work_light)
        yield Image.composite(frame, work_base, fade)


for folder in sorted(args.actions.iterdir()):
    if folder.is_dir():
        write_pack(folder.name, idle_action(folder))
write_pack("working", working_frames())
