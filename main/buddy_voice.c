#include "buddy_voice.h"
#include "buddy_network.h"
#include "buddy_settings.h"
#include "buddy_io_contracts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_codec_dev.h"
#include "bsp/esp-bsp.h"

#define MAX_CAPTURE_BYTES (16000*2*30)
static esp_codec_dev_handle_t mic;
static atomic_bool busy,cancelled,capturing;
static atomic_uint turn_id;
static bool response_pcm;
static portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
static buddy_voice_snapshot_t snapshot;
static void state(buddy_voice_state_t value,const char *error){portENTER_CRITICAL(&mux);snapshot.state=value;strlcpy(snapshot.error,error?error:"",sizeof(snapshot.error));portEXIT_CRITICAL(&mux);}
void buddy_voice_snapshot(buddy_voice_snapshot_t *out){portENTER_CRITICAL(&mux);*out=snapshot;portEXIT_CRITICAL(&mux);}
static int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
static esp_err_t http_event(esp_http_client_event_t *e){
    if(atomic_load(&cancelled))return ESP_FAIL;
    if(e->event_id==HTTP_EVENT_ON_HEADER&&!strcasecmp(e->header_key,"Content-Type"))response_pcm=buddy_pcm_valid(e->header_value);
    if(e->event_id==HTTP_EVENT_ON_HEADER&&!strcasecmp(e->header_key,"X-Rem-Text")){
        char decoded[256];size_t j=0;const char *p=e->header_value;
        for(size_t i=0;p[i]&&j+1<sizeof(decoded);i++){
            if(p[i]=='%'&&p[i+1]&&p[i+2]){int a=hex(p[i+1]),b=hex(p[i+2]);if(a<0||b<0)break;decoded[j++]=(char)(a*16+b);i+=2;}
            else decoded[j++]=p[i];
        }decoded[j]=0;portENTER_CRITICAL(&mux);strlcpy(snapshot.text,decoded,sizeof(snapshot.text));portEXIT_CRITICAL(&mux);
    }return ESP_OK;
}
static void worker(void *unused){
    (void)unused;uint8_t *audio=heap_caps_malloc(MAX_CAPTURE_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    esp_http_client_handle_t client=NULL;esp_codec_dev_handle_t speaker=buddy_settings_speaker();bool mic_open=false,speaker_open=false;const char *error=NULL;
    if(!audio){error="录音内存不足";goto done;}
    esp_codec_dev_sample_info_t input={.sample_rate=16000,.channel=2,.channel_mask=1,.bits_per_sample=16};
    if(!mic||esp_codec_dev_open(mic,&input)!=0){error="麦克风不可用";goto done;}mic_open=true;
    size_t used=0;state(BUDDY_VOICE_LISTENING,NULL);
    while(atomic_load(&capturing)&&!atomic_load(&cancelled)&&used+1024<=MAX_CAPTURE_BYTES){
        if(esp_codec_dev_read(mic,audio+used,1024)!=0){error="录音失败";goto done;}used+=1024;
    }
    esp_codec_dev_close(mic);mic_open=false;
    if(atomic_load(&cancelled))goto done;
    if(used<3200){error="录音太短，请重试";goto done;}
    const buddy_network_config_t *net=buddy_network_config();char url[192],auth[160];snprintf(url,sizeof(url),"%s/v1/turn/%08x",net->url,(unsigned)atomic_load(&turn_id));snprintf(auth,sizeof(auth),"Bearer %s",net->token);response_pcm=false;
    esp_http_client_config_t cfg={.url=url,.timeout_ms=60000,.buffer_size=2048,.event_handler=http_event,.crt_bundle_attach=esp_crt_bundle_attach};client=esp_http_client_init(&cfg);
    if(!client){error="语音连接失败";goto done;}
    state(BUDDY_VOICE_TRANSCRIBING,NULL);esp_http_client_set_method(client,HTTP_METHOD_POST);esp_http_client_set_header(client,"Authorization",auth);esp_http_client_set_header(client,"Content-Type","audio/pcm;rate=16000;channels=1;format=s16le");
    if(esp_http_client_open(client,used)!=ESP_OK){error="无法连接 Mac 语音服务";goto done;}
    for(size_t off=0;off<used&&!atomic_load(&cancelled);){int n=esp_http_client_write(client,(char*)audio+off,(used-off)>4096?4096:(used-off));if(n<=0){error="录音上传失败";goto done;}off+=n;}
    heap_caps_free(audio);audio=NULL;if(atomic_load(&cancelled))goto done;
    if(esp_http_client_fetch_headers(client)<0||esp_http_client_get_status_code(client)!=200||!response_pcm){error="语音服务未就绪或请求失败";goto done;}
    esp_codec_dev_sample_info_t output={.sample_rate=16000,.channel=1,.bits_per_sample=16};
    if(!speaker||esp_codec_dev_open(speaker,&output)!=0){error="扬声器不可用";goto done;}speaker_open=true;esp_codec_dev_set_out_vol(speaker,buddy_settings_get_volume());state(BUDDY_VOICE_SPEAKING,NULL);
    uint8_t chunk[1026];size_t carry=0,total=0;
    while(!atomic_load(&cancelled)){
        int n=esp_http_client_read(client,(char*)chunk+carry,1024);if(n<0){error="播报中断";goto done;}if(n==0){if(!esp_http_client_is_complete_data_received(client))error="播报数据不完整";break;}
        size_t size=carry+n,even=size&~(size_t)1;total+=even;if(total>16000*2*60){error="播报长度超限";goto done;}
        if(even&&esp_codec_dev_write(speaker,chunk,even)!=0){error="音频播放失败";goto done;}carry=size-even;if(carry)chunk[0]=chunk[even];
    }
    if(carry&&!atomic_load(&cancelled))error="音频格式错误";
done:
    if(client){esp_http_client_cleanup(client);}
    if(mic_open){esp_codec_dev_close(mic);}
    if(speaker_open){esp_codec_dev_close(speaker);}
    if(audio){heap_caps_free(audio);}
    atomic_store(&capturing,false);state(error&&!atomic_load(&cancelled)?BUDDY_VOICE_ERROR:BUDDY_VOICE_IDLE,error);atomic_store(&busy,false);vTaskDelete(NULL);
}
void buddy_voice_init(void){
    buddy_network_init();mic=bsp_audio_codec_microphone_init();const buddy_network_config_t *c=buddy_network_config();snapshot.configured=c->url[0]&&c->token[0];snapshot.state=snapshot.configured?BUDDY_VOICE_IDLE:BUDDY_VOICE_UNAVAILABLE;
}
esp_err_t buddy_voice_start(void){
    if(!snapshot.configured){state(BUDDY_VOICE_ERROR,"请先配置网络和语音服务");return ESP_ERR_INVALID_STATE;}
    if(!buddy_network_connected()){state(BUDDY_VOICE_ERROR,"Wi-Fi 尚未连接");return ESP_ERR_INVALID_STATE;}
    bool expected=false;if(!atomic_compare_exchange_strong(&busy,&expected,true))return ESP_ERR_INVALID_STATE;
    atomic_store(&turn_id,esp_random());atomic_store(&cancelled,false);atomic_store(&capturing,true);portENTER_CRITICAL(&mux);snapshot.text[0]=0;portEXIT_CRITICAL(&mux);
    if(xTaskCreatePinnedToCore(worker,"rem_voice",12288,NULL,4,NULL,0)!=pdPASS){atomic_store(&busy,false);atomic_store(&capturing,false);state(BUDDY_VOICE_ERROR,"无法启动语音");return ESP_ERR_NO_MEM;}return ESP_OK;
}
void buddy_voice_stop_capture(void){atomic_store(&capturing,false);}
static void cancel_remote(void *id){
    const buddy_network_config_t *net=buddy_network_config();char url[192],auth[160];snprintf(url,sizeof(url),"%s/v1/turn/%08x",net->url,(unsigned)(uintptr_t)id);snprintf(auth,sizeof(auth),"Bearer %s",net->token);
    esp_http_client_config_t c={.url=url,.timeout_ms=2000,.crt_bundle_attach=esp_crt_bundle_attach};esp_http_client_handle_t h=esp_http_client_init(&c);
    if(h){esp_http_client_set_method(h,HTTP_METHOD_DELETE);esp_http_client_set_header(h,"Authorization",auth);esp_http_client_perform(h);esp_http_client_cleanup(h);}vTaskDelete(NULL);
}
void buddy_voice_cancel(void){
    bool active=atomic_load(&busy);atomic_store(&cancelled,true);atomic_store(&capturing,false);state(BUDDY_VOICE_IDLE,NULL);
    if(active)xTaskCreate(cancel_remote,"rem_cancel",4096,(void*)(uintptr_t)atomic_load(&turn_id),3,NULL);
}
