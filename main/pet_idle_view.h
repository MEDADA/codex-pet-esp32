#pragma once

#include "lvgl.h"

/* Called under the existing display lock. */
void pet_idle_view_init(lv_obj_t *avatar);
void pet_idle_view_set_active(bool active);
void pet_idle_view_show_still(void);

enum { PET_IDLE_FRAME_COUNT = 96 };
const uint8_t *pet_idle_frame_data(unsigned index, size_t *size);
