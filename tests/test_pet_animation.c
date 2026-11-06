#include <assert.h>
#include <stdio.h>
#include "pet_animation.h"
#include "pet_presentation.h"
int main(void){
 pet_animation_t a={0};pet_animation_frame_t f;
 f=pet_animation_step(&a,0,false,false);assert(!f.blink&&!f.question&&!f.coding);
 f=pet_animation_step(&a,4199,true,false);assert(!f.blink&&f.question&&!f.coding);
 f=pet_animation_step(&a,4200,false,true);assert(f.blink&&!f.question&&f.coding);
 f=pet_animation_step(&a,4319,false,false);assert(f.blink&&!f.question&&!f.coding);
 f=pet_animation_step(&a,4320,false,false);assert(!f.blink);
 unsigned next=0;
 for(unsigned t=4321;t<=11120;t++){f=pet_animation_step(&a,t,false,false);if(f.blink){next=t;break;}}
 assert(next>=7620&&next<=11120);
 f=pet_animation_step(&a,next+10000,true,false);assert(!f.blink&&f.question);
 a=(pet_animation_t){0};
 pet_animation_step(&a,0xfffffff0u,false,false);
 f=pet_animation_step(&a,4183,false,false);assert(!f.blink);
 f=pet_animation_step(&a,4184,false,false);assert(f.blink);
 f=pet_animation_step(&a,4304,false,false);assert(!f.blink);
 a=(pet_animation_t){0};f=pet_animation_step(&a,100000,false,false);assert(!f.blink);
 f=pet_animation_step(&a,104200,true,false);assert(f.blink&&f.question);
 f=pet_animation_step(&a,104201,false,true);assert(f.blink&&!f.question&&f.coding);
 buddy_agent_state_t agent={.status_valid=true,.status_ms=10,.status_id=2};
 pet_presentation_t p=pet_present(&agent,true,true,false,10,20);
 f=pet_animation_step(&a,104202,p.thinking,p.coding);assert(f.coding&&!f.question);
 p=pet_present(&agent,false,false,false,10,21);
 f=pet_animation_step(&a,104203,p.thinking,p.coding);assert(!f.coding&&!f.question);
 agent.status_id=3;p=pet_present(&agent,true,true,false,10,22);
 f=pet_animation_step(&a,104204,p.thinking,p.coding);assert(!f.coding&&!f.question);
 agent.status_id=1;p=pet_present(&agent,true,true,false,10,23);
 f=pet_animation_step(&a,104205,p.thinking,p.coding);assert(f.question&&!f.coding);
 p=pet_present(&agent,true,true,false,60010,60011);
 f=pet_animation_step(&a,104206,p.thinking,p.coding);assert(!f.question&&!f.coding);
 pet_animation_tap(&a,110000);
 f=pet_animation_step(&a,110000,false,false);assert(f.sparkle&&f.blink);
 f=pet_animation_step(&a,110160,false,false);assert(f.sparkle&&f.offset_y<0);
 f=pet_animation_step(&a,110801,false,false);assert(!f.sparkle&&f.offset_x==0&&f.offset_y==0);
 pet_animation_tap(&a,0xfffffff0u);
 f=pet_animation_step(&a,64,false,true);assert(f.sparkle&&f.coding);
 f=pet_animation_step(&a,900,false,false);assert(!f.sparkle&&f.offset_x==0&&f.offset_y==0);
 a=(pet_animation_t){0};
 pet_animation_step(&a,0,false,false);
 pet_animation_tap(&a,1000);pet_animation_tap(&a,1159);
 f=pet_animation_step(&a,1160,false,false);assert(f.offset_y<0&&f.offset_x==0);
 pet_animation_tap(&a,1160);f=pet_animation_step(&a,1160,false,false);assert(f.offset_x<0&&f.offset_y==0);
 pet_animation_tap(&a,2000);f=pet_animation_step(&a,1999,false,false);assert(f.sparkle);
 puts("PASS blink timing, wrap, delayed tick, cue transitions and recreation");
}
