# Idle workstream — current review snapshot

**Release:** `V2.8`  
**Status:** `review` (not approved for firmware / not ESP32 tested)  
**Owner:** Idle art and interactive-motion workstream  
**Created:** 2026-11-04

## Current Library artifact

`CodexPet/idle/codex_pet_idle_v2_8_review.zip`

Open the ZIP's `README.md` and `preview/idle_touch_interactive_v2_8.html`, followed by `preview/idle_base_v2_5.mp4`, `preview/idle_specials_v2_6.mp4`, and the four `preview/touch_*.mp4` files.

## Version pointer and status boundaries

- Idle **current visual review**: V2.8 (four touch interactions: head, cheek, hands, tail).
- Idle **previous canonical handoff**: `CodexPet/idle/codex_pet_idle_v1_handoff.zip` — preserved unchanged.
- `CodexPet/shared/approved_front_master.png` — shared approved art, preserved unchanged.
- Coding and other project modules — owned by their separate workstreams, untouched.
- Faster V2.4 blink and V2.5 core Idle are the adopted visual baseline. V2.6 curiosity/shy and V2.7–V2.8 reactions are preview-only implementations, not certified final sprite rig.
- IMU six-axis artwork and real touch input mapping are the next Idle tasks.
- Coding transition is a direct scene cut (turnaround canceled).

## New version protocol

Do not overwrite this archive or any other module. Future Idle changes should create `codex_pet_idle_v2_9_review.zip` or a unique higher version and a new versioned status note. Update the top-level `PROJECT_INDEX.md` canonical pointer only after the user approves the new version and after checking its latest current Library version.