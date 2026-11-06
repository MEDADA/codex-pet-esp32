#!/usr/bin/env python3
"""Convert the static stage and the Coding V34 single-screen scene."""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter
from build_pet_v11_assets import pack_image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("unpacked", type=Path)
parser.add_argument("preview", type=Path)
args = parser.parse_args()
out = Path(__file__).resolve().parents[1] / "main/generated"
stage = Image.open(args.unpacked / "assets/stage_base_466.png").convert("RGB")
assert stage.size == (466, 466)
coding = Image.open(args.unpacked / "coding_v34/assets/coding_reference_466.png").convert("RGBA")
assert coding.size == (466, 466)
mask = Image.new("L", coding.size)
ImageDraw.Draw(mask).ellipse((4, 4, 461, 461), fill=255)
coding.putalpha(mask.filter(ImageFilter.GaussianBlur(3)))
coding = coding.resize((322, 322), Image.Resampling.LANCZOS)
(out / "pet_trial_stage.bin").write_bytes(pack_image(stage, False))
(out / "pet_trial_coding.bin").write_bytes(pack_image(coding, True))
stage.save(args.preview / "stage.png")
coding.save(args.preview / "coding.png")
