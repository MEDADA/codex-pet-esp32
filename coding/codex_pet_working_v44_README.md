# Codex Pet Working V44 - Identity Consistency Review

Status: **REVIEW** (not approved / not device tested).

Preserves V43 Pose4 terminal-holding Working character and existing 30-second reference animation. This candidate corrects ONLY three character-identity mismatches with the approved Idle front master: (1) convert purple-tinted hair to the original silver-white palette, (2) remove the extra black X hairpin while retaining the cat-and-star beret badge, (3) replace the shiny silver boot cuffs with black ruffled cuff styling matching the mother template.

## Outputs
- `preview/master_consistency_review.jpg`: mother/V43/V44 direct comparison
- `preview/rendered_frame0.png`: final 466x466 reference scene
- `preview/coding_v44_30s.webp` / `.gif`: re-rendered 30s loop using corrected character base
- `assets/character_pose4_corrected_466.png`: transparent corrected character sprite
- `source/coding_v44_character_layers.ora`: flat editable layers (not a full rig)
- `manifest.json`: source hashes and change scope

## Boundaries
No changes to Idle, approved Shared master, background art or firmware. Thinking/Coding/Working share one visual animation and do not reset when switching aliases. No device performance claims. Await user visual approval before promoting the new look.