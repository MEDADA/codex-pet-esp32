# Codex Pet Coding V42 — Review Notes

Status: **REVIEW**, not accepted, not hardware validated.

- V41 → V42: audited true dirty-update areas from the unchanged 30-second Pose4 animation. Evaluated 150 reference frames, 5 Hz, 466x466, 16px tiles.
- Every compositor replay using only recorded dirty rectangles was RGB pixel-identical to full reference rendering.
- Mean updated fraction was 29.0436% of a whole frame; worst-case 31.8297%. These are **pixels marked for redraw**, NOT measured QSPI traffic or speed.
- Added 8-sprite alpha-packed eye atlas (128x117) with atlas positions, little-endian RGB565/A8 buffers and precise placement metadata.
- No new anatomy, expressions, content or background. Head remains a broad 2D warp; source eye artwork still needs final visual approval.
- HTML includes no MP4; actual phone and ESP32 device tests pending.
- Old V41 and all other workstreams untouched; root index is not modified.

## Working/Thinking/Coding aliases
2026-11-05 PROJECT_INDEX: thinking, coding, working share one visual scene and must use the SAME uninterrupted animation timeline. Do not reset the working clock for thinking-to-coding alias changes. Only Idle <-> Working is an instant scene cut.