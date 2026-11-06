#pragma once

#include "lvgl.h"

typedef enum {
    PET_CLIP_NONE,
    PET_CLIP_CURIOUS, PET_CLIP_SHY,
    PET_CLIP_LEFT, PET_CLIP_RIGHT, PET_CLIP_FORWARD, PET_CLIP_BACKWARD,
    PET_CLIP_SHAKE,
    PET_CLIP_HEAD, PET_CLIP_CHEEK, PET_CLIP_HANDS, PET_CLIP_TAIL,
    PET_CLIP_WORKING, PET_CLIP_IDLE,
} pet_clip_id_t;

void pet_motion_clip_init(lv_obj_t *avatar);
bool pet_motion_clip_begin(pet_clip_id_t id);
bool pet_motion_clip_active(void);
void pet_motion_clip_pause_idle(void);
void pet_motion_clip_stop(void);
