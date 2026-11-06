#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "buddy_io_contracts.h"
int main(void){
    char ssid[33],pass[65];memset(ssid,'s',32);ssid[32]=0;memset(pass,'a',64);pass[64]=0;
    uint8_t a[32],b[64];assert(buddy_wifi_copy(a,b,ssid,pass));assert(a[31]=='s'&&b[63]=='a');
    assert(!buddy_wifi_copy(a,b,ssid,"short"));pass[63]='z';assert(!buddy_wifi_copy(a,b,ssid,pass));
    assert(buddy_pcm_valid("audio/pcm;rate=16000;channels=1;format=s16le"));
    assert(buddy_pcm_valid("Audio/PCM; channels=1; format=s16le; rate=16000"));
    assert(!buddy_pcm_valid("audio/pcm;rate=16000;channels=2;format=s16le"));
    assert(!buddy_pcm_valid("audio/pcm;rate=16000"));
    puts("PASS full-size Wi-Fi credentials and mono PCM contract");
}
