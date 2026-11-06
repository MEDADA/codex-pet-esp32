# Codex Pet — Coding V32 Library Candidate

**Status:** `review` (not approved; not flashed or benchmarked on ESP32)  
**Owner:** Coding art and animation workstream only  
**Date:** 2026-11-04  
**Target Library directory:** `/CodexPet/coding/`  
**Prior Coding candidate:** `codex_pet_coding_v31_20261104_review.zip` (retained unchanged)

## What changed in V32

- Replaced V31's provisional solid-color background with a byte-identical copy of Idle V2.9's `baseline/idle_background.png` (466×466). Original SHA-256: `6e01f9ff6b547660d16805e07429291b812306e53a3e9b1861f986b7bc591d24`.
- Retained the V31 original transparent 1254×1254 side-back character master, scaled uniformly to 330×330 at position (66,66). Visible alpha bbox in the screen coordinate system: `(134,66)-(365,395)`. No full-character distortion and no new identity generated.
- Arranged three blue holographic screens within a circular display-safe layout, including the lower-right status overlay shifted inward to avoid off-circle cropping.
- Updated small display graphics and local glow, retaining an intact body and hair silhouette.
- Added 11 independent OpenRaster layers, cropped layer assets, crop coordinates, exact background hash and standalone 466×466 mobile previews.

## Source dependencies

- `/CodexPet/idle/codex_pet_idle_v2_9_review.zip`: background-only, copied as an immutable dependency snapshot. The V2.9 **Idle release remains `review`**.
- `/CodexPet/coding/codex_pet_coding_v31_20261104_review.zip`: Coding side-back master and previous layout reference.
- `/CodexPet/shared/approved_front_master.png`: canonical character identity reference; never modified.

## Acceptance checks

- Exact Idle-background bytes: SHA-256 verified.
- OpenRaster: 11 layers, recompose matches merged PNG pixel-for-pixel.
- Resolution: all scene assets are aligned to 466×466; 480×480 Idle preview is explicitly scaled for visual comparison only.
- Coding mobile MP4: 8 seconds, H.264, 12 FPS, 96 frames; mobile controls exercised in Chromium.
- All files inside the package have ASCII-only paths.

## Known limitations

- The latest Idle V2.9 is itself not approved and its art reference preview is 480px. Character size is visually aligned, not signed off against a jointly approved anchor spec.
- The side-back character is a complete image; independent head, hands and tail motion are **not** present in V32. Do not introduce cloth or hair seams by pretending this is a completed rig.
- The code panels are graphics, not live Codex task data; no fake progress percentage is displayed.
- ESP32 FPS, Flash/PSRAM allocation and display flush speed are **not** tested. Codex retains firmware integration responsibility.

## Collaboration safety

Save this candidate as a **new version** in `/CodexPet/coding/`. Do not overwrite V31, Idle, Shared, States or Integration. Do not update `PROJECT_INDEX.md` until the user explicitly approves a Coding version and the current index version has been checked again.