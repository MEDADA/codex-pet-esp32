# Task for Codex: integrate new approved Idle animation

User decision: **skip all turnaround animation**. Idle -> Coding is an instant state cut to the *already-existing* rear-view Coding scene. Coding -> Idle is an instant cut back. Do not add an interim turning pose, dissolve or bobbing transition.

## Requested work

1. Import `assets/idle_background_466.png` only if it fits the existing dial. Existing product status/clock widgets are authoritative; preview labels/time are placeholders.
2. Convert `frames/000.png` ... `frames/047.png` (220×309 RGBA) to the firmware's supported flash image format. Display with top-left origin `(123,72)` on 466×466 canvas. Loop at 12 FPS / 4 seconds.
3. Do **not** move the whole sprite vertically; source has locked boots and fixed top hair/ears. Do not split long hair into independently rotated pieces.
4. Prioritize stable frame streaming; never preload all 48 decoded sprites into PSRAM.
5. Map Idle -> Coding and Coding -> Idle to an immediate visual cut. Do not change existing bridge/network/touch logic.
6. If 12 FPS exceeds actual device resources, use `assets/idle_base_220x309.png` as an intentional still fallback, then benchmark smaller cached frame intervals.

## Verification

- Verify no boot movement at any animation frame; boot rows are pixel-identical in source.
- Visually inspect the blink at source frames 029–031. Eye retouch is only a procedural draft.
- Verify no white hair seam or dark alpha halo on the actual AMOLED.
- Real clock/status text must be generated from current firmware state, not rendered demo time 09:29.
- Record actual decoded memory, frame time, visual consistency and error logs after integration.

Do not treat the preview movie as firmware or benchmark evidence. This scope contains **Idle only**. A separate rear Coding scene is not included or approved by this artifact.