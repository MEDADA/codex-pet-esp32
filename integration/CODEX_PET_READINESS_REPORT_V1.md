# Codex Pet — Integration & Flashing Readiness Audit

Date: 2026-11-04
Target hardware: Waveshare ESP32-S3-Touch-AMOLED-1.75C, 466x466 circular AMOLED, CO5300 QSPI, CST9217 touch, QMI8658 six-axis IMU, ESP32-S3R8, 8 MB PSRAM, external 32 MB Flash (verify actual chip and partition in firmware).

## Decision

**GO for Codex firmware integration and staged trial flashing. NO-GO for treating the currently uploaded visual ZIP/MP4/HTML files as a final ready-to-flash firmware image.**

This is an audit of ChatGPT Library artifacts and build scripts, not a hardware bench test. Original running firmware source / build environment / partition scheme / production bridge were not present in the Library artifacts audited; Codex should use its existing tested firmware baseline, not replace the driver or BLE protocol with desktop animation prototypes.

## Audited Library snapshots

| Workstream | Exact Library path | Status | Notes |
|---|---|---|---|
| Approved character | `/CodexPet/shared/approved_front_master.png` | Stable shared reference | Preserve exactly |
| Idle | `/CodexPet/idle/codex_pet_idle_v3_0_review.zip` | Review candidate; user states Idle design done, but V3.0 not formally promoted in index | Integrated baseline + 11 action clips; on-device integration pending |
| Coding | `/CodexPet/coding/codex_pet_coding_v34_20261104_review.zip` | Review candidate | User-approved full-back single large screen *direction*, but art masks and rig not fully repainted |
| Background/lightfx | `/CodexPet/background/codex_pet_background_v3_review.zip` | Review candidate | V3 maps lighting to previous **Coding V33 three-screen** coordinates, not to V34 single screen |
| Root index | `/CodexPet/PROJECT_INDEX.md` | Out of date | Describes Idle V1 as available canonical; update after final art approvals |
| Integrated firmware | `/CodexPet/integration/` | No firmware source or device bench output found | Need Codex's existing working project |
| Other states | `/CodexPet/states/` | Empty in Library | Success light envelope exists, but complete Thinking/Error/Sleep final assets not confirmed |

## Structural tests performed on the exact files

- All 3 ZIP archives: CRC test passed, paths safe to extract, no detected corruption.
- Idle V3.0: `python tests/test_controller.py` PASS (priority, latest queued touch, action interruption); `python tests/test_assets.py` PASS (shared master hash, 11 clips, 9 pose images, 12 embedded video sources, ASCII paths).
- Coding V34: `python tests/test_release.py` PASS (8-layer ORA, 120 H.264 review frames, 466x466, ASCII paths).
- Background V3: bundled browser QA JSON reports 19 images loaded, 0 JS errors, pause identical, 3 states; independent on-device/Canvas QA NOT performed in this audit.
- MP4 decode: all three media clips decode fully using ffmpeg; clips are desktop/mobile review movies, not microcontroller playback resources.
  - Idle V3.0 38.5s 24fps 480x480 (mobile demonstration only).
  - Coding V34 10s 12fps 466x466.
  - Background V3 9s 12fps 466x466.
- Base image consistency: `coding_v34/assets/idle_background_v2_9_reference.png` and `background_v3/assets/stage_base_466.png` are byte-identical SHA256 `6e01f9ff6b547660d16805e07429291b812306e53a3e9b1861f986b7bc591d24` (466x466 RGB). **Coding V34's flattened main scene itself is not guaranteed to share this base pixel-perfectly.**

## Integration blockers / important caveats

1. **Version authority:** Idle V3.0, Coding V34 and Background V3 each still advertise `review`; the index is outdated. Obtain explicit release baseline or use these candidates *only for technical trial integration*. Freeze exact hashes and manifests for repeatability.
2. **Lighting mismatch:** Background V3 projects its three cyan light sources on Coding **V33** screens; Coding **V34** is one main screen. Reposition/repaint light masks and verify the target scene before a final release.
3. **Partly flattened Coding art:** V34 background/screen/chair/character overlap in the same original illustration. Head, hands and tail are provisional crops and inpainting; small moves may work, but not complete natural skeletal deformation. Freeze movements that reveal cracks.
4. **Device asset formats not produced:** Idle V3.0 uses desktop MP4 clips and WebP pose references; Coding and Background use H.264/Canvas. Convert to actual firmware-supported static/sprite formats, preserve transparency and coordinate anchors. Do **not** attempt to flash APNG/HTML/MP4 as firmware.
5. **No verified firmware build, flashing, or measurements in these packages:** Need Codex local current firmware project, LVGL/ESP-IDF/Arduino version, display driver, build config, flash partitions, wireless/BLE state protocol, tested touch and IMU calibration, decoded frame cache and board FPS/PSRAM/internal heap profile.
6. **User's scope:** Idle and Coding are the first release; do not silently claim Thinking/Error/Sleep fully done just because a visual prototype once existed. Lightfx Success is a design only.

## First firmware milestone: minimal two-state MVP

A. Back up current successfully flashed board firmware / project and record a reproducible Git commit or local backup. Identify the partition table, flash/PSRAM size and panel driver, and note the original working BLE/Codex bridge.

B. Freeze resource IDs:
- `idle.v3_0.review` as initial *candidate*, subject to owner approval; or ask for the final accepted Idle version if newer than current Library.
- `coding.v34.review` as initial *candidate*, with motion restricted to small safe head/hand/tail offsets.
- `background.v3.review` as a reference; remap its Coding glow to V34's one-screen geometry before enabling.

C. Implement a small `Idle <-> Coding` scene cut without turn-around, preserving status / clock / BLE in the common UI. Add at most low-cost static/glow layers first. Ensure the idle character feet stay fixed and Idle exits cleanly on state change.

D. For assets, favor compressed images in Flash/SD + 1 or 2 cached decoded frame/sprite buffers, and small ROI updates. At 466×466, a single full RGB565 framebuffer is 434,312 B; an RGBA8888 frame is 868,624 B. Two full RGBA frames cost about 1.66 MiB before LVGL, DMA, stacks, fonts or network. Compare exact needs to actual usable PSRAM and flash partitions. Never load whole movie clips decompressed.

E. Incrementally flash and verify on the device: Idle static -> Idle motion -> state cut -> Coding static -> small body motions -> coding screen updates -> ambient lights -> touch/IMU with wireless. Record measured p50/p95 render and flush time, achieved FPS, internal free heap + largest block, PSRAM free + largest block, cold/reboot soak, crash logs and visual artifacts.

F. After stable two-state demo, separately integrate Thinking, Success, Error, Sleep and their event transitions only when their final artifacts and status policy are confirmed. Success light should be a one-shot event, not triggered by an arbitrary ACK.

## Acceptance criteria for release flashing

- [ ] Art owner confirms `idle.v3_0` or publishes final accepted snapshot.
- [ ] Coding V34 and corrected matching lightfx version are signed off together.
- [ ] Original firmware source and working build config are available to Codex.
- [ ] Correct 466x466 layout, touch coordinates, IMU orientation and normal/rotated device cases tested.
- [ ] Device-supported art format and flash storage layout selected; partition free space verified.
- [ ] No full-character vertical drift, seam cracks, UI overlays obscured or major cross-state flash.
- [ ] Stable target FPS and acceptable memory under Wi-Fi/BLE traffic measured on actual board.
- [ ] Firmware binary, rollback backup, tested flashing commands and install log preserved.

## Recommended request to Codex

> Please use the latest snapshots under ChatGPT Library `/CodexPet/` listed here. Integrate into the *existing working ESP32-S3-Touch-AMOLED-1.75C firmware*, not as a greenfield driver rewrite. Start with Idle V3.0 + Coding V34 + static stage, then remap Background V3's V33 three-screen lightfx to Coding V34's one-screen display. Preserve instant cut Idle -> Coding. Convert sprites to actual supported LVGL/firmware formats; do not play the H.264/HTML previews on-board. Build, first flash a minimal test, report real FPS, RAM/PSRAM, flash partitions, display/touch/IMU behavior and rendering artifacts. Do not call this a final release until candidate approvals and tests are complete.