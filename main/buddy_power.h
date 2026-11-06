#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool present,valid,charging; uint8_t percent; } buddy_power_snapshot_t;
buddy_power_snapshot_t buddy_power_decode(uint8_t status1,uint8_t status2,uint8_t percent);
buddy_power_snapshot_t buddy_power_read(void);
