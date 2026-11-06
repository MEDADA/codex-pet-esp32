# Codex Pet Coding V39 — Review Candidate

- **STATUS:** REVIEW; Coding-owned only, not accepted and not hardware validated.
- Character: same approved Pose4 holding-terminal raster asset as V38. No changes to identity, posture, fingers, shared background or light effects.
- Expression: focus, mild thought/brow attention, tiny satisfied mouth; these are local retouched raster references, NOT hand-painted skeletal expressions.
- Head: small delayed tilt (max approx 1.6 degrees) and nod (max 2.2px) with smooth masked raster deformation, NOT true side-view head yaw.
- Eyes: reuse V38 per-eye separated sources and 5-state eyelid overlays; gaze leads head. Blink events retain approximate 0.3s duration.
- Terminal: same two internal scroll patches (synthetic demonstration content).
- Previews: full 12s animated WebP and GIF, offline mobile HTML with individual keypose inspection; no dependency on MP4.
- Source: 17-layer ORA (3 compositing and 14 hidden reference layers), character/stage source and rig_v39.json.
- All filename/ZIP paths ASCII; all earlier Coding versions retained. Do not change Idle, Shared, Background or PROJECT_INDEX before acceptance.
- Limitations: brow/mouth retouch subtle, eyelid transitions need user visual approval. Desktop render does not validate hardware FPS/RAM/flush performance.