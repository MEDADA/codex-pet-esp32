#include "pet_motion_clip.h"
#include "pet_frame_layout.h"
#include "pet_frame_schedule.h"
#include "pet_idle_view.h"
#include "misc/cache/instance/lv_image_cache.h"

#include <stdio.h>
#include "esp_heap_caps.h"
#include "esp_jpeg_dec.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

enum { MAX_FRAMES = 240, MAX_JPEG = 65536, MAX_PIXELS = PET_WORK_W * PET_WORK_H * 2 };
static const char *const names[] = {
    [PET_CLIP_CURIOUS]="curious", [PET_CLIP_SHY]="shy",
    [PET_CLIP_LEFT]="imu_left", [PET_CLIP_RIGHT]="imu_right",
    [PET_CLIP_FORWARD]="imu_forward", [PET_CLIP_BACKWARD]="imu_backward",
    [PET_CLIP_SHAKE]="shake", [PET_CLIP_HEAD]="touch_head",
    [PET_CLIP_CHEEK]="touch_cheek", [PET_CLIP_HANDS]="touch_hands",
    [PET_CLIP_TAIL]="touch_tail", [PET_CLIP_WORKING]="working", [PET_CLIP_IDLE]="idle",
};

typedef struct {
    pet_clip_id_t id;
    uint32_t generation;
    uint64_t ordinal;
    unsigned slot;
} frame_request_t;

typedef struct {
    frame_request_t request;
    unsigned count;
    const uint8_t *pixels;
} frame_result_t;

/* One in-flight request. The worker owns storage and the requested back buffer.
 * The UI owns the displayed slot until it replaces the image source. */
static QueueHandle_t requests, results;
static lv_obj_t *avatar;
static lv_image_dsc_t frames[2];
static pet_clip_id_t active;
static uint32_t generation;
static uint64_t started, shown;
static unsigned count;
static int front = -1;
static bool pending;

static unsigned frame_width(pet_clip_id_t id)
{
    return id == PET_CLIP_WORKING ? PET_WORK_W : PET_IDLE_W;
}

static unsigned frame_height(pet_clip_id_t id)
{
    return id == PET_CLIP_WORKING ? PET_WORK_H : PET_IDLE_H;
}

/* Worker-only state. */
static FILE *stream;
static uint8_t *compressed, *pixels[2];
static uint32_t offsets[MAX_FRAMES + 1];
static uint32_t stream_count;
static bool mount_attempted, mounted;

static bool open_clip(pet_clip_id_t id)
{
    if (stream) fclose(stream);
    stream = NULL;
    if (id == PET_CLIP_IDLE) return true;

    if (!mount_attempted) {
        const esp_vfs_spiffs_conf_t config = {
            .base_path="/pet", .partition_label="storage", .max_files=1,
            .format_if_mount_failed=false,
        };
        mount_attempted = true;
        mounted = esp_vfs_spiffs_register(&config) == ESP_OK;
    }
    if (!mounted) return false;

    char path[48];
    snprintf(path, sizeof(path), "/pet/%s.mjp", names[id]);
    stream = fopen(path, "rb");
    if (!stream) return false;
    if (fread(&stream_count, sizeof(uint32_t), 1, stream) != 1 ||
        stream_count == 0 || stream_count > MAX_FRAMES ||
        fread(offsets, sizeof(offsets[0]), stream_count + 1, stream) != stream_count + 1 ||
        offsets[0] != 0) goto fail;
    for (unsigned i = 0; i < stream_count; i++) {
        if (offsets[i+1] <= offsets[i] || offsets[i+1] - offsets[i] > MAX_JPEG) goto fail;
    }
    if (fseek(stream, 0, SEEK_END) ||
        ftell(stream) != (long)((stream_count + 2) * sizeof(uint32_t) + offsets[stream_count])) goto fail;
    return true;

fail:
    fclose(stream);
    stream = NULL;
    return false;
}

static bool decode_jpeg(const uint8_t *data, size_t size, uint8_t *out, unsigned width, unsigned height)
{
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.output_type = JPEG_PIXEL_FORMAT_RGB565_LE;
    jpeg_dec_handle_t decoder = NULL;
    if (jpeg_dec_open(&config, &decoder) != JPEG_ERR_OK) return false;
    jpeg_dec_io_t io = {.inbuf=(uint8_t *)data, .inbuf_len=(int)size, .outbuf=out};
    jpeg_dec_header_info_t info = {0};
    int length = 0;
    const bool ok = jpeg_dec_parse_header(decoder, &io, &info) == JPEG_ERR_OK &&
                    info.width == width && info.height == height &&
                    jpeg_dec_get_outbuf_len(decoder, &length) == JPEG_ERR_OK &&
                    length == (int)(width * height * 2) &&
                    jpeg_dec_process(decoder, &io) == JPEG_ERR_OK;
    jpeg_dec_close(decoder);
    return ok;
}

static void record_work_frame(uint32_t read_us, uint32_t decode_us)
{
    static uint32_t samples, max_read_us, max_decode_us;
    static uint64_t total_read_us, total_decode_us;
    total_read_us += read_us;
    total_decode_us += decode_us;
    if (read_us > max_read_us) max_read_us = read_us;
    if (decode_us > max_decode_us) max_decode_us = decode_us;
    if (++samples == 48) {
        ESP_LOGI("pet_perf", "working n=%u read_mean_us=%u read_max_us=%u jpeg_mean_us=%u jpeg_max_us=%u",
                 (unsigned)samples, (unsigned)(total_read_us/samples), (unsigned)max_read_us,
                 (unsigned)(total_decode_us/samples), (unsigned)max_decode_us);
        samples = max_read_us = max_decode_us = 0;
        total_read_us = total_decode_us = 0;
    }
}

static const uint8_t *decode_frame(frame_request_t request, unsigned frame_count)
{
    const unsigned index = request.ordinal % frame_count;
    const unsigned width = frame_width(request.id), height = frame_height(request.id);
    const uint8_t *data;
    size_t size;
    const int64_t read_started = esp_timer_get_time();
    if (request.id == PET_CLIP_IDLE) {
        data = pet_idle_frame_data(index, &size);
        /* Frame zero is already RGB565 in flash. */
        if (index == 0) return size == width * height * 2 ? data : NULL;
    } else {
        if (!compressed) compressed = heap_caps_malloc(MAX_JPEG, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!compressed) return NULL;
        size = offsets[index + 1] - offsets[index];
        const long header = (long)((stream_count + 2) * sizeof(uint32_t));
        if (fseek(stream, header + offsets[index], SEEK_SET) ||
            fread(compressed, 1, size, stream) != size) return NULL;
        data = compressed;
    }

    uint8_t **slot = &pixels[request.slot];
    if (!*slot) *slot = heap_caps_aligned_alloc(16, MAX_PIXELS, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!*slot) return NULL;
    const int64_t decode_started = esp_timer_get_time();
    if (!decode_jpeg(data, size, *slot, width, height)) return NULL;
    if (request.id == PET_CLIP_WORKING)
        record_work_frame((uint32_t)(decode_started-read_started),
                          (uint32_t)(esp_timer_get_time()-decode_started));
    return *slot;
}

static void decoder_task(void *arg)
{
    (void)arg;
    uint32_t open_generation = 0;
    bool opened = false;
    for (;;) {
        frame_request_t request;
        xQueueReceive(requests, &request, portMAX_DELAY);
        if (request.generation != open_generation || !opened) {
            opened = open_clip(request.id);
            open_generation = request.generation;
        }
        frame_result_t result = {
            .request = request,
            .count = request.id == PET_CLIP_IDLE ? PET_IDLE_FRAME_COUNT : stream_count,
        };
        if (opened) result.pixels = decode_frame(request, result.count);
        xQueueSend(results, &result, portMAX_DELAY);
    }
}

void pet_motion_clip_stop(void)
{
    active = PET_CLIP_NONE;
    ++generation;
    count = 0;
}

static uint64_t due_frame(uint64_t now)
{
    if (!count) return 0;
    if (active == PET_CLIP_WORKING) return pet_frame_due(now, started, count, 30000);
    if (active == PET_CLIP_IDLE) return pet_frame_due(now, started, count, 8000);
    return pet_frame_due(now, started, 6, 1000);
}

static void publish(const frame_result_t *result)
{
    const unsigned slot = result->request.slot;
    const unsigned width = frame_width(active), height = frame_height(active);
    lv_image_cache_drop(&frames[slot]);
    frames[slot] = (lv_image_dsc_t){
        .header = {.magic=LV_IMAGE_HEADER_MAGIC, .cf=LV_COLOR_FORMAT_RGB565,
                   .w=width, .h=height, .stride=width*2},
        .data_size = width * height * 2,
        .data = result->pixels,
    };
    lv_image_set_src(avatar, &frames[slot]);
    front = slot;
    shown = result->request.ordinal;
}

static void advance(lv_timer_t *timer)
{
    (void)timer;
    const uint64_t now = esp_timer_get_time() / 1000;
    const uint64_t due = due_frame(now);
    const bool loop = active == PET_CLIP_WORKING || active == PET_CLIP_IDLE;
    if (count && !loop && due >= count) pet_motion_clip_stop();

    frame_result_t result;
    if (pending && xQueuePeek(results, &result, 0) == pdTRUE) {
        const bool current = pet_frame_is_current(generation, result.request.generation);
        if (!current || result.request.ordinal <= due) {
            xQueueReceive(results, &result, 0);
            pending = false;
            if (current) {
                if (!result.pixels) {
                    ESP_LOGW("pet_clip", "%s decode failed at frame %u", names[active],
                             (unsigned)result.request.ordinal);
                    pet_motion_clip_stop();
                    if (result.request.id == PET_CLIP_IDLE) pet_idle_view_show_still();
                    return;
                }
                if (!count) {
                    count = result.count;
                    started = now;
                    ESP_LOGI("pet_clip", "playing %s frames=%u", names[active], count);
                    if (active != PET_CLIP_IDLE)
                        lv_obj_set_pos(avatar, active == PET_CLIP_WORKING ? PET_WORK_X : PET_IDLE_X,
                                               active == PET_CLIP_WORKING ? PET_WORK_Y : PET_IDLE_Y);
                }
                publish(&result);
            }
        }
    }

    if (pending || active == PET_CLIP_NONE) return;
    const uint64_t next = count ? pet_frame_request(due, shown) : 0;
    if (count && !loop && next >= count) return;
    const frame_request_t request = {
        .id = active, .generation = generation, .ordinal = next,
        .slot = pet_frame_back_buffer(front),
    };
    pending = xQueueSend(requests, &request, 0) == pdTRUE;
}

void pet_motion_clip_init(lv_obj_t *image)
{
    avatar = image;
    if (requests) return;
    requests = xQueueCreate(1, sizeof(frame_request_t));
    results = xQueueCreate(1, sizeof(frame_result_t));
    lv_timer_t *timer = lv_timer_create(advance, 12, NULL);
    if (requests && results && timer &&
        xTaskCreatePinnedToCore(decoder_task, "pet_decode", 6144, NULL, 5, NULL, 0) == pdPASS) return;
    if (timer) lv_timer_delete(timer);
    if (results) vQueueDelete(results);
    if (requests) vQueueDelete(requests);
    requests = results = NULL;
    ESP_LOGW("pet_clip", "decoder unavailable; using still images");
}

bool pet_motion_clip_begin(pet_clip_id_t id)
{
    pet_motion_clip_stop();
    if (!requests || id <= PET_CLIP_NONE || id > PET_CLIP_IDLE) return false;
    active = id;
    advance(NULL);
    return true;
}

bool pet_motion_clip_active(void)
{
    return active != PET_CLIP_NONE;
}

void pet_motion_clip_pause_idle(void)
{
    if (active == PET_CLIP_IDLE) pet_motion_clip_stop();
}
