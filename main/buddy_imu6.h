#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { float ax,ay,az,gx,gy,gz; } pet_imu_sample_t;
bool buddy_imu6_read(pet_imu_sample_t *sample);
/* CTRL2=0x26 (8g), CTRL3=0x56 (512dps), little-endian QMI data. */
static inline pet_imu_sample_t buddy_imu6_decode(const uint8_t b[12]) {
    float values[6];
    for(int i=0;i<6;i++)values[i]=(int16_t)((uint16_t)b[2*i]|((uint16_t)b[2*i+1]<<8))/(i<3?4096.f:64.f);
    pet_imu_sample_t result={values[0],values[1],values[2],values[3],values[4],values[5]};
    return result;
}
#ifdef __cplusplus
}
#endif
