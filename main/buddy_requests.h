#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef struct {
    uint32_t session,request,received_ms;
    uint32_t digest,result_request,sent_request,send_ms;
    uint16_t ttl_s,operation;
    uint8_t source,kind,result;
    uint8_t preview_length;
    bool valid,pending,complete,verified;
} buddy_request_t;
bool buddy_request_receive(buddy_request_t *r,const uint8_t *data,size_t len,uint32_t now);
size_t buddy_request_encode(buddy_request_t *r,uint8_t action,uint32_t expected,uint32_t now,uint8_t out[14]);
bool buddy_request_ack(buddy_request_t *r,const uint8_t *data,size_t len);
void buddy_request_clear(buddy_request_t *r);
uint32_t buddy_request_fingerprint(uint8_t source,uint8_t status,const char *tool,const char *workspace,const char *model,const char *preview);
bool buddy_request_content_matches(const buddy_request_t *r,uint8_t source,uint8_t status,const char *tool,const char *workspace,const char *model,const char *preview);
void buddy_request_check_timeout(buddy_request_t *r,uint32_t now);
