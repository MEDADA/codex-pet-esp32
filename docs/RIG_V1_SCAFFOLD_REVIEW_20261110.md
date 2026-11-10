# Rig V1 scaffold review — 2026-11-10

Status: **source scaffold generated; rig NOT accepted and NO new animation**.

## Art and source baseline
- 466x466, V45-approved source image SHA-256: `62186f7e349d25df7103e48949e3f5b6bea53f1b059a5c952d7f956537f5fbd3`.
- 19 visible layers from prior V50 ORA recompose to exactly the approved V45 rest image: **0 changed pixels**.
- New Godot 4 scaffold: 44 nested Skeleton2D/Bone2D joints; 19 layer-to-bone attachments; trial 3-bone Polygon2D grid skinning for 7 hair groups and tail. These are initial procedural weights, **not accepted painted weights**.
- Terminal left/right grip markers are authored, but there is no working two-hand constraint/IK.

## Blocking gaps
Five P0 occlusion-paint blockers: head/neck, left wrist, right wrist, underneath terminal, and hair roots. Two P1 blockers: tail root and independent iris/eyelid artwork. Existing layers are cut-outs and lack sufficient hidden art to pass full pose sweep.

## Test status
PASS: source SHA and pixel-exact rest composition, Python structural checks, ASCII-only filenames and ZIP CRC.
NOT TESTED: actual Godot engine import/compilation/runtime, joint stress poses, reliable silhouette and seam checks, device performance. Godot binary was not available in the generation environment.
The source contains `godot/scripts/validate_scene.gd` and `docs/TEST_COMMANDS.md` for Codex to run desktop validation.

## Storage
Full zip (including the Godot source scene, source PNGs/ORA, GDScript, bone map, offline HTML structural review, overlays, QA): ChatGPT Library at `/CodexPet/shared/rig_v1/codex_pet_rig_v1_scaffold_20261110.zip`, and downloadable from the ChatGPT conversation. **Binary ZIP is not uploaded to GitHub by this change.**

## Next gate
1. Run Godot desktop structural checks.
2. Paint hidden body/face/wrist/scalp/tail art and split irises/lids.
3. Refine mesh topology/weights, run individual +/- joint sweeps and check seams.
4. Implement terminal-grip constraints.
5. Only then begin Working animation clips.

Installed IDLE V3.0 and approved on-device WORKING are frozen; no firmware changes. Thinking and Coding remain aliases of one WORKING scene.
