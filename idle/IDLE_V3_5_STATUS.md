# Codex Pet Idle V3.5 — independent tail activity review

Date: 2026-11-05
Owner: Idle art and motion only
Release status: **review**; new V3.5 candidate, not on-device deployed
Installed, frozen baseline: **Idle V3.0**, absolutely unchanged.

## New files in ChatGPT Library

- `CodexPet/idle/codex_pet_idle_v3_5_tail_motion_review.zip`
- `CodexPet/idle/IDLE_V3_5_STATUS.md`

## V3.5 visual deliverable

- `idle_v3_5_preview.html`: offline, self-contained preview with two video modes.
- `preview/idle_tail_loop_8s.mp4`: 8s base idle motion; 4-link tail, 24 FPS.
- `preview/idle_tail_specials_24s.mp4`: 24s timeline for three low-frequency special gestures (flick, curl, curious lift).
- `preview/tail_closeup_motion.mp4`: magnified tail-only review.
- `preview/tail_motion_storyboard.jpg` and `preview/tail_action_closeups.jpg`: pose and occlusion comparison.
- `assets/tail_source_rgba_480.png`: independent tail cutout with provisional tip fill, not an approved hand-painted sprite.
- `assets/v34_garment_reference.mp4`: unchanged existing garment/hair/skirt baseline dependency.
- `src/build_v35.py` and `src/tail_rig_v35.json`: deterministic offline raster skinning & pose parameters.

## Technical design

Four fixed-length FK links, with the root anchored to the skirt/hip attachment point. The tail has its own 8-second low-amplitude base cycle rather than following the 4-second chest breathing phase. Higher-amplitude gestures are one-shots (not stacked) and include damping. Root masking removes the exposed original tail before placing the new tail sprite. Script validates fixed link lengths and no pose changes outside the tail region. All visual previews are made for 480px mobile view; target display is 466x466.

## Remaining art/firmware constraints

- Hidden tail root fill and inner curled tip source reconstruction remain **provisional**; artist retouch is required to eliminate all dark halos at enlarged or extreme pose edges.
- This is a 2D offline inverse-skinning preview, not a native LVGL skeletal puppet or verified ESP32 renderer.
- Timing in the 24s showcase is illustrative; live random scheduling, touch/IMU prioritization, PSRAM and frame-rate tests still belong to Codex integration.
- Preserve V3.0 installed version, V3.1–V3.4 research history, Shared approved character, Coding/Working, Background, States and Integration unchanged.
- Keep `PROJECT_INDEX.md` installed version pointer at V3.0 until the user explicitly approves replacement.