# Codex Pet — Project Index

Updated: 2026-11-13
Target: Waveshare ESP32-S3, 466×466 round AMOLED.

## Authoritative scene contract

There are exactly TWO persistent visual scenes:

- **IDLE**: idle / standby. One persistent idle scene.
- **WORKING**: thinking / coding / working. All aliases use the same visual scene and animation; changing aliases must not restart the animation.

IDLE ↔ WORKING is an instant scene cut. No walking or turn-around transition. SUCCESS is a one-shot event/overlay, not a third persistent scene.

## Module ownership

- `shared/`: immutable approved character master, shared stage and conventions.
- `idle/`: Idle-only art and animation. Installed Idle V3.0 is frozen; never overwrite it.
- `coding/`: WORKING scene character, expression, terminals and animation. Thinking is an alias.
- `background/`: shared stage and lighting effects only.
- `states/`: one-shot event concepts such as SUCCESS; not persistent scenes.
- `integration/`: handoffs, manifests, hashes and integration notes.
- `docs/`: project-level documentation.

## Current candidates (2026-11-13)

- Shared master: [approved_front_master.png](shared/approved_front_master.png)
- Idle: [V3.16 repair review candidate](idle/releases/idle_v316.zip); installed V3.0 remains frozen.
- Working: [V47 art handoff candidate](coding/releases/working_v47.zip) (review; device verification required).
- Working Rig: [V1.8 Stage 7 Godot runtime gate](coding/WORKING_RIG_V18_STAGE7_RUNTIME_GATE.md). This is a validation handoff, **not** a Godot-runtime-passed or device-ready release. The Stage 7 ZIP is attached in the related ChatGPT conversation; it has not been committed as a GitHub binary.
- Background: [V5 review candidate](background/releases/background_v5.zip).
- Success: V1 design brief only; not a completed firmware-ready asset.

These are art/integration candidates, not certified firmware releases. The ESP32 firmware, BLE, touch, IMU, state handling and board acceptance are owned by the firmware workstream.

## Integrity and versioning

All resource paths and filenames must be ASCII. Keep each workstream isolated. Add new versioned candidates; do not overwrite approved or flashed assets. Retain source archives, manifests and SHA-256 checksums. Verify archive CRC and hashes after migration.

## Handoff and development

See the [candidate checksums](integration/ASSET_IMPORT_MANIFEST_20261109.md), [repository workflow](docs/REPOSITORY_WORKFLOW.md), and [firmware branch](https://github.com/MEDADA/codex-pet-esp32/tree/firmware/esp32). Historical Library migration is tracked separately in [migration status](docs/MIGRATION_STATUS_20261109.md).