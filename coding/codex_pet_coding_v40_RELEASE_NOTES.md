# Codex Pet / Coding V40 — 30-second natural reading loop

**STATUS: REVIEW** — Coding workstream only. No firmware validation.

## New in V40
- Reuses exact V39 Pose4 holding-terminal character, eye textures, and stage backdrop. No new character generation, no finger movements, no shared lighting changes.
- Extends visual animation from 12s to 30s with irregular, hand-authored reading beats: focused scanning, revisiting a line, brief thoughtful pause, tiny satisfied smile, then resume work.
- Eyes move ahead of small head tilt. Blink times are nonuniform, with an occasional quick double blink. Distinct expression regions activate without whole-body bobbing.
- Synthetic terminal code continues scrolling within the tablet (not live IDE status). Movement and code phase return to neutral at the loop seam.
- Preview prioritized: animated WebP (30s), animated GIF (30s), eight static keyposes, offline HTML pose selector.
- Source: OpenRaster with 3 display layers + 14 **hidden references**; motion curves/notes JSON, Python renderer and QA.

## Known limitations
- Eyes/eyelids and expressions remain retouched 2D raster sprites, not finished Live2D facial anatomy. Head rotation remains sub-2-degree soft warp, not true 3D yaw.
- Neither browser/phone on real device nor ESP32 PSRAM/FPS/LVGL has been verified. Naturalness needs user visual acceptance.
- Static background is a read-only copy from V39, not a declaration that the background workstream is final.

## Collaboration
- V39 and earlier work preserved. Only add V40 candidate under `/CodexPet/coding/`. Do not overwrite Idle, Shared, Background, or `PROJECT_INDEX.md` without acceptance.
- All file, folder, and ZIP-internal paths are ASCII-only.

## Desktop browser smoke test
Chromium headless (430×900 viewport) loaded the animation and all nine static pose controls, switched to Puzzled, and resumed WebP playback without page errors. This is not a physical phone test.