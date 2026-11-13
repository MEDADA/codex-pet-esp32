# Codex Pet — Working V1.8 Stage 7 Godot Runtime Gate

Date: 2026-11-13
Status: **SOURCE_PREFLIGHT_PASS / GODOT_RUNTIME_NOT_EXECUTED / VISUAL_SIGNOFF_PENDING / DEVICE_NOT_READY**.

## Stage 7 scope
This is an engineering validation handoff, not a new art revision. Approved Working V1.8 timing, blink/gaze, hair rhythm, planted legs and V40 terminal scroll remain the reference. IDLE V3.0 and installed WORKING are untouched.

## Stage 7 package
- `codex_pet_working_v18_stage7_godot_runtime_gate_20261113.zip` (15 MB) is attached in the related ChatGPT conversation; **not committed as a binary to this repository**. SHA-256: `3f77b9e7441dd9c7489723172101618b56c34ae7575eb415d25f430207d5875c`. The available GitHub connector supports text-file commits but has no binary/release-asset upload operation in this session.
- Adds isolated `rig_v18_stage7_mesh_validation.tscn` (enables 8-mesh Polygon2D pilot; default project main scene unchanged).
- Adds `verify_stage7.gd` for 46 bones, 19 animation tracks, path resolution, 8 meshes, skeleton/bone paths, normalized 3-bone weights, 15 animated hair/tail tracks, terminal scroll and fixed legs/boots.
- Adds `capture_stage7.gd` for 10 actual Godot viewport PNGs and a contact sheet (must run without `--headless`). Terminal scroll is sampled at the same timeline timestamp using a test-only manual-time switch; normal runtime remains unchanged.
- Adds source-only `stage7_static_qa.py`, shell runner, CPU-only Stage 6 reference preview and visual review checklist.
- ZIP SHA-256: `e8932f6dd31a3276fb1e0b0a2baf584c83a3cb539328d067c27e19354e0a6c2a`.

## Verified in authoring environment
- 15 Stage 7 source preflight checks PASS: ASCII paths, 25 external resources resolve, 46 bones, 19 track paths resolve, 20s loop, probe scene enables mesh, default main scene unchanged, 8 mesh specs, Polygon2D-relative skeleton/bone paths, rest-pose weights, separate terminal scroll script, deterministic terminal capture, runtime track checks, planted leg tracks.
- Existing static art check PASS: approved 466x466 neutral composite has 0 differing pixels against V45 master.
- Existing independent NumPy geometry check PASS: 8 meshes, 893 vertices, 1482 triangles, 15 mid/tip/tail tracks influence vertices, no sampled triangle inversion.
- Existing `stage3_track_qa.py` still reports 15 tracks inactive in the **default unmeshed scene**; Stage 7 probe deliberately enables the mesh pilot. This is a known scene-mode distinction, not a passed Godot runtime result.

## Not executed / blockers
- No Godot executable is installed in the authoring container. Network DNS is unavailable, so Godot could not be downloaded. No Godot import, GDScript runtime, GPU skinning or actual Godot viewport capture is claimed.
- Stage 5/6 underpaint images remain source-derived drafts, not complete manually painted hidden anatomy.
- Visual seam sign-off, hand/terminal grip and ESP32 memory/FPS/decode testing remain pending.

## Run on the Codex workstation
Use Godot 4.7.2 stable on a disposable project copy, then run:
```bash
GODOT_BIN=godot tools/run_stage7_checks.sh
godot --path godot --script res://scripts/capture_stage7.gd
```
Open `godot/scenes/rig_v18_stage7_mesh_validation.tscn` and run F6. Review `user://stage7_captures/` using `docs/STAGE7_VISUAL_REVIEW_CHECKLIST.md`.

Do not promote to device assets until the Godot screenshots pass visual review and the firmware owner measures flash/PSRAM/FPS/decode performance.