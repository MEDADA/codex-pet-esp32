#include "buddy_fonts.h"
#include "esp_log.h"

/* Prefer bundled LVGL CJK subset fonts. Full NotoSansSC in SPIFFS was ~10MB and
 * FreeType rasterization cost too much RAM on this board. A tiny subset TTF may
 * still live in the storage partition for tooling, but runtime uses embedded fonts.
 */
void buddy_dynamic_fonts_init(void){
    ESP_LOGI("fonts", "using bundled CJK subset fonts (no FreeType SPIFFS load)");
}

const lv_font_t *buddy_font_resolve(const lv_font_t *font){
    return font;
}
