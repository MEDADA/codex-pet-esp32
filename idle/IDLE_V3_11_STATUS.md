# Codex Pet — Idle V3.11 review

- **Release:** `review`, visual prototype only; **NOT installed / NOT flashed**.
- **Frozen production Idle:** V3.0 (user confirmed on real device).
- **Updated art:** V3.10 C-shaped tail and 4-bone fixed-length rig retained; V3.11 per-frame skirt/fur seam matte repair added.
- **Preview:** 8s breathing/eyes/hair/skirt/tail loop + touch-head, touch-cheek, touch-tail, left/right tilted gestures; 28s aggregate video and six-button HTML.
- **Editable:** OpenRaster static frame, separated full tail RGBA, seam guide mask, reproducible source.
- **QA:** 672 video frames at 24fps; 8-second loop difference 2.66 mean abs RGB in tail ROI; 5 gesture clips/96 frames each; H264 baseline; no changes outside the character tail ROI; ASCII ZIP paths.
- **Limitations:** live event preemption, 6-axis hardware tracking and LVGL/memory remain unverified; dark line softened programmatically, not final manually painted under-skirt seam; gesture body poses remain based on pre-rendered approved V3.0 sources.
- **Library target:** `/CodexPet/idle/codex_pet_idle_v3_11_root_seam_review.zip`, plus standalone preview and status. Do not alter PROJECT_INDEX installed pointer or other modules.