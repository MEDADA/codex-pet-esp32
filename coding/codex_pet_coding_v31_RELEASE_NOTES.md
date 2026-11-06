# Codex Pet — Coding V31 Library Candidate

**Status:** `review` (NOT approved, NOT device-tested)  
**Owner:** Coding workstream only  
**Library path:** `/CodexPet/coding/codex_pet_coding_v31_20261104_review.zip`

## Summary
Snapshot of the latest Coding deliverable from this chat, retaining the approved existing rear/side silver-haired cat-ear character, dark provisional stage and **three blue holographic monitor layers**. Independent code/content and light-effects layers, 11-layer OpenRaster, 466x466 mobile preview and 8-second H.264 MP4. This is not the old purple multi-monitor workspace.

## Shared dependencies and boundaries
- Read `/CodexPet/PROJECT_INDEX.md` before using this package.
- Shared official character reference: `/CodexPet/shared/approved_front_master.png`. Its observed SHA-256 is recorded in `library_release.json`; this module does not overwrite or duplicate the authoritative Library object.
- The Coding source back/side artwork was supplied by the earlier V18 project and remains intact (`source/approved_back_reference_1254.png`). It has not been re-drawn to independently rig a face, hands or tail.
- `assets/shared_idle_background_466.png` is **provisional** sampled dark background, not evidence that the latest independently developed Idle background has been imported. Keep it replaceable.
- No files in `/CodexPet/idle/`, `/CodexPet/shared/`, `/CodexPet/states/` or `/CodexPet/integration/` are to be mutated as part of Coding iteration.

## Practical next step
Obtain the current Idle background / stage base via `/CodexPet/shared/` and confirm identical round-screen alignment, then redraw physically correct Coding hand/thumb/wrist key poses with hidden-area paint. Build independent head/arm/tail layers and assess motion at actual sprite scale. Do not simply rotate the full character image.

## Validation
- 466x466 PNG and 8-second 466x466 H.264 review clip.
- 11 OpenRaster edit layers, ZIP structure and source asset validation.
- No ESP32-S3 RAM/FPS or LVGL claims: Codex performs firmware integration and actual measurements.
- Package/internal paths use English ASCII only.

## Candidate versioning
Create a **new** numbered Coding file for future iterations; do not silently overwrite this review snapshot. Update root `PROJECT_INDEX.md` only upon approval and after checking the latest Library version, per project rules.