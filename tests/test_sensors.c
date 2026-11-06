#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "buddy_power.h"
#include "buddy_motion.h"
int main(void){
    buddy_power_snapshot_t p=buddy_power_decode(0x08,0x20,86);
    assert(p.present&&p.valid&&p.charging&&p.percent==86);
    p=buddy_power_decode(0,0,0);assert(!p.present&&!p.valid);
    p=buddy_power_decode(8,0,255);assert(p.present&&!p.valid);
    buddy_motion_filter_t f={0};buddy_motion_output_t o;
    buddy_imu_sample_t a={.z=1};buddy_motion_update(&f,&a,0,&o);assert(!o.wake);
    buddy_motion_update(&f,&a,2000,&o);assert(!o.wake);
    a.x=.8f;a.z=.6f;
    for(int n=0;n<20;n++)buddy_motion_update(&f,&a,2100+n*50,&o);
    assert(o.offset_x>0&&o.offset_x<=10);assert(o.direction<16);
    a.x=NAN;buddy_motion_update(&f,&a,4000,&o);assert(!o.wake);
    puts("PASS battery validity and bounded motion");
}
