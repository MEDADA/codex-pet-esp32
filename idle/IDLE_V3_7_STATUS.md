# Codex Pet / Idle V3.7 tail-root & interaction-art candidate

- Date: 2026-11-05
- Status: `review` — not approved for flash, not a hardware-tested production asset
- Frozen installed Idle: **V3.0**, unchanged
- Latest tail art baseline: V3.6 approved C-shaped texture and 4-link skeleton
- Workstream: `/CodexPet/idle/` only

## Library candidate

- `/CodexPet/idle/codex_pet_idle_v3_7_tail_root_interactions_review.zip`
- `/CodexPet/idle/IDLE_V3_7_STATUS.md`
- `/CodexPet/idle/idle_v3_7_interactive.html`

## Contents

- V3.6 -> V3.7 root-image before/after, old-tail halo removal and hair/skirt root occlusion matte.
- Still-accepted 275 x 270 tail PNG, previous target scene video, masking reference, joint rest coordinates.
- Four independently rendered 8 s x 24 FPS MP4 actions: base Idle, touch tail, head pat tail response, simulated left-right tilt tail response.
- 32 s concatenated review clip, compact GIF, 4-pose storyboard, self-contained offline HTML with zoom and reaction selection.
- Frame-by-frame per-link joint offset arrays, 4 fixed-length bone constraints, reproducing source scripts, README and Codex art handoff.

## Validation and unresolved risks

- Automated checks: 4 × 192-frame MP4 decode, 480px video frame size, 4 fixed rest bone lengths, command-loop settling, ASCII path layout.
- The old ghost arc is greatly reduced in the review, but root connection and reconstructed dark background still need manual visual retouch for all poses.
- Touch/IMU events are **simulated offline on the tail only**: no live ESP32 QMI8658 readings, no physical touch coordinate mapping, no runtime cross-action bone blending, no proven native LVGL renderer.
- Preview file switching can reset pose; do not use it as evidence that interruptions are seamless.
- The original full-body mother plate (`shared/approved_front_master.png`) was not replaced, and the alternate illustration used to source the tail must not replace it.

## Next

Review tail-root matte at enlarged size, tail touch reaction amplitude, and left/right counterbalance. If approved, integrate current-bone-angle continuity with Idle touch/tilt controller and then let Codex perform the on-device render and memory tests.

Do not update the canonical installed pointer in `PROJECT_INDEX.md` until separately approved.