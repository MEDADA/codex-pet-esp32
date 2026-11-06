#pragma once
#include <stdint.h>
#include <stdbool.h>

#define BUDDY_BTN_NONE 0
#define BUDDY_BTN_SHORT 1
#define BUDDY_BTN_LONG 2
#define BUDDY_BTN_SUPER_LONG 3

typedef enum {
    BUDDY_BTN_SRC_NONE = 0,
    BUDDY_BTN_SRC_BOOT, /* GPIO0 */
    BUDDY_BTN_SRC_PWR,  /* AXP2101 power key */
} buddy_btn_src_t;

typedef struct {
    buddy_btn_src_t src;
    int ev;
} buddy_btn_t;

void buddy_button_init(void);
buddy_btn_t buddy_button_poll(uint32_t now_ms);
bool buddy_button_pmu_read(uint8_t reg,uint8_t *value);
