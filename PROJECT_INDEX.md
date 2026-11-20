# Codex Pet — Project Index

Updated: 2026-11-20  
Target: Waveshare ESP32-S3, 466 × 466 round AMOLED.

This file is the canonical status index. When a release note conflicts with this index, use the release note for technical detail and this index for current project status.

## Scene contract

There are exactly **two persistent visual scenes**:

- **IDLE** — standby / idle.
- **WORKING** — thinking / coding / working. These are aliases of one scene and must not restart the animation when the label changes.

IDLE ↔ WORKING is an instant scene cut. No walking or turn-around transition is required. **SUCCESS** is a one-shot event/overlay, not a third persistent scene.

## Module ownership

| Path | Responsibility |
| --- | --- |
| `shared/` | Immutable approved character master and shared conventions |
| `idle/` | IDLE-only artwork and animation |
| `coding/` | WORKING-scene character, expressions, terminals, and animation |
| `background/` | Shared stage, background, and lighting |
| `states/` | One-shot event concepts such as SUCCESS |
| `integration/` | Handoffs, manifests, hashes, and integration notes |
| `docs/` | Project-level documentation and policies |
| `history/` | Historical indexes and migration records |

## Current asset status

| Module | Version | Status | Important note |
| --- | --- | --- | --- |
| Shared master | `approved_front_master.png` | Approved baseline | Do not replace without explicit review |
| IDLE | V3.0 | Installed / frozen | Existing device baseline; never overwrite |
| IDLE | V3.16 | Review candidate | Not a replacement for installed V3.0 |
| WORKING | V45/V46 | Historical approved art baseline | Preserve as rollback/reference material |
| WORKING | V47 | Review/integration candidate | Device verification and user acceptance are separate gates |
| WORKING | V49 | Art review only | Release notes are in GitHub; package was not included in this repository snapshot |
| WORKING | V50 | Latest documented art review candidate | Package is external to this repository snapshot; not device-certified |
| Background | V5 | Review candidate | Not measured on actual ESP32 |
| SUCCESS | V1 specification | Design brief | Not a completed firmware-ready asset |

The existence of a release note does not mean its binary package is present in this repository. Check the exact path and Git LFS object before handing a package to Codex.

## Release and acceptance gates

1. **Art candidate** — source, preview, and release notes exist.
2. **Art accepted** — visual review explicitly passed.
3. **Integration candidate** — manifest, conversion assumptions, and handoff are complete.
4. **Device verified** — target-board memory, frame rate, rendering, and interaction checks have evidence.
5. **Installed baseline** — explicitly recorded as deployed; immutable unless a new version is accepted.

Do not collapse these statuses into a single "release" label.

## Integrity and versioning

- All resource paths and filenames must be ASCII.
- Keep workstreams isolated; add new versions rather than overwriting approved assets.
- Retain editable sources, release notes, manifests, and SHA-256 checksums where available.
- Validate Git LFS objects and archive CRC before declaring a package downloadable.
- Firmware, BLE, touch, IMU, state handling, and board acceptance belong to the firmware workstream.

## Entry points

- [Repository README](README.md)
- [Repository workflow](docs/REPOSITORY_WORKFLOW.md)
- [File management policy](docs/FILE_MANAGEMENT.md)
- [Animation rig architecture](docs/ANIMATION_RIG_ARCHITECTURE_V1.md)
- [Asset import manifest](integration/ASSET_IMPORT_MANIFEST_20261109.md)
- [Firmware branch](https://github.com/MEDADA/codex-pet-esp32/tree/firmware/esp32)
