#include "buddy_state_evidence.h"
bool buddy_apply_time_hint(buddy_agent_state_t *st,const uint8_t *data,uint16_t len,uint32_t now){
 if(!st||!data||(len!=2&&len!=3&&len!=4&&len!=8)||data[0]!=BUDDY_TIME_HINT_FRAME||data[1]>23||(len>=3&&data[2]>59)||(len>=4&&data[3]>59))return false;
 if(len==8&&!buddy_date_valid(((uint16_t)data[4]<<8)|data[5],data[6],data[7]))return false;
 uint8_t hour=data[1];
        if (len >= 3) {
            uint8_t minute = data[2];
            if (minute > 59) minute = 59;
            uint8_t second = 0;
            if (len >= 4) {
                second = data[3];
                if (second > 59) second = 59;
            }
            st->hour_hint = hour;
            st->minute_hint = minute;
            st->second_hint = second;
            st->time_has_minute = true;
            st->time_sync_ms = now;
            if (len >= 8) {
                uint16_t year = ((uint16_t)data[4] << 8) | data[5];
                uint8_t month = data[6];
                uint8_t day = data[7];
                if (year >= 2000 && year <= 2099 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
                    st->year_hint = year;
                    st->month_hint = month;
                    st->day_hint = day;
                    st->date_valid = true;
                }
            }
        } else {
            /* Hour-only: preserve locally advanced minute/second. */
            if (st->time_valid) {
                uint8_t ch = 0, cm = 0, cs = 0;
                buddy_wall_time_now(st, now, &ch, &cm, &cs);
                if (hour != ch) {
                    st->hour_hint = hour;
                    st->minute_hint = cm;
                    st->second_hint = cs;
                    st->time_sync_ms = now;
                }
                /* same hour: leave hints + sync alone so minutes keep ticking */
                /* Hour-only input cannot establish minute precision. */
            } else {
                st->hour_hint = hour;
                st->minute_hint = 0;
                st->second_hint = 0;
                st->time_has_minute = false;
                st->time_sync_ms = now;
            }
        }
        st->time_valid = true;
 return true;
}
void buddy_status_invalidate(buddy_agent_state_t *st){st->status_valid=false;st->last_event=0;}
void buddy_status_received(buddy_agent_state_t *st,uint32_t now){st->status_valid=true;st->status_ms=now;}
bool buddy_status_fresh(const buddy_agent_state_t *st,uint32_t now){return st&&st->status_valid&&(uint32_t)(now-st->status_ms)<BUDDY_BLE_TIMEOUT_MS;}
