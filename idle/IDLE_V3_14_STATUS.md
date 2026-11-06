# Idle V3.14 candidate status

- Candidate: `codex_pet_idle_v3_14_head_hair_review.zip` under `CodexPet/idle/`.
- Status: **review**; not approved or flashed.
- Frozen installed state: **Idle V3.0**; must remain unchanged.
- Scope: head/ears, six hair groups, four skirt-hem regions; **no tail geometry changes**.
- Source references: V3.13 ambient approved art and V3.3 independent hair/skirt masks.
- Implementation: shared rigid head pivot `(241,264)` for face, hat and ear landmarks; independent delayed strand motion plus separate hem rhythm. Pixels around the status label, watch digits and shoes are preserved.
- Review: 16s at 24 FPS (384 frames) in both full and side-by-side MP4; phone review HTML and 466px scaled reference.
- Analytical ear distance error: `<0.0001px` for configured rigid bone landmarks. This is a kinematic calculation, not subpixel image-recognition proof.
- Hair/skirt controller correlation over this scripted 16s clip: around `-0.45`, i.e. no forced synchronous signal.
- Limitations: still a raster-image remap and weighted-control demo, not complete independently skinned head/hair polygon meshes; body touch pose rig remains prerecorded; hidden hair roots not fully art-retouched; no native firmware metrics.
- Project visual contract: IDLE is a single scene; Working/Thinking/Coding another single shared scene; instant cut between them; no third persistent success scene.

Never overwrite `CodexPet/PROJECT_INDEX.md` installed-version pointer until user accepts a new hardware release. Do not modify `shared/`, `coding/`, `background/`, `integration/`, or `states/`.