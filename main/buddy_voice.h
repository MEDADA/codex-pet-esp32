#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "buddy_scene.h"
typedef struct { buddy_voice_state_t state; char text[256]; char error[96]; bool configured; } buddy_voice_snapshot_t;
void buddy_voice_init(void);
esp_err_t buddy_voice_start(void);
void buddy_voice_stop_capture(void);
void buddy_voice_cancel(void);
void buddy_voice_snapshot(buddy_voice_snapshot_t *out);
bool buddy_network_connected(void);
