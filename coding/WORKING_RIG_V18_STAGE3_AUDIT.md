# Working V1.8 Rig - Stage 3 Audit (2026-11-12)

Status: STATIC_PASS / BROWSER_MIRROR_PASS / GODOT_NOT_RUN / RIG_VISUAL_BLOCKED.

## Findings
- Fixed Polygon2D.add_bone() bone path construction: path is now obtained relative to Polygon2D instead of Skeleton2D. This fix still needs a real Godot test.
- Audit of 19 tracks found 15 hair-mid/tip and tail-mid tracks do not affect any visible Sprite2D while experimental mesh pilot is disabled.
- Added verify_stage3.gd to test 46 bones, 19 animation tracks, 8 pilot meshes, relative bone paths, three-bone skin weights, and planted legs.
- Added a self-contained browser inspection: V1.8 approved video compared with a source-driven 2D bone-hierarchy Canvas mirror. Eight browser pose checks ran without JavaScript errors. This is NOT a Godot renderer.
- An optional experimental mesh preview exposes triangle edge defects; it does not establish acceptable art or GPU skinning.

## Remaining release blockers
Actual Godot import and animation render were unavailable in this authoring environment. Skin rendering, hair root hidden art, neck and sleeve occlusion, eye blink overlays, terminal grip IK, and device FPS/flash/PSRAM tests have not been verified. Do NOT flash Stage 3.

## Codex handoff
Stage 3 ZIP filename: codex_pet_working_v18_stage3_rig_audit_20261112.zip (download from the ChatGPT conversation; binary NOT uploaded here).
On the extracted folder run:
    python3 tools/static_qa.py
    python3 tools/stage3_track_qa.py
    godot --headless --path godot --editor --quit
    godot --headless --path godot --script res://scripts/verify_stage3.gd
Then capture 0, 2.4, 7.8, 13.35, 17.55, 19.98 second mesh poses and compare against the approved V1.8 video.

Preserve flashed IDLE V3.0 and existing WORKING. No user-approved visual curves were changed in this patch.