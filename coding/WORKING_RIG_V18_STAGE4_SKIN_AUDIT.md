# Codex Pet WORKING V1.8 - Stage 4 Skinning Audit (2026-11-12)

Status: SOURCE_GEOMETRY_PASS / VISUAL_OCCLUSION_FAIL / GODOT_NOT_RUN / DEVICE_NOT_READY.

## Results
- Stage 4 keeps the approved V1.8 20-second visual reference unchanged (blink/gaze/hair/terminal).
- Experimental 3-joint skin weights now use a root-locked polyline projection with smooth mid/tip blending.
- 8 skin meshes, 893 vertices, 1482 triangles; checked 9 sample times.
- All 15 previously inert mid/tip animation tracks individually produce nonzero vertex motion in the NumPy model when mesh skinning is enabled.
- Zero inverted triangles at tested sample times. This is mathematical mesh-vertex QA, NOT a Godot rendering pass.
- 12 source/static validations pass; neutral asset composite equals approved source in the original draw-order test.
- CPU textured skinning reveals 198 newly transparent pixels at t=13.35s relative to CPU rest composite. Browser triangle-skinning mirror shows edge aliasing. Occlusion art is still incomplete.
- Browser 2D prototype had no JavaScript page errors at six key times; neither browser nor CPU preview proves Godot GPU skinning.

## Source, tests, and review
- Stage 4 source archive: codex_pet_working_v18_stage4_skin_probe_20261112.zip, linked in the corresponding ChatGPT conversation. Binary is NOT uploaded to this repository.
- Files: godot/scenes/rig_v18_skin_probe.tscn, godot/scripts/verify_stage4.gd, tools/mesh_influence_qa.py, preview/working_v18_stage4_weight_probe.html, qa/STAGE4_GATE_REPORT.json.
- Canonical rig_v18_engineering.tscn keeps experimental mesh DISABLED; separate skin-probe scene enables it.

## Next blocking gate
Codex workstation must run:
    python3 tools/static_qa.py
    python3 tools/mesh_influence_qa.py
    godot --headless --path godot --editor --quit
    godot --headless --path godot --script res://scripts/verify_stage4.gd
Capture real Godot frames, paint missing hidden hair/root, neck, sleeve and tablet-back layers, and compare visually at 466x466 and 4x. Do not use automatic full-character underlay as a substitute: it ghosts the eyes/hands.

V1.8 visual acceptance is not equivalent to current Godot rig acceptance. Do not flash this prototype, replace installed IDLE V3.0 or overwrite active WORKING.