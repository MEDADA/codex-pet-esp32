#include "buddy_motion.h"
#include <math.h>
#include <string.h>
void buddy_motion_update(buddy_motion_filter_t *f,const buddy_imu_sample_t *s,uint32_t now,buddy_motion_output_t *o){
    memset(o,0,sizeof(*o));
    if(!f||!s||!isfinite(s->x)||!isfinite(s->y)||!isfinite(s->z))return;
    float mag=sqrtf(s->x*s->x+s->y*s->y+s->z*s->z);
    if(mag<.5f||mag>1.7f)return;
    if(!f->initialized){f->initialized=true;f->x=s->x;f->y=s->y;f->z=s->z;f->armed=fabsf(s->z)>.85f;}
    f->x+=.18f*(s->x-f->x);f->y+=.18f*(s->y-f->y);f->z+=.18f*(s->z-f->z);
    o->offset_x=(int8_t)fmaxf(-10,fminf(10,f->x*12));o->offset_y=(int8_t)fmaxf(-6,fminf(6,f->y*8));
    float angle=atan2f(f->x,-f->y);if(angle<0)angle+=6.28318530718f;
    o->direction=((int)(angle/0.3926990817f+.5f))%16;
    if(fabsf(f->z)>.88f)f->armed=true;
    if(f->armed&&fabsf(f->z)<.75f&&(uint32_t)(now-f->last_wake)>1500){o->wake=true;f->last_wake=now;f->armed=false;}
}
