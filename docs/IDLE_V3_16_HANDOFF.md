# Idle V3.16 — Codex Integration Handoff

Status: REVIEW / integration candidate only. Not an approved replacement for the installed release.
Target: Waveshare ESP32-S3, 466×466 round AMOLED.

## Source and safety

- Keep installed Idle V3.0, its configuration and backup intact.
- Test V3.16 only in an isolated candidate build.
- Use the immutable shared master from `shared/approved_front_master.png`.
- This archive is visual/animation source material, not a firmware image.

## Candidate contents

- Ambient loop and scripted action previews.
- Repaired tail artwork and four-link tail rig reference.
- Head influence and motion tracks.
- Touch/tilt action clips.
- Per-frame motion and QA reports plus SHA-256 manifest.

## Changes in this candidate

- Ear tips and bases follow common upper-head motion.
- Tail root uses a smoothed continuous pelvis track and a soft C-shaped four-link tail.
- Old detached tail cue lines removed.
- Entry/exit blends improved for touch and expressive actions.

## Integration contract

Exactly two persistent scenes: IDLE and WORKING. Thinking/Coding share WORKING without animation reset. IDLE ↔ WORKING uses an instant cut. Do not add a persistent SUCCESS scene. Maintain existing BLE, touch and IMU interfaces.

No claim is made here about ESP32 FPS, PSRAM use, LVGL integration, rendering stability or flash success; those require device-side validation.
