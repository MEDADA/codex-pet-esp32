#include "buddy_ui.h"
#include "buddy_display_guard.h"
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "buddy_ble.h"
#include "buddy_button.h"
#include "buddy_settings.h"
#include "buddy_rtc.h"
#include "fonts/buddy_fonts.h"
#include "pet_view_lvgl9.h"
#include "pet_motion.h"
#include "esp_log.h"
#include "esp_system.h"

static bool initialized;
static QueueHandle_t presentations;
static bool restart_armed;
static uint32_t first_power_press;
enum { RESTART_DOUBLE_PRESS_MS = 700 };
static uint32_t now_ms(void){return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);}

static void render_presentation(lv_timer_t *timer)
{
 (void)timer;
 pet_presentation_t presentation;
 if(xQueueReceive(presentations,&presentation,0)==pdTRUE)
  pet_view_show(&presentation,now_ms());
}

void buddy_ui_init(void){
 if(initialized||!buddy_display_try_lock(50))return;
 pet_motion_start();
 buddy_dynamic_fonts_init();
 lv_obj_t *screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_set_style_bg_color(screen,lv_color_hex(0x050711),0);
 pet_view_create(screen);
 presentations=xQueueCreate(1,sizeof(pet_presentation_t));
 if(presentations&&!lv_timer_create(render_presentation,30,NULL)){
  vQueueDelete(presentations);
  presentations=NULL;
 }
 if(!presentations)ESP_LOGW("pet_v11","presentation queue unavailable; using display lock");
 lv_screen_load(screen);
 bsp_display_brightness_set(buddy_settings_get_brightness());
 initialized=true;
 bsp_display_unlock();
 ESP_LOGI("pet_v11","V11 stage initialized; optional IMU glint feedback");
}
void buddy_ui_tick(void){
 if(!initialized){buddy_ui_init();return;}
 buddy_settings_persist();
 uint32_t now=now_ms();buddy_btn_t b=buddy_button_poll(now);
 if(restart_armed && now-first_power_press>RESTART_DOUBLE_PRESS_MS)
  restart_armed=false;
 if(b.ev!=BUDDY_BTN_NONE){
  const bool short_power=b.src==BUDDY_BTN_SRC_PWR&&b.ev==BUDDY_BTN_SHORT;
  if(short_power&&restart_armed){
   ESP_LOGI("pet_v11","PWR double press: restarting");
   esp_restart();
  }
  restart_armed=short_power;
  first_power_press=now;
 }
 static buddy_power_snapshot_t power;
 static buddy_rtc_time_t rtc_time;
 static bool rtc_valid, sampled;
 static uint32_t last_rtc_poll, last_power_poll;
 if(!sampled || now-last_rtc_poll>=1000){
  rtc_valid=buddy_rtc_read(&rtc_time);
  last_rtc_poll=now;
 }
 if(!sampled || now-last_power_poll>=4000){
  buddy_power_snapshot_t next_power=buddy_power_read();
  if(!sampled||power.present!=next_power.present||power.valid!=next_power.valid||
     power.charging!=next_power.charging||power.percent!=next_power.percent){
   ESP_LOGI("buddy_power","present=%d valid=%d charging=%d percent=%u",
            next_power.present,next_power.valid,next_power.charging,next_power.percent);
  }
  power=next_power;
  last_power_poll=now;
 }
 sampled=true;
 buddy_agent_state_t agent;
 buddy_mode_t mode;
 bool connected,authenticated;
 uint32_t last_data;
 buddy_ble_snapshot(&agent,&mode,&connected,&authenticated,&last_data,NULL);
 if(mode==BUDDY_MODE_PAIR_CONFIRM){
  if((uint32_t)(now-buddy_ble_pair_request_ms())>BUDDY_PAIR_CONFIRM_MS)buddy_ble_reject_pair();
  else if(b.src==BUDDY_BTN_SRC_BOOT&&b.ev==BUDDY_BTN_SHORT)buddy_ble_accept_pair();
  else if(b.src==BUDDY_BTN_SRC_BOOT&&(b.ev==BUDDY_BTN_LONG||b.ev==BUDDY_BTN_SUPER_LONG))buddy_ble_reject_pair();
 }else if(b.ev!=BUDDY_BTN_NONE){
  if(b.src==BUDDY_BTN_SRC_BOOT&&b.ev==BUDDY_BTN_SUPER_LONG){buddy_ble_clear_pair();buddy_ble_disconnect_client("pair reset");}
  else if(b.src==BUDDY_BTN_SRC_BOOT&&b.ev==BUDDY_BTN_LONG)pet_motion_recalibrate();
 }
 uint32_t render_now=now_ms();
 pet_presentation_t p=pet_present(&agent,connected,authenticated,mode==BUDDY_MODE_PAIR_CONFIRM,last_data,render_now);
 p.power=power;
 if(rtc_valid&&(!p.bridge_live||p.clock[0]=='-')){
  snprintf(p.clock,sizeof(p.clock),"%02u:%02u",rtc_time.hour,rtc_time.minute);
  snprintf(p.date,sizeof(p.date),"%02u/%02u",rtc_time.month,rtc_time.day);
 }
 if(presentations){
  xQueueOverwrite(presentations,&p);
 }else if(buddy_display_try_lock(50)){
  pet_view_show(&p,render_now);
  bsp_display_unlock();
 }
}
