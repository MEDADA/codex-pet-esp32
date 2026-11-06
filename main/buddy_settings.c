#include "buddy_settings.h"
#include "buddy_display_guard.h"

#include <stdatomic.h>

#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "esp_codec_dev.h"

static const char *TAG = "buddy_set";
static const char *NVS_NS = "buddy";

static atomic_uint s_bright = 70;
static uint8_t s_vol = 60;
static esp_codec_dev_handle_t s_spk;
static bool s_vol_hw;
static bool s_wake=true;
static atomic_uint s_bright_dirty;

static void load(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
    uint8_t b = s_bright, v = s_vol;
    if (nvs_get_u8(h, "bright", &b) == ESP_OK) s_bright = b;
    if (nvs_get_u8(h, "vol", &v) == ESP_OK) s_vol = v;
    uint8_t wake=1;if(nvs_get_u8(h,"wake",&wake)==ESP_OK)s_wake=wake!=0;
    nvs_close(h);
}

static esp_err_t save_u8(const char *key, uint8_t v)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_u8(h, key, v);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

static void apply_vol(void)
{
    if (!s_spk) {
        ESP_LOGI(TAG, "volume=%u%% (UI+NVS only; speaker codec not ready)", s_vol);
        return;
    }
    int ret = esp_codec_dev_set_out_vol(s_spk, (int)s_vol);
    if (ret != 0) {
        ESP_LOGW(TAG, "esp_codec_dev_set_out_vol(%u) failed %d", s_vol, ret);
    } else {
        ESP_LOGI(TAG, "volume=%u%% applied to ES8311", s_vol);
    }
}

void buddy_settings_init(void)
{
    load();
    if (s_bright < 10) s_bright = 10;
    if (s_bright > 100) s_bright = 100;
    if (s_vol > 100) s_vol = 100;

#ifdef CONFIG_BUDDY_VOICE_BRIDGE
    s_spk = bsp_audio_codec_speaker_init();
    if (s_spk) {
        s_vol_hw = true;
        apply_vol();
        ESP_LOGI(TAG, "speaker codec ready (ES8311)");
    } else {
        s_vol_hw = false;
        ESP_LOGW(TAG, "speaker codec init failed — volume is NVS/UI placeholder");
    }
#else
    /* No audio consumer in this build: skip I2S + ES8311 bring-up entirely. */
    s_spk = NULL;
    s_vol_hw = false;
    ESP_LOGI(TAG, "audio disabled (CONFIG_BUDDY_VOICE_BRIDGE=n); bright=%u", s_bright);
#endif
    ESP_LOGI(TAG, "prefs bright=%u vol=%u", s_bright, s_vol);
}

uint8_t buddy_settings_get_brightness(void) { return s_bright; }

void buddy_settings_set_brightness(uint8_t percent)
{
    if (percent < 10) percent = 10;
    if (percent > 100) percent = 100;
    bool changed = atomic_exchange(&s_bright, percent) != percent;
    /* Serialize panel commands with LVGL and keep NVS off the BLE host. */
    if (changed) s_bright_dirty = true;
}

void buddy_settings_persist(void)
{
    static TickType_t failed_at;
    static bool retry_pending;
    TickType_t now = xTaskGetTickCount();
    if (retry_pending && now - failed_at < pdMS_TO_TICKS(1000)) return;
    if (!atomic_exchange(&s_bright_dirty, false)) return;
    if (!buddy_display_try_lock(50)) {
        s_bright_dirty = true;
        return;
    }
    uint8_t percent = s_bright;
    bsp_display_brightness_set(percent);
    bsp_display_unlock();
    esp_err_t err = save_u8("bright", percent);
    retry_pending = err != ESP_OK;
    if (retry_pending) {
        failed_at = now;
        s_bright_dirty = true;
        ESP_LOGW(TAG, "brightness save failed: %s", esp_err_to_name(err));
    }
}

uint8_t buddy_settings_get_volume(void) { return s_vol; }

void buddy_settings_set_volume(uint8_t percent)
{
    if (percent > 100) percent = 100;
    s_vol = percent;
    apply_vol();
    save_u8("vol", s_vol);
}

bool buddy_settings_volume_hw_ok(void) { return s_vol_hw; }
bool buddy_settings_get_wake(void){return s_wake;}
void buddy_settings_set_wake(bool enabled){s_wake=enabled;save_u8("wake",enabled);}
void *buddy_settings_speaker(void){return s_spk;}
