#include "pet_view_lvgl9.h"
#include "generated/pet_v41_layers.h"
#include "fonts/buddy_fonts.h"
#include <stdlib.h>
#include <string.h>
#include "pet_animation.h"
#include "pet_idle_view.h"
#include "pet_motion_clip.h"
#include "pet_frame_layout.h"
#include "pet_motion.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
static lv_obj_t *battery,*battery_fill,*battery_tip,*battery_charge;
static lv_obj_t *bg,*state,*clock_text,*date_text,*led,*avatar,*question,*bridge_dot;
static lv_obj_t *idle_tap,*tilt_dot;
static pet_clip_id_t active_clip, queued_touch, tilt_latch;
static uint32_t queued_at, next_special;
static bool shy_next;
static lv_display_t *stats_display;
static int64_t render_start_us,render_window_us,render_total_us;
static uint32_t render_count,render_max_us;
static int64_t flush_started_us, wait_started_us;
static uint64_t flush_total_us, wait_total_us;
static unsigned flush_count, wait_count;
static void flush_metrics(lv_event_t *event){
 const int64_t now=esp_timer_get_time();
 switch(lv_event_get_code(event)){
 case LV_EVENT_FLUSH_START: flush_started_us=now; break;
 case LV_EVENT_FLUSH_FINISH:
  flush_total_us+=now-flush_started_us;
  ++flush_count;
  break;
 case LV_EVENT_FLUSH_WAIT_START: wait_started_us=now; break;
 case LV_EVENT_FLUSH_WAIT_FINISH:
  wait_total_us+=now-wait_started_us;
  ++wait_count;
  break;
 default: break;
 }
}
static void render_metrics(lv_event_t *event){
 int64_t now=esp_timer_get_time();
 if(lv_event_get_code(event)==LV_EVENT_RENDER_START){render_start_us=now;return;}
 if(!render_start_us)return;
 uint32_t duration=(uint32_t)(now-render_start_us);render_start_us=0;
 render_total_us+=duration;++render_count;if(duration>render_max_us)render_max_us=duration;
 if(now-render_window_us>=10000000&&render_count){
  ESP_LOGI("pet_render","n=%u mean_us=%u max_us=%u (LVGL render region, not panel FPS)",
           (unsigned)render_count,(unsigned)(render_total_us/render_count),(unsigned)render_max_us);
  ESP_LOGI("pet_flush","submit_n=%u submit_mean_us=%u wait_n=%u wait_mean_us=%u",
           flush_count,flush_count?(unsigned)(flush_total_us/flush_count):0,
           wait_count,wait_count?(unsigned)(wait_total_us/wait_count):0);
  flush_count=wait_count=0;
  flush_total_us=wait_total_us=0;
  render_window_us=now;render_total_us=0;render_count=render_max_us=0;
 }
}
static pet_animation_t animation;
static pet_presentation_t previous;
static bool painted, working;
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int w,const lv_font_t *font,uint32_t color){
 lv_obj_t *o=lv_label_create(parent);lv_label_set_text(o,text);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
 lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(color),0);return o;
}
static lv_obj_t *feedback_dot(lv_obj_t *parent,int x,int y,int size,uint32_t color){
 lv_obj_t *dot=lv_obj_create(parent);
 lv_obj_remove_style_all(dot);
 lv_obj_set_size(dot,size,size);
 lv_obj_set_pos(dot,x,y);
 lv_obj_set_style_radius(dot,LV_RADIUS_CIRCLE,0);
 lv_obj_set_style_bg_opa(dot,LV_OPA_COVER,0);
 lv_obj_set_style_bg_color(dot,lv_color_hex(color),0);
 lv_obj_remove_flag(dot,LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_flag(dot,LV_OBJ_FLAG_HIDDEN);
 return dot;
}
static bool start_clip(pet_clip_id_t clip){
 if(!pet_motion_clip_begin(clip)){
  active_clip=PET_CLIP_NONE;
  if(working){
   lv_image_set_src(avatar,&pet_v41_character);
   lv_obj_set_pos(avatar,pet_v41_x,pet_v41_y);
  }else{
   pet_idle_view_set_active(true);
   lv_obj_set_pos(avatar,PET_IDLE_X,PET_IDLE_Y);
  }
  return false;
 }
 active_clip=clip;
 if(!working)pet_idle_view_set_active(false);
 return true;
}

static void on_pet_tap(lv_event_t *event){
 (void)event;
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
 const uint32_t now=(uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);
 pet_animation_tap(&animation,now);
 if(working)return;
 lv_indev_t *input=lv_indev_active();
 if(!input)return;
 lv_point_t point;
 lv_indev_get_point(input,&point);
 const int x=(97+(point.x-PET_IDLE_X)*272/PET_IDLE_W)*480/466;
 const int y=(67+(point.y-PET_IDLE_Y)*320/PET_IDLE_H)*480/466;
 pet_clip_id_t clip=PET_CLIP_NONE;
 if(x>=315&&x<=402&&y>=258&&y<=381)clip=PET_CLIP_TAIL;
 else if(x>=201&&x<=297&&y>=277&&y<=360)clip=PET_CLIP_HANDS;
 else if(x>=153&&x<=331&&y>=63&&y<=177)clip=PET_CLIP_HEAD;
 else if(x>=163&&x<=317&&y>=179&&y<=270)clip=PET_CLIP_CHEEK;
 if(clip==PET_CLIP_NONE)return;
 if(active_clip>=PET_CLIP_HEAD&&active_clip<=PET_CLIP_TAIL){
  queued_touch=clip;
  queued_at=now;
 }else{
  start_clip(clip);
 }
#endif
}
static void visibility(lv_obj_t *object,bool visible){
 if(visible)lv_obj_remove_flag(object,LV_OBJ_FLAG_HIDDEN);
 else lv_obj_add_flag(object,LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *image_at(lv_obj_t *parent,const lv_image_dsc_t *src,int x,int y){
 lv_obj_t *image=lv_image_create(parent);
 if(src)lv_image_set_src(image,src);
 lv_obj_set_pos(image,x,y);
 return image;
}

static void create_battery(lv_obj_t *parent){
 battery=feedback_dot(parent,280,25,1,0x141622);
 lv_obj_set_size(battery,22,11);
 lv_obj_set_style_radius(battery,3,0);
 lv_obj_set_style_border_width(battery,1,0);
 lv_obj_set_style_border_color(battery,lv_color_hex(0xaeb6cb),0);
 lv_obj_set_style_pad_all(battery,0,0);
 battery_fill=feedback_dot(battery,1,1,1,0xc4ccdf);
 lv_obj_set_size(battery_fill,18,7);
 lv_obj_set_style_radius(battery_fill,1,0);
 battery_tip=feedback_dot(parent,303,28,3,0xaeb6cb);
 lv_obj_set_size(battery_tip,2,5);
 battery_charge=lv_label_create(parent);
 lv_label_set_text(battery_charge,LV_SYMBOL_CHARGE);
 lv_obj_set_pos(battery_charge,287,22);
 lv_obj_set_style_text_font(battery_charge,&lv_font_montserrat_14,0);
 lv_obj_set_style_text_color(battery_charge,lv_color_hex(0xf8f8ff),0);
 visibility(battery_charge,false);
}

static void show_battery(buddy_power_snapshot_t power){
 visibility(battery,power.present);
 visibility(battery_tip,power.present);
 visibility(battery_fill,power.valid&&power.percent>0);
 visibility(battery_charge,power.present&&power.charging);
 if(power.valid&&power.percent>0){
  lv_obj_set_width(battery_fill,(18*power.percent+99)/100);
  const uint32_t color=power.charging?0x33855e:power.percent<=15?0xf0ba65:0xc4ccdf;
  lv_obj_set_style_bg_color(battery_fill,lv_color_hex(color),0);
 }
}

void pet_view_create(lv_obj_t *parent){
 lv_display_t *display=lv_display_get_default();
 if(display!=stats_display){
  lv_display_add_event_cb(display,render_metrics,LV_EVENT_RENDER_START,NULL);
  lv_display_add_event_cb(display,render_metrics,LV_EVENT_RENDER_READY,NULL);
  lv_display_add_event_cb(display,flush_metrics,LV_EVENT_FLUSH_START,NULL);
  lv_display_add_event_cb(display,flush_metrics,LV_EVENT_FLUSH_FINISH,NULL);
  lv_display_add_event_cb(display,flush_metrics,LV_EVENT_FLUSH_WAIT_START,NULL);
  lv_display_add_event_cb(display,flush_metrics,LV_EVENT_FLUSH_WAIT_FINISH,NULL);
  stats_display=display;
  render_window_us=esp_timer_get_time();
 }

 bg=image_at(parent,&pet_idle_stage,0,0);
 avatar=image_at(parent,NULL,PET_IDLE_X,PET_IDLE_Y);
 lv_obj_add_flag(avatar,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(avatar,on_pet_tap,LV_EVENT_CLICKED,NULL);
 pet_idle_view_init(avatar);
 pet_idle_view_set_active(true);
 bridge_dot=feedback_dot(parent,228,24,10,0x717887);
 visibility(bridge_dot,true);
 create_battery(parent);
 state=label(parent,"OFFLINE",148,48,170,&font_buddy_cn_16,0x8a90a3);
 led=feedback_dot(parent,177,55,7,0x8a90a3);
 visibility(led,true);
 clock_text=label(parent,"--:--",50,386,366,&font_buddy_clock_64,0xffffff);
 date_text=label(parent,"--/--",158,442,150,&font_buddy_cn_16,0xaaa0bc);
 question=label(parent,"?",370,148,24,&font_buddy_cn_20,0xcab6ff);
 visibility(question,false);
 idle_tap=feedback_dot(parent,357,137,20,0xffda8b);
 lv_obj_set_style_outline_width(idle_tap,2,0);
 lv_obj_set_style_outline_pad(idle_tap,4,0);
 lv_obj_set_style_outline_color(idle_tap,lv_color_hex(0xffda8b),0);
 tilt_dot=feedback_dot(parent,365,237,12,0x79ddbd);
 animation=(pet_animation_t){0};
 working=painted=false;
 active_clip=queued_touch=tilt_latch=PET_CLIP_NONE;
 next_special=(uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS)+20000;
 shy_next=false;
 ESP_LOGI("pet_scene","scene=IDLE");
}

void pet_view_show(const pet_presentation_t *p,uint32_t now){
 pet_animation_frame_t frame={0};
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
 pet_animation_set_activity(&animation,p->activity,now);
 frame=pet_animation_step(&animation,now,p->thinking,p->coding);
#endif
 // Transitional events retain the last scene. The status label still reflects the event.
 const bool next_working=(p->thinking||p->coding) ||
   (working && !(p->activity==PET_ACTIVITY_IDLE && strcmp(p->status,"IDLE")==0));
 if(next_working!=working){
  working=next_working;
  lv_image_set_src(bg,working?&pet_work_stage:&pet_idle_stage);
  pet_motion_clip_stop();
  active_clip=queued_touch=tilt_latch=PET_CLIP_NONE;
  pet_idle_view_set_active(!working);
  if(working){
   lv_image_set_src(avatar,&pet_v41_character);
   lv_obj_set_pos(avatar,pet_v41_x,pet_v41_y);
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
   start_clip(PET_CLIP_WORKING);
#endif
  }else{
   lv_obj_set_pos(avatar,PET_IDLE_X,PET_IDLE_Y);
   next_special=now+20000;
  }
  ESP_LOGI("pet_scene","scene=%s",working?"WORKING":"IDLE");
 }
 if(active_clip!=PET_CLIP_NONE&&!pet_motion_clip_active()){
  active_clip=PET_CLIP_NONE;
  if(working){
   lv_image_set_src(avatar,&pet_v41_character);
   lv_obj_set_pos(avatar,pet_v41_x,pet_v41_y);
  }else{
   const pet_clip_id_t touch=queued_touch;
   queued_touch=PET_CLIP_NONE;
   if(touch==PET_CLIP_NONE||(uint32_t)(now-queued_at)>3000){
    pet_idle_view_set_active(true);
   }else{
    start_clip(touch);
   }
  }
 }

 visibility(question,p->activity==PET_ACTIVITY_QUESTION);
 visibility(idle_tap,frame.sparkle&&animation.reacting);
 pet_motion_frame_t motion;
 pet_motion_snapshot(&motion);
 const int tilt_x=motion.valid&&motion.calibrated?(int)lroundf(motion.gaze_x*9):0;
 const int tilt_y=motion.valid&&motion.calibrated?(int)lroundf(motion.gaze_y*9):0;
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
 if(!working){
  const int ax=abs(tilt_x), ay=abs(tilt_y);
  pet_clip_id_t direction=PET_CLIP_NONE;
  if(ax>=13||ay>=10){
   direction=ax>=ay?(tilt_x<0?PET_CLIP_LEFT:PET_CLIP_RIGHT):
                     (tilt_y<0?PET_CLIP_FORWARD:PET_CLIP_BACKWARD);
  }
  if(ax<6&&ay<5)tilt_latch=PET_CLIP_NONE;
  if(active_clip==PET_CLIP_NONE){
   if(motion.shake){
    start_clip(PET_CLIP_SHAKE);
   }else if(direction!=PET_CLIP_NONE&&direction!=tilt_latch){
    tilt_latch=direction;
    start_clip(direction);
   }else if(ax<5&&ay<5&&(int32_t)(now-next_special)>=0){
    start_clip(shy_next?PET_CLIP_SHY:PET_CLIP_CURIOUS);
    shy_next=!shy_next;
    next_special=now+22000;
   }
  }
 }
#endif
 const bool tilt_visible=tilt_x!=0||tilt_y!=0;
 visibility(tilt_dot,tilt_visible);
 if(tilt_visible)lv_obj_set_pos(tilt_dot,(working?70:365)+tilt_x,237+tilt_y);
 if(!painted||strcmp(previous.bridge,p->bridge)||previous.bridge_live!=p->bridge_live){
  lv_obj_set_style_bg_color(bridge_dot,lv_color_hex(p->bridge_color),0);
 }
 if(!painted||strcmp(previous.status,p->status)||previous.accent!=p->accent){
  ESP_LOGI("pet_state","status=%s activity=%d thinking=%d coding=%d",p->status,p->activity,p->thinking,p->coding);
  lv_label_set_text(state,p->status);lv_obj_set_style_text_color(state,lv_color_hex(p->accent),0);lv_obj_set_style_bg_color(led,lv_color_hex(p->accent),0);
 }
 if(!painted||strcmp(previous.clock,p->clock))lv_label_set_text(clock_text,p->clock);
 if(!painted||strcmp(previous.date,p->date))lv_label_set_text(date_text,p->date);
 if(!painted||previous.power.present!=p->power.present||previous.power.valid!=p->power.valid||
    previous.power.charging!=p->power.charging||previous.power.percent!=p->power.percent)
  show_battery(p->power);
 previous=*p;painted=true;
}
