# GitHub Migration Status — 2026-11-09

Repository: https://github.com/MEDADA/codex-pet-esp32
Default branch: main
Visibility: Public

## Verified on GitHub

- `README.md`
- `PROJECT_INDEX.md` (updated 2026-11-09)
- `docs/IMPORT_FROM_BUNDLE.md`
- `docs/IDLE_V3_16_HANDOFF.md`
- `docs/MIGRATION_STATUS_20261109.md`
- `integration/ASSET_IMPORT_MANIFEST_20261109.md`
- `coding/WORKING_V47_RELEASE_NOTES.md`
- `tools/import_bundle.sh`

## Asset reconstruction progress

The following archives have now been reconstructed from staged Library base64 chunks in the working environment and passed ZIP CRC tests:

- Idle V3.16 review archive — 13 MiB.
- Working V47 art handoff — 12 MiB.
- Background V5 review archive — 4.3 MiB.
- Approved shared master PNG — 1.7 MiB.

Exact SHA-256 values are recorded in [ASSET_IMPORT_MANIFEST_20261109.md](../integration/ASSET_IMPORT_MANIFEST_20261109.md).

**Important: these binary assets are reconstructed locally but have NOT yet been uploaded to GitHub.** The connected GitHub interface supports text-file writes and Git object APIs, but this execution path does not provide a way to stream local binary files from the working filesystem into those APIs. The container also cannot resolve github.com for a direct authenticated Git push. Therefore do not treat the repo as the complete asset source of truth yet.

## Remaining work

1. Transfer the four verified binary files to an authenticated Git client or a supported binary-upload path.
2. Commit them under stable ASCII paths, for example:
   - `shared/approved_front_master.png`
   - `idle/releases/idle_v316.zip`
   - `coding/releases/working_v47.zip`
   - `background/releases/background_v5.zip`
3. Fetch each committed file back and compare SHA-256 against the manifest.
4. Continue migrating historical releases and integration materials in versioned batches.

## Safety / correctness

- Keep installed Idle V3.0 frozen.
- Preserve the two-scene contract: IDLE and WORKING; Thinking/Coding are aliases; SUCCESS is one-shot feedback.
- Do not label review candidates as device-certified or ready-to-flash firmware.
- Keep filenames and paths ASCII.
- Do not overwrite main with a bundle import. Review imported content before merging.
