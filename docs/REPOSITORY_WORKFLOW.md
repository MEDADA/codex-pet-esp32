# Repository Workflow

This document defines how artwork, animation candidates, and integration handoffs are added to the repository without breaking existing device builds.

## Module boundaries

| Path | Maintained content |
| --- | --- |
| `shared/` | Approved character master and shared references |
| `idle/` | IDLE artwork and versioned candidates |
| `coding/` | WORKING artwork, rigs, previews, and versioned candidates |
| `background/` | Shared background and lighting |
| `states/` | One-shot events such as SUCCESS |
| `integration/` | Handoffs, manifests, checksums, and integration notes |
| `docs/`, `history/` | Policies, architecture, and historical records |

## Branching and review

- Keep `main` as the stable, documented asset baseline.
- Make structural cleanup and multi-file changes on a dedicated branch.
- Use focused commits with imperative messages.
- Review binary additions, deletions, and LFS pointer changes explicitly.
- Merge only after links, manifests, and release status have been checked.

## Asset naming

- Use ASCII-only paths and filenames.
- Prefer `<module>_<version>_<purpose>.<ext>` for new versioned assets.
- Keep source files, preview files, QA evidence, and release packages distinguishable.
- Do not silently replace an existing versioned file.

## Required handoff metadata

Every candidate intended for integration should provide:

- Module and version.
- Review status and date.
- Canonical repository path(s).
- SHA-256 for distributable archives and important binary assets.
- Display dimensions and frame/timing information where relevant.
- Known limitations and unverified assumptions.
- Explicit statement of whether device validation has passed.

## Acceptance gates

Track these separately:

1. Art candidate.
2. Art accepted by the project owner.
3. Integration candidate.
4. Target-device verified.
5. Installed baseline.

A preview or successful desktop render is not evidence of ESP32 performance, memory use, or hardware acceptance.

## Scene contract

There are two persistent scenes: IDLE and WORKING. Thinking and Coding are aliases of WORKING. SUCCESS is a one-shot event. Scene aliases must not create duplicate animation timelines.

## Binary asset handling

Git LFS is configured for common binary formats. After adding or moving LFS-managed files:

1. Confirm the Git LFS object exists.
2. Pull the object in a clean clone.
3. Verify archive CRC where applicable.
4. Verify the SHA-256 recorded in the manifest.
5. Confirm preview and handoff links resolve.

A small pointer file in the Git tree is expected for LFS-managed content; it is not the binary payload.

## Safety rules

- Never overwrite installed IDLE V3.0 or approved shared masters without explicit approval.
- Never delete a historical artifact solely because a newer version exists.
- Never claim a package is available based only on a release note; verify the actual file and LFS object.
- Keep firmware implementation and device acceptance under the firmware workstream.
