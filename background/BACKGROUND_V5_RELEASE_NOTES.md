# Background V5 - Low-Cost Glow Movement

- Date: 2026-11-05
- Status: review (not approved for flash)
- Source: V4 background review; characters are read-only V43 and Idle references.
- New: pre-baked cyan monitor-sheen sprite (104x42), optional 40x40 micro-glow.
- Lower runtime redraw target: two moving sprite layers instead of multiple halos / 64px orbit variants.
- Fixed 8Hz 48-step integer lookup motion path, no live gradients / rotation / scaling / blur. Five-step prebaked Success intensity envelope.
- Idle scene unchanged in character and stage geometry; Working aliases preserve phase; Success remains one-shot.
- Not measured on actual ESP32. Do not overwrite V4, other module resources or root index.