# Codex Pet Coding V34 — Review Candidate

- Scope: Coding only, without editing Idle or `shared/` masters.
- Visual master changed by explicit user approval: **full-back cat-girl + one giant central screen + blue highlights**, dropping previous 3-screen side-back composition.
- Original master is a single 1254px flattened image; the stage, figure and screens cannot be fully separated without repainting concealed pixels.
- Includes extracted head, left/right hands, and tail, with pixel masks and inpainted underlay. Head motion is a small silhouette roll, not a true left/right view. Source-style reference and a provisional rig definition are included.
- Includes 10-second, 120-frame 466px H.264 review clip; six pose stills; OpenRaster prototype; animation design and Codex integration notes.
- Validity: rendering tested on desktop, **no ESP32 FPS or RAM benchmark**.
- Review status until user approves; do not update `PROJECT_INDEX.md`. Preserve old V33 and the separate Idle packages.
- The new illustration includes navy floor-light details not pixel-identical with shared Idle background; a stricter shared-background variant would require a remaster of the single-screen setting.

## Latest Library index coordination (2026-11-04)

The freshly updated `/CodexPet/PROJECT_INDEX.md` declares Idle art complete in its owning chat and assigns stage/background lighting to a separate `background_lighting/` workstream. This Coding V34 preview follows the user-approved single-screen picture, but **must not be promoted as the common background or override Idle art**. No background files or canonical pointers are written outside `/CodexPet/coding/`. Before product integration, consume the actual shared stage from the background workstream when published; it currently contains no files.