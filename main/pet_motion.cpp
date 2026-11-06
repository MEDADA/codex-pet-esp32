#include "pet_motion_tracker.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"

static portMUX_TYPE guard=portMUX_INITIALIZER_UNLOCKED;
static pet_motion_frame_t published{};
static uint32_t published_ms;
static bool reset_requested,started;
static uint32_t now_ms(){return uint32_t(xTaskGetTickCount()*portTICK_PERIOD_MS);}
static void sample_task(void *){
    PetMotionTracker tracker;
    TickType_t next=xTaskGetTickCount();
    uint32_t last_log=0,last_mem=0;bool was_calibrated=false;
    for(;;){
        uint32_t now=now_ms();bool reset;
        portENTER_CRITICAL(&guard);reset=reset_requested;reset_requested=false;portEXIT_CRITICAL(&guard);
        if(reset){tracker.recalibrate();ESP_LOGI("pet_imu","neutral recalibration requested; hold still for 2s");}
        pet_imu_sample_t raw{};bool valid=buddy_imu6_read(&raw);
        auto frame=tracker.update(raw,valid,now);
        portENTER_CRITICAL(&guard);published=frame;published_ms=now;portEXIT_CRITICAL(&guard);
        if(frame.calibrated&&!was_calibrated)ESP_LOGI("pet_imu","calibrated roll=%.2f pitch=%.2f",frame.roll,frame.pitch);
        was_calibrated=frame.calibrated;
        if(uint32_t(now-last_log)>=2000){
            ESP_LOGI("pet_imu","valid=%d calibrated=%d a[g]=%.3f,%.3f,%.3f g[dps]=%.2f,%.2f,%.2f roll=%.2f pitch=%.2f glint=%.2f,%.2f shake=%d",
                     frame.valid,frame.calibrated,raw.ax,raw.ay,raw.az,raw.gx,raw.gy,raw.gz,frame.roll,frame.pitch,frame.gaze_x,frame.gaze_y,frame.shake);
            last_log=now;
        }
        if(uint32_t(now-last_mem)>=10000){
            ESP_LOGI("pet_mem","internal free=%u min=%u largest=%u psram free=%u largest=%u stack_hwm=%u",
                unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)),
                unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)),
                unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)),
                unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),
                unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)),unsigned(uxTaskGetStackHighWaterMark(nullptr)));
            last_mem=now;
        }
        vTaskDelayUntil(&next,pdMS_TO_TICKS(40));
    }
}
extern "C" void pet_motion_start(void){
#ifdef CONFIG_BUDDY_PET_V11_IMU
    if(started)return;
    started=xTaskCreate(sample_task,"pet_imu",4096,nullptr,2,nullptr)==pdPASS;
    if(!started)ESP_LOGW("pet_imu","worker unavailable; glints disabled");
#else
    (void)sample_task;
    (void)started;
#endif
}
extern "C" void pet_motion_snapshot(pet_motion_frame_t *out){
    portENTER_CRITICAL(&guard);*out=published;uint32_t stamp=published_ms;portEXIT_CRITICAL(&guard);
    if(uint32_t(now_ms()-stamp)>250)*out=pet_motion_frame_t{};
}
extern "C" void pet_motion_recalibrate(void){
    portENTER_CRITICAL(&guard);reset_requested=true;published=pet_motion_frame_t{};portEXIT_CRITICAL(&guard);
}
