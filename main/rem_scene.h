#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

/** Lightweight: full-screen compose disabled; rem_scene_present always NULL. */
bool rem_scene_init(void);
const lv_image_dsc_t *rem_scene_present(uint8_t bg_index,
                                        const lv_image_dsc_t *sprite,
                                        int rem_x,
                                        int rem_y);
bool rem_scene_last_changed(void);
