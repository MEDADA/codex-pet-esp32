# Codex Pet ESP32

A modular 2D character-art and animation asset project for the Waveshare ESP32-S3 with a 466 × 466 round AMOLED display.

The repository is the source of truth for reviewed artwork, versioned asset handoffs, integration metadata, and project documentation. Firmware implementation and device-side validation are maintained separately.

## Project status

- Display target: 466 × 466 pixels, circular AMOLED.
- Persistent scenes: **IDLE** and **WORKING**.
- Thinking and Coding are aliases of the same WORKING scene.
- SUCCESS is a one-shot event, not a persistent scene.
- Installed IDLE V3.0 is frozen and must not be overwritten.
- Art candidates are not firmware-certified releases unless device verification is explicitly recorded.

See [Project Index](PROJECT_INDEX.md) for authoritative versions and handoff status.

## Repository map

| Path | Responsibility |
| --- | --- |
| `shared/` | Approved character master and shared references |
| `idle/` | IDLE artwork, animation candidates, and release records |
| `coding/` | WORKING artwork, rigs, previews, and release records |
| `background/` | Shared background and lighting assets |
| `states/` | One-shot state concepts such as SUCCESS |
| `integration/` | Asset manifests, hashes, and firmware handoffs |
| `docs/` | Architecture, workflows, and repository policies |
| `history/` | Superseded indexes and migration history |
| `tools/` | Small maintenance and import utilities |

## Getting started

1. Read [PROJECT_INDEX.md](PROJECT_INDEX.md).
2. Read [Repository Workflow](docs/REPOSITORY_WORKFLOW.md) before adding or replacing assets.
3. Select a candidate explicitly from its module's release notes and manifest.
4. Verify the SHA-256 checksum in the relevant import manifest before integration.
5. Follow the firmware workstream's instructions for conversion, build, and device validation.

Clone the repository:

```bash
git clone https://github.com/MEDADA/codex-pet-esp32.git
cd codex-pet-esp32
```

Binary assets may use Git LFS. Install Git LFS before cloning or pulling large assets:

```bash
git lfs install
git lfs pull
```

## Asset and release principles

- Keep paths and filenames ASCII-only for reliable cross-platform tooling.
- Add new versioned candidates; do not overwrite accepted or installed assets.
- Keep editable sources, previews, manifests, and release notes clearly distinguished.
- Preserve historical review records when they explain a decision or support rollback.
- Never describe an art-review candidate as device-ready without measured device evidence.

## Contributing

See [Repository Workflow](docs/REPOSITORY_WORKFLOW.md) and [File Management Policy](docs/FILE_MANAGEMENT.md). Changes to approved masters, installed assets, or release status require explicit review.

## License

No license has been declared yet. Until a license is added by the project owner, do not assume that public availability grants permission to reuse, redistribute, or modify the artwork.
