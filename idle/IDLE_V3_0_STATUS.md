# Codex Pet Idle — V3.0 integrated motion review

**Latest workstream candidate:** `V3.0` — **status: review**, 2026-11-04.  
**Accepted visual baseline:** V2.5 Idle + V2.4 faster blink, V2.6 two specials, V2.7/V2.8 four touch interactions and V2.9 IMU gestures, as separately reviewed.  
**This V3.0 integration is not yet hardware certified.**

## Latest Library files

- `CodexPet/idle/codex_pet_idle_v3_0_review.zip`: one self-contained source/preview archive, including `README.md`, `manifest.json`, 11 trimmed visual action clips, base Idle, nine preview orientation poses, local event controller tests, 38.5s movie, and phone HTML.
- `CodexPet/idle/IDLE_V3_0_STATUS.md`: the versioned pointer and collaboration boundary.

## Behavior contract

Idle baseline runs breathing/blink/hair/skirt/tail. Touch overrides special/IMU action; extra touches use one latest-only pending queue. Shake suppressed during touch. Continuous tilt target is retained and shown after the exclusive gesture finishes. Random two special gestures are opt-in and run only while neutral and idle. Idle -> Coding remains an **instant scene cut**, with no turnaround animation.

## No cross-workstream overwrites

`CodexPet/shared/approved_front_master.png` remains unchanged. The Coding, States and Integration folders are owned by other chats. The V2.9, V2.8 and historical V1 Idle releases remain untouched. `CodexPet/PROJECT_INDEX.md` canonical pointer is not upgraded to V3.0 until the user explicitly accepts this integrated version.

## Quality boundary

Source-level controller + HTML tests and MP4 full decode pass. This is a **visual interaction prototype**, not real LVGL/H.264 device playback. QMI8658 calibration, touch mapping, actual memory/FPS and firmware burn-in remain Codex tasks.

## Next after approval

Prepare a limited art resource / event handoff for Codex, validate timing and exact coordinates on 466×466 hardware, and record device-side evidence without modifying other workstreams.