#include "buddy_network.h"
#include "buddy_io_contracts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "buddy_net";
static buddy_network_config_t config;
static bool connected,initialized,setup,wifi_started;
static httpd_handle_t server;
static char setup_nonce[33];
static esp_netif_t *sta,*ap;

static void events(void *arg,esp_event_base_t base,int32_t id,void *data){
    (void)arg;(void)data;
    if(base==IP_EVENT&&id==IP_EVENT_STA_GOT_IP)connected=true;
    if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_DISCONNECTED){connected=false;if(!setup)esp_wifi_connect();}
    if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_START&&config.ssid[0])esp_wifi_connect();
}
static void load(nvs_handle_t h,const char *key,char *out,size_t cap){size_t n=cap;nvs_get_str(h,key,out,&n);out[cap-1]=0;}

/* Bring up Wi-Fi driver only when credentials or setup portal need it.
 * Keeps SPI LCD DMA headroom on rem companion boots without voice Wi-Fi. */
static bool ensure_wifi_driver(void){
    if(wifi_started)return true;
    if(esp_netif_init()!=ESP_OK)return false;
    esp_err_t ret=esp_event_loop_create_default();
    if(ret!=ESP_OK&&ret!=ESP_ERR_INVALID_STATE)return false;
    if(!sta){sta=esp_netif_create_default_wifi_sta();if(!sta)return false;}
    wifi_init_config_t c=WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&c)!=ESP_OK)return false;
    esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,events,NULL);
    esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,events,NULL);
    wifi_started=true;
    return true;
}

static bool start_sta_if_configured(void){
    if(!config.ssid[0])return false;
    if(!ensure_wifi_driver())return false;
    wifi_config_t w={0};
    if(!buddy_wifi_copy(w.sta.ssid,w.sta.password,config.ssid,config.password))return false;
    if(esp_wifi_set_mode(WIFI_MODE_STA)!=ESP_OK)return false;
    if(esp_wifi_set_config(WIFI_IF_STA,&w)!=ESP_OK)return false;
    if(esp_wifi_start()!=ESP_OK)return false;
    ESP_LOGI(TAG,"Wi-Fi STA start ssid=%s",config.ssid);
    return true;
}

void buddy_network_init(void){
    if(initialized)return;
    strlcpy(config.ssid,CONFIG_BUDDY_WIFI_SSID,sizeof(config.ssid));
    strlcpy(config.password,CONFIG_BUDDY_WIFI_PASSWORD,sizeof(config.password));
    strlcpy(config.url,CONFIG_BUDDY_VOICE_URL,sizeof(config.url));
    strlcpy(config.token,CONFIG_BUDDY_VOICE_TOKEN,sizeof(config.token));
    nvs_handle_t h;
    if(nvs_open("rem_net",NVS_READONLY,&h)==ESP_OK){
        load(h,"ssid",config.ssid,sizeof(config.ssid));
        load(h,"pass",config.password,sizeof(config.password));
        load(h,"url",config.url,sizeof(config.url));
        load(h,"token",config.token,sizeof(config.token));
        nvs_close(h);
    }
    size_t url_len=strlen(config.url);
    while(url_len&&config.url[url_len-1]=='/')config.url[--url_len]=0;
    initialized=true;
    if(config.ssid[0]){
        if(!start_sta_if_configured()){
            ESP_LOGW(TAG,"Wi-Fi STA deferred start failed");
        }
    }else{
        ESP_LOGI(TAG,"no Wi-Fi SSID — defer radio for LCD/BLE headroom");
    }
}
bool buddy_network_connected(void){return connected;}
const buddy_network_config_t *buddy_network_config(void){return &config;}
static const char *form="<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width'><title>Rem 网络设置</title><h2>Rem 网络设置</h2><form method=post action=/save><input type=hidden name=csrf value='%s'><p>Wi-Fi 名称 <input name=ssid maxlength=32 required></p><p>Wi-Fi 密码 <input name=pass type=password maxlength=64></p><p>Mac 语音桥地址 <input name=url placeholder='http://192.168.1.10:8765' maxlength=159></p><p>设备令牌 <input name=token type=password maxlength=128></p><button>保存并重启</button></form>";
static esp_err_t get_form(httpd_req_t *r){char page[1536];snprintf(page,sizeof(page),form,setup_nonce);httpd_resp_set_type(r,"text/html; charset=utf-8");httpd_resp_set_hdr(r,"Cache-Control","no-store");return httpd_resp_sendstr(r,page);}
static bool field(const char *data,const char *key,char *out,size_t cap){
    char raw[512];if(httpd_query_key_value(data,key,raw,sizeof(raw))!=ESP_OK){out[0]=0;return true;}
    size_t j=0;for(size_t i=0;raw[i];i++){
        int ch=raw[i];if(ch=='+')ch=' ';else if(ch=='%'){
            if(!raw[i+1]||!raw[i+2]||!isxdigit((unsigned char)raw[i+1])||!isxdigit((unsigned char)raw[i+2]))return false;
            char hex[]={raw[i+1],raw[i+2],0};ch=strtol(hex,NULL,16);i+=2;
        }
        if(ch==0||ch=='\r'||ch=='\n'||j+1>=cap){return false;}
        out[j++]=(char)ch;
    }out[j]=0;return true;
}
static void reboot_task(void *p){(void)p;vTaskDelay(pdMS_TO_TICKS(1500));esp_restart();}
static esp_err_t save_form(httpd_req_t *r){
    if(r->content_len<=0||r->content_len>1024)return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Invalid form");
    char data[1025];int total=0;while(total<r->content_len){int n=httpd_req_recv(r,data+total,r->content_len-total);if(n<=0)return ESP_FAIL;total+=n;}data[total]=0;
    buddy_network_config_t next={0};
    char nonce[33];if(!field(data,"csrf",nonce,sizeof(nonce))||strcmp(nonce,setup_nonce))return httpd_resp_send_err(r,HTTPD_403_FORBIDDEN,"Invalid setup session");
    if(!field(data,"ssid",next.ssid,sizeof(next.ssid))||!field(data,"pass",next.password,sizeof(next.password))||!field(data,"url",next.url,sizeof(next.url))||!field(data,"token",next.token,sizeof(next.token))||!next.ssid[0])return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Invalid configuration");
    if(next.url[0]&&strncmp(next.url,"http://",7)&&strncmp(next.url,"https://",8))return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Invalid bridge URL");
    if(next.url[0]&&strlen(next.token)<16)return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Device token needs at least 16 characters");
    uint8_t ssid_check[32],pass_check[64];if(!buddy_wifi_copy(ssid_check,pass_check,next.ssid,next.password))return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Invalid Wi-Fi credentials");
    nvs_handle_t h;if(nvs_open("rem_net",NVS_READWRITE,&h)!=ESP_OK)return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"Save failed");
    esp_err_t ret=nvs_set_str(h,"ssid",next.ssid);ret|=nvs_set_str(h,"pass",next.password);ret|=nvs_set_str(h,"url",next.url);ret|=nvs_set_str(h,"token",next.token);if(ret==ESP_OK)ret=nvs_commit(h);nvs_close(h);
    if(ret!=ESP_OK)return httpd_resp_send_err(r,HTTPD_500_INTERNAL_SERVER_ERROR,"Save failed");
    httpd_resp_set_type(r,"text/plain; charset=utf-8");httpd_resp_sendstr(r,"已保存，Rem 即将重启。关闭此页面并重新连接原 Wi-Fi。");xTaskCreate(reboot_task,"rem_restart",2048,NULL,1,NULL);return ESP_OK;
}
bool buddy_network_setup(char *hint,size_t size){
    if(!initialized||setup){return false;}
    if(!ensure_wifi_driver()){return false;}
    setup=true;connected=false;
    if(!ap){ap=esp_netif_create_default_wifi_ap();}
    if(!ap){setup=false;return false;}
    uint32_t id=esp_random();char ssid[33],pass[16];snprintf(ssid,sizeof(ssid),"Rem-Setup-%04X",(unsigned)(id&0xffff));snprintf(pass,sizeof(pass),"%08lX%04X",(unsigned long)esp_random(),(unsigned)(esp_random()&0xffff));snprintf(setup_nonce,sizeof(setup_nonce),"%08x%08x%08x%08x",(unsigned)esp_random(),(unsigned)esp_random(),(unsigned)esp_random(),(unsigned)esp_random());
    wifi_config_t w={0};strlcpy((char*)w.ap.ssid,ssid,sizeof(w.ap.ssid));strlcpy((char*)w.ap.password,pass,sizeof(w.ap.password));w.ap.authmode=WIFI_AUTH_WPA2_PSK;w.ap.max_connection=1;w.ap.channel=1;
    if(esp_wifi_set_mode(WIFI_MODE_APSTA)!=ESP_OK||esp_wifi_set_config(WIFI_IF_AP,&w)!=ESP_OK||esp_wifi_start()!=ESP_OK){setup=false;return false;}
    httpd_config_t c=HTTPD_DEFAULT_CONFIG();c.stack_size=6144;
    if(httpd_start(&server,&c)!=ESP_OK){setup=false;return false;}
    httpd_uri_t get={.uri="/",.method=HTTP_GET,.handler=get_form},post={.uri="/save",.method=HTTP_POST,.handler=save_form};
    httpd_register_uri_handler(server,&get);httpd_register_uri_handler(server,&post);
    snprintf(hint,size,"%s\n密码 %s\n192.168.4.1",ssid,pass);return true;
}
