# Codex Pet - Shared Project Index

Updated: 2026-11-05
Device: Waveshare ESP32-S3 with 466x466 round AMOLED.

## Visual state contract - authoritative user correction (2026-11-05)

There are exactly TWO persistent visual scenes:

1. **IDLE**: `idle`, `standby`, `待机` are the same SINGLE idle scene. One Idle character/art/animation system.
2. **WORKING**: `thinking`, `coding`, `working`, `思考中`, `编程中`, `工作中` use the SAME visual scene and the SAME animation. Do not cut scenes or restart animation when transitioning from Thinking to Coding.

IDLE <-> WORKING uses an instant scene cut. NO character turn-around animation or walking transition. Optional success lighting is a real-event overlay/reference from background, not a third persistent scene.

## Today's integration resource lock

- **Integrated Codex handoff**: `integration/codex_pet_integrated_handoff_20261105.zip` (the ZIP includes untouched upstream source archives, the shared master, `config/visual_state_mapping.json`, SHA256 hashes, README, and QA caveats).
- **State mapping quick reference**: `integration/visual_state_mapping_20261105.json`.
- **Idle**: `idle/codex_pet_idle_v3_0_review.zip` (art complete as user confirms; upstream status remains `review`, device verification still pending).
- **Working**: `coding/codex_pet_coding_v41_20261105_review.zip` and `coding/codex_pet_coding_v41_device_candidates.zip` (latest 2026-11-05 resources, source status `review`).
- **Background / lighting**: `background/codex_pet_background_v3_review.zip` (art complete as user confirms; source status `review`; blue light geometry still cites the older V33 workspace and must be adapted to V41).
- **Immutable shared master**: `shared/approved_front_master.png`.

## Integration evidence / constraints

- The shared master is byte-identical to Idle V3.0's approved master snapshot.
- Background V3 `assets/stage_base_466.png` is byte-identical to Working V41 `assets/stage_dependency_466.png`. **Stage image match does NOT imply that the V33 lighting anchors already match the V41 figure/screens.**
- Idle V3.0 contains 480px mobile preview videos; the physical panel is 466x466. Browser videos and HTML are not firmware asset decoders.
- Working V41 supplies cropped raw little-endian RGB565 and A8 data, animation/reference manifests, masks and notes. These are not native LVGL descriptors and not independently rigged skeleton assets.
- No physical ESP32 FPS, PSRAM use, rendering stability or firmware flash success has been validated by the art workstreams. Codex owns firmware/ESP32 integration and device acceptance.

## Workstream boundaries

- `shared/`: immutable character, common stage, shared coordinates and conventions.
- `idle/`: Idle action assets only.
- `coding/`: Working (formerly Coding) character, expression and terminals only; also handles the Thinking alias without a new scene.
- `background/`: common stage and state-specific light effects; never own characters, BLE, states or firmware.
- `integration/`: combined version lock, firmware integration handoffs, test reports.
- `states/`: older proposals are NOT new persistent states unless separately requested by the user.

Save new candidates with versioned ASCII filenames. Avoid overwriting other module releases or changing the approved master. When a new version is accepted, update this index after checking its current Library version.