#include <assert.h>
#include <stdio.h>
#include "buddy_imu6.h"
int main(void){
 uint8_t b[12]={0,0x10,0,0xf0,0,0,0x40,0,0xc0,0xff,0,0x20};
 pet_imu_sample_t s=buddy_imu6_decode(b);
 assert(s.ax==1&&s.ay==-1&&s.az==0&&s.gx==1&&s.gy==-1&&s.gz==128);
 puts("PASS QMI little-endian accel g and gyro dps decoding");
}
