#pragma once
#include "buddy_imu6.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    bool valid, calibrated, shake;
    float gaze_x,gaze_y,roll,pitch;
    pet_imu_sample_t raw;
} pet_motion_frame_t;
void pet_motion_start(void);
void pet_motion_snapshot(pet_motion_frame_t *out);
void pet_motion_recalibrate(void);
#ifdef __cplusplus
}
#endif
