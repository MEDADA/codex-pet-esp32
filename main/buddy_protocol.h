#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "buddy_scene.h"

#define BUDDY_SERVICE_UUID        "0000beef-0000-1000-8000-00805f9b34fb"
#define BUDDY_WRITE_CHAR_UUID     "0000beef-0001-1000-8000-00805f9b34fb"
#define BUDDY_NOTIFY_CHAR_UUID    "0000beef-0002-1000-8000-00805f9b34fb"

#define BUDDY_HOST_ID_LEN 6
#define BUDDY_MAX_TOOL_NAME 17
#define BUDDY_MAX_WORKSPACE 18
#define BUDDY_MAX_MODEL 18

#define BUDDY_BRIGHTNESS_FRAME 0xFE
#define BUDDY_ORIENTATION_FRAME 0xFD
#define BUDDY_WORKSPACE_FRAME 0xFC
#define BUDDY_MSG_PREVIEW_FRAME 0xFB
#define BUDDY_STATS_FRAME 0xFA
#define BUDDY_MODEL_FRAME 0xF9
#define BUDDY_SUBAGENT_FRAME 0xF8
#define BUDDY_EVENT_FRAME 0xF7
#define BUDDY_TIME_HINT_FRAME 0xF6
#define BUDDY_TOOL_HIST_FRAME 0xF5

#define BUDDY_PAIR_REQUEST 0xE0
#define BUDDY_UNPAIR 0xE1
#define BUDDY_PAIR_ACCEPTED 0xE0
#define BUDDY_PAIR_REJECTED 0xE1
#define BUDDY_PAIR_PENDING 0xE2

#define BUDDY_CMD_APPROVE 0xF0
#define BUDDY_CMD_DENY 0xF1
#define BUDDY_CMD_SKIP 0xF2

#define BUDDY_BLE_TIMEOUT_MS 60000UL
#define BUDDY_PAIR_CONFIRM_MS 30000UL

typedef enum {
    BUDDY_MODE_ONBOARD = 0,
    BUDDY_MODE_DEMO,
    BUDDY_MODE_AGENT,
    BUDDY_MODE_PAIR_CONFIRM,
} buddy_mode_t;

typedef struct {
    uint8_t source_id;
    uint8_t status_id;
    bool status_valid; /* Current-connection primary frame evidence, not wire data. */
    uint32_t status_ms;
    char tool_name[BUDDY_MAX_TOOL_NAME + 1];
    char workspace[BUDDY_MAX_WORKSPACE + 1];
    char model[BUDDY_MAX_MODEL + 1];
    uint8_t brightness_percent;
    uint8_t orientation;
    uint8_t active_sessions;
    uint8_t total_sessions;
    uint16_t tool_calls;
    uint8_t duration_min;
    uint8_t subagents;
    uint8_t last_event;
    uint32_t event_ms;
    char preview[145];
    char tool_history[180];
    char preview_segments[8][17];
    char history_segments[10][13];
    /* Wall clock from TIME_HINT (0xF6); advanced locally after sync.
     * Frame: 0xF6 H [M [S [year_hi year_lo month day]]] */
    uint8_t hour_hint;
    uint8_t minute_hint;
    uint8_t second_hint;
    uint16_t year_hint;
    uint8_t month_hint;
    uint8_t day_hint;
    bool time_valid;
    bool time_has_minute; /* false if Mac only sent hour (legacy) */
    bool date_valid;
    uint32_t time_sync_ms;
} buddy_agent_state_t;

static inline buddy_scene_t buddy_status_to_scene(uint8_t status)
{
    switch (status) {
    case 1:
    case 2:
        return BUDDY_SCENE_WORK;
    case 3:
        return BUDDY_SCENE_ALERT;
    case 4:
        return BUDDY_SCENE_QUESTION;
    default:
        return BUDDY_SCENE_SLEEP;
    }
}

/* Brand / product names stay English. */
static inline const char *buddy_source_name(uint8_t id)
{
    static const char *names[] = {
        "Claude", "Codex", "Gemini", "Cursor", "Copilot", "Trae", "Qoder", "Factory",
        "CodeBuddy", "StepFun", "OpenCode", "Qwen", "AntiGrav", "WorkBuddy", "Hermes", "Kimi"
    };
    return (id < 16) ? names[id] : "?";
}

/* Fixed UI status strings in Simplified Chinese. */
static inline const char *buddy_status_name(uint8_t id)
{
    switch (id) {
    case 0: return "空闲";
    case 1: return "处理中";
    case 2: return "运行中";
    case 3: return "需批准";
    case 4: return "需回答";
    default: return "未知";
    }
}

static inline int buddy_days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1 || month > 12) return 30;
    int d = mdays[month - 1];
    if (month == 2) {
        bool leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
        if (leap) d = 29;
    }
    return d;
}
static inline bool buddy_date_valid(uint16_t year,uint8_t month,uint8_t day){
    return year>=2000&&year<=2099&&month>=1&&month<=12&&day>=1&&day<=buddy_days_in_month(year,month);
}

/* Apply elapsed ms since sync onto the last TIME_HINT snapshot. */
static inline void buddy_wall_time_now(const buddy_agent_state_t *st, uint32_t now_ms,
                                       uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint32_t base_h = st->hour_hint;
    uint32_t base_m = st->time_has_minute ? st->minute_hint : 0;
    uint32_t base_s = st->time_has_minute ? st->second_hint : 0;
    uint32_t elapsed_s = 0;
    if (st->time_valid) {
        elapsed_s = (now_ms - st->time_sync_ms) / 1000u;
    }
    uint32_t total = base_h * 3600u + base_m * 60u + base_s + elapsed_s;
    total %= 86400u;
    *hour = (uint8_t)(total / 3600u);
    *minute = (uint8_t)((total % 3600u) / 60u);
    *second = (uint8_t)(total % 60u);
}

/* Date advanced by midnights crossed since sync (requires date_valid). */
static inline void buddy_wall_date_now(const buddy_agent_state_t *st, uint32_t now_ms,
                                       uint16_t *year, uint8_t *month, uint8_t *day)
{
    *year = st->year_hint;
    *month = st->month_hint;
    *day = st->day_hint;
    if (!st->date_valid || !st->time_valid) {
        return;
    }
    uint32_t base_h = st->hour_hint;
    uint32_t base_m = st->time_has_minute ? st->minute_hint : 0;
    uint32_t base_s = st->time_has_minute ? st->second_hint : 0;
    uint32_t elapsed_s = 0;
    if (st->time_valid) {
        elapsed_s = (now_ms - st->time_sync_ms) / 1000u;
    }
    uint32_t total = base_h * 3600u + base_m * 60u + base_s + elapsed_s;
    uint32_t days_fwd = total / 86400u;
    uint16_t y = st->year_hint;
    uint8_t mo = st->month_hint;
    uint8_t d = st->day_hint;
    while (days_fwd > 0) {
        int dim = buddy_days_in_month(y, mo);
        int remain = dim - d;
        if ((int)days_fwd <= remain) {
            d = (uint8_t)(d + days_fwd);
            days_fwd = 0;
        } else {
            days_fwd -= (uint32_t)(remain + 1);
            d = 1;
            mo++;
            if (mo > 12) {
                mo = 1;
                y++;
            }
        }
    }
    *year = y;
    *month = mo;
    *day = d;
}
