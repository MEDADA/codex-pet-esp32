# Codex Pet / Idle V3.13 Status

- Workstream: Idle art / animation preview only.
- Candidate: `CodexPet/idle/codex_pet_idle_v3_13_interaction_review.zip`.
- Status: `review`, **not flashed**, **not accepted as the new installed version**.
- Frozen current device version: **Idle V3.0**, do not modify.
- Accepted soft tail shape: V3.12; actual RGBA tail asset and link lengths are identical to the V3.12 source, not a different redraw.
- Coordinate target: ESP32-S3 round AMOLED 466 x 466; browser videos are 480 x 480 previews.

## New work in V3.13

- 4-joint continuous tail spring runs across ambient, head pat, cheek poke, tail tap, hand touch, left tilt, right tilt, curious, shy, shake, and recovery.
- Tail state (`angles` + `angular velocity`) continues across action boundaries and is driven by separate impulses/envelopes for each event.
- Front hair occlusion follows V3.12; accepted C-tail shape unchanged.
- User-approved V3.0 face/head/garment action clips are reused, not independently re-rigged.
- Exported an actual 760-frame (31.667s) contiguous preview, 11-clip mobile selector, visual pose board and source code.

## Verified

- Tail 4-bone link lengths are constant; no frame-level reset at timeline boundaries.
- Tail compositor confined to its intended rectangular ROI; top of head and shoes unaffected by tail rendering.
- Full video and clip decoders, 480px video size, archive ASCII paths, package hashes.
- Mobile HTML controls rendered in headless Chromium with 11 buttons; no script exceptions during selection.

## Outstanding

- Previewed body/face clips use 2D scene blending, not physically continuous full-body bones.
- Tail root seam remains a provisional small V3.11 paint correction, not final all-angle hand retouch.
- No actual IMU/touch sensor event driving, no physical display/PSRAM/FPS benchmark.
- True instant interruption at arbitrary frame and all touch+tilt concurrent events still require runtime validation.

## Source of truth

Keep the immutable shared master unchanged. `PROJECT_INDEX.md` installed version pointer remains V3.0. Do not overwrite any file outside `idle/`. This candidate can be reviewed and the visual language reused by Codex after explicit user acceptance.