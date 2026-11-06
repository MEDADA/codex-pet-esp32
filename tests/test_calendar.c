#include <assert.h>
#include <stdio.h>
#include "buddy_protocol.h"
int main(void){
    assert(buddy_date_valid(2024,2,29));assert(!buddy_date_valid(2026,2,29));assert(!buddy_date_valid(2026,4,31));
    buddy_agent_state_t s={.hour_hint=23,.minute_hint=59,.second_hint=59,.year_hint=2024,.month_hint=2,.day_hint=28,.time_valid=true,.time_has_minute=true,.date_valid=true,.time_sync_ms=0xfffffff0u};
    uint8_t h,m,sec,mo,d;uint16_t y;
    buddy_wall_time_now(&s,1984,&h,&m,&sec);assert(h==0&&m==0&&sec==1);
    buddy_wall_date_now(&s,1984,&y,&mo,&d);assert(y==2024&&mo==2&&d==29);
    puts("PASS calendar leap date and timer wrap");
}
