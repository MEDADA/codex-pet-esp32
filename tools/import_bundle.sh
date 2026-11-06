#!/usr/bin/env bash
set -euo pipefail
BUNDLE="${1:-codex_pet_library_20261106.bundle}"
REPO="${2:-MEDADA/codex-pet-esp32}"
WORKDIR="${3:-codex-pet-library-import}"
if [[ ! -f "$BUNDLE" ]]; then
  printf 'Bundle not found: %s\n' "$BUNDLE" >&2
  exit 2
fi
if [[ -e "$WORKDIR" ]]; then
  printf 'Directory already exists: %s\n' "$WORKDIR" >&2
  exit 2
fi
git bundle verify "$BUNDLE"
git clone "$BUNDLE" "$WORKDIR"
cd "$WORKDIR"
git remote remove origin
git remote add origin "https://github.com/${REPO}.git"
# Push to a separate branch. Never force or replace main.
git push -u origin HEAD:refs/heads/library-import
printf 'Done. Review https://github.com/%s/tree/library-import\n' "$REPO"
