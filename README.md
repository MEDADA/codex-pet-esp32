# Codex Pet — ESP32-S3 Art Asset Repository

> **Migration status: preparation only.** The full ChatGPT Library binary archive has **not** yet been pushed to GitHub. This README is hosted directly on GitHub to avoid broken ChatGPT file downloads.

Device: ESP32-S3 / 466×466 round AMOLED. This repository is for art/animation resources and handoff notes, not a ready-to-flash firmware distribution.

## Runtime scene contract

| Scene | Mapped events | Notes |
|---|---|---|
| IDLE | idle, standby | One permanent idle scene |
| WORKING | thinking, coding, working | One permanent working scene; alias changes preserve animation phase |
| SUCCESS | genuine task completion | Short one-shot feedback, **not** a third permanent scene |

IDLE ↔ WORKING uses an instant scene cut, without turn-around animation. Never replace the working, already-flashed firmware without a backup and user approval.

## Library migration snapshot (2026-11-06)

- Library path: `/CodexPet/`
- 211 original files; approximately 445.6 MiB total.
- Existing flashed Idle V3.0 is frozen.
- Latest **review candidates**, not firmware releases: Idle V3.16, Working V47, Background V5; Success V1 is a design brief.
- Full binary Git snapshot was assembled locally, but transfer to this GitHub repository is pending.

The intended structure is `shared/`, `idle/`, `coding/`, `background/`, `states/`, `integration/`, `docs/`.

## Bulk import

Please see [docs/IMPORT_FROM_BUNDLE.md](docs/IMPORT_FROM_BUNDLE.md). The tool [tools/import_bundle.sh](tools/import_bundle.sh) is hosted here on GitHub, so it can be read or downloaded without a ChatGPT sandbox link.

Repository visibility is currently **Public**, matching the selected project setup. Only publish assets and source materials you have the rights to share.
