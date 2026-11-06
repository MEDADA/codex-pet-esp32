# Codex Pet Idle — V3.6 Tail Rig Review

Date: 2026-11-05
Owner: Idle art and motion. **Status: review / not flashed**.
Current installed, frozen, user-confirmed: **Idle V3.0**. No replacement of V3.0 was performed.

## New library files

- `CodexPet/idle/codex_pet_idle_v3_6_tail_rig_review.zip`
- `CodexPet/idle/idle_v3_6_preview.html`
- `CodexPet/idle/IDLE_V3_6_STATUS.md`

## Specific changes

- Extracted the user's approved balanced, fluffy C-curved tail artwork into a 275x270 transparent tail-only source. The rest of that newer character image is reference **only**, not an approved character master.
- Rebuilt a 4-link / 5-joint fixed-length tail chain and weighted inverse-skinning motion. Tail anchored under the original skirt.
- Generated an isolated 466x466 RGBA tail-placement layout reference, editable OpenRaster layers, bone JSON with 275px anchors and 466 conversion, reproducible extraction/rendering scripts and QC assets.
- Produced 8-second idle, flick, curl and raise mobile preview clips, plus 32-second combined demonstration and offline mobile HTML.
- Kept face, hat, eyes, boots and existing clothing outside the tail-area pixel region unchanged in the reconstructed preview. Skirt occlusion at the root remains provisional, with dark-stage underpainting; needs manual hidden fur retouch.
- Exact 8-second periodic bone-motion controls. New tail motion is independent of chest's 4-second breath.

## QA and limits

- PNG alpha/source file verified; ORA internal layer structure and ZIP archives checked.
- 4 bone lengths are constant within <0.001 source px across generated frames.
- MP4 H.264 decoding verified; video previews use 480x480. Target device is 466x466.
- No claim of hand-painted completed tail root / seam-free motion at arbitrary angles.
- No real-device LVGL, PSRAM, FPS, touch or QMI8658 integration was performed by this art workstream.

## Collaboration policy

Only the `idle/` workstream receives new files. Do NOT overwrite existing Idle versions, shared character master, Coding/Working, background, integration, or states. The top-level `PROJECT_INDEX.md` remains untouched until the user accepts this version for integration and the index's current version is re-read.