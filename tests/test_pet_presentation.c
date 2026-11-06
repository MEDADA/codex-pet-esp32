#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pet_presentation.h"
int main(void){
 buddy_agent_state_t a={.status_valid=true,.status_ms=10};
 pet_presentation_t p=pet_present(&a,true,true,false,10,11);
 assert(!strcmp(p.status,"IDLE"));
 a.status_id=1;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"THINKING"));
 a.status_id=2;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"CODING"));
 a.status_id=3;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"APPROVAL"));
 a.status_id=4;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"QUESTION"));
 a.status_id=0;a.last_event=2;a.event_ms=10;
 p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"ERROR"));
 a.last_event=1;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"SUCCESS"));
 p=pet_present(&a,true,true,false,10,6010);assert(!strcmp(p.status,"IDLE"));
 a.status_id=3;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"APPROVAL"));
 a.last_event=0;
 a.status_id=250;p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"UNKNOWN"));
 p=pet_present(&a,true,false,false,10,11);assert(!strcmp(p.status,"OFFLINE"));
 p=pet_present(&a,false,true,false,10,11);assert(!strcmp(p.status,"OFFLINE"));
 p=pet_present(&a,true,true,false,10,60010);assert(!strcmp(p.status,"OFFLINE"));
 p=pet_present(&a,false,false,true,10,60010);assert(!strcmp(p.status,"PAIRING"));
 assert(!strcmp(p.clock,"--:--"));assert(!strcmp(p.date,"--/--"));
 a.time_valid=true;a.time_has_minute=false;a.hour_hint=13;
 p=pet_present(&a,true,true,false,0,10);assert(!strcmp(p.clock,"--:--"));
 a.time_has_minute=true;a.hour_hint=23;a.minute_hint=59;a.second_hint=59;
 a.time_sync_ms=0xfffffff0u;a.date_valid=true;a.year_hint=2024;a.month_hint=2;a.day_hint=28;
 p=pet_present(&a,true,true,false,0xfffffff0u,1984);assert(!strcmp(p.clock,"00:00"));assert(!strcmp(p.date,"02/29"));
 a.month_hint=13;p=pet_present(&a,true,true,false,0,1984);assert(!strcmp(p.date,"--/--"));
 a.hour_hint=25;p=pet_present(&a,true,true,false,0,1984);assert(!strcmp(p.clock,"--:--"));
 a.hour_hint=12;a.status_id=2;a.status_valid=false;
 p=pet_present(&a,true,true,false,10,11);assert(!strcmp(p.status,"OFFLINE"));
 a.status_valid=true;a.status_ms=10;
 p=pet_present(&a,true,true,false,60010,60011);assert(!strcmp(p.status,"OFFLINE"));
 p=pet_present(NULL,true,true,false,0,10);assert(!strcmp(p.status,"OFFLINE"));
 a.status_valid=true;a.status_ms=10;
 p=pet_present(&a,true,true,false,10,11);assert(p.bridge_live&&!strcmp(p.bridge,"CodeIsland · 已连接"));
 p=pet_present(&a,false,true,false,10,11);assert(!p.bridge_live&&!strcmp(p.bridge,"CodeIsland · 未连接"));
 p=pet_present(&a,true,false,false,10,11);assert(!p.bridge_live&&!strcmp(p.bridge,"CodeIsland · 待配对"));
 a.status_valid=false;p=pet_present(&a,true,true,false,10,11);assert(!p.bridge_live&&!strcmp(p.bridge,"CodeIsland · 同步中"));
 a.status_valid=true;a.status_ms=10;p=pet_present(&a,true,true,false,60011,60011);assert(!p.bridge_live&&!strcmp(p.bridge,"CodeIsland · 同步中"));
 puts("PASS pet state truth, request states, clock validation and timer wrap");
}
