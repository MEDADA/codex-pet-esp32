# Codex Pet — GitHub migration status (2026-11-06)

**Staging completed; GitHub remote upload BLOCKED pending creation of a dedicated private repository.**

- Connected GitHub login: `MEDADA`.
- Proposed remote: `https://github.com/MEDADA/codex-pet-esp32` (not found when checked).
- Available GitHub connector actions do not include `create_repository`, so no user repository was created or modified.
- Full preserved ChatGPT Library snapshot: 211 original files / 445.62 MiB (modules Idle, Working, Background, States, Shared, Integration).
- Git history: one initial commit `03b2c0405ce7c887afe78fb27e99ec458dad8696` on `main`, 220 tracked files (211 originals + 9 new or archived metadata files).
- Source integrity: 48 ZIP archives tested, no CRC errors; all original file paths ASCII; SHA-256 inventory generated.
- Bundle: `codex_pet_library_20261106.bundle` (~406 MiB), full Git history, verified.
- Bundle SHA-256: `10cb94cd3154b1d091891277ecfc6131b2d62a947d84fbbdfda9e27ec3ce9972`.

## One-time creation and upload

1. In GitHub, create an **empty private** repository owned by `MEDADA`, named `codex-pet-esp32`. Do not initialize it with README/license/gitignore.
2. Download both `codex_pet_library_20261106.bundle` and `upload_codex_pet_to_github.sh` into one folder.
3. On a Mac/Linux terminal in that folder, run `bash upload_codex_pet_to_github.sh` (requires `git` and authenticated GitHub access).
4. Confirm repository content and share the resulting URL for subsequent cross-chat GitHub edits.

Alternatively, the bundle can be cloned manually using the steps inside `docs/MIGRATION_GUIDE.md` in the repository.

## Content distinction

- Latest visual review candidates: Idle V3.16, Working V47, Background V5.
- Success V1 remains a design brief.
- The true installed firmware / board benchmark source is not in the Library snapshot. This repository must not be treated as a ready-to-flash firmware repository.
- `IDLE` and `WORKING` are the only persistent visual states; thinking/coding map to WORKING without restart; success is an event overlay.
- Old Library project index preserved for audit; newer GitHub `PROJECT_INDEX.md` explicitly supersedes it.