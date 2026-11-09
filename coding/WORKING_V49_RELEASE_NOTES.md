# Codex Pet WORKING V49 — review candidate

Status: ART REVIEW ONLY, not approved for device deployment.

This version retains the V45-approved 466x466 character and does not overwrite IDLE, background, V45–V48, or shared assets.

## V49 changes
- Four independently animated source RGBA hair locks: left root, outer, inner and tip curls.
- Three control points per lock (12 controls) with a 24-second continuous read/focus/compile cycle.
- Source-backed underpaint and local synthetic fill to hide cracks exposed by small lock motion. This is **not** complete hand-painted hidden anatomy.
- Motion limits from 48 sampled poses: maximum measured hair displacement 3.404 px.
- GIF: 96 frames / 24 seconds. Animated WebP: 144 frames / 24 seconds.
- Protected terminal, hands, boots and eyes: zero changed core pixels on sampled tests. Hair core transparency holes: 0 on sampled tests.
- Neutral character reassembles exactly from the 19 original V45-derived visible layers.

## Important limits
Other three hair groups remain constrained, not independently rotated. No large head turn; no complete hidden-hair painting; no native ESP32/LVGL memory benchmark; not visually approved.

## Assets
The art handoff ZIP and device candidates ZIP were generated locally in this ChatGPT conversation; **the binary ZIPs are not stored in this GitHub repository by this commit**. Download the ZIPs from the corresponding ChatGPT message. The full handoff includes source/build_v49.py, references/v48/, a .ora source, a rig JSON, motion CSV, preview GIF/WebP and offline HTML.

## State contract
Thinking / Coding are aliases of one WORKING scene and share a continuous animation timeline. Transition IDLE→WORKING can cut directly, without walking. Preserve existing approved IDLE.

Do not replace approved device assets until art review and hardware verification.
