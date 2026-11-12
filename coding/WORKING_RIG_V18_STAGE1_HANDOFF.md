# Codex Pet / WORKING Rig V1.8 Visual Sign-off + Engineering Stage 1

Date: 2026-11-11
Status:
- **V1.8 visual rhythm: APPROVED BY USER.**
- **Godot rig engineering candidate: STATIC QA ONLY; not rendered or deployed.**

## Approved visual contract
- 466 x 466 circular canvas and unchanged silver-white-haired cat-ear character.
- WORKING = THINKING = CODING (one persistent scene/timeline); IDLE V3.0 remains frozen.
- 20-second loop, quick gaze movements with maximum 1.5px travel and ~0.22-0.30s saccades.
- Six irregular blink centers: 2.40, 6.15, 10.65, 13.35, 13.70, 17.55 seconds.
- Very subtle head movement, continuous low-amplitude curling hair motion, no body/feet bobbing.
- Grip/tablet motion stays subtle, avoid reintroducing knee tears.
- Legacy V40 procedural terminal code scroll: 4.8px/s and 10-second cycle; independently timed from body animation. **Exact on-device firmware resource is not confirmed.**

## Stage 1 Godot handoff completed locally
- A source ZIP `codex_pet_working_v18_rig_stage1_20261111.zip` was generated in the current ChatGPT conversation (download from the conversation; **not uploaded here**).
- Existing 44-bone Godot scaffold retained; two dedicated iris bones added.
- 20-second `AnimationPlayer` authored with 19 tracks for eye gaze, head, hair chains, tail and blink overlay.
- V40 terminal screen animation source rule implemented as a separate Godot script.
- Visual baseline MP4 included as a read-only reference. All paths ASCII; archive CRC and static track-path/loop tests passed.
- V1.8 was originally raster-baked: head and hair bone tracks in the rig are **approximations**, not source-identical curves.
- Eye splitting is close but **not pixel-exact**. Around 28 visible left-eye pixels and 16 right-eye pixels differ by more than 2 RGB levels. Review as a blocker if noticed.

## Required gates before device export
1. Install/run Godot 4, import the scene and run `verify_stage1.gd`. **Not yet run in the authoring environment.**
2. Visually compare 20-second bone output against the approved MP4, including zoomed closed/open eyes.
3. Finish true hidden-art paint in neck, sleeves, torso/tablet overlap, knee margin and all animated hair roots.
4. Stress test separate joint rotation / weight painting, inspect seams/alpha gaps at 466x466.
5. Complete and test terminal grip/IK before moving tablet/hand bones in rig. Their motion was intentionally left fixed in Stage 1.
6. Let firmware Codex team benchmark packed animation buffers and measured FPS; no firmware update made in this step.

**Do not promote the source candidate to a final device release until gates 1-6 pass.**
