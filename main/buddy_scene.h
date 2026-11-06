#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUDDY_SCENE_SLEEP=0, BUDDY_SCENE_WORK, BUDDY_SCENE_ALERT, BUDDY_SCENE_QUESTION,
    BUDDY_SCENE_SUCCESS, BUDDY_SCENE_ERROR, BUDDY_SCENE_OFFLINE,
    BUDDY_SCENE_CHARGING, BUDDY_SCENE_LISTENING, BUDDY_SCENE_SPEAKING,
    BUDDY_SCENE_PAIRING
} buddy_scene_t;
typedef enum {
    BUDDY_VOICE_UNAVAILABLE=0, BUDDY_VOICE_IDLE, BUDDY_VOICE_LISTENING,
    BUDDY_VOICE_TRANSCRIBING, BUDDY_VOICE_THINKING, BUDDY_VOICE_SPEAKING, BUDDY_VOICE_ERROR
} buddy_voice_state_t;
typedef struct {
    bool connected, authenticated, fresh, pairing, charging;
    uint8_t status, last_event;
    uint32_t event_age_ms;
    buddy_voice_state_t voice;
} buddy_scene_input_t;
typedef struct { uint8_t background; uint32_t ink, accent; const char *label; } buddy_palette_t;
buddy_scene_t buddy_scene_resolve(const buddy_scene_input_t *in);
buddy_palette_t buddy_scene_palette(buddy_scene_t scene);
