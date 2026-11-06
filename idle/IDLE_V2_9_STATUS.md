# Codex Pet Idle — current motion-review snapshot

**Latest Idle art review:** `V2.9`  
**Status:** `review` — user motion-art acceptance and ESP32 hardware checks pending  
**Owner:** Idle artwork, Idle local actions, 466x466 preview, touchscreen and 6-axis motion *visuals*  
**Date:** 2026-11-04

## New Library resources

- `CodexPet/idle/codex_pet_idle_v2_9_review.zip` — latest review-only artifact with the stand-alone `idle_v2_9_interactive.html`, five IMU motion MP4 clips, combined preview video, source scripts, specification, and four V2.7/V2.8 touch example clips.
- `CodexPet/idle/IDLE_V2_9_STATUS.md` — this pointer and workstream state.

## Prior versions retained

- `CodexPet/idle/codex_pet_idle_v2_8_review.zip` — previous review baseline with 4 touch actions and 2 special Idle actions; **not overwritten**.
- `CodexPet/idle/IDLE_V2_8_STATUS.md` — previous status, retained.
- `CodexPet/idle/codex_pet_idle_v1_handoff.zip` — historical canonical Idle integration handoff, still not hardware validated.
- `CodexPet/shared/approved_front_master.png` — unchanged. The V2.9 renderer consumes the already accepted V2.5 movie rather than repainting the character.

## V2.9 scope and quality

- L/R tilt, F/B nod, gentle shake and inertia-like hair / skirt / tail responses in a *visual simulator*.
- Top status/header and lower shoes/clock region pixel-locked in generated preview frames. No full-body bobbing.
- Offline HTML slider and touch-button review, optional phone DeviceOrientation API input with user permission; no ESP32 communication.
- Manual review and physical IMU axis calibration, sensor fusion, reaction thresholds, playback memory/FPS and BLE-Wi-Fi concurrency testing are still outstanding; these are Codex integration tasks.
- Immediate visual switch between Idle and Coding remains unchanged. No turn-around animation.

## Collaboration constraints

Only add future files under `CodexPet/idle/`. Do not modify `coding/`, `states/`, `integration/`, or `shared/`. Do not alter `CodexPet/PROJECT_INDEX.md` canonical pointer until the new version is approved and the current Library version is re-read. All new filenames and ZIP-internal paths are English ASCII.

## Next visual review

1. User reviews orientation sensitivity, forward/back nod, shake intensity and tail recovery in the mobile page/video.
2. Refine motion amplitude/artifacted hair joins if needed.
3. After user approval, prepare a slimmer firmware-targeted art delta resource description for Codex (hardware integration stays separate).