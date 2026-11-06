#include <cassert>
#include <cmath>
#include <cstdio>
#include "pet_motion_tracker.hpp"
int main(){
 PetMotionTracker t;
 pet_imu_sample_t s={0,0,1,0,0,0};
 auto f=t.update(s,true,100);assert(f.valid&&!f.calibrated);
 for(unsigned now=140;now<=2300;now+=40)f=t.update(s,true,now);
 assert(f.calibrated&&std::fabs(f.gaze_x)<.1f&&std::fabs(f.gaze_y)<.1f);
 s.ay=.25f;s.az=.968246f;
 for(unsigned now=2340;now<=3100;now+=40)f=t.update(s,true,now);
 assert(f.calibrated&&f.gaze_x>0&&f.gaze_x<=3&&std::fabs(f.gaze_y)<=2);
 f=t.update(s,false,3500);assert(!f.valid&&f.gaze_x==0);
 f=t.update(s,true,3540);assert(f.calibrated);
 t.recalibrate();
 for(unsigned now=4000;now<7200;now+=40){s.gx=12;f=t.update(s,true,now);assert(!f.calibrated);}
 s={0,0,1,0,0,0};
 for(unsigned now=7240;now<9500;now+=40)f=t.update(s,true,now);
 assert(f.calibrated);
 s.ax=NAN;f=t.update(s,true,9540);assert(!f.valid&&f.gaze_x==0);
 PetMotionTracker wrap;s={0,0,1,0,0,0};
 for(unsigned i=0;i<60;i++)f=wrap.update(s,true,0xfffffff0u+i*40u);
 assert(f.calibrated);
 PetMotionTracker moving;
 for(unsigned i=0;i<100;i++){s.ax=(i%2)*.1f;s.az=1;f=moving.update(s,true,i*40);assert(!f.calibrated);}
 PetMotionTracker gap;s={0,0,1,0,0,0};
 for(unsigned i=0;i<39;i++)gap.update(s,true,i*40);
 f=gap.update(s,true,5000);assert(!f.calibrated);
 PetMotionTracker cut;s={0,.0174524f,-.999848f,0,0,0};
 for(unsigned i=0;i<60;i++)f=cut.update(s,true,i*40);
 assert(f.calibrated);
 s.ay=-.0174524f;
 for(unsigned i=60;i<70;i++)f=cut.update(s,true,i*40);
 f=cut.update(s,false,2800);assert(!f.valid);
 for(unsigned i=71;i<85;i++)f=cut.update(s,true,i*40);
 assert(std::fabs(f.gaze_x)<.1f);
 PetMotionTracker high;
 s={0,0,1,0,0,0};
 for(unsigned i=0;i<60;i++)f=high.update(s,true,0xffffd000u+i*40u);
 assert(f.calibrated&&!f.shake);
 puts("PASS neutral calibration, movement rejection, glint bounds, dropout, NaN, reset and wrap");
}
