#pragma once
#include <stdbool.h>
#include "bsp/esp-bsp.h"
static inline bool buddy_display_try_lock(uint32_t timeout_ms){
    return bsp_display_lock(timeout_ms)==ESP_OK;
}
