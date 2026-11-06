# Codex Pet — Idle V3.16 frame-audit repair candidate

Date: 2026-11-06  
Scope: Idle animation artwork and preview, not ESP32 firmware.  
**Installed / frozen V3.0 remains unchanged.**  
This is a *review candidate*, requiring human visual acceptance before any integration.

## What changed versus V3.15

1. **Ears and hat:** Removed the previous spatially varying remap through the upper ear silhouette. Both ear tips and their lower attachment points are in a full-strength common head transform; reference face expression clips remain intact. Maximum additional roll: 2.0°. This does *not* constitute a fully retouched separated ear sprite/Live2D mesh.
2. **Tail jump:** The accepted V3.12 four-link white C-shaped tail is re-skinned from the raw person pose frames, never from a rendered frame that already contains the new tail. The pelvis parent is a continuous floating-point track smoothed over event boundaries (no per-frame integer root snapping). The tail is rendered after the upper-head pose and beneath skirt + right hair foreground.
3. **Legacy touch-tail cue lines:** Explicit connected-component removal only in the background on touch-tail frames, leaving the furry tail and skirt intact. Removed 77 short detached cue components across the clip by the specified detector. The old FX is still present in the immutable V3.0 source assets; only this candidate removes its projected result.
4. **Pose transition:** Event entry and exit blending extended from 6/8 to 12/15 frames to avoid abrupt source-body changes. No new facial image was generated.

## Quantitative checks (full 760-frame video; 480x480, 24 FPS)

| Metric | V3.15 | V3.16 | Context |
|---|---:|---:|---|
| Ear ROI maximum adjacent-frame mean absolute RGB change | 11.218 | 9.604 | Lower is smoother, but not a contour-geometry proof |
| Tail-root ROI maximum adjacent-frame mean absolute RGB change | 12.004 | 7.592 | Same event sequence and spatial ROI |
| Tail ROI 95th-percentile RGB difference | 2.833 | 1.610 | Per-frame value |
| Ear / tail-root frames exceeding mean RGB change 10 | 3 / 6 | 0 / 0 | Threshold-based review heuristic |
| Pelvis parent raw / smoothed maximum adjacent-frame step | 1.414px raw | 0.551px smoothed | In 480px preview coordinates |
| Tail joint-length maximum numerical error | — | 0.0000153 source px | Ideal constraints, not a visual-art verdict |
| Tail maximum angle difference between adjacent frames | — | 0.01374 radians | From V3.13 continuous joint sequence |

**8-second ambient loop:** 192 frames; tail angle endpoint deviation 0.00093 rad; root loop endpoint deviation 0px; decoded first-last frame average RGB difference 1.38 levels. These check continuity, not a real-device FPS.

High risk frames rechecked: F129, F203, F209, F278, F280, F356, F385, F660, F683, F697 and transitions.

## Outstanding limits

- Face/body inputs are still *previously approved raster poses*, not separate true ear/hat meshes with manually painted hidden roots; a raster head influence region still exists below the rigid upper assembly.
- Some inherited tiny seams/anti-aliasing in hair, skirt and tail junction remain plausible and need user visual review at 466x466 and 2x crop.
- The test reel is scripted, not live unpredictable simultaneous sensor/touch events. Codex owns real IMU/touch interruption tests, LVGL/raster conversion, PSRAM and FPS measurements.
- Do not interpret numerical ROI changes as a guarantee of perfect art. A smaller difference can also reflect restrained motion.

## Reproduction

Run `python src/build_v316.py`, then `python src/build_ambient_v316.py`, then `python tests/compare_motion.py` in the extracted package root. Requires Python, NumPy, OpenCV, Pillow, SciPy, Matplotlib and ffmpeg. Output paths and archive members use ASCII-only names.

No changes to `CodexPet/PROJECT_INDEX.md`, immutable shared master, Working/Coding, Background, States or Integration. New candidate is review-only under `CodexPet/idle/`.