# Codex Pet — Idle V3.16 Integration Handoff

Date: 2026-11-06
Status: REVIEW / integration candidate only. Not an approved replacement for the installed release.
Device: Waveshare ESP32-S3, round AMOLED 466x466.

## Source of truth

1. Read ChatGPT Library `/CodexPet/PROJECT_INDEX.md` for the authoritative two-scene state contract and module ownership.
2. Read `/CodexPet/idle/codex_pet_idle_v3_16_repair_review.zip`, starting with `README.md` and `docs/IDLE_V3_16_STATUS.md` in the archive.
3. Inspect the installed firmware and **keep its working Idle V3.0, configuration and backup intact**. Use an isolated candidate branch/build; do not overwrite the installed release merely to test this candidate.
4. Consult `/CodexPet/integration/` for existing integration conventions, and `/CodexPet/shared/approved_front_master.png` for immutable character reference. Do not modify other workstreams.
5. If you cannot access ChatGPT Library, request the V3.16 ZIP from the user; never assume a ChatGPT sandbox link is accessible in your own runtime.

## Key archive files (paths relative to archive root)

- `preview/idle_v316_ambient_8s.mp4`, `preview/idle_v316_ambient.gif`: base Idle loop visual acceptance reference.
- `preview/idle_v316_repaired_32s.mp4`: scripted 760-frame action sequence (24 FPS, 480x480).
- `preview/idle_v316_device_466.mp4`: 466x466 scaled **preview**, NOT native firmware frames.
- `idle_v316_interactive.html`: phone/web visual preview, NOT device UI code.
- `src/build_v316.py`, `src/build_ambient_v316.py`: reproducible offline render scripts.
- `assets/tail_rebuilt_full_rgba_295x280.png`, `assets/tail_rig_v312.json`: accepted C-shape tail artwork and 4-link rig.
- `assets/head_influence_v316.png`, `assets/head_angle_v316.npy`, `assets/pelvis_smoothed_v316.npy`, `assets/tail_joint_angles_continuous.npy`: reference mask and motion tracks.
- `assets/actions/`: separate scripted touch/tilt/expressive action clips.
- `tests/motion_per_frame_v316.csv`, `tests/qa_v316.json`, `tests/motion_comparison_760.json`: offline QA evidence.
- `manifest_sha256.json`: package integrity manifest.

## What changed

- Ear tips and ear bases use common full-strength upper-head motion rather than gradient stretching. This is still a constrained raster preview, not true independently painted ear meshes.
- Tail root uses a smoothed continuous floating-point pelvis track and the accepted 4-link soft C-tail, preserving right-side hair in front of the tail.
- Detached old touch-tail cue lines have been removed from the candidate render.
- Longer entry/exit blends for prerecorded expressions/poses, including head-pat, cheek-poke, tail-touch, curiosity, shyness and IMU tilt/shake.

## Device integration request

- Keep **exactly two persistent scenes**: IDLE (standby) and WORKING (thinking/coding share the same scene). IDLE <-> WORKING changes by direct scene cut, not turn-around animation. Do not invent a third persistent success scene.
- Maintain the existing BLE/CodeIsland/state/touch/IMU interfaces. Only replace or add Idle-specific assets / visual animation code in an isolated candidate build.
- Adapt resources for the real 466x466 display and existing graphics framework, based on actual memory/FPS constraints; MP4, GIF, HTML, `.npy` and Python scripts are **not ready-to-flash assets**. Decide whether to preprocess frames, use packed RGB565/A8 sprites, or a practical equivalent; document tradeoffs.
- Preserve foot anchoring, ear silhouette and hat integrity, smooth tail at action transitions, hair-over-tail occlusion and skirt/garment separation.
- Check the known problem frames around F129, F203-225, F277-280, F356, F385 and F682-697 against the candidate references. These frame indices refer to the *offline test reel*, not live device time.
- Test real touch hit areas, tilt in both axes, shake, repeated input and interruption/return to Idle, as well as Idle <-> WORKING scene switching. Measure real-device frame rate, frame-time spikes, PSRAM usage, decoding workload, build size and rendering stability.
- Preserve an exact rollback path to installed V3.0 (firmware/assets/config version and hash or equivalent).

## Acceptance and stop conditions

1. Do not declare V3.16 an installed final release just because its offline videos decode or its numerical QA passes.
2. Provide device photos/videos or capture evidence, performance and memory measurements, a list of visual regressions and a side-by-side check against V3.0.
3. Stop and report if ear tips distort, tail jumps/duplicates, hair-over-tail order is broken, extra lines return, cracks appear in hidden areas, boots drift or screen edges clip.
4. Ask the user to approve actual promotion/flash/replacement after the isolated candidate has passed real-device checks. Do not change `/CodexPet/PROJECT_INDEX.md` or overwrite immutable upstream Library sources without explicit approval.

## Ownership

Codex owns firmware, device asset conversion, BLE, state mapping, real touch/IMU and flash acceptance. The art workstream owns character visuals, corrective artwork, rig design and offline animation previews. Keep this boundary.