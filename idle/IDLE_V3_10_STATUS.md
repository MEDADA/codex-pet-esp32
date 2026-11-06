# Codex Pet Idle V3.10 — status

- Status: **REVIEW**. V3.0 frozen and installed; V3.10 not installed.
- Goal: structural replacement of V3.9 tail-root source and occlusion, not a superficial recolor.
- New real asset: `assets/tail_rebuilt_full_rgba_295x280.png` extracted from the already accepted `assets/accepted_shape_reference.png` (not from the contaminated V3.6 tail texture).
- Editable layers: `assets/tail_v310_layers.ora` (tail texture and hidden root/rig guides).
- Rig: 4 fixed-length links, parent `body_pelvis` tracked with ±6px bounds in offline preview.
- Preview: four 4s source-pose clips + 16s showcase at 480px, 24fps; 466px still. Includes before/after root closeups.
- Known art issue: fine edge/contrast at the skirt-root overlap remains visible at 4× zoom. Final artist-clean occlusion and physical device acceptance pending.
- Known technical issue: not native ESP32 assets, not full live character rig; independent device input/interruptions not verified.
- Shared index: do not change installed current pointer.
- Library target: `CodexPet/idle/codex_pet_idle_v3_10_tail_root_review.zip` and versioned status/interactive preview. No other module edits.