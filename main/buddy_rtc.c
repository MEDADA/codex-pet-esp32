#include "buddy_rtc.h"

#include <sys/time.h>
#include <time.h>
#include "buddy_protocol.h"
#include "esp_log.h"

bool buddy_rtc_read(buddy_rtc_time_t *out)
{
    const time_t now = time(NULL);
    struct tm calendar;
    if (!gmtime_r(&now, &calendar) || calendar.tm_year < 100 || calendar.tm_year > 199)
        return false;

    *out = (buddy_rtc_time_t){
        .year = calendar.tm_year + 1900,
        .month = calendar.tm_mon + 1,
        .day = calendar.tm_mday,
        .hour = calendar.tm_hour,
        .minute = calendar.tm_min,
        .second = calendar.tm_sec,
    };
    return true;
}

bool buddy_rtc_sync(const buddy_rtc_time_t *time)
{
    if (time->year < 2000 || time->year > 2099 ||
        !buddy_date_valid(time->year, time->month, time->day) ||
        time->hour > 23 || time->minute > 59 || time->second > 59) return false;

    // The bridge supplies local wall time. Use a timezone-neutral calendar
    // encoding so reading it back cannot apply an additional timezone shift.
    unsigned days = 10957; // 1970-01-01 to 2000-01-01
    for (uint16_t year = 2000; year < time->year; ++year)
        days += 365 + (year % 4 == 0);
    for (uint8_t month = 1; month < time->month; ++month)
        days += buddy_days_in_month(time->year, month);
    days += time->day - 1;
    const time_t seconds = (time_t)days * 86400 + time->hour * 3600 +
                           time->minute * 60 + time->second;

    struct timeval previous;
    if (gettimeofday(&previous, NULL) == 0 &&
        previous.tv_sec >= seconds - 2 && previous.tv_sec <= seconds + 2) return true;

    const struct timeval value = {.tv_sec = seconds};
    if (settimeofday(&value, NULL) != 0) return false;
    ESP_LOGI("buddy_rtc", "ESP32-S3 clock synchronized");
    return true;
}

bool buddy_rtc_apply_hint(const uint8_t *data, uint16_t len)
{
    if (!data || len < 4 || data[0] != BUDDY_TIME_HINT_FRAME) return false;

    buddy_rtc_time_t time = {0};
    time.hour = data[1];
    time.minute = data[2];
    time.second = data[3];
    if (time.hour > 23 || time.minute > 59 || time.second > 59) return false;

    if (len >= 8) {
        time.year = ((uint16_t)data[4] << 8) | data[5];
        time.month = data[6];
        time.day = data[7];
        if (!buddy_date_valid(time.year, time.month, time.day)) return false;
    } else {
        /* Hour/minute-only hint carries no calendar day. Keep the day the
         * system clock already has; if it has none there is nothing to
         * anchor an absolute time to. */
        buddy_rtc_time_t current;
        if (!buddy_rtc_read(&current)) return false;
        time.year = current.year;
        time.month = current.month;
        time.day = current.day;
    }
    return buddy_rtc_sync(&time);
}
