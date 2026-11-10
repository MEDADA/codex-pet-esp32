# Codex Pet — Skeletal Animation Architecture V1 (Proposed)

Status: TECHNICAL DESIGN PROPOSAL ONLY. No V52 character rig or animation has been built. No approved firmware or assets are replaced.
Date: 2026-11-10
Target: Waveshare ESP32-S3-Touch-AMOLED-1.75, 466x466, 8MB PSRAM, 16MB flash. Firmware owner validates actual free memory and frame bandwidth.

## Decision
Use **Godot 4 desktop authoring only**, with Skeleton2D/Bone2D hierarchy, weighted Polygon2D UV mesh, AnimationPlayer and AnimationTree, transparent PNG-sequence export, then offline encode into per-clip, cropped, compressed device assets. **Godot does not run on the ESP32.** Spine Professional is a commercial authoring alternative, not an implementation dependency.

This solves authoring reusability and prevents faking joint movement by rotating pre-flattened crops. It does not automatically invent hidden paintings or guarantee MCU playback; these require explicit work and gates.

## Non-negotiable art input
- Start from `shared/approved_front_master.png` and the accepted source proportions. Use V45-V51 only as references, not a deformed, canonical base.
- Rebuild **occlusion-complete** RGBA art layers. Behind hair: full scalp, ears and face. Behind collar: neck and shoulders. Behind hands/tablet: sleeves, wrists, torso and tablet grip. Behind tail: true root attachment. Fill unseen pixels with painted source art, **not** cloned edge smearing.
- Preserve editable, ordered source layers and opacity. Expose normal alpha masks and overdraw. All file/directory names ASCII. Avoid side views and walk-in transitions.
- Reconstruct reference/neutral pose exactly enough for visual review before motion.

## One reusable rig, many animation clips
Suggested logical bones (final bone count decided only after art review):
- `root`, `pelvis`, `torso`, `chest`, `neck`, `head`, `face`
- `ear_l`, `ear_r`, `eye_l`, `eye_r`, `lid_l`, `lid_r`
- `hair_fringe_root/mid/tip`, `hair_left_face_root/mid/tip`, `hair_right_face_root/mid/tip`; plus four left curl chains; back-hair chain as needed
- `shoulder_l`, `upperarm_l`, `forearm_l`, `wrist_l`, mirrored right side; `terminal` with left/right grip targets
- `tail_root`, `tail_mid`, `tail_tip`; `skirt_l`, `skirt_r`, feet/boots fixed in planted poses

All independently moving textured pieces are either (a) rigid attachments with joint pivots and overlap, or (b) properly weighted meshes. Never move isolated crop boundaries that have no painted occlusion.

For handheld tablet: animate `terminal` as the intended target and constrain both wrist targets to grip sockets; two-arm IK is a candidate but must be tested in the chosen Godot release (Godot 2D IK API may be experimental). Lower arms/sleeves follow rather than passing through the terminal.

For eyes: separate iris mesh/pupil masks under eyelids, both parented to head. For head: neck-weighted mesh and scarf/collar overdraw; all hat, ears, front and back hair inherit head motion plus secondary-chain motion.

## Clip contract
Persistent visuals remain exactly two: `IDLE` and `WORKING` (thinking/coding same active timeline), plus one-shot `SUCCESS` event. `IDLE` installed V3.0 is frozen. Keep shared background/light separate.

Initial clip library:
- `working_loop`: read, scan left/right, operate terminal, brief focused pause; no overall bobbing
- `working_blink`: blink overlay/track
- `working_success`: one-shot from current WORKING pose, with reset/return point defined
- `idle_loop_candidate`: preserve approved installed Idle; future separate candidate only
- `look_left`, `look_right`, `terminal_tap`, `tail_sway`: reusable additive tracks where supported

Animations are keyframed on the rig, never image-crop transforms. Establish common rest and return poses to avoid transition jumps. Facial movement must remain perceptible at native 466px, not only in zoomed previews.

## Desktop/device boundary and frame budget
One raw RGB565 466x466 frame is 434,312 bytes (~424KiB). At 15fps for 24s, uncompressed frames alone consume 156,352,320 bytes (~149MiB), before other art/system data. This is impractical for direct flash storage. Do not ship all raw full frames.

Export transparent frame sequences at 466x466, then encode *by changed region or by small sprite clip* with key frames, variable rectangles, alpha, indexed/local palette or compressed RGB565 as measurements allow. Keep static background and terminal screen effects separate. Evaluate changed-rectangle playback against existing firmware rather than imposing a renderer rewrite. Full animations can be baked, while later real-time procedural tilt would require a separately benchmarked runtime bone/attachment subset.

No FPS, memory or compression saving claims may be treated as verified without profiling on the actual firmware/hardware.

## Implementation gates
0. `ART_FREEZE`: reassemble canonical rest-pose art and obtain visual approval.
1. `OCCLUSION_COMPLETE`: every seam has painted underlayers; evidence images for + and - extreme poses, no black gaps or exposed crop edges.
2. `RIG_PROOF`: pose sweep head +/-7deg, wrists +/-10deg, tablet +/-7deg, hair +/-12deg, tail +/-15deg (proposed diagnostic limits, shrink to art quality if necessary). Weight/UV topology, face and hands must not tear. Test each bone independently, not just joint motion.
3. `MOTION_PROOF`: eye tracking, head movement, both-hand tablet grip are clearly visible on a native 466px round preview; end/start match; no root drift or shoe movement.
4. `EXPORT_PROOF`: reproducible export from the same Godot project with ASCII paths and versioned artifacts, native PNG and offline preview.
5. `DEVICE_GATE`: firmware team checks flash budget, PSRAM, compressed decode speed, measured FPS, no BLE/input regression.
6. `APPROVAL`: only approved review candidate can become a device update. Never overwrite installed IDLE V3.0 or currently flashed WORKING.

## Expected source layout
```text
shared/rig_v1/
  master/
    character_layers/
    occlusion_paint/
  godot/
    project.godot
    scenes/character_rig.tscn
    animations/
    scripts/
  manifest/
    bone_map.json
    art_provenance.json
    constraints.json
  qa/
    pose_sweep/
    seam_report.json
    motion_review/
coding/releases/working_v52_candidate/
  previews/
  baked_clips/
  export_manifest.json
```
This is an intended layout, not a claim those files exist.

## Source references
- Godot 2D skeleton / UV weighting: https://docs.godotengine.org/en/4.4/tutorials/animation/2d_skeletons.html
- Godot AnimationTree: https://docs.godotengine.org/en/stable/tutorials/animation/animation_tree.html
- Godot movie writer: https://docs.godotengine.org/en/stable/tutorials/editor/command_line_tutorial.html
- Godot MIT license: https://godotengine.org/license/
- Spine meshes and export: https://en.esotericsoftware.com/spine-meshes ; https://en.esotericsoftware.com/spine-export
- Board specs: https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.75
- LVGL image transform constraints: https://lvgl.io/docs/open/widgets/image
