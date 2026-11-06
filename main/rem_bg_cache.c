#include "rem_bg_cache.h"

#include <string.h>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_jpeg_dec.h"
#include "esp_jpeg_common.h"
#include "generated/rem_assets.h"

static const char *TAG = "rem_bg";

enum { REM_BG_COUNT = 6 };

static lv_image_dsc_t s_dsc;
static uint8_t *s_pixels;
static int s_index = -1;

static bool decode_to_cache(uint8_t index)
{
    if (index >= REM_BG_COUNT) {
        return false;
    }
    if (s_index == (int)index && s_pixels) {
        return true;
    }

    const lv_image_dsc_t *src = rem_backgrounds[index];
    if (!src || !src->data || src->data_size < 4) {
        ESP_LOGE(TAG, "bg%u missing jpeg", index);
        return false;
    }

    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.output_type = JPEG_PIXEL_FORMAT_RGB565_LE;
    config.rotate = JPEG_ROTATE_0D;

    jpeg_dec_handle_t jpeg_dec = NULL;
    if (jpeg_dec_open(&config, &jpeg_dec) != JPEG_ERR_OK) {
        ESP_LOGE(TAG, "bg%u jpeg_dec_open failed", index);
        return false;
    }

    jpeg_dec_io_t io = {0};
    jpeg_dec_header_info_t info = {0};
    io.inbuf = (uint8_t *)src->data;
    io.inbuf_len = (int)src->data_size;

    bool ok = false;
    uint8_t *out = NULL;
    if (jpeg_dec_parse_header(jpeg_dec, &io, &info) != JPEG_ERR_OK) {
        ESP_LOGE(TAG, "bg%u parse_header failed", index);
        goto done;
    }

    int out_len = 0;
    if (jpeg_dec_get_outbuf_len(jpeg_dec, &out_len) != JPEG_ERR_OK || out_len <= 0) {
        out_len = (int)info.width * (int)info.height * 2;
    }

    /* Always PSRAM — never consume internal DMA memory needed by LCD SPI. */
    out = heap_caps_aligned_calloc(16, 1, (size_t)out_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!out) {
        ESP_LOGE(TAG, "bg%u PSRAM alloc %d failed", index, out_len);
        goto done;
    }

    io.outbuf = out;
    if (jpeg_dec_process(jpeg_dec, &io) != JPEG_ERR_OK) {
        ESP_LOGE(TAG, "bg%u decode failed", index);
        heap_caps_free(out);
        out = NULL;
        goto done;
    }

    /* Swap atomically after decode so LVGL never sees a freed buffer. */
    uint8_t *old = s_pixels;
    s_pixels = out;
    s_index = (int)index;
    memset(&s_dsc, 0, sizeof(s_dsc));
    s_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    s_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    s_dsc.header.w = info.width;
    s_dsc.header.h = info.height;
    s_dsc.header.stride = (uint32_t)info.width * 2u;
    s_dsc.data_size = (uint32_t)out_len;
    s_dsc.data = out;
    if (old) {
        heap_caps_free(old);
    }
    ESP_LOGI(TAG, "bg%u cached %ux%u (%d bytes PSRAM, single-slot)", index, info.width, info.height, out_len);
    ok = true;

done:
    jpeg_dec_close(jpeg_dec);
    return ok;
}

bool rem_bg_cache_init(void)
{
    /* Warm default/offline background only — one slot. */
    return decode_to_cache(5);
}

const lv_image_dsc_t *rem_bg_cached(uint8_t index)
{
    if (!decode_to_cache(index)) {
        if (s_pixels) {
            return &s_dsc;
        }
        return NULL;
    }
    return &s_dsc;
}

const uint8_t *rem_bg_pixels(void)
{
    return s_pixels;
}

uint16_t rem_bg_width(void)
{
    return s_dsc.header.w;
}

uint16_t rem_bg_height(void)
{
    return s_dsc.header.h;
}

uint8_t rem_bg_index(void)
{
    return s_index < 0 ? 0xff : (uint8_t)s_index;
}
