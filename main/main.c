#include <stdio.h>

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"

#include "buddy_ble.h"
#include "buddy_ui.h"
#include "buddy_button.h"
#include "buddy_settings.h"
#include "buddy_motion.h"
#include "buddy_voice.h"

static const char *TAG = "buddy_main";

static void ui_task(void *arg)
{
    (void)arg;
    while (1) {
        buddy_ui_tick();
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "CodeIsland Buddy for Waveshare ESP32-S3-Touch-AMOLED-1.75C");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    buddy_button_init();

    ESP_LOGI(TAG, "starting display...");
#ifdef CONFIG_BUDDY_PET_V11
    // Run rendering opposite the lower-priority JPEG worker.
    bsp_display_cfg_t display = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_0,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE,
        .touch_flags = {.swap_xy = 0, .mirror_x = 1, .mirror_y = 1},
    };
    display.lv_adapter_cfg.task_core_id = 1;
    bsp_display_start_with_config(&display);
#else
    bsp_display_start();
#endif

    buddy_settings_init();
    buddy_motion_init();
#ifdef CONFIG_BUDDY_VOICE_BRIDGE
    buddy_voice_init();
#endif
    buddy_ui_init();
    buddy_ble_init();

    /* NimBLE host is pinned to core 0; keep LVGL/UI on core 1 so GATT
     * floods cannot starve touch/button handling. */
    xTaskCreatePinnedToCore(ui_task, "buddy_ui", 16384, NULL, 7, NULL, 1);
    ESP_LOGI(TAG, "ready — open CodeIsland Settings > Buddy");
}
