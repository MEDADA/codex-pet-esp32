#include "rem_scene.h"
#include "esp_log.h"

static const char *TAG = "rem_scene";

/* Lightweight build: no full-screen PSRAM compose. */
bool rem_scene_init(void)
{
    ESP_LOGI(TAG, "lightweight: scene compose disabled");
    return false;
}

const lv_image_dsc_t *rem_scene_present(uint8_t bg_index,
                                        const lv_image_dsc_t *sprite,
                                        int rem_x,
                                        int rem_y)
{
    (void)bg_index;
    (void)sprite;
    (void)rem_x;
    (void)rem_y;
    return NULL;
}

bool rem_scene_last_changed(void)
{
    return false;
}
