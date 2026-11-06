# Import full Codex Pet Library snapshot

The full Library snapshot contains 211 original files (approximately 445.6 MiB) and was exported as a Git bundle named `codex_pet_library_20261106.bundle` (approximately 406 MiB).

**GitHub does not contain that large bundle or its media yet.** The old ChatGPT attachment link may fail because of the file size. A Git bundle is a local Git transfer archive, not a GitHub web link.

## Recommended safe import

1. Set this repository to **Private** before importing original character art.
2. Get the local `.bundle` onto your computer. For failed ChatGPT downloads, request smaller module bundles/chunks rather than retrying the 406 MiB attachment.
3. Download `tools/import_bundle.sh` from this repository (or copy its commands), place it in the same folder as the bundle.
4. Run `bash import_bundle.sh codex_pet_library_20261106.bundle`.
5. The script pushes a separate `library-import` branch, without replacing `main`. Review the branch before merging.

The imported history contains the source snapshot and associated `README.md`, `.gitattributes`, `.gitignore`, `docs/MIGRATION_GUIDE.md`, `docs/SOURCE_FILE_SHA256.txt`, and module release notes. Do not treat offline animation previews as verified on the ESP32.

If the bundle still cannot be transferred, a smaller latest-candidate subset (Idle, Working, Background, master) is a better interim option. The rest can be imported as archives later.
