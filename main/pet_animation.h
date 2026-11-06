#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum {
    PET_ACTIVITY_IDLE=0, PET_ACTIVITY_THINKING, PET_ACTIVITY_CODING,
    PET_ACTIVITY_APPROVAL, PET_ACTIVITY_QUESTION, PET_ACTIVITY_SUCCESS,
    PET_ACTIVITY_ERROR, PET_ACTIVITY_OFFLINE
} pet_activity_t;
typedef struct {
    bool activity_set;
    pet_activity_t activity;
    uint32_t activity_ms;
    bool initialized, blinking;
    uint32_t phase_ms, wait_ms, rng, tap_ms;
    bool reacting;
    uint8_t tap_variant;
} pet_animation_t;
typedef struct { bool blink, question, coding, sparkle; int8_t offset_x,offset_y; } pet_animation_frame_t;
void pet_animation_set_activity(pet_animation_t *animation,pet_activity_t activity,uint32_t now);
void pet_animation_tap(pet_animation_t *animation,uint32_t now);
/* UI-owned state; unsigned elapsed arithmetic supports timer wrap. */
pet_animation_frame_t pet_animation_step(pet_animation_t *animation, uint32_t now,
                                         bool thinking, bool coding);
