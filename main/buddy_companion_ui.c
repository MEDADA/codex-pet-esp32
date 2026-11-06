#include "buddy_ui.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "buddy_ble.h"
#include "buddy_button.h"
#include "buddy_settings.h"
#include "buddy_power.h"
#include "buddy_motion.h"
#include "buddy_voice.h"
#include "fonts/buddy_fonts.h"
#include "generated/rem_assets.h"
#include "rem_bg_cache.h"
#include "esp_log.h"

/* Settings overlay removed: swipe-down / long-press / PWR no longer open a heavy panel.
   Keep idle home + BLE/status + task detail (approve/deny/skip). Brightness uses NVS default. */
enum {HOME,DETAIL};
static lv_obj_t *screen,*background,*top,*date,*clock_label,*pet,*gaze,*pill,*status,*task,*project,*panel,*body,*result,*approve,*deny,*skip;
static lv_obj_t *details_scroll;
static buddy_agent_state_t agent;
static buddy_mode_t mode;
static buddy_power_snapshot_t power;
static buddy_motion_output_t motion;
static buddy_voice_snapshot_t voice;
static buddy_request_t request;
static bool connected,authenticated,awake=true;
static uint32_t last_data,last_power,last_motion,last_activity,opened_request,opened_digest;
static uint8_t view;
static int animation=-1,last_bg=-1;
/* Single-slot bg crossfade without a second RGB565: fade image opa out → decode/swap → fade in.
   Total ~2500ms. Status/text/pet keep updating; decode happens while image is transparent. */
enum { BG_FADE_IDLE=0, BG_FADE_OUT=1, BG_FADE_IN=2, BG_FADE_HALF_MS=500 };
static int bg_fade_phase=BG_FADE_IDLE;
static int bg_target=-1;
static uint32_t bg_fade_t0=0;
static uint32_t now_ms(void){return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS);}
static void wake(void){last_activity=now_ms();if(!awake){bsp_display_brightness_set(buddy_settings_get_brightness());awake=true;}}
static void hidden(lv_obj_t *o,bool value){if(value)lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);}
static lv_obj_t *label(lv_obj_t *p,const lv_font_t *font,int x,int y,int w){
    lv_obj_t *o=lv_label_create(p);lv_obj_set_style_text_font(o,buddy_font_resolve(font),0);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
    lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_color(o,lv_color_hex(0x132e49),0);lv_label_set_text(o,"");return o;
}
/* Lightweight pet loop: lv_image src swap only (no animimg, no rem_scene compose).
   Subsample frames + ~3fps so status/BLE UI keeps priority over alpha redraw. */
static const lv_image_dsc_t *const *anim_frames;
static uint8_t anim_count;
static uint8_t anim_idx;
static uint32_t anim_last_ms;
static uint8_t anim_pick(const lv_image_dsc_t *const *src, uint8_t n, const lv_image_dsc_t *out[4])
{
    if(!src||n==0){return 0;}
    if(n<=3){for(uint8_t i=0;i<n;i++)out[i]=src[i];return n;}
    /* Keep 3–4 keyframes: first / mid / last (and +1 for longer clips). */
    out[0]=src[0];
    if(n>=8){out[1]=src[n/3];out[2]=src[(2*n)/3];out[3]=src[n-1];return 4;}
    out[1]=src[n/2];out[2]=src[n-1];return 3;
}
static void animate(int id){
    if(animation==id){return;}
    animation=id;anim_idx=0;anim_last_ms=0;
    const lv_image_dsc_t *const *frames=rem_sleep;uint8_t n=6;
    switch(id){
    case BUDDY_SCENE_WORK:frames=rem_work;n=6;break;
    case BUDDY_SCENE_ALERT:case BUDDY_SCENE_PAIRING:case BUDDY_SCENE_SPEAKING:frames=rem_alert;n=4;break;
    case BUDDY_SCENE_QUESTION:case BUDDY_SCENE_LISTENING:frames=rem_question;n=6;break;
    case BUDDY_SCENE_SUCCESS:frames=rem_success;n=5;break;
    case BUDDY_SCENE_ERROR:frames=rem_error;n=8;break;
    case BUDDY_SCENE_OFFLINE:frames=rem_offline;n=6;break;
    default:frames=rem_sleep;n=6;break;
    }
    static const lv_image_dsc_t *picked[4];
    anim_count=anim_pick(frames,n,picked);
    anim_frames=(const lv_image_dsc_t *const *)picked;
    if(anim_count&&anim_frames[0]) lv_image_set_src(pet,anim_frames[0]);
}
static void anim_step(uint32_t now){
    if(!anim_frames||anim_count<=1){return;}
    if(anim_last_ms&&(uint32_t)(now-anim_last_ms)<320u){return;}
    anim_last_ms=now;
    anim_idx=(uint8_t)((anim_idx+1u)%anim_count);
    if(anim_frames[anim_idx]) lv_image_set_src(pet,anim_frames[anim_idx]);
}
static void close_panel(lv_event_t *e){(void)e;wake();view=HOME;hidden(panel,true);}
static void action(lv_event_t *e){
    wake();uint8_t code=(uintptr_t)lv_event_get_user_data(e);
    lv_label_set_text(result,buddy_ble_request_action(code,opened_request,opened_digest)?"等待 Mac 确认…":"请求已失效或暂不可用");
}
static lv_obj_t *button(lv_obj_t *p,const char *text,int x,int y,int w,lv_event_cb_t cb,void *data){
    lv_obj_t *o=lv_button_create(p);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,38);lv_obj_set_style_radius(o,19,0);lv_obj_set_style_bg_color(o,lv_color_hex(0xe1edf4),0);lv_obj_set_style_shadow_width(o,0,0);
    lv_obj_t *l=label(o,&font_buddy_cn_16,0,0,w-12);lv_label_set_text(l,text);lv_obj_center(l);if(cb)lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,data);return o;
}
static void show_details(lv_event_t *e){
    if(voice.state>=BUDDY_VOICE_LISTENING&&voice.state<=BUDDY_VOICE_SPEAKING){buddy_voice_cancel();return;}
    (void)e;wake();view=DETAIL;opened_request=request.request;opened_digest=request.digest;hidden(panel,false);hidden(details_scroll,false);hidden(body,false);
    char text[1200];snprintf(text,sizeof(text),"%s\n%s\n%s\n%s\n\n会话 %u/%u · 工具 %u\n时长 %u 分 · 子任务 %u\n\n%s\n%s",buddy_source_name(agent.source_id),agent.workspace,agent.model,agent.tool_name,agent.active_sessions,agent.total_sessions,agent.tool_calls,agent.duration_min,agent.subagents,agent.preview,agent.tool_history);
    if(!authenticated||!connected||(uint32_t)(now_ms()-last_data)>=BUDDY_BLE_TIMEOUT_MS)snprintf(text,sizeof(text),"内容未同步");
    lv_label_set_text(body,text);bool can=request.valid&&request.verified&&authenticated&&connected&&request.source==agent.source_id&&(uint32_t)(now_ms()-request.received_ms)<request.ttl_s*1000u;
    hidden(approve,!can||request.kind!=1);hidden(deny,!can||request.kind!=1);hidden(skip,!can||request.kind!=2);
    lv_label_set_text(result,(agent.status_id==3||agent.status_id==4)&&!can?"请回 Mac 查看完整请求":"");
}
static void gesture(lv_event_t *e){
    wake();if(view!=HOME)return;lv_indev_t *i=lv_indev_active();if(!i)return;
    /* Swipe up opens task detail only; settings swipe removed. */
    if(lv_indev_get_gesture_dir(i)==LV_DIR_TOP)show_details(e);
}
static lv_color_t wash_for_bg(uint8_t id){
    switch(id){
    case 1: return lv_color_hex(0xd7ecf8); /* work blue */
    case 2: return lv_color_hex(0xf7e7c8); /* alert amber */
    case 3: return lv_color_hex(0xe8ddf5); /* question purple */
    case 4: return lv_color_hex(0xf6d9d6); /* error red */
    case 5: return lv_color_hex(0xe2e8ee); /* offline gray */
    default: return lv_color_hex(0xdcefe4); /* idle/success green */
    }
}
static void bg_set_image_opa(lv_opa_t opa){
    if(!background)return;
    lv_obj_set_style_image_opa(background,opa,0);
}
static void bg_apply_index(uint8_t idx){
    lv_obj_set_style_bg_color(screen,wash_for_bg(idx),0);
    const lv_image_dsc_t *bg=rem_bg_cached(idx);
    if(bg){
        lv_image_set_src(background,bg);
        lv_obj_remove_flag(background,LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(background);
    }else{
        lv_obj_add_flag(background,LV_OBJ_FLAG_HIDDEN);
    }
    last_bg=(int)idx;
    ESP_LOGI("buddy_ui","photo bg swap idx=%u ok=%d", (unsigned)idx, bg!=NULL);
}
static void bg_fade_step(uint32_t now){
    if(bg_fade_phase==BG_FADE_IDLE){
        if(bg_target>=0 && bg_target!=last_bg){
            bg_fade_phase=BG_FADE_OUT;
            bg_fade_t0=now;
            ESP_LOGI("buddy_ui","photo bg fade-out ->%d", bg_target);
        }
        return;
    }
    uint32_t elapsed=(uint32_t)(now-bg_fade_t0);
    if(bg_fade_phase==BG_FADE_OUT){
        if(elapsed>=BG_FADE_HALF_MS){
            bg_set_image_opa(LV_OPA_TRANSP);
            /* Decode+swap while transparent so JPEG hitch is behind wash color. */
            bg_apply_index((uint8_t)bg_target);
            bg_fade_phase=BG_FADE_IN;
            bg_fade_t0=now;
            ESP_LOGI("buddy_ui","photo bg fade-in %d", last_bg);
        }else{
            uint32_t opa=255u-(elapsed*255u)/BG_FADE_HALF_MS;
            bg_set_image_opa((lv_opa_t)opa);
        }
        return;
    }
    if(bg_fade_phase==BG_FADE_IN){
        if(elapsed>=BG_FADE_HALF_MS){
            bg_set_image_opa(LV_OPA_COVER);
            bg_fade_phase=BG_FADE_IDLE;
            /* Retarget if scene changed mid-fade. */
            if(bg_target>=0 && bg_target!=last_bg){
                bg_fade_phase=BG_FADE_OUT;
                bg_fade_t0=now;
                ESP_LOGI("buddy_ui","photo bg fade-out ->%d (queued)", bg_target);
            }
        }else{
            uint32_t opa=(elapsed*255u)/BG_FADE_HALF_MS;
            bg_set_image_opa((lv_opa_t)opa);
        }
    }
}
void buddy_ui_init(void){
    /* Photo bg: single-slot JPEG→PSRAM RGB565 (~434KB). Rem: sparse keyframe loop on lv_image.
       rem_scene full-frame compose stays off. Settings panel stays removed. */
    buddy_dynamic_fonts_init();
    rem_bg_cache_init();
    bsp_display_lock(50);screen=lv_obj_create(NULL);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_color(screen,wash_for_bg(5),0);lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);
    background=lv_image_create(screen);lv_obj_set_pos(background,0,0);lv_obj_add_flag(background,LV_OBJ_FLAG_GESTURE_BUBBLE);
    {const lv_image_dsc_t *boot=rem_bg_cached(5);if(boot){lv_image_set_src(background,boot);lv_obj_remove_flag(background,LV_OBJ_FLAG_HIDDEN);last_bg=5;bg_target=5;bg_set_image_opa(LV_OPA_COVER);}else{lv_obj_add_flag(background,LV_OBJ_FLAG_HIDDEN);last_bg=-1;bg_target=-1;}}
    top=label(screen,&font_buddy_cn_16,100,43,266);date=label(screen,&font_buddy_cn_16,83,71,300);clock_label=label(screen,&font_buddy_clock_64,50,91,366);
    pet=lv_image_create(screen);lv_image_set_src(pet,rem_sleep[0]);lv_obj_set_pos(pet,157,171);lv_obj_add_flag(pet,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(pet,show_details,LV_EVENT_CLICKED,NULL);
    gaze=lv_image_create(screen);lv_obj_set_pos(gaze,157,171);hidden(gaze,true);lv_obj_add_flag(gaze,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(gaze,show_details,LV_EVENT_CLICKED,NULL);
    pill=lv_obj_create(screen);lv_obj_set_size(pill,212,37);lv_obj_set_pos(pill,127,373);lv_obj_set_style_radius(pill,20,0);lv_obj_set_style_bg_color(pill,lv_color_white(),0);lv_obj_set_style_bg_opa(pill,195,0);lv_obj_set_style_border_width(pill,0,0);lv_obj_remove_flag(pill,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_flag(pill,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(pill,show_details,LV_EVENT_CLICKED,NULL);
    status=label(pill,&font_buddy_cn_20,0,0,210);lv_obj_center(status);task=label(screen,&font_buddy_cn_16,73,414,320);lv_label_set_long_mode(task,LV_LABEL_LONG_DOT);project=label(screen,&font_buddy_cn_16,118,437,230);lv_label_set_long_mode(project,LV_LABEL_LONG_DOT);
    panel=lv_obj_create(screen);lv_obj_set_pos(panel,55,63);lv_obj_set_size(panel,356,342);lv_obj_set_style_radius(panel,28,0);lv_obj_set_style_bg_color(panel,lv_color_hex(0xf5f9fc),0);lv_obj_set_style_bg_opa(panel,245,0);lv_obj_set_style_border_width(panel,0,0);lv_obj_set_style_pad_all(panel,8,0);lv_obj_remove_flag(panel,LV_OBJ_FLAG_SCROLLABLE);
    button(panel,"返回",6,0,80,close_panel,NULL);details_scroll=lv_obj_create(panel);lv_obj_set_pos(details_scroll,8,48);lv_obj_set_size(details_scroll,320,200);lv_obj_set_style_bg_opa(details_scroll,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(details_scroll,0,0);lv_obj_set_style_pad_all(details_scroll,0,0);lv_obj_set_scroll_dir(details_scroll,LV_DIR_VER);body=label(details_scroll,&font_buddy_cn_16,0,0,302);lv_obj_set_height(body,LV_SIZE_CONTENT);lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);lv_obj_set_style_text_align(body,LV_TEXT_ALIGN_LEFT,0);
    deny=button(panel,"拒绝",12,255,148,action,(void*)2);approve=button(panel,"批准",178,255,148,action,(void*)1);skip=button(panel,"跳过",95,255,148,action,(void*)3);result=label(panel,&font_buddy_cn_16,5,300,324);
    hidden(panel,true);lv_obj_move_background(background);lv_obj_move_foreground(top);lv_obj_move_foreground(date);lv_obj_move_foreground(clock_label);lv_obj_move_foreground(pill);lv_obj_move_foreground(task);lv_obj_move_foreground(project);lv_obj_add_event_cb(screen,gesture,LV_EVENT_GESTURE,NULL);lv_screen_load(screen);bsp_display_brightness_set(buddy_settings_get_brightness());bsp_display_unlock();last_activity=now_ms();awake=true;
}
void buddy_ui_tick(void){
    buddy_settings_persist();
    static int last_logged_scene=-1; static uint8_t last_logged_status=0xff;
    static bool last_logged_conn=false,last_logged_auth=false; static uint32_t last_hb_ms=0;
    uint32_t now=now_ms();buddy_btn_t b=buddy_button_poll(now);
    buddy_agent_state_t next_agent;buddy_request_t next_request;buddy_mode_t next_mode;bool next_connected,next_authenticated;uint32_t next_last;
    buddy_voice_snapshot_t next_voice;buddy_power_snapshot_t next_power=power;buddy_motion_output_t next_motion=motion;bool motion_wake=false;
    buddy_ble_snapshot(&next_agent,&next_mode,&next_connected,&next_authenticated,&next_last,&next_request);buddy_voice_snapshot(&next_voice);
    if(now-last_power>=1000){next_power=buddy_power_read();last_power=now;}
    if(now-last_motion>=100){motion_wake=buddy_motion_poll(now,&next_motion)&&next_motion.wake&&buddy_settings_get_wake();last_motion=now;}
    bsp_display_lock(50);agent=next_agent;request=next_request;voice=next_voice;power=next_power;motion=next_motion;mode=next_mode;connected=next_connected;authenticated=next_authenticated;last_data=next_last;
    if(motion_wake)wake();
    if(mode==BUDDY_MODE_PAIR_CONFIRM){if(now-buddy_ble_pair_request_ms()>BUDDY_PAIR_CONFIRM_MS)buddy_ble_reject_pair();else if(b.src==BUDDY_BTN_SRC_BOOT&&b.ev==BUDDY_BTN_SHORT)buddy_ble_accept_pair();else if(b.src==BUDDY_BTN_SRC_BOOT&&(b.ev==BUDDY_BTN_LONG||b.ev==BUDDY_BTN_SUPER_LONG))buddy_ble_reject_pair();}
    else if(b.ev!=BUDDY_BTN_NONE){wake();if(b.src==BUDDY_BTN_SRC_BOOT&&b.ev==BUDDY_BTN_SUPER_LONG){buddy_ble_clear_pair();buddy_ble_disconnect_client("pair reset");}else {if(b.src==BUDDY_BTN_SRC_PWR&&b.ev==BUDDY_BTN_SHORT){if(view!=HOME)close_panel(NULL);}else if(b.ev==BUDDY_BTN_SHORT)show_details(NULL);}}
    buddy_scene_input_t in={.connected=connected,.authenticated=authenticated,.fresh=(uint32_t)(now-last_data)<BUDDY_BLE_TIMEOUT_MS,.pairing=mode==BUDDY_MODE_PAIR_CONFIRM,.charging=power.charging,.status=agent.status_id,.last_event=agent.last_event,.event_age_ms=(uint32_t)(now-agent.event_ms),.voice=voice.state};buddy_scene_t scene=buddy_scene_resolve(&in);buddy_palette_t colors=buddy_scene_palette(scene);
    if((int)scene!=last_logged_scene||agent.status_id!=last_logged_status||connected!=last_logged_conn||authenticated!=last_logged_auth){
        ESP_LOGI("buddy_ui","photo scene=%d status=%u conn=%d auth=%d fresh=%d label=%s",
                 (int)scene,(unsigned)agent.status_id,(int)connected,(int)authenticated,(int)in.fresh,colors.label?colors.label:"");
        last_logged_scene=(int)scene;last_logged_status=agent.status_id;last_logged_conn=connected;last_logged_auth=authenticated;
    }
    if(now-last_hb_ms>5000){ESP_LOGI("buddy_ui","photo hb scene=%d conn=%d auth=%d status=%u",(int)scene,(int)connected,(int)authenticated,(unsigned)agent.status_id);last_hb_ms=now;}
    if(scene==BUDDY_SCENE_ALERT||scene==BUDDY_SCENE_QUESTION||scene==BUDDY_SCENE_PAIRING){wake();}
    /* Idle auto-dim disabled: keep fixed default brightness. */
    if(bg_target!=(int)colors.background){
        bg_target=(int)colors.background;
        ESP_LOGI("buddy_ui","photo bg want idx=%d (shown=%d phase=%d)",bg_target,last_bg,bg_fade_phase);
    }
    bg_fade_step(now);
    animate(scene);
    bool looking=scene==BUDDY_SCENE_SLEEP&&buddy_settings_get_wake()&&(motion.offset_x||motion.offset_y);int sign=agent.orientation?-1:1;uint8_t direction=(motion.direction+(agent.orientation?8:0))%16;hidden(pet,looking);hidden(gaze,!looking);if(looking)lv_image_set_src(gaze,rem_look[direction]);lv_obj_set_pos(pet,157+sign*motion.offset_x,171+sign*motion.offset_y);lv_obj_set_pos(gaze,157+sign*motion.offset_x,171+sign*motion.offset_y);
    char text[128];if(power.valid)snprintf(text,sizeof(text),"%s   %s%u%%",authenticated?"已连接":"未连接",power.charging?"充电 ":"",power.percent);else snprintf(text,sizeof(text),"%s",authenticated?"已连接":"未连接");lv_label_set_text(top,text);lv_obj_set_style_text_color(top,lv_color_hex(colors.ink),0);lv_obj_set_style_text_opa(top,LV_OPA_COVER,0);
    uint8_t h,m,s;uint16_t y;uint8_t mo,d;if(agent.time_valid&&agent.time_has_minute){buddy_wall_time_now(&agent,now,&h,&m,&s);snprintf(text,sizeof(text),"%02u:%02u",h,m);}else snprintf(text,sizeof(text),"--:--");lv_label_set_text(clock_label,text);lv_obj_set_style_text_color(clock_label,lv_color_hex(colors.ink),0);lv_obj_set_style_text_opa(clock_label,LV_OPA_COVER,0);
    if(agent.date_valid){buddy_wall_date_now(&agent,now,&y,&mo,&d);snprintf(text,sizeof(text),"%u月%u日",mo,d);}else text[0]=0;lv_label_set_text(date,text);lv_obj_set_style_text_color(date,lv_color_hex(colors.ink),0);lv_obj_set_style_text_opa(date,LV_OPA_COVER,0);lv_label_set_text(status,colors.label);lv_obj_set_style_text_color(status,lv_color_hex(colors.ink),0);lv_obj_set_style_text_color(task,lv_color_hex(colors.ink),0);lv_obj_set_style_text_color(project,lv_color_hex(colors.ink),0);
    text[0]=0;bool active=scene==BUDDY_SCENE_WORK||scene==BUDDY_SCENE_ALERT||scene==BUDDY_SCENE_QUESTION||scene==BUDDY_SCENE_ERROR||scene==BUDDY_SCENE_SUCCESS;if(active&&agent.tool_name[0])snprintf(text,sizeof(text),"%s · %s",buddy_source_name(agent.source_id),agent.tool_name);if(scene==BUDDY_SCENE_OFFLINE)snprintf(text,sizeof(text),"%s",connected&&authenticated?"等待同步":"等待连接");if(scene==BUDDY_SCENE_PAIRING)snprintf(text,sizeof(text),"BOOT 短按接受 · 长按拒绝");if(voice.state==BUDDY_VOICE_ERROR)snprintf(text,sizeof(text),"%.90s",voice.error);if(scene==BUDDY_SCENE_LISTENING)snprintf(text,sizeof(text),"%s",voice.state==BUDDY_VOICE_LISTENING?"松开结束":"正在处理…");if(scene==BUDDY_SCENE_SPEAKING)snprintf(text,sizeof(text),"%.110s",voice.text);lv_label_set_text(task,text);lv_label_set_text(project,active?agent.workspace:"");
    /* Status/text first; pet frame after so CodeIsland UI wins if redraw is tight. */
    if(!looking)anim_step(now);
    if(view==DETAIL){
        bool changed=request.request!=opened_request||request.digest!=opened_digest||!request.verified;
        if(changed||request.result){hidden(approve,true);hidden(deny,true);hidden(skip,true);}
        if(request.result&&request.result_request==opened_request)lv_label_set_text(result,request.result==1?"Mac 已接收":request.result==3?"等待超时，请回 Mac 查看":"操作未完成");
        else if(changed)lv_label_set_text(result,"请求已更新或内容未同步");
    }
    bsp_display_unlock();
}
