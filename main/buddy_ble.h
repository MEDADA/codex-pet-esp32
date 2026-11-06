#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "buddy_protocol.h"
#include "buddy_requests.h"

typedef void (*buddy_ble_notify_fn)(uint8_t marker);

void buddy_ble_init(void);
bool buddy_ble_is_connected(void);
bool buddy_ble_is_authenticated(void);
bool buddy_ble_is_paired(void);
buddy_mode_t buddy_ble_get_mode(void);
void buddy_ble_set_mode(buddy_mode_t mode);
void buddy_ble_snapshot(buddy_agent_state_t *out,buddy_mode_t *mode,bool *connected,bool *authenticated,uint32_t *last_ms,buddy_request_t *request);
uint32_t buddy_ble_last_data_ms(void);
const char *buddy_ble_device_name(void);
const uint8_t *buddy_ble_pending_host_id(void);
uint32_t buddy_ble_pair_request_ms(void);

void buddy_ble_accept_pair(void);
void buddy_ble_reject_pair(void);
void buddy_ble_clear_pair(void);
void buddy_ble_notify_byte(uint8_t value);
void buddy_ble_request_snapshot(buddy_request_t *out);
bool buddy_ble_request_action(uint8_t action,uint32_t expected,uint32_t expected_digest);
void buddy_ble_disconnect_client(const char *reason);
