#pragma once
#include <stdint.h>
#include <stdbool.h>

void buddy_settings_init(void);

uint8_t buddy_settings_get_brightness(void);
void buddy_settings_set_brightness(uint8_t percent); /* 10..100, applies and persists on UI tick */

uint8_t buddy_settings_get_volume(void);
void buddy_settings_set_volume(uint8_t percent); /* 0..100, applies ES8311 if available + NVS */

bool buddy_settings_volume_hw_ok(void);
bool buddy_settings_get_wake(void);
void buddy_settings_set_wake(bool enabled);
void *buddy_settings_speaker(void);

/* Flush preferences whose writers must not block (BLE task). */
void buddy_settings_persist(void);
