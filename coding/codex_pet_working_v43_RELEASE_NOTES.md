# Codex Pet Working / Coding V43 — Art Handoff Candidate

**Date:** 2026-11-05  
**Owner:** Coding/Working visual workstream  
**Status:** `review` / `art_handoff_candidate` — **not firmware-approved**

## What is actually ready

- Preserves exactly the approved Pose4 seated holding-terminal character and original shared stage pixels. Only Coding/Working eye expression artwork changed.
- Replaces problematic V42 eyelid slabs with per-eye source-derived closing rasters (4 non-open states per eye, 8 RGBA PNGs plus little-endian RGB565 and A8 candidates). No duplicate lash in closed reference frame.
- 30-second reference animation (WebP + GIF), 466x466 eight keyposes, and full, offline HTML with static fallback. Hand motions remain disabled at user request.
- Continues V40 gaze-first, head-following ~1.6-degree soft warp, subtle thought/smile events and terminal scrolling. Not new head anatomy / Live2D skeletal mesh.
- Neutral t=0 frame is pixel-identical to V40 neutral. t=0 and t=30 agree, and 300 RGB565 reference frames replay pixel-exact in the optional zlib tile prototype.
- Device handoff includes the neutral RGB565, crop RGB565+A8, eight per-eye sprite pairs, working alias contract, 5-second **diagnostic-only** tile stream, source render scripts and QA results.

## Performance and format boundary

- Full 30s / 10fps experimental zlib tile stream: **18.44 MiB**, too heavy to assume it should ship on a 32MB flash device. It is **not the default** handoff, not native LVGL, and actual zlib decoding + display performance is unmeasured.
- 5s tile diagnostic is included only for Codex decoder bring-up; it does not substitute for a compact working animation system.
- Head motion uses a soft masked raster warp; no fully isolated head meshes or concealed-area hand-painted art exist. Do not claim GPU/LVGL compatibility without actual implementation.
- Stage and background lighting are external dependencies; this package cannot overwrite shared/background or idle versions.
- Thinking / Coding / Working are exactly one persistent WORKING scene; alias transitions preserve animation phase.

## Files

- `preview/mobile_preview.html` — completely offline mobile review
- `preview/coding_v43_30s_mobile.webp` and `.gif` — 30s visual review
- `preview/blink_before_after.jpg`, `preview/eight_pose_review.jpg` — quality review
- `source/render_v43.py`, `source/build_v43.py`, `source/build_motion.py` — reproducible source
- `source/coding_v43_eye_keyposes.ora` — editable keypose guide, hidden layers are references
- `source/sprite_manifest.json` — pixel-accurate cropped sprite locations
- `device/` — little-endian RGB565+A8 + optional five-second tile decoder sample
- `qa/` — automated review results

## Handoff criterion

Visual-art integration **candidate ready for Codex trial**. **Not** final user artwork approval nor hardware acceptance. Codex validates RAM, FPS, display flush, color and scene cut; do not promote PROJECT_INDEX version pointer until acceptance. All new paths ASCII-only.