#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
SYMBOLS=$(tr -d '\n' < tools/buddy_cn_symbols.txt)
FONT="$ROOT/managed_components/lvgl__lvgl/tests/src/test_files/fonts/noto/NotoSansSC-Regular.ttf"
export PATH="${HOME}/.nvm/versions/node/v24.22.1/bin:${HOME}/Library/Python/3.9/bin:$PATH"
for size in 16 20; do
  npx --yes lv_font_conv \
    --font "$FONT" --size "$size" --bpp 4 --format lvgl \
    -r 0x20-0x7F --symbols "$SYMBOLS" \
    --no-compress --force-fast-kern-format \
    --lv-include lvgl.h --lv-font-name "font_buddy_cn_${size}" \
    -o "main/fonts/font_buddy_cn_${size}.c"
done
# optional subset TTF for storage partition
UNICODES="U+0020-007E,$(python3 - <<'PY'
from pathlib import Path
s=Path('tools/buddy_cn_symbols.txt').read_text(encoding='utf-8').strip()
print(','.join(f'U+{ord(c):04X}' for c in s))
PY
)"
pyftsubset "$FONT" --unicodes="$UNICODES" --layout-features='*' --glyph-names \
  --symbol-cmap --legacy-cmap --notdef-glyph --notdef-outline --recommended-glyphs \
  --name-IDs='*' --name-legacy --name-languages='*' \
  --output-file=assets/fontfs/NotoSansSC-Regular.ttf
ls -lah main/fonts/font_buddy_cn_16.c main/fonts/font_buddy_cn_20.c assets/fontfs/NotoSansSC-Regular.ttf
