# Codex Pet — Idle V3.3 hair/skirt decoupling status

- Date: 2026-11-05
- Workstream: Idle artwork / interaction-visual animation only
- Candidate: `CodexPet/idle/codex_pet_idle_v3_3_layer_separation_review.zip`
- Release status: `review`, **not approved**, **not flashed**
- Frozen installed current version: **Idle V3.0** (immutable, user-confirmed)
- V3.2 is retained separately as historical hair breathing prototype.

## V3.3 changes

- 6 independent RGBA hair cards / masks (rear left/right + 4 inner curls).
- 5 separate skirt card assets / masks (stationary base + 4 hem sections); their controller is not tied to the hair breath driver.
- Hair continues to follow respiration with delayed rear/inner phases; skirt has a slower independent swing and folded hem rhythm.
- Separate editable hair/skirt automatic underpaint images and 4 hidden inner-lock root patches with 6px+ motion allowance. They are draft reconstruction, not completed hand-painted occlusion artwork.
- 8s x 24 FPS MP4 views for hair only / skirt only / both, a four-option offline HTML page and zoom detail comparison. ORA source layers and exact offline build scripts included.

## Quality and remaining checks

- RGB video art/foot/hat data locks and sampled pixel QA confirmed in source renderer.
- Automatic underpainting may look soft/haloed in enlarged rotation frames; needs artist review and retouch before final production. Do not imply no seams in every pose.
- The video uses flattened source V3.0 frames and RGBA card extraction, not a wholly finished independently illustrated skeletal puppet.
- No actual ESP32 frame-rate, PSRAM, full IMU/touch interaction stress test, or native LVGL rendering test.
- Do not modify `CodexPet/PROJECT_INDEX.md` installed pointer on this review candidate. Do not overwrite `shared/`, `coding/`, `states/`, `integration/`, `background/`.