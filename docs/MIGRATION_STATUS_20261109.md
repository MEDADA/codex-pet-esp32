# GitHub Migration Status — 2026-11-09

Repository: https://github.com/MEDADA/codex-pet-esp32
Default branch: main
Visibility: Public

## Verified on GitHub

- README.md
- docs/IMPORT_FROM_BUNDLE.md
- tools/import_bundle.sh

## Not yet migrated

The full Library archive snapshot is 211 original files / approximately 445.6 MiB. The original 406 MiB Git bundle is not present in the current working container and has not been pushed. Do not claim the full project has migrated.

The Library contains staged base64 transfer chunks for selected current assets, including:
- approved shared master
- Idle V3.16 review archive
- Working V47 art handoff
- Background V5 review archive

These staged chunks still need reconstruction, checksum verification, and upload. The current GitHub repo therefore remains a migration landing page plus instructions, not the complete asset source of truth.

## Safety / correctness

- Keep installed Idle V3.0 frozen.
- Preserve the two-scene contract: IDLE and WORKING; Thinking/Coding are aliases; SUCCESS is one-shot feedback.
- Do not label review candidates as device-certified or ready-to-flash firmware.
- Keep filenames and paths ASCII.
- Do not overwrite main with a bundle import. Review imported content before merging.
