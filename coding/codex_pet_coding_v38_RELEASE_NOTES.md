# Codex Pet — Coding V38 | Review

Current selected visual: **Pose4 / holding_terminal**. This module stays on the approved Pose4 character, unchanged stage, and one scrolling transparent terminal.

## Change from V37
- Independent left/right eye source sprites, iris control masks and **10 separate eyelid keyframe assets** (5 per eye).
- Two-channel reading gaze with slight down-looking focus; coupled head tilt stays within +/- 2.15 degrees.
- Faster eyelid events, including occasional double blink.
- Same continuous 12-second code scroll on the tablet. No finger/thumb movement, per latest user direction.
- GIF, animated WebP and stand-alone HTML designed for iOS cases where MP4 fails.

## Review boundary
The eye sprites are source-derived raster overlays and deformation channels, NOT fully painted independent eyeballs or a finished Live2D mesh. Extreme head yaw, individual eyebrow rig, and hand animation are not included. ESP32 performance, memory and playback require Codex device testing.

## Integration and provenance
- Write only to `/CodexPet/coding`.
- Base art: V37's unchanged approved Pose4 character.
- Background: byte-identical to V37's `stage_dependency_466.png`; readonly.
- Existing historical Coding versions remain untouched.
- Main preview: `mobile_preview.html`, `preview/coding_v38_mobile.webp`.
- `source/coding_v38_eye_layers.ora`: visible stage / character / terminal plus hidden isolated eye sources & eyelid states.
- `source/rig_v38.json`: exact positions and ranges.

STATUS: review. No change to Library project index until user approval.