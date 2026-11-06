#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
static inline bool buddy_wifi_copy(uint8_t ssid_out[32],uint8_t pass_out[64],const char *ssid,const char *password){
    size_t s=strlen(ssid),p=strlen(password);
    if(s<1||s>32||(p!=0&&(p<8||p>64)))return false;
    if(p==64)for(size_t i=0;i<p;i++)if(!isxdigit((unsigned char)password[i]))return false;
    memset(ssid_out,0,32);memset(pass_out,0,64);memcpy(ssid_out,ssid,s);memcpy(pass_out,password,p);return true;
}
static inline bool buddy_pcm_valid(const char *type){
    if(!type||strlen(type)>=128)return false;
    char data[128];size_t n=strlen(type);for(size_t i=0;i<=n;i++)data[i]=(char)tolower((unsigned char)type[i]);
    char *part=data;unsigned flags=0;int field=0;
    while(part){char *next=strchr(part,';');if(next)*next++=0;
        while(isspace((unsigned char)*part))part++;
        char *end=part+strlen(part);while(end>part&&isspace((unsigned char)end[-1]))*--end=0;
        if(field++==0){if(strcmp(part,"audio/pcm"))return false;}
        else{unsigned bit=!strcmp(part,"rate=16000")?1:!strcmp(part,"channels=1")?2:!strcmp(part,"format=s16le")?4:0;if(!bit||(flags&bit))return false;flags|=bit;}
        part=next;
    }return flags==7;
}
