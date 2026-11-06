# Codex Pet / Idle V3.15 — 760-frame defect audit

Date: 2026-11-06. Type: diagnostic only, **no animation or art replacement**. Frozen installed: **V3.0**. Examined candidate: **V3.15**. Playback: **760 frames / 24 fps / 31.667 seconds / 480 × 480**. Physical target: 466 × 466.

## Scope and method

- Read original `src/build_v315.py`, `tests/qa_v315.json`, V3.13 tail renderer `src/render_v313.py`, `src/render_v312.py` and `tail_v310_base.py`.
- Decoded **every frame** of V3.13 source and V3.15 output; calculated per-frame absolute RGB change in ear, face, tail, tail-root, hair and skirt regions. Compared inherited motion with V3.15-added motion.
- Checked relevant high-change frames visually with enlarged source/output side-by-side images. This is automated complete-frame scanning with selected visual inspection; it is **not** a claim that each pixel of 760 images was individually hand-retouched or manually signed off.
- Re-ran the tail-root pixel template-matching algorithm over the source clip. Note: the reconstructed root trajectory is an approximate audit based on the already composited V3.13 video, not the original per-frame runtime root log.

## Findings (priority order)

### P0 — Distorted ear tips / hat-to-ear geometry (fails visual review)

- Peak ear-region changes around **F129–F132 (5.375–5.500 s, cheek interaction)** and **F203–F205 (8.458–8.542 s, tail-touch interaction)**. These peaks exist in V3.13, so the problem is inherited in part.
- V3.15 adds further motion spikes around **F682–F697 (28.417–29.042 s, shake)**; the new ear-region pixel change can exceed the source by approximately **4.7 mean RGB levels** in an observed peak frame.
- The v315 `headw` mask is an **inverse raster remap with spatially varying weight** and protected pixels. There are no separate left/right ear sprites, no hat-attached ear-root bones, and no per-frame **image-space ear silhouette integrity test**.
- The old QA result `ear_distance_drift_analytic_px≈0.00004` tests the *ideal mathematical pivot landmarks*, not the raster silhouette. **Do not cite that test as proof that the actual ear contours remain rigid.**
- A basic bright-region apex tracker indicates that ear tips can move in the output; therefore saying they are literally fixed for all frames would be inaccurate. The viewer's complaint is the **appearance of attachment/stretch distortion**, especially when the existing pre-rendered pose is additionally remapped. Required fix: isolate head+hat+ears as one rigidly transformed parent (or explicitly parent both ear sprites to the head), let only *hair distal sections* use soft deformation, paint missing occluded pixels.

### P0 — Tail looks as if it jumps (fails smoothness review)

- Tail-root and lower-skirt fast changes cluster around **F277–F280 (11.542–11.667 s, hand-touch entering)**; other visible spikes occur near **F356 (14.833 s, tilt left)** and **F660 (27.500 s, shake)**.
- Both versions show comparable spikes: **F278 tail-region mean frame delta: V3.13≈7.56, V3.15≈7.20**; tail movement is predominantly inherited rather than introduced by V3.15. This rules out “just increase four-bone damping” as a complete fix.
- V3.13 joint angles are spring-stateful, but the source figure is blended between whole prerecorded clips (`source_frame`), and the tail parent position is determined by `cv2.matchTemplate` integer grid offsets (`track_pelvis`). Example audit offsets: **F277 (0,-1), F278 (0,-2), F279 (1,-3), F280 (1,-4)**; subpixel continuity is lost even if joint angles are smooth. Those are observed template-tracking offsets on the composited reference, not necessarily a 4-pixel jump in a single frame.
- V3.15 then warps that already-composed tail-containing image with `cv2.remap`, making body/tail matte movement interact. Required fix: give tail an explicit continuous pelvis bone parent, calculate bone pose in real-valued coordinates through the entire event stream, render after head/body posing and before skirt/hair foreground; do not simply crossfade two tail renderings.

### P1 — Extra lines and edges

- Near **F203–F225** in tail-touch there are thin light slash/arc marks beside the tail already **present in V3.13 source** (legacy cue effects). They should be an explicit removable FX layer rather than fused into the main character video.
- At skirt/inner-tail and hair root boundaries the layered source shows sharp dark/light arc fragments and aliasing, partly because source actions and old tail/hair silhouettes are composited before V3.15's second raster remap. Detailed frame comparison is needed before classifying each such arc as intended skirt edge vs unwanted seam.
- Required fix: remove legacy cue strokes in their original effect layer, keep C-shaped tail untouched, reconstruct hair/skirt hidden underpaint and correct draw order. Avoid generic `blur`, large alpha gradient, or painting a dark patch over the whole area.

## V3.16 repair gating sequence

1. **Ear/head layer separation first.** Create real RGBA ear + hat/head components (or one rigid head assembly); author parent pivots on the exact image pixels. Test ear tip-to-base and both-ear spacing from output silhouettes, not only forward-kinematic coordinates. Test 0°, ±4.3°, touch_head, touch_cheek, shake.
2. **Continuous tail root parenting second.** Replace integer `matchTemplate` root relocation with a real-valued pelvis transform. Keep V3.12-approved C-tail asset/four-link geometry. Inspect jerk/acceleration across F277–F280, F356, F660, and all 16 event transitions.
3. **Occlusion cleanup third.** Separate underpaint (`body_back`), `tail`, `skirt_front`, `hair_front`, FX. Remove inherited touch-tail white strokes when not desired; patch each flagged edge on editable RGBA layers and test over dark/bright backgrounds.
4. **Final merged QA.** Re-render 760-frame test reel with same event order and a 466×466 output. Compare with V3.15 per-event crops. Require **no unnaturally static ear tips, no abrupt tail-root pixel snaps, no legacy stray line overlays and no new cracks**. Watch all event transitions and note hardware verification still remains Codex's responsibility.

## Audit deliverables

- `frame_audit_760.csv`: 760 per-frame measurements, reference vs final, flags and root-tracking estimates.
- `motion_timeline.png`: source vs V3.15 change history for ear, tail-root, tail, skirt.
- `audit_high_risk_frame_pairs.jpg`: 5 representative close comparisons from highest-risk spans.
- `stray_lines.jpg`: inherited touch-tail light/line artifacts in both clips.
- `contact_ears.jpg`, `contact_tail.jpg`: frames across representative events.
- `v315_frame_audit_viewer.html`: offline source-vs-output frame/region scrubber.
- This audit does **not** authorize updating frozen V3.0 or advancing the canonical installed version pointer. No new art variant was approved or flashed.