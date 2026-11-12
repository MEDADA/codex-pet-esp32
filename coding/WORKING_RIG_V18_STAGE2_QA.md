# Working Rig V1.8 — Engineering Stage 2 QA (2026-11-12)

Status: **STATIC_PASS / GODOT_RUNTIME_NOT_TESTED / DEVICE_NOT_READY**.

The approved V1.8 visual reference is frozen. Stage 2 corrects the engineering source, **not** the approved animation.

## Implemented
- Repaired separated left/right iris and eye-base alpha blending, so a Python RGBA rest-pose composite of all visual layers matches the approved 466x466 master exactly: 0 differing pixels.
- Fixed experimental Polygon2D grid outer contour from scanline ordering to clockwise perimeter, with internal vertices following it.
- Pinned skin weight reference points to bone rest metadata rather than current animated global positions.
- Disabled experimental mesh pilot in the default scene to preserve the approved rest appearance.
- Added Python `tools/static_qa.py` and Godot `scripts/verify_stage2.gd`, plus runbook and SHA-256 file list.
- 12 static checks passed (source file paths, 46 bones, 19 tracks, 20-second loop, visual baseline, 8 mesh topologies, neutral composite, terminal scrolling constants).
- ZIP checksum (SHA256) `df84fffe2f7d383ffc9edd9753624b045a9216508afa7c9d49360f8d6bb976f8`; available from associated ChatGPT conversation download **not uploaded to repository**.

## Mandatory checks before hardware export
1. Run Godot 4 (stable) import: `godot --headless --path godot --editor --quit`.
2. Run `godot --headless --path godot --script res://scripts/verify_stage2.gd` on the extracted project.
3. Capture actual Godot animation and compare against approved V1.8 in high zoom, including iris/eyelids, hair roots and tail.
4. Complete hidden art (neck/sleeves/tablet-back/hair roots) and manually review active bone posing at extremes.
5. Verify two-hand grip/IK and ESP32 memory/decode/frame budget with firmware team.

**Not tested here:** actual Godot importer, GDScript runtime, sprite GPU skinning, joint seam quality or physical device. Do not replace installed IDLE V3.0 or on-device WORKING.
