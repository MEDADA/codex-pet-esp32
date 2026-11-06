# Codex Pet Idle V3.15 - candidate status

- Date: 2026-11-06
- State: **review**, unapproved for flashing. Frozen installed Idle: **V3.0**.
- Package: `/CodexPet/idle/codex_pet_idle_v3_15_expression_rig_review.zip`
- Scope: head/hat/ears/face rigid constraints, V3.13 expression reuse, hair/skirt independent spring controls. Tail geometry kept from V3.12/V3.13.

## Achieved
- 16 scripted segments, 760 frames at 24 FPS, full motion and side-by-side MP4.
- Original V3.13 blink and expression frames remain source; head and eyes are mapped with the same rigid head area.
- Additive head action envelope returns to neutral on every event boundary.
- Six hair groups respond with their own phases; four skirt groups remain on distinct rhythm.
- Locked header, status text, shoes and lower clock region.

## Not achieved / remaining QA
- Expressions and touch body movement are still prerecorded source poses, not independently skinned polygon meshes.
- Head/hair occlusion in extreme angles needs additional painter retouch; raster fields do not eliminate all seam risk.
- Arbitrary live touch/IMU event interruption, actual 466x466 device FPS/PSRAM, and LVGL integration not yet verified.
- Do not advance canonical project index or overwrite frozen V3.0, Coding/Working, shared master, etc.

## QA recorded
- Chromium headless mobile page tested via content injection; 10 event controls and comparison mode worked; no page errors.
- 760 frames at 24 fps for main MP4; all exported previews probed.
- Event head angle returns to neutral at the end of each segment; ear point distance stays invariant analytically.
- Facial closeups require user aesthetic acceptance and manual seam inspection before native conversion.