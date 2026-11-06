#pragma once
#include "lvgl.h"

LV_FONT_DECLARE(font_buddy_cn_16);
LV_FONT_DECLARE(font_buddy_cn_20);
LV_FONT_DECLARE(font_buddy_clock_64);
void buddy_dynamic_fonts_init(void);
const lv_font_t *buddy_font_resolve(const lv_font_t *font);
