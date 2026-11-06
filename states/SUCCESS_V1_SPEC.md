# Codex Pet - Success Feedback V1 (Draft Art Brief)

Date: 2026-11-05
Status: DRAFT / NOT ART-APPROVED / NOT DEVICE-VALIDATED
Owner: states/success visual workstream; firmware event detection and rendering owned by Codex.
Display: 466 x 466 round AMOLED. Preserve existing approved assets, layers and coordinate system.

## Product contract

There are exactly TWO persistent visual scenes:
- `IDLE`: standby/idle/待机. One approved idle scene.
- `WORKING`: thinking/coding/working/思考中/编程中/工作中. One approved working scene; alias changes must not restart its animation.

`SUCCESS` is NOT a third persistent scene. It is a non-looping, ~2.65-second response to a genuine successful task completion. It starts from WORKING, uses an immediate cut to the accepted front-facing Idle appearance plus a brief character expression overlay and existing Success lighting, and returns to IDLE when done. Do not create a turnaround animation.

## Creative direction

- Character: approved original silver-white wavy hair, black beret + white cat ears, tiny headset, purple-grey eyes, silver high-neck long-sleeved top, black layered skirt, black platform boots, pale tail. Do not change proportions, exposed skin, costume or hairstyle.
- Emotion: graceful, relieved, cute and lightly celebratory; NOT exaggerated dancing or a large jumping movement.
- Possible one-shot gesture: eyes brighten/smile, slight head tilt (~2 degrees), one hand lifts a little in greeting, tail perks up; feet and skirt base stay anchored. For V1, a smile/eye change plus tail movement is acceptable if hand gesture cannot be produced without breaking artwork.
- Palette: Background V3 Success mint #67EDC6 plus soft gold #F6DBA7; no pure white flash or intrusive text.
- Stage/background: preserve approved shared stage pixels; no additional monitors or replacement Coding artwork.

## Timeline - reuse existing Background V3 Success envelopes

| Time | Lighting (existing source) | Character proposal |
|---|---|---|
| 0.00-0.25s | ignite: mint core lights gently | Hard cut from WORKING to front-facing accepted Idle, happy surprise/small smile |
| 0.25-1.15s | expand: mint ring and soft gold rise | Slight head tilt, brief blink/smile; optional small hand gesture |
| 1.15-2.65s | decay: bloom fades | Tail gently returns; smile settles, feet remain fixed |
| >=2.65s | do not loop burst | Return to normal IDLE and resume its usual animation |

Background V3 has a residual dim mint phase after 2.65s; for this integrated V1, fade it into the normal Idle violet ambience at the cut instead of displaying a third persistent Success screen.

## Trigger & concurrency behavior (integration proposal)

- Trigger only for a genuine, top-level successful completion of an active task; not each streamed token, individual tool call, tool acknowledgement, or assistant partial output.
- Failed, cancelled or interrupted task does NOT trigger Success. Error is a separate future feedback design.
- New work start during Success interrupts the one-shot and switches immediately to WORKING.
- Duplicate completion event for the same task ID must not retrigger the burst.
- If the device has just returned to IDLE and no genuine job completion occurred, do NOT fake Success.
- Touch/IMU: avoid triggering large specials during the brief Success. Codex can queue or ignore according to its existing firmware priority policy.
- Reduced motion: single smile and subtle mint glow is sufficient; ensure visible positive acknowledgement.

## Source dependencies (read only)

- `/CodexPet/shared/approved_front_master.png`
- `/CodexPet/idle/codex_pet_idle_v3_0_review.zip`
- `/CodexPet/coding/codex_pet_working_v43_art_handoff_20261105.zip` (latest found on 2026-11-05; may differ from currently flashed firmware)
- `/CodexPet/background/codex_pet_background_v3_review.zip`, specifically `docs/LIGHTFX_STATE_SPEC.md`, `device_candidates/success_*.png`, and `preview/success_466.png`

The Background V3 Coding light anchors refer to older V33 screens. The **Success-centered mint/gold effect** is reusable as a starting point, but the current Working V43 Coding illumination still needs separate geometry QA. Do not overwrite existing two core scenes.

## Success art delivery request to a separate visual-design chat

Deliver only:
1. One 466x466 round-stage static hero preview and a phone-playable short loop-free success video/GIF (~2.65-3.0s).
2. Approved-master-conformant image layers or cropped event sprites with absolute x,y anchors, transparency and a color-space note.
3. `manifest.json` with version, canvas, frame timing and layer order; all filenames ASCII.
4. Clear record of what is a browser visual simulation vs ESP32-ready raw bytes.

No new permanent state, no duplicate Idle animation, no firmware/BLE/touch changes and no editing shared approved masters. Publish as `CodexPet/states/success_v1_review.zip` only after visual review.

## Device acceptance (Codex)

- Genuine completion event triggers once, fades once and returns to IDLE.
- Idle->Working visual contract remains intact. Working alias changes keep same animation phase.
- No full-screen white flash, no foot drift or hair seams, no recurring ring in Idle, no long-term bright static spot.
- Measure actual FPS, PSRAM, display flush time, transition tearing and memory allocations on ESP32-S3 before final release.

This document is a design/integration brief. It is NOT an artwork asset package nor proof of device performance.