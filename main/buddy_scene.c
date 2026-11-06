#include "buddy_scene.h"

buddy_scene_t buddy_scene_resolve(const buddy_scene_input_t *in) {
    if (!in) return BUDDY_SCENE_OFFLINE;
    if (in->pairing) return BUDDY_SCENE_PAIRING;
    /* Local voice remains usable with Wi-Fi while BLE is disconnected. */
    if (in->voice==BUDDY_VOICE_LISTENING || in->voice==BUDDY_VOICE_TRANSCRIBING || in->voice==BUDDY_VOICE_THINKING)
        return BUDDY_SCENE_LISTENING;
    if (in->voice==BUDDY_VOICE_SPEAKING) return BUDDY_SCENE_SPEAKING;
    if (!in->connected || !in->authenticated || !in->fresh) return BUDDY_SCENE_OFFLINE;
    if (in->status==3) return BUDDY_SCENE_ALERT;
    if (in->status==4) return BUDDY_SCENE_QUESTION;
    if (in->last_event && in->event_age_ms<6000) {
        if (in->last_event==2) return BUDDY_SCENE_ERROR;
        if (in->last_event==1) return BUDDY_SCENE_SUCCESS;
    }
    if (in->status==1 || in->status==2) return BUDDY_SCENE_WORK;
    if (in->charging) return BUDDY_SCENE_CHARGING;
    return BUDDY_SCENE_SLEEP;
}
buddy_palette_t buddy_scene_palette(buddy_scene_t scene) {
    switch(scene) {
    case BUDDY_SCENE_WORK: return (buddy_palette_t){1,0x0b2946,0x168dcb,"工作中"};
    case BUDDY_SCENE_ALERT: return (buddy_palette_t){2,0x59380a,0xb96804,"待批准"};
    case BUDDY_SCENE_QUESTION: return (buddy_palette_t){3,0x422665,0x8048be,"待回答"};
    case BUDDY_SCENE_SUCCESS: return (buddy_palette_t){0,0x164b36,0x238663,"已完成"};
    case BUDDY_SCENE_ERROR: return (buddy_palette_t){4,0x63211c,0xad3733,"出错了"};
    case BUDDY_SCENE_OFFLINE: return (buddy_palette_t){5,0x344350,0x657482,"离线"};
    case BUDDY_SCENE_CHARGING: return (buddy_palette_t){0,0x164b36,0x238663,"充电中"};
    case BUDDY_SCENE_LISTENING: return (buddy_palette_t){0,0x164b36,0x238663,"正在聆听"};
    case BUDDY_SCENE_SPEAKING: return (buddy_palette_t){1,0x0b2946,0x168dcb,"正在播报"};
    case BUDDY_SCENE_PAIRING: return (buddy_palette_t){2,0x59380a,0xb96804,"请求配对"};
    default: return (buddy_palette_t){0,0x164b36,0x238663,"空闲"};
    }
}
