#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t year;
    uint8_t month, day, hour, minute, second;
} buddy_rtc_time_t;

bool buddy_rtc_read(buddy_rtc_time_t *time);
bool buddy_rtc_sync(const buddy_rtc_time_t *time);

/* Apply a validated 0xF6 time hint to the system clock. Frame:
 *   0xF6 H M S          (calendar day taken from the current system date)
 *   0xF6 H M S YHi YLo month day
 * Returns false when the frame is malformed or cannot anchor to a date. */
bool buddy_rtc_apply_hint(const uint8_t *data, uint16_t len);
