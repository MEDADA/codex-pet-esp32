#include "buddy_requests.h"
#include <string.h>
static uint32_t read32(const uint8_t *p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static void put32(uint8_t *p,uint32_t v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
void buddy_request_clear(buddy_request_t *r){
    buddy_request_t old=*r;memset(r,0,sizeof(*r));
    r->operation=old.operation;r->sent_request=old.sent_request;r->send_ms=old.send_ms;
    r->result=old.result;r->result_request=old.result_request;
}
bool buddy_request_receive(buddy_request_t *r,const uint8_t *d,size_t n,uint32_t now){
    if(!d||n!=20||d[0]!=0xe9||d[1]!=1||d[2]>15)return false;
    uint32_t session=read32(d+3),request=read32(d+7);
    uint16_t ttl=(uint16_t)d[11]<<8|d[12];
    if(!ttl){buddy_request_clear(r);return true;}
    if(ttl>60||!session||!request||d[13]<1||d[13]>2||d[18]>128||d[19]>1)return false;
    uint32_t digest=read32(d+14);
    bool same=r->valid&&r->session==session&&r->request==request&&r->source==d[2]&&r->kind==d[13]&&r->digest==digest;
    if(!same)buddy_request_clear(r);
    r->session=session;r->request=request;r->source=d[2];r->kind=d[13];r->ttl_s=ttl;r->received_ms=now;r->valid=true;r->digest=digest;r->preview_length=d[18];r->complete=d[19]==1;r->verified=false;
    return true;
}
size_t buddy_request_encode(buddy_request_t *r,uint8_t action,uint32_t expected,uint32_t now,uint8_t out[14]){
    if(!r->valid||!r->complete||!r->verified||r->pending||(r->result==1&&r->result_request==r->request)||expected!=r->request||(uint32_t)(now-r->received_ms)>=(uint32_t)r->ttl_s*1000)return 0;
    if(action<1||action>3||(r->kind==1&&action==3)||(r->kind==2&&action!=3))return 0;
    r->operation++;if(!r->operation)r->operation++;
    out[0]=0xea;out[1]=1;out[2]=r->operation>>8;out[3]=r->operation;out[4]=action;out[5]=r->source;
    put32(out+6,r->session);put32(out+10,r->request);r->pending=true;r->result=0;r->result_request=0;r->sent_request=r->request;r->send_ms=now;return 14;
}
bool buddy_request_ack(buddy_request_t *r,const uint8_t *d,size_t n){
    if(!d||n!=5||d[0]!=0xeb||d[1]!=1||!r->sent_request||((uint16_t)d[2]<<8|d[3])!=r->operation||d[4]>3)return false;
    r->pending=false;r->result=d[4]==0?1:2;r->result_request=r->sent_request;return true;
}
static uint32_t crc_byte(uint32_t crc,uint8_t byte){crc^=byte;for(int n=0;n<8;n++)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);return crc;}
uint32_t buddy_request_fingerprint(uint8_t source,uint8_t status,const char *tool,const char *workspace,const char *model,const char *preview){
    uint32_t crc=crc_byte(crc_byte(0xffffffffu,source),status);
    const char *fields[]={tool,workspace,model,preview};
    for(int f=0;f<4;f++){size_t n=strlen(fields[f]);crc=crc_byte(crc,(uint8_t)n);for(size_t i=0;i<n;i++)crc=crc_byte(crc,(uint8_t)fields[f][i]);}
    return crc^0xffffffffu;
}
bool buddy_request_content_matches(const buddy_request_t *r,uint8_t source,uint8_t status,const char *tool,const char *workspace,const char *model,const char *preview){
    bool kind=(r->kind==1&&status==3)||(r->kind==2&&status==4);
    return r->valid&&r->complete&&kind&&source==r->source&&r->preview_length>0&&strlen(preview)==r->preview_length&&r->digest==buddy_request_fingerprint(source,status,tool,workspace,model,preview);
}
void buddy_request_check_timeout(buddy_request_t *r,uint32_t now){
    if(r->pending&&(uint32_t)(now-r->send_ms)>=15000){r->pending=false;r->result=3;r->result_request=r->sent_request;}
}
