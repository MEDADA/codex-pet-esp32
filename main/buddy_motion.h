#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { float x,y,z; } buddy_imu_sample_t;
typedef struct { bool initialized,armed; float x,y,z; uint32_t last_wake; } buddy_motion_filter_t;
typedef struct { int8_t offset_x,offset_y; uint8_t direction; bool wake; } buddy_motion_output_t;
void buddy_motion_update(buddy_motion_filter_t *f,const buddy_imu_sample_t *sample,uint32_t now,buddy_motion_output_t *out);
void buddy_motion_init(void);
bool buddy_motion_poll(uint32_t now,buddy_motion_output_t *out);
