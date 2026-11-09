# WORKING V50 - Seven-hair-layer animation (art review)

Status: **REVIEW ONLY**. This is a new candidate and does not replace any approved on-device version.

## Changes since V49
- Source-faithful 466 x 466 character derived from the established V45 artwork.
- Added independently deformable `hair_fringe`, `hair_left_face` and `hair_right_face` layers.
- Total seven independent RGBA hair layers and 21 root/mid/tip controls.
- 24-second loop for the single WORKING scene; THINKING and CODING are aliases sharing one timeline.
- Source-colored 3px local edge extensions and displacement-aware fill for limited rotations. These are **procedural guides, NOT complete painted hidden hair**.
- Fringe source-edge freeze and immutable hat/ear core guard resolve observed horizontal seams.
- Protected QA sample: unchanged eye, hand, terminal, boot, face and hat/ear cores; no sampled opaque hair core transparency holes.
- Exactly 96 GIF frames / 24,000ms, 144 WebP frames / 24,000ms.

## Art delivery
The V50 source ZIP (~19.5MB), simplified candidate ZIP (~3.2MB), and standalone HTML preview are archived in ChatGPT Library under `/CodexPet/coding/` and made available in the V50 chat response. **These binary files are not uploaded to GitHub by this change**.

Complete art source package contains `source/working_v50_editable.ora`, `source/rig_v50.json`, `source/motion_samples_6fps.csv`, `source/build_v50.py`, 7 sprite layers, local occlusion guides, offline preview GIF/WebP, QA report, baseline references, and Codex handoff notes. All paths are ASCII.

## Limits and gates
Not a complete Live2D mesh. No large head turns, no hand-painted anatomy for fully hidden regions, no native ESP32 performance/PSRAM test, no firmware change and no user art sign-off. Preserve current IDLE V3.0 and V45-V49. Confirm on-device memory/frame budget before replacing anything.
