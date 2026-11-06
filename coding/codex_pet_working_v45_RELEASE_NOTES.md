# V45 - Working character restoration & technical rig candidate

Status **REVIEW**. This new candidate replaces the *look* of failed V44 pixel retouches with the latest explicitly user-approved rendered character image; it is not a V44 in-place overwrite.

- 466x466 neutral pose from original 1254x1254 image, black matte excluded from sprite.
- No X hair clip; exact silver-white hair, beret badge, black ruffled boots from approved design.
- New 14-region ORA partition file; neutral reconstruction pixel exact to extracted sprite.
- 19 planned pivot points in `rig_v45.json`, with explicit non-ready anatomical underlays.
- 30 second low-amplitude head-motion and terminal-code proof; old eye/hand animation not carried over.
- Self-contained mobile HTML, 30s animated WebP and GIF, static grid, transparency on checkerboard QA.
- Cropped RGB565+A8 binary candidates and manifests for Codex; not native LVGL files and not tested on hardware.
- User validation pending. Previous V44/V43 and all other workstreams unchanged.