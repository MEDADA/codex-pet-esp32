# Codex Pet / Idle workstream - V3.2 candidate status

- **Latest experimental Idle art:** V3.2 (dual-layer hair + breathing)
- **Status:** `review`, NOT flash-ready
- **Frozen current installed Idle:** V3.0 (2026-11-05, user confirmed on device). DO NOT OVERWRITE.
- **Approved front character:** `CodexPet/shared/approved_front_master.png` (immutable).
- **Persistent scenes:** Idle and Working. Working/Thinking/Coding are one scene; immediate cut Idle↔Working.

## New Library path

`CodexPet/idle/codex_pet_idle_v3_2_hair_breath_review.zip`

## Reviewer files

- `idle_v3_2_interactive.html`: self-contained, switch between V3.0, rear-hair only, inner-curls only and both.
- `preview/idle_v3_2_both.mp4`: 8-second 24fps mobile animation.
- `preview/hair_detail_comparison.mp4`: source/preview crop comparison.
- `preview/hair_rig_overlay.jpg`: rear (cyan) and inner (purple) bone chain influence areas.
- `src/hair_rig_v3_2.json`: six chains and timing.
- `assets/layers`, `assets/masks`: independently extracted art references and masks.

## Honesty and constraints

A two-layer, independently driven, inverse-skinned 2D visual preview is now implemented. **This is not yet a complete independent multi-piece skeletal model ready for the ESP32**. Hidden hair underpaint is necessary before moving separate sprite layers. Face/ears and shoes/status regions are locked in the preview; arbitrary touch+tilt simultaneous runtime is not yet QA'd.

Keep V3.0 and all other workstreams untouched. Do not advance PROJECT_INDEX canonical current installed pointer based on this unapproved review. Codex owns actual device rendering, calibration, performance and flashing.