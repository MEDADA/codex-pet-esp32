# Codex Pet — Idle V3.4 review status

**Date:** 2026-11-05
**Workstream:** `CodexPet/idle/` only
**Latest candidate:** `codex_pet_idle_v3_4_garment_tail_review.zip`
**Status:** `review` (NOT installed / NOT flash-ready)
**Frozen in-use Idle:** `V3.0` — untouched.

V3.4 adds separate left/right silver upper-garment control layers (subtle breathing while retaining complete coverage) and a four-link FK-controlled cat tail, separate from V3.3 hair and skirt. The tail root is masked and has a draft underpaint. All are actual extracted RGBA layers, also available in ORA; animated full body previews are still software prototypes, not native ESP32 rig assets.

Review the `idle_v34_interactive.html` and three independent clips for garment-only/tail-only/combined. Pay close attention to tail root/tip edges and cloth seam continuity. The source player is 480×480 while the device is 466×466.

**Unfinished:** Tail hidden fur repaint and final raster skin-weight polish; independent under-garment geometry not built; real LVGL/PSRAM/FPS/IMU validation by Codex. No other modules or shared master files were replaced.