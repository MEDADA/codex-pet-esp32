#pragma once
#include "buddy_protocol.h"
#include "pet_animation.h"
#include "buddy_power.h"
typedef struct {
 const char *status;
 const char *bridge;
 bool bridge_live;
 pet_activity_t activity;
 uint32_t accent, bridge_color;
 char clock[8],date[8]; /* "HH:MM" + NUL; headroom so %02u cannot truncate */
 bool thinking,coding;
 buddy_power_snapshot_t power;
} pet_presentation_t;
pet_presentation_t pet_present(const buddy_agent_state_t *a,bool connected,bool authenticated,bool pairing,uint32_t last_ms,uint32_t now_ms);
