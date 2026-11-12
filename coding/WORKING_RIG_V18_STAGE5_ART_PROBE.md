# WORKING V1.8 Stage 5 — Texture Edge and Occlusion Art Probe (2026-11-12)

Status: CPU_VISUAL_IMPROVED / STAGE4_CPU_REST_UNCHANGED / GODOT_NOT_RUN / ART_NOT_APPROVED.

## Completed in source candidate
- Generated alpha-safe copies of 8 hair/tail textures; all source alpha and every nontransparent RGBA pixel unchanged. Only transparent RGB near opaque art edges was extended to avoid dark sampling fringes.
- Made 8 local source-colored underpaint layers, constrained to the opaque rest-pose pixels of each source. These are procedural art drafts, NOT artist-painted missing scalp, wrists, neck or torso.
- Fixed regressions in the Godot experimental rig source: clockwise perimeter vertex order, stable rest-pose polyline weights, and relative Polygon2D bone paths.
- Prepared separate optional stage5 test scene and a self-contained offline 8-pose comparison preview.
- In the CPU renderer at t=13.35s, newly transparent pixels relative to Stage4 CPU rest decreased from 165 to 89. Some are legitimate contour movements; this is NOT a strict seam count.
- Stage4 CPU 0s compositing remains pixel-identical. That CPU image includes closed-eye overlay and is NOT the approved V1.8 neutral animation.

## Blockers
- Full hidden art behind fringe/face curls, neck, wrist/sleeve, tablet, skirt and tail root must still be properly illustrated.
- True Godot import, Polygon2D GPU skinning, occlusion and loop QA NOT RUN due missing Godot binary in current environment.
- Godot experimental scene currently tests alpha-safe textures; CPU-specific local underpainting is not integrated into Godot source and must not be represented as a Godot-tested result.
- No ESP32 asset or firmware changes; don't replace active Idle V3.0 or Working.

## Handoff
Patch ZIP: codex_pet_working_v18_stage5_art_probe_20261112.zip
SHA256: cf938835fc5111656126d2d856d15abde07118ecd704c71052def5acb8b11dec
Available in ChatGPT Library: /CodexPet/coding/codex_pet_working_v18_stage5_art_probe_20261112.zip and its conversation download. The ZIP is NOT in this GitHub repository.
Apply onto a separate copy of the Stage4 source archive only. See README_STAGE5.md.
Next gate: manual occlusion paint and Godot engine 466x466 pose sweep; deliver visual signoff before any device export.