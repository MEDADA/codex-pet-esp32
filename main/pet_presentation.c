#include "pet_presentation.h"
#include "buddy_state_evidence.h"
#include <stdio.h>
pet_presentation_t pet_present(const buddy_agent_state_t *a,bool connected,bool authenticated,bool pairing,uint32_t last_ms,uint32_t now){
 pet_presentation_t p={.status="OFFLINE",.activity=PET_ACTIVITY_OFFLINE,.bridge="CodeIsland · 未连接",.bridge_color=0x717887,.accent=0x8a90a3,.clock="--:--",.date="--/--"};
 p.bridge_live=a&&buddy_status_fresh(a,now)&&connected&&authenticated&&(uint32_t)(now-last_ms)<BUDDY_BLE_TIMEOUT_MS;
 if(pairing||(connected&&!authenticated)){
  p.bridge="CodeIsland · 待配对";
  p.bridge_color=0xf0ba65;
 }else if(connected&&authenticated){
  p.bridge=p.bridge_live?"CodeIsland · 已连接":"CodeIsland · 同步中";
  p.bridge_color=p.bridge_live?0x64e8a6:0x74bbff;
 }
 if(pairing){p.activity=PET_ACTIVITY_APPROVAL;p.status="PAIRING";p.accent=0xf0ba65;}
 else if(a&&buddy_status_fresh(a,now)&&connected&&authenticated&&(uint32_t)(now-last_ms)<BUDDY_BLE_TIMEOUT_MS){
  if(a->status_id!=3&&a->status_id!=4&&a->last_event&&(uint32_t)(now-a->event_ms)<6000u){
   p.activity=a->last_event==2?PET_ACTIVITY_ERROR:a->last_event==1?PET_ACTIVITY_SUCCESS:PET_ACTIVITY_IDLE;
   p.status=a->last_event==2?"ERROR":a->last_event==1?"SUCCESS":"UNKNOWN";
   p.accent=a->last_event==2?0xfc8294:a->last_event==1?0x4fe2a0:0x9ba6c7;
  }else switch(a->status_id){
   case 0:p.activity=PET_ACTIVITY_IDLE;p.status="IDLE";p.accent=0x9ba6c7;break;
   case 1:p.activity=PET_ACTIVITY_THINKING;p.status="THINKING";p.accent=0xbba5ef;p.thinking=true;break;
   case 2:p.activity=PET_ACTIVITY_CODING;p.status="CODING";p.accent=0x62dfc0;p.coding=true;break;
   case 3:p.activity=PET_ACTIVITY_APPROVAL;p.status="APPROVAL";p.accent=0xf0ba65;break;
   case 4:p.activity=PET_ACTIVITY_QUESTION;p.status="QUESTION";p.accent=0xc9a7ff;break;
   default:p.status="UNKNOWN";p.accent=0xfc8294;break;
  }
 }
 if(a&&a->time_valid&&a->time_has_minute&&a->hour_hint<24&&a->minute_hint<60&&a->second_hint<60){
  uint8_t h,m,s;buddy_wall_time_now(a,now,&h,&m,&s);
  snprintf(p.clock,sizeof(p.clock),"%02u:%02u",h,m);
  if(a->date_valid&&buddy_date_valid(a->year_hint,a->month_hint,a->day_hint)){
   uint16_t y;uint8_t mo,d;buddy_wall_date_now(a,now,&y,&mo,&d);
   snprintf(p.date,sizeof(p.date),"%02u/%02u",mo,d);
  }
 }
 return p;
}
