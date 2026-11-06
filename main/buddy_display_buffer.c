#include "esp_lv_adapter_display.h"
#include "esp_log.h"

lv_display_t *__real_esp_lv_adapter_register_display(const esp_lv_adapter_display_config_t *config);

lv_display_t *__wrap_esp_lv_adapter_register_display(const esp_lv_adapter_display_config_t *config)
{
    // Keep partial refresh and allocate an internal tile at startup, before BLE.
    // A PSRAM tile can require a temporary internal SPI DMA copy for each flush.
    esp_lv_adapter_display_config_t local = *config;
    local.profile.buffer_height = 4;
    local.profile.use_psram = false;
    local.profile.require_double_buffer = false;
    lv_display_t *display = __real_esp_lv_adapter_register_display(&local);
    if (display) {
        ESP_LOGI("buddy_display", "Internal four-row draw buffer");
        return display;
    }
    return __real_esp_lv_adapter_register_display(config);
}
