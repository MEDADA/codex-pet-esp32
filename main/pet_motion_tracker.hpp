#pragma once
#include "pet_motion.h"
#include "pet_behavior.hpp"
#include <cmath>

class PetMotionTracker {
public:
    void recalibrate() {
        calibrated_=false;count_=0;previous_valid_=false;have_roll_=false;
        behavior_=pet_mvp::Behavior{};shake_active_=have_shaken_=false;
    }
    pet_motion_frame_t update(const pet_imu_sample_t &s,bool valid,uint32_t now) {
        pet_motion_frame_t out{};
        out.raw=s;
        if(shake_active_&&(uint32_t)(now-shake_ms_)>=420)shake_active_=false;
        bool finite=std::isfinite(s.ax)&&std::isfinite(s.ay)&&std::isfinite(s.az)&&
                    std::isfinite(s.gx)&&std::isfinite(s.gy)&&std::isfinite(s.gz);
        float acc=std::sqrt(s.ax*s.ax+s.ay*s.ay+s.az*s.az);
        float gyro=std::sqrt(s.gx*s.gx+s.gy*s.gy+s.gz*s.gz);
        if(!valid||!finite||acc<.2f||acc>5.f){
            count_=0;previous_valid_=false;
            return out;
        }
        out.valid=true;
        constexpr float degrees=57.2957795131f;
        out.roll=std::atan2(s.ay,s.az)*degrees;
        out.pitch=std::atan2(-s.ax,std::hypot(s.ay,s.az))*degrees;
        /* Roll branch cut cannot be a large expression discontinuity. */
        if(have_roll_){
            while(out.roll-last_roll_>180)out.roll-=360;
            while(out.roll-last_roll_<-180)out.roll+=360;
        }
        float dt=previous_valid_?(now-last_ms_)/1000.f:.04f;
        bool stable=acc>.9f&&acc<1.1f&&gyro<3.f;
        if(previous_valid_&&(uint32_t)(now-last_ms_)>200){count_=0;sum_roll_=sum_pitch_=0;}
        if(count_){
            stable=stable&&std::fabs(s.ax-first_.ax)<.025f&&std::fabs(s.ay-first_.ay)<.025f&&std::fabs(s.az-first_.az)<.025f;
        }
        if(!calibrated_){
            if(!stable){count_=0;sum_roll_=sum_pitch_=0;}
            else {
                if(!count_){first_=s;start_ms_=now;sum_roll_=sum_pitch_=0;}
                ++count_;sum_roll_+=out.roll;sum_pitch_+=out.pitch;
                if(count_>=40&&(uint32_t)(now-start_ms_)>=2000){
                    behavior_.calibrate(sum_roll_/count_,sum_pitch_/count_);calibrated_=true;
                }
            }
        }
        if(calibrated_){
            behavior_.sense(out.roll,out.pitch,acc,gyro,dt,now);
            auto expression=behavior_.at(now);
            out.gaze_x=expression.gaze_x;out.gaze_y=expression.gaze_y;
            if(std::fabs(acc-1.f)>.38f&&gyro>80.f&&(!have_shaken_||(uint32_t)(now-shake_ms_)>=2000)){
                shake_ms_=now;shake_active_=have_shaken_=true;
            }
            out.shake=shake_active_;
        }
        out.calibrated=calibrated_;
        last_ms_=now;last_roll_=out.roll;previous_valid_=true;have_roll_=true;
        return out;
    }
private:
    pet_mvp::Behavior behavior_;
    bool calibrated_=false,previous_valid_=false,have_roll_=false;
    uint32_t count_=0,start_ms_=0,last_ms_=0,shake_ms_=0;
    bool shake_active_=false,have_shaken_=false;
    float sum_roll_=0,sum_pitch_=0,last_roll_=0;
    pet_imu_sample_t first_{};
};
