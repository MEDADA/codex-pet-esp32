#include <assert.h>
#include <stdio.h>
#include "buddy_state_evidence.h"
int main(void){
 buddy_agent_state_t a={0};uint8_t hour[]={0xf6,13};
 assert(buddy_apply_time_hint(&a,hour,2,100));assert(a.time_valid&&!a.time_has_minute);
 assert(buddy_apply_time_hint(&a,hour,2,200));assert(!a.time_has_minute);
 hour[1]=14;assert(buddy_apply_time_hint(&a,hour,2,300));assert(!a.time_has_minute);
 uint8_t full[]={0xf6,14,37,58,0x07,0xea,11,3};
 assert(buddy_apply_time_hint(&a,full,8,1000));assert(a.time_has_minute&&a.date_valid&&a.minute_hint==37);
 assert(buddy_apply_time_hint(&a,hour,2,2000));assert(a.time_has_minute);
 uint8_t h,m,s;buddy_wall_time_now(&a,2000,&h,&m,&s);assert(h==14&&m==37&&s==59);
 full[6]=13;assert(!buddy_apply_time_hint(&a,full,8,3000));
 buddy_status_received(&a,0xfffffff0u);assert(buddy_status_fresh(&a,1984));
 buddy_status_invalidate(&a);assert(!buddy_status_fresh(&a,1984));
 buddy_status_received(&a,100);assert(buddy_status_fresh(&a,60099));assert(!buddy_status_fresh(&a,60100));
 puts("PASS primary-state lifecycle and repeated hour-only frame sequence");
}
