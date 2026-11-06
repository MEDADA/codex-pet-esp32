#include "pet_animation.h"


void pet_animation_set_activity(pet_animation_t *a,pet_activity_t activity,uint32_t now){
    if(!a->activity_set||a->activity!=activity){
        a->activity=activity;a->activity_ms=now;a->activity_set=true;
    }
}
static void activity_motion(const pet_animation_t *a,uint32_t now,pet_animation_frame_t *f){
    if(!a->activity_set)return;
    uint32_t age=now-a->activity_ms,t;
    switch(a->activity){
    case PET_ACTIVITY_IDLE:
        t=age%3200;f->offset_y=(int8_t)(t<1600?2*t/1600:2*(3200-t)/1600);break;
    case PET_ACTIVITY_THINKING:
        t=age%2800;
        f->offset_x=(int8_t)(t<400?-(int)(6*t/400):t<1000?-6:t<1800?-6+(int)(12*(t-1000)/800):t<2400?6:6-(int)(6*(t-2400)/400));
        break;
    case PET_ACTIVITY_CODING:
        t=age%1400;f->offset_y=(int8_t)(t<220?4*t/220:t<440?4*(440-t)/220:0);break;
    case PET_ACTIVITY_APPROVAL:
        t=age%2400;
        if(t>=480&&t<840)t-=480;
        f->offset_y=(int8_t)(t<180?-(int)(6*t/180):t<360?-(int)(6*(360-t)/180):0);break;
    case PET_ACTIVITY_QUESTION:
        t=age%1800;f->question=true;
        f->offset_x=(int8_t)(t<450?-(int)(4*t/450):t<1350?-4+(int)(8*(t-450)/900):4-(int)(4*(t-1350)/450));break;
    case PET_ACTIVITY_SUCCESS:
        f->sparkle=age<800;
        f->offset_y=(int8_t)(age<200?-(int)(8*age/200):age<400?-(int)(8*(400-age)/200):0);break;
    case PET_ACTIVITY_ERROR:
        f->offset_x=age<640?((age/80)%2?4:-4):0;break;
    case PET_ACTIVITY_OFFLINE:f->offset_y=2;break;
    }
}

pet_animation_frame_t pet_animation_step(pet_animation_t *a, uint32_t now,
                                         bool thinking, bool coding)
{
    if (!a->initialized) {
        a->initialized = true;
        a->blinking = false;
        a->phase_ms = now;
        a->wait_ms = 4200;
        a->rng = 0x9e3779b9u;
    }
    uint32_t elapsed = now - a->phase_ms;
    if (a->blinking && elapsed >= 120) {
        a->blinking = false;
        a->phase_ms = now;
        a->rng ^= a->rng << 13;
        a->rng ^= a->rng >> 17;
        a->rng ^= a->rng << 5;
        a->wait_ms = 3300 + a->rng % 3501;
    } else if (!a->blinking && elapsed >= a->wait_ms) {
        a->blinking = true;
        a->phase_ms = now;
    }
    pet_animation_frame_t frame={.blink=a->blinking,.question=thinking,.coding=coding};
    activity_motion(a,now,&frame);
    if(a->reacting){
        uint32_t age=now-a->tap_ms;
        /* A just-delivered UI event may be newer than a caller snapshot. */
        if((int32_t)age<0)age=0;
        if(age>=800)a->reacting=false;
        else{
            frame.sparkle=true;
            if(age<120)frame.blink=true;
            if(a->tap_variant&1){
                frame.offset_y=age<120?-3:age<240?-7:age<360?-3:0;
            }else{
                frame.offset_x=age<120?-4:age<240?4:age<360?-2:0;
            }
        }
    }
    return frame;
}

void pet_animation_tap(pet_animation_t *a,uint32_t now){
    if(!a->initialized)(void)pet_animation_step(a,now,false,false);
    if(a->reacting&&(uint32_t)(now-a->tap_ms)<160)return;
    a->tap_ms=now;a->reacting=true;++a->tap_variant;
}
