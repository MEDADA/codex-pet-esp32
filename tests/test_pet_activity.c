#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "pet_animation.h"
int main(void){
 pet_animation_t a={0};pet_animation_frame_t f;
 pet_animation_set_activity(&a,PET_ACTIVITY_THINKING,100);
 f=pet_animation_step(&a,700,true,false);assert(f.offset_x<0&&f.question);
 f=pet_animation_step(&a,2100,true,false);assert(f.offset_x>0);
 pet_animation_set_activity(&a,PET_ACTIVITY_CODING,2200);
 f=pet_animation_step(&a,2420,false,true);assert(f.offset_x==0&&f.offset_y>0&&f.coding&&!f.question);
 pet_animation_set_activity(&a,PET_ACTIVITY_APPROVAL,3000);
 f=pet_animation_step(&a,3180,false,false);assert(f.offset_y<0&&!f.coding);
 pet_animation_set_activity(&a,PET_ACTIVITY_QUESTION,4000);
 f=pet_animation_step(&a,4500,false,false);assert(f.question&&f.offset_x!=0);
 pet_animation_set_activity(&a,PET_ACTIVITY_SUCCESS,5000);
 f=pet_animation_step(&a,5200,false,false);assert(f.offset_y<0&&f.sparkle);
 f=pet_animation_step(&a,6000,false,false);assert(!f.sparkle&&f.offset_y==0);
 pet_animation_set_activity(&a,PET_ACTIVITY_ERROR,7000);
 f=pet_animation_step(&a,7080,false,false);assert(f.offset_x!=0);
 f=pet_animation_step(&a,7800,false,false);assert(f.offset_x==0);
 pet_animation_set_activity(&a,PET_ACTIVITY_OFFLINE,8000);
 f=pet_animation_step(&a,8100,false,false);assert(!f.coding&&!f.question&&f.offset_y==2);
 for(int mode=PET_ACTIVITY_IDLE;mode<=PET_ACTIVITY_OFFLINE;mode++){
  a=(pet_animation_t){0};pet_animation_set_activity(&a,(pet_activity_t)mode,0xfffffff0u);
  for(unsigned age=0;age<6000;age+=40){
   f=pet_animation_step(&a,0xfffffff0u+age,mode==PET_ACTIVITY_THINKING,mode==PET_ACTIVITY_CODING);
   assert(abs(f.offset_x)<=8&&abs(f.offset_y)<=8);
  }
 }
 puts("PASS real-state character motion, immediate transitions, one-shot events, wrap and bounds");
}
