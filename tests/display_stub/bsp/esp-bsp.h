#pragma once
#include <stdint.h>
/* Same return convention as actual Waveshare BSP: esp_err_t, ESP_OK == 0. */
#define ESP_OK 0
int bsp_display_lock(uint32_t timeout_ms);
