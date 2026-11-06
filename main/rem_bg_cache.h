#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Single-slot JPEG→PSRAM RGB565 cache (~434KB). Swap on scene change; no multi-slot warm. */
bool rem_bg_cache_init(void);
const lv_image_dsc_t *rem_bg_cached(uint8_t index);
const uint8_t *rem_bg_pixels(void);
uint16_t rem_bg_width(void);
uint16_t rem_bg_height(void);
uint8_t rem_bg_index(void);

#ifdef __cplusplus
}
#endif
