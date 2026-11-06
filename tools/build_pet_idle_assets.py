#!/usr/bin/env python3
"""Pack Idle v1 source frames as independent RGB565+A8 blocks in Flash."""
import argparse
import struct
import zlib
from pathlib import Path

from PIL import Image
from build_pet_v11_assets import pack_image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("frames", type=Path)
args = parser.parse_args()
blob = bytearray()
offsets = [0]
for index in range(48):
    with Image.open(args.frames / f"{index:03}.png") as image:
        if image.size != (220, 309) or image.mode != "RGBA":
            raise ValueError(f"Invalid Idle frame {index}")
        pixels = pack_image(image, True)
    # Keep the first frame directly drawable when PSRAM or decoding fails.
    encoded = pixels if index == 0 else zlib.compress(pixels, 9)
    if index > 0 and zlib.decompress(encoded) != pixels:
        raise ValueError(f"Round-trip mismatch at frame {index}")
    blob.extend(encoded)
    offsets.append(len(blob))

target = Path(__file__).resolve().parents[1] / "main/generated/pet_idle_frames.bin"
target.write_bytes(struct.pack("<49I", *offsets) + blob)
print(f"{target}: {target.stat().st_size:,} bytes")
