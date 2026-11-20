# Contributing

Thanks for helping improve Codex Pet ESP32.

## Before changing files

1. Read [PROJECT_INDEX.md](PROJECT_INDEX.md) for authoritative asset status.
2. Read [Repository Workflow](docs/REPOSITORY_WORKFLOW.md) and [File Management Policy](docs/FILE_MANAGEMENT.md).
3. Keep changes within the module you own.
4. Use ASCII-only paths and filenames.

## Asset contributions

Every new candidate should include a versioned source package, concise release notes, a manifest with SHA-256 checksums, and a preview or QA evidence where appropriate.

Never overwrite installed IDLE V3.0, approved shared masters, or previously published versioned assets. Add a new version instead.

## Pull requests

A pull request should state:

- What changed and why.
- Which module and version are affected.
- Whether binary assets or Git LFS pointers changed.
- How links, archive integrity, and checksums were verified.
- What remains unverified, especially device-side performance or acceptance.

Keep PRs focused. Do not combine unrelated asset redesigns with repository-wide cleanup.

## Review standards

A desktop preview is not device certification. Clearly distinguish art review, owner acceptance, integration readiness, and target-device verification.
