# File Management Policy

## Purpose

Keep the public repository understandable, reproducible, and safe for both human contributors and coding agents.

## Keep

- Approved masters and source artwork.
- Versioned candidates that document a meaningful iteration.
- Release notes, manifests, QA evidence, and integration handoffs.
- Previews that materially help reviewers assess an asset.
- Historical records needed to explain decisions or support rollback.

## Review before removal

A file may be proposed for removal only when all applicable checks pass:

1. It is not referenced by README, project index, release notes, manifests, scripts, or handoffs.
2. It is not an approved, installed, or rollback asset.
3. It is not the only editable source or QA evidence for a version.
4. Its content is duplicated or reproducibly generated elsewhere, or it is clearly temporary.
5. Its removal is reviewed in a dedicated cleanup change.

When uncertain, move the file to a clearly named archive location rather than deleting it. Do not rewrite Git history to remove ordinary superseded assets.

## Git LFS

Binary files matching `.gitattributes` patterns may be stored as Git LFS pointers. A  pointer-sized blob in the Git tree is expected and does not prove that the underlying object is missing.

Before declaring an LFS asset broken, verify that the LFS object can be fetched from a clean clone and compare its checksum with the manifest. Do not delete pointer files as a space-saving measure.

## Naming and paths

- Use ASCII-only directory and file names.
- Use lowercase directory names and descriptive filenames.
- Include module and version in new distributable filenames.
- Avoid names such as `final-final2`, `new`, `test2`, or date-only names without context.

## Release package requirements

Each distributable package should have:

- A versioned filename.
- A matching release note.
- A SHA-256 checksum.
- A clear status: review, accepted, integration-ready, or device-verified.
- A documented source of truth and retrieval URL.

## Cleanup procedure

1. Inventory the complete recursive tree.
2. Search references to each proposed deletion.
3. Inspect file type, size, and Git LFS tracking.
4. Preserve a rollback path.
5. Apply cleanup on a dedicated branch.
6. Validate links, manifests, scripts, and LFS retrieval.
7. Open a pull request with a file-by-file summary.

## Licensing

Do not add or infer a license without the project owner's explicit choice. Public visibility alone does not grant reuse rights.
