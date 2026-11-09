# Codex Pet — Project Index

Updated: 2026-11-09
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

## Latest known candidate pointers (Library snapshot, 2026-11-06)

- Shared master: `shared/approved_front_master.png`
- Idle: V3.16 repair review candidate; installed V3.0 remains frozen.
- Working: V47 art handoff candidate (review; device verification required).
- Background: V5 review candidate.
- Success: V1 design brief only; not a completed firmware-ready asset.

These are art/integration candidates, not certified firmware releases. The ESP32 firmware, BLE, touch, IMU, state handling and board acceptance are owned by the firmware workstream.

## Integrity and versioning

All resource paths and filenames must be ASCII. Keep each workstream isolated. Add new versioned candidates; do not overwrite approved or flashed assets. Retain source archives, manifests and SHA-256 checksums. Verify archive CRC and hashes after migration.

## Migration status

See [docs/MIGRATION_STATUS_20261109.md](docs/MIGRATION_STATUS_20261109.md). The full binary Library snapshot has not yet been uploaded; this repository is not yet the complete asset source of truth.
