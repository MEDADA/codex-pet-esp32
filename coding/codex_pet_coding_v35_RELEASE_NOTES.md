# V35 Coding Release Notes

Date: 2026-11-04
Status: `review` (unapproved visual prototype)

Baseline: user-approved Pose 4, "holding a transparent terminal". Previous Coding V34 full-back single-screen design is historical, not used in this composition.

New:
- Exact character appearance is taken by alpha extraction from the approved Pose 4 sheet rather than replaced by a new text-to-image character.
- Transparent sprite + original shared stage from background V3 review.
- Head scans gently left/right and nods while the body/boots remain stationary.
- Content rows inside the terminal scroll continuously and independently (left+right columns); the central cat icon remains visible.
- Native 466x466 loop MP4, mobile HTML, ORA, rig JSON, and source scripts.

Limits:
- Source is a flattened illustration: no real independent head/eyelid/hand/tail bones have been painted.
- Blink, finger taps, and soft tail motion are NOT yet production-quality; deliberately excluded from the playback rather than faked.
- Shared background V3 remains a review dependency, not an approved background or an edited asset.
- No device-side tests, PSRAM benchmark, LVGL integration or firmware flashing.
- New version is additive in coding/, prior Coding snapshots preserved, root index unchanged.