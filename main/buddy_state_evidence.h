#pragma once
#include "buddy_protocol.h"
/* Caller holds BLE lock. These helpers do not change the wire protocol. */
bool buddy_apply_time_hint(buddy_agent_state_t *st,const uint8_t *data,uint16_t len,uint32_t now);
void buddy_status_invalidate(buddy_agent_state_t *st);
void buddy_status_received(buddy_agent_state_t *st,uint32_t now);
bool buddy_status_fresh(const buddy_agent_state_t *st,uint32_t now);
