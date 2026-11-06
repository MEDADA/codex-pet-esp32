#include "buddy_ble.h"
#include "buddy_state_evidence.h"
#include "buddy_rtc.h"
#include "buddy_settings.h"

#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_mac.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "buddy_ble";

static char s_device_name[20] = "Buddy";
static uint8_t s_own_addr_type;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t s_notify_val_handle;
static bool s_connected;
static bool s_authenticated;
static bool s_paired;
static uint8_t s_paired_host[BUDDY_HOST_ID_LEN];
static uint8_t s_pending_host[BUDDY_HOST_ID_LEN];
static uint32_t s_pair_request_ms;
static buddy_mode_t s_mode = BUDDY_MODE_ONBOARD;
static buddy_agent_state_t s_state;
static uint32_t s_last_data_ms;
static SemaphoreHandle_t s_lock;
static buddy_request_t s_request;
static bool s_has_ever_connected;

/* Self-heal against Mac→Buddy write floods that exhaust NimBLE MBUF/ACL. */
static uint32_t s_write_window_start_ms;
static uint16_t s_writes_in_window;
static uint8_t s_mbuf_fail_streak;
static uint8_t s_last_time_hint_h = 0xFF;
static uint8_t s_last_time_hint_m = 0xFF;
static uint16_t s_last_time_hint_y;
static uint8_t s_last_time_hint_mo;
static uint8_t s_last_time_hint_d;
static bool s_last_time_hint_has_date;

#define BUDDY_WRITE_RATE_WINDOW_MS 1000
/* Mac connect flush can blast preview/history (tens of withoutResponse
 * frames) in <1s. 36 was too tight and caused connect↔flood loops that
 * kept the UI stuck offline. Raise the ceiling and drop excess writes
 * instead of disconnecting; mbuf-fail streak still self-heals ACL. */
#define BUDDY_WRITE_RATE_LIMIT 120
#define BUDDY_MBUF_FAIL_LIMIT 8

static void lock(void) { xSemaphoreTake(s_lock, portMAX_DELAY); }
static void unlock(void) { xSemaphoreGive(s_lock); }

static uint32_t now_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static void load_pairing(void)
{
    nvs_handle_t h;
    if (nvs_open("buddy", NVS_READONLY, &h) != ESP_OK) {
        return;
    }
    uint8_t ps = 0;
    size_t len = BUDDY_HOST_ID_LEN;
    if (nvs_get_u8(h, "ps", &ps) == ESP_OK && ps) {
        if (nvs_get_blob(h, "ph", s_paired_host, &len) == ESP_OK && len == BUDDY_HOST_ID_LEN) {
            s_paired = true;
        }
    }
    uint8_t bright = 70;
    if (nvs_get_u8(h, "bright", &bright) == ESP_OK) {
        s_state.brightness_percent = bright;
    } else {
        s_state.brightness_percent = 70;
    }
    nvs_close(h);
}

static void save_pairing(bool paired)
{
    nvs_handle_t h;
    if (nvs_open("buddy", NVS_READWRITE, &h) != ESP_OK) {
        return;
    }
    if (paired) {
        nvs_set_u8(h, "ps", 1);
        nvs_set_blob(h, "ph", s_paired_host, BUDDY_HOST_ID_LEN);
    } else {
        nvs_set_u8(h, "ps", 0);
        nvs_erase_key(h, "ph");
    }
    nvs_commit(h);
    nvs_close(h);
}

static void __attribute__((unused)) save_brightness(uint8_t pct)
{
    nvs_handle_t h;
    if (nvs_open("buddy", NVS_READWRITE, &h) != ESP_OK) {
        return;
    }
    nvs_set_u8(h, "bright", pct);
    nvs_commit(h);
    nvs_close(h);
}

void buddy_ble_notify_byte(uint8_t value)
{
    if (s_conn_handle == BLE_HS_CONN_HANDLE_NONE || s_notify_val_handle == 0) {
        return;
    }
    struct os_mbuf *om = ble_hs_mbuf_from_flat(&value, 1);
    if (!om) {
        s_mbuf_fail_streak++;
        ESP_LOGW(TAG, "mbuf alloc fail streak=%u", s_mbuf_fail_streak);
        if (s_mbuf_fail_streak >= BUDDY_MBUF_FAIL_LIMIT) {
            s_mbuf_fail_streak = 0;
            buddy_ble_disconnect_client("mbuf exhausted");
        }
        return;
    }
    s_mbuf_fail_streak = 0;
    int rc = ble_gatts_notify_custom(s_conn_handle, s_notify_val_handle, om);
    if (rc != 0) {
        ESP_LOGW(TAG, "notify failed rc=%d", rc);
    }
}

static bool host_match(const uint8_t *a, const uint8_t *b)
{
    return memcmp(a, b, BUDDY_HOST_ID_LEN) == 0;
}

static int handle_write(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;

    uint8_t data[24];
    uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
    if (len == 0 || len > sizeof(data)) {
        if (len > sizeof(data)) {
            len = sizeof(data);
        }
    }
    ble_hs_mbuf_to_flat(ctxt->om, data, sizeof(data), &len);
    if(!len)return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

    uint32_t write_now = now_ms();
    if (write_now - s_write_window_start_ms > BUDDY_WRITE_RATE_WINDOW_MS) {
        s_write_window_start_ms = write_now;
        s_writes_in_window = 0;
    }
    s_writes_in_window++;
    if (s_writes_in_window > BUDDY_WRITE_RATE_LIMIT) {
        /* Drop excess instead of tearing down the link — disconnect made
         * CodeIsland immediately reconnect and re-blast secondaries. */
        if (s_writes_in_window == BUDDY_WRITE_RATE_LIMIT + 1) {
            ESP_LOGW(TAG, "write flood %u/s — drop excess (no disconnect)",
                     (unsigned)s_writes_in_window);
        }
        return 0;
    }

    /* Preview/history frames are high-volume; keep INFO logs for status/time. */
    if (len && (data[0] == BUDDY_MSG_PREVIEW_FRAME || data[0] == BUDDY_TOOL_HIST_FRAME)) {
        ESP_LOGD(TAG, "write len=%u first=0x%02X", len, data[0]);
    } else {
        ESP_LOGI(TAG, "write len=%u first=0x%02X", len, len ? data[0] : 0);
    }

    // Pair request
    if (len >= 1 + BUDDY_HOST_ID_LEN && data[0] == BUDDY_PAIR_REQUEST) {
        uint8_t incoming[BUDDY_HOST_ID_LEN];
        memcpy(incoming, data + 1, BUDDY_HOST_ID_LEN);
        lock();
        bool paired = s_paired;
        bool match = paired && host_match(s_paired_host, incoming);
        bool confirming = (s_mode == BUDDY_MODE_PAIR_CONFIRM);
        unlock();

        if (match) {
            lock();
            s_authenticated = true;
            s_has_ever_connected = true;
            s_mode = BUDDY_MODE_AGENT;
            s_last_data_ms = now_ms();
            unlock();
            buddy_ble_notify_byte(BUDDY_PAIR_ACCEPTED);
            ESP_LOGI(TAG, "pair accepted (known host)");
        } else if (!paired && !confirming) {
            lock();
            memcpy(s_pending_host, incoming, BUDDY_HOST_ID_LEN);
            s_pair_request_ms = now_ms();
            s_mode = BUDDY_MODE_PAIR_CONFIRM;
            unlock();
            buddy_ble_notify_byte(BUDDY_PAIR_PENDING);
            ESP_LOGI(TAG, "pair pending — press BOOT to accept");
        } else {
            buddy_ble_notify_byte(BUDDY_PAIR_REJECTED);
            ESP_LOGI(TAG, "pair rejected");
            // disconnect shortly from UI/task side if needed
        }
        return 0;
    }

    // Unpair
    if (len >= 1 + BUDDY_HOST_ID_LEN && data[0] == BUDDY_UNPAIR) {
        uint8_t incoming[BUDDY_HOST_ID_LEN];
        memcpy(incoming, data + 1, BUDDY_HOST_ID_LEN);
        lock();
        bool should = s_paired && host_match(s_paired_host, incoming);
        unlock();
        if (should) {
            buddy_ble_clear_pair();
            lock();
            s_authenticated = false;
            s_mode = BUDDY_MODE_ONBOARD;
            unlock();
            ESP_LOGI(TAG, "unpaired");
        }
        return 0;
    }

    lock();
    bool auth = s_authenticated;
    unlock();
    if (!auth) {
        ESP_LOGW(TAG, "frame rejected (not authenticated)");
        return 0;
    }

    if(len==2&&data[0]==0xe8&&data[1]==1){
        uint8_t reply[]={0xe8,1,1};
        struct os_mbuf *om=ble_hs_mbuf_from_flat(reply,sizeof(reply));
        if(om)ble_gatts_notify_custom(s_conn_handle,s_notify_val_handle,om);
        return 0;
    }
    if(data[0]==0xe9||data[0]==0xeb){
        lock();
        if(data[0]==0xe9)buddy_request_receive(&s_request,data,len,now_ms());
        else buddy_request_ack(&s_request,data,len);
        unlock();return 0;
    }

    if (len == 2 && data[0] == BUDDY_BRIGHTNESS_FRAME) {
        uint8_t pct = data[1];
        if (pct < 10) pct = 10;
        if (pct > 100) pct = 100;
        buddy_settings_set_brightness(pct);
        lock();
        s_state.brightness_percent = buddy_settings_get_brightness();
        unlock();
        return 0;
    }

    if (len == 2 && data[0] == BUDDY_ORIENTATION_FRAME) {
        lock();
        s_state.orientation = (data[1] == 1) ? 1 : 0;
        unlock();
        return 0;
    }

    if (len >= 2 && data[0] == BUDDY_WORKSPACE_FRAME) {
        uint8_t n = data[1];
        if (n > BUDDY_MAX_WORKSPACE) n = BUDDY_MAX_WORKSPACE;
        lock();
        memset(s_state.workspace, 0, sizeof(s_state.workspace));
        if (n && len >= 2u + n) memcpy(s_state.workspace, data + 2, n);
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len >= 2 && data[0] == BUDDY_MODEL_FRAME) {
        uint8_t n = data[1];
        if (n > BUDDY_MAX_MODEL) n = BUDDY_MAX_MODEL;
        lock();
        memset(s_state.model, 0, sizeof(s_state.model));
        if (n && len >= 2u + n) memcpy(s_state.model, data + 2, n);
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len >= 6 && data[0] == BUDDY_STATS_FRAME) {
        lock();
        s_state.active_sessions = data[1];
        s_state.total_sessions = data[2];
        s_state.tool_calls = ((uint16_t)data[3] << 8) | data[4];
        s_state.duration_min = data[5];
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len >= 2 && data[0] == BUDDY_SUBAGENT_FRAME) {
        lock();
        s_state.subagents = data[1];
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len >= 2 && data[0] == BUDDY_EVENT_FRAME) {
        lock();
        s_state.last_event = data[1];
        s_state.event_ms = now_ms();
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len >= 2 && data[0] == BUDDY_TIME_HINT_FRAME) {
        if((len!=2&&len!=3&&len!=4&&len!=8)||data[1]>23||(len>=3&&data[2]>59)||(len>=4&&data[3]>59))return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        if(len==8&&!buddy_date_valid(((uint16_t)data[4]<<8)|data[5],data[6],data[7]))return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        /* 0xF6 H [M [S [year_hi year_lo month day]]]
         * Legacy Mac 1.0.40 sends hour-only; must NOT reset local M/S on each push. */
        uint8_t hour = data[1];
        if (hour > 23) hour = 23;
        uint32_t now = now_ms();
        if (len >= 3) {
            uint8_t minute = data[2];
            if (minute > 59) minute = 59;
            uint16_t y = 0; uint8_t mo = 0, d = 0; bool has_date = false;
            if (len >= 8) {
                y = ((uint16_t)data[4] << 8) | data[5];
                mo = data[6];
                d = data[7];
                if (buddy_date_valid(y,mo,d)) {
                    has_date = true;
                }
            }
            bool same_minute = (s_last_time_hint_h == hour && s_last_time_hint_m == minute);
            bool same_date = (!has_date && !s_last_time_hint_has_date)
                || (has_date && s_last_time_hint_has_date
                    && s_last_time_hint_y == y && s_last_time_hint_mo == mo && s_last_time_hint_d == d);
            if (same_minute && same_date) {
                /* Drop redundant TIME_HINT (Mac used to send every heartbeat with seconds). */
                lock();
                s_last_data_ms = now;
                unlock();
                return 0;
            }
            s_last_time_hint_h = hour;
            s_last_time_hint_m = minute;
            s_last_time_hint_has_date = has_date;
            if (has_date) {
                s_last_time_hint_y = y;
                s_last_time_hint_mo = mo;
                s_last_time_hint_d = d;
            }
        }
        lock();
        buddy_apply_time_hint(&s_state,data,len,now);
        s_last_data_ms = now;
        unlock();
        /* Only the system clock is rendered; keep it authoritative. */
        buddy_rtc_apply_hint(data, len);
        return 0;
    }

    if (len >= 3 && (data[0] == BUDDY_MSG_PREVIEW_FRAME || data[0] == BUDDY_TOOL_HIST_FRAME)) {
        lock();
        if(data[0]==BUDDY_MSG_PREVIEW_FRAME&&len>=4){
            uint8_t n=data[3]&0x7f,index=data[1];
            if(index<8&&n<=16&&len==4+n){
                if(index==0)memset(s_state.preview_segments,0,sizeof(s_state.preview_segments));
                memset(s_state.preview_segments[index],0,17);memcpy(s_state.preview_segments[index],data+4,n);
                s_state.preview[0]=0;for(int i=0;i<8;i++)strlcat(s_state.preview,s_state.preview_segments[i],sizeof(s_state.preview));
            }
        }else if(data[0]==BUDDY_TOOL_HIST_FRAME){
            uint8_t n=data[2]&0x7f,index=data[1];
            if(index<10&&n<=11&&len==3+n){
                if(index==0)memset(s_state.history_segments,0,sizeof(s_state.history_segments));
                memset(s_state.history_segments[index],0,13);memcpy(s_state.history_segments[index],data+3,n);
                s_state.tool_history[0]=0;for(int i=0;i<10;i++)if(s_state.history_segments[i][0]){strlcat(s_state.tool_history,s_state.history_segments[i],sizeof(s_state.tool_history));strlcat(s_state.tool_history,"\n",sizeof(s_state.tool_history));}
            }
        }
        s_last_data_ms = now_ms();
        unlock();
        return 0;
    }

    if (len < 3) {
        return 0;
    }

    uint8_t tool_len = data[2];
    if(data[0]>=16||data[1]>4||tool_len>BUDDY_MAX_TOOL_NAME||len!=3u+tool_len)return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    if (tool_len > BUDDY_MAX_TOOL_NAME) tool_len = BUDDY_MAX_TOOL_NAME;

    lock();
    if(s_state.source_id!=data[0]){
        s_state.workspace[0]=0;s_state.model[0]=0;s_state.preview[0]=0;s_state.tool_history[0]=0;
        memset(s_state.preview_segments,0,sizeof(s_state.preview_segments));memset(s_state.history_segments,0,sizeof(s_state.history_segments));s_state.last_event=0;
    }
    if(s_state.status_id!=data[1]&&data[1]!=0)s_state.last_event=0;
    s_state.source_id = data[0];
    s_state.status_id = data[1];
    buddy_status_received(&s_state,now_ms());
    memset(s_state.tool_name, 0, sizeof(s_state.tool_name));
    if (tool_len && len >= 3u + tool_len) {
        memcpy(s_state.tool_name, data + 3, tool_len);
    }
    s_last_data_ms = now_ms();
    s_mode = BUDDY_MODE_AGENT;
    unlock();

    ESP_LOGI(TAG, "agent %s status=%s tool=%s",
             buddy_source_name(data[0]), buddy_status_name(data[1]), s_state.tool_name);
    return 0;
}

static int gatt_access(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return handle_write(conn_handle, attr_handle, ctxt, arg);
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        uint8_t zero = 0;
        os_mbuf_append(ctxt->om, &zero, 1);
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

/* 128-bit UUIDs: 0000beef-0000-1000-8000-00805f9b34fb etc.
 * NimBLE stores UUID128 in little-endian byte order. */
static const ble_uuid128_t s_svc_uuid =
    BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
                     0x00, 0x10, 0x00, 0x00, 0xef, 0xbe, 0x00, 0x00);
static const ble_uuid128_t s_write_uuid =
    BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
                     0x00, 0x10, 0x01, 0x00, 0xef, 0xbe, 0x00, 0x00);
static const ble_uuid128_t s_notify_uuid =
    BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
                     0x00, 0x10, 0x02, 0x00, 0xef, 0xbe, 0x00, 0x00);

static const struct ble_gatt_svc_def s_gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &s_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &s_write_uuid.u,
                .access_cb = gatt_access,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                .uuid = &s_notify_uuid.u,
                .access_cb = gatt_access,
                .val_handle = &s_notify_val_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 },
        },
    },
    { 0 },
};

static void start_advertise(void);

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            lock();
            s_conn_handle = event->connect.conn_handle;
            s_connected = true;
            buddy_status_invalidate(&s_state);
            s_authenticated = false;
            unlock();
            ESP_LOGI(TAG, "connected handle=%u", s_conn_handle);
        } else {
            lock();
            s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            s_connected = false;
            buddy_status_invalidate(&s_state);
            unlock();
            start_advertise();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        lock();memset(&s_request,0,sizeof(s_request));
        ESP_LOGI(TAG, "disconnected reason=%d", event->disconnect.reason);
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_connected = false;
        buddy_status_invalidate(&s_state);
        s_authenticated = false;
        s_mbuf_fail_streak = 0;
        s_writes_in_window = 0;
        if (s_mode == BUDDY_MODE_PAIR_CONFIRM) {
            s_mode = BUDDY_MODE_ONBOARD;
        }
        unlock();
        start_advertise();
        return 0;
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "subscribe notify=%d indicate=%d",
                 event->subscribe.cur_notify, event->subscribe.cur_indicate);
        return 0;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        start_advertise();
        return 0;
    default:
        return 0;
    }
}

static void start_advertise(void)
{
    /* Keep ADV payload under 31 bytes: flags + complete name only.
     * Put the 128-bit service UUID in the scan response so Mac can still
     * filter by service UUID during active scan. */
    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)s_device_name;
    fields.name_len = strlen(s_device_name);
    fields.name_is_complete = 1;
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "adv fields rc=%d", rc);
        return;
    }

    struct ble_hs_adv_fields rsp = {0};
    rsp.uuids128 = (ble_uuid128_t *)&s_svc_uuid;
    rsp.num_uuids128 = 1;
    rsp.uuids128_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&rsp);
    if (rc != 0) {
        ESP_LOGW(TAG, "scan rsp fields rc=%d (continuing)", rc);
    }

    struct ble_gap_adv_params adv = {0};
    adv.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &adv, gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "adv start rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "advertising as %s", s_device_name);
    }
}

static void on_sync(void)
{
    int rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(TAG, "ensure addr rc=%d", rc);
        return;
    }
    rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "infer addr rc=%d", rc);
        return;
    }
    start_advertise();
}

static void on_reset(int reason)
{
    ESP_LOGW(TAG, "nimble reset reason=%d", reason);
}

static void host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void buddy_ble_init(void)
{
    s_lock = xSemaphoreCreateMutex();
    memset(&s_state, 0, sizeof(s_state));
    s_state.brightness_percent = 70;
    load_pairing();

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(s_device_name, sizeof(s_device_name), "Buddy-%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed (%s) — BLE disabled, UI continues",
                 esp_err_to_name(err));
        return;
    }
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.gatts_register_cb = NULL;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_gatts_count_cfg(s_gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg rc=%d — BLE disabled, UI continues", rc);
        return;
    }
    rc = ble_gatts_add_svcs(s_gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs rc=%d — BLE disabled, UI continues", rc);
        return;
    }
    rc = ble_svc_gap_device_name_set(s_device_name);
    if (rc != 0) {
        ESP_LOGW(TAG, "gap device name set rc=%d — continuing", rc);
    }

    nimble_port_freertos_init(host_task);
    ESP_LOGI(TAG, "BLE init done name=%s paired=%d", s_device_name, s_paired);
}

bool buddy_ble_is_connected(void) { return s_connected; }
bool buddy_ble_is_authenticated(void) { return s_authenticated; }
bool buddy_ble_is_paired(void) { return s_paired; }

buddy_mode_t buddy_ble_get_mode(void)
{
    lock();
    buddy_mode_t m = s_mode;
    unlock();
    return m;
}

void buddy_ble_set_mode(buddy_mode_t mode)
{
    lock();
    s_mode = mode;
    unlock();
}

void buddy_ble_request_snapshot(buddy_request_t *out){lock();*out=s_request;unlock();}
bool buddy_ble_request_action(uint8_t action,uint32_t expected,uint32_t expected_digest){
    uint8_t out[14];lock();
    bool current=(s_request.kind==1&&s_state.status_id==3)||(s_request.kind==2&&s_state.status_id==4);
    s_request.verified=buddy_request_content_matches(&s_request,s_state.source_id,s_state.status_id,s_state.tool_name,s_state.workspace,s_state.model,s_state.preview);
    size_t len=(s_connected&&s_authenticated&&buddy_status_fresh(&s_state,now_ms())&&current&&s_request.digest==expected_digest)?buddy_request_encode(&s_request,action,expected,now_ms(),out):0;
    unlock();if(!len)return false;
    struct os_mbuf *om=ble_hs_mbuf_from_flat(out,len);
    if(!om||ble_gatts_notify_custom(s_conn_handle,s_notify_val_handle,om)!=0){lock();s_request.pending=false;s_request.result=2;s_request.result_request=s_request.sent_request;unlock();return false;}
    return true;
}
void buddy_ble_snapshot(buddy_agent_state_t *out,buddy_mode_t *mode,bool *connected,bool *authenticated,uint32_t *last_ms,buddy_request_t *request){
    lock();buddy_request_check_timeout(&s_request,now_ms());*out=s_state;*mode=s_mode;*connected=s_connected;*authenticated=s_authenticated;*last_ms=s_last_data_ms;
    if (request) {
        *request=s_request;
        request->verified=buddy_request_content_matches(request,s_state.source_id,s_state.status_id,
            s_state.tool_name,s_state.workspace,s_state.model,s_state.preview);
    }
    unlock();
}
uint32_t buddy_ble_last_data_ms(void) { return s_last_data_ms; }
const char *buddy_ble_device_name(void) { return s_device_name; }
const uint8_t *buddy_ble_pending_host_id(void) { return s_pending_host; }
uint32_t buddy_ble_pair_request_ms(void) { return s_pair_request_ms; }

void buddy_ble_accept_pair(void)
{
    lock();
    memcpy(s_paired_host, s_pending_host, BUDDY_HOST_ID_LEN);
    s_paired = true;
    s_authenticated = true;
    s_has_ever_connected = true;
    s_mode = BUDDY_MODE_AGENT;
    s_last_data_ms = now_ms();
    unlock();
    save_pairing(true);
    buddy_ble_notify_byte(BUDDY_PAIR_ACCEPTED);
    ESP_LOGI(TAG, "pair accepted via BOOT");
}

void buddy_ble_reject_pair(void)
{
    lock();
    s_mode = BUDDY_MODE_ONBOARD;
    unlock();
    buddy_ble_notify_byte(BUDDY_PAIR_REJECTED);
    buddy_ble_disconnect_client("pair rejected");
}

void buddy_ble_clear_pair(void)
{
    lock();
    memset(s_paired_host, 0, sizeof(s_paired_host));
    s_paired = false;
    s_authenticated = false;
    unlock();
    save_pairing(false);
}

void buddy_ble_disconnect_client(const char *reason)
{
    ESP_LOGI(TAG, "disconnect (%s)", reason ? reason : "");
    if (s_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ble_gap_terminate(s_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }
}
