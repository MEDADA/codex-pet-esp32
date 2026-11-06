# Codex Pet Idle V3.12 — Review Status

Date: 2026-11-06
Workstream: `/CodexPet/idle/` only.

## Version lock
- Frozen on-device Idle: V3.0. No overwrite.
- Most recent prior candidate: V3.11.
- New review candidate: V3.12 (soft tail and hair-front occlusion); **not user approved / not flashed**.
- Shared approved front master, Working/Coding, Background, Integration and States are out of scope.

## Delta versus V3.11
- Preserved accepted V3.10/V3.11 C-shaped tail art and 4 bone lengths.
- Explicit V3.3 right-side hair occluder rendered above the tail. Historical tail-contaminated hair masks are geometrically clipped to a conservative real-hair region.
- Tail ambient spring retimed to independently damped root/mid/tip joints, an 8-second two-harmonic rhythm and sparse distal flick; not synchronized 1:1 to torso breathing.
- Kept V3.11 narrow seam retouch; did not redesign face/skirt/hair/boots.
- Preview: ambient 8s, touch-tail 4s, synchronized before/after 8s, GIF, HTML; screenshots and reproducible source.

## Automated checks and limitations
- 192 ambient frames, 96 touch frames, 24 FPS.
- Four-link length error below 0.001 source px; loop pose difference below 0.012 rad.
- Repaint confined to tail ROI; head/ears and shoes in source frame preserved.
- Tail-to-hair matte has non-empty real intersection. Corrected an early test that accidentally clipped the rounded tail tip.
- Source hair masks and tail root seam underpainting are still *visual prototypes* and require 466x466 physical QA.
- Arbitrary IMU and touch preemption, actual LVGL rendering and device FPS/PSRAM remain untested.

Do not modify `PROJECT_INDEX.md` installed-version pointer until explicit user approval.