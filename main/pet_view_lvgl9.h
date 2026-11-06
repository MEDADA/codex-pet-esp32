#pragma once
#include "lvgl.h"
#include "pet_presentation.h"
/* Caller must hold BSP display lock. All lifetime owned by parent. */
void pet_view_create(lv_obj_t *parent);
void pet_view_show(const pet_presentation_t *presentation,uint32_t now_ms);
