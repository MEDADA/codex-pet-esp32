#include <assert.h>
#include <stdio.h>
#include "buddy_scene.h"

int main(void) {
    buddy_scene_input_t in = {.connected=true, .authenticated=true, .fresh=true};
    assert(buddy_scene_resolve(&in) == BUDDY_SCENE_SLEEP);
    in.status=2; assert(buddy_scene_resolve(&in)==BUDDY_SCENE_WORK);
    in.connected=false; assert(buddy_scene_resolve(&in)==BUDDY_SCENE_OFFLINE);
    in.connected=true;in.fresh=false; assert(buddy_scene_resolve(&in)==BUDDY_SCENE_OFFLINE);
    in.fresh=true;in.status=3;in.charging=true;
    assert(buddy_scene_resolve(&in)==BUDDY_SCENE_ALERT);
    in.voice=BUDDY_VOICE_LISTENING;
    assert(buddy_scene_resolve(&in)==BUDDY_SCENE_LISTENING);
    in.voice=BUDDY_VOICE_IDLE;
    assert(buddy_scene_resolve(&in)==BUDDY_SCENE_ALERT);
    in.status=4; assert(buddy_scene_resolve(&in)==BUDDY_SCENE_QUESTION);
    in.status=0;in.last_event=1;in.event_age_ms=100;
    assert(buddy_scene_resolve(&in)==BUDDY_SCENE_SUCCESS);
    in.event_age_ms=6000;assert(buddy_scene_resolve(&in)==BUDDY_SCENE_CHARGING);
    in.last_event=2;in.event_age_ms=0;assert(buddy_scene_resolve(&in)==BUDDY_SCENE_ERROR);
    in.connected=false;assert(buddy_scene_resolve(&in)==BUDDY_SCENE_OFFLINE);
    in.pairing=true;assert(buddy_scene_resolve(&in)==BUDDY_SCENE_PAIRING);
    assert(buddy_scene_palette(BUDDY_SCENE_WORK).background==1);
    assert(buddy_scene_palette(BUDDY_SCENE_ALERT).background==2);
    assert(buddy_scene_palette(BUDDY_SCENE_QUESTION).background==3);
    assert(buddy_scene_palette(BUDDY_SCENE_ERROR).background==4);
    assert(buddy_scene_palette(BUDDY_SCENE_OFFLINE).background==5);
    puts("PASS scene priorities, expiration, offline truth, palettes");
    return 0;
}
