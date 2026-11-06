#include "buddy_button.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"

/* ESP32-S3 BOOT button is GPIO0 (active low). */
#define BOOT_GPIO GPIO_NUM_0
#define DEBOUNCE_MS 30
#define LONG_MS 600
#define SUPER_LONG_MS 3000

/* AXP2101 on shared BSP I2C — PWR side button is PMU PEK. */
#define AXP_ADDR 0x34
#define AXP_REG_INTEN2 0x41
#define AXP_REG_INTSTS1 0x48
#define AXP_REG_INTSTS2 0x49
#define AXP_REG_INTSTS3 0x4A
#define AXP_REG_IRQ_OFF 0x22
#define AXP_REG_PEK_KEY 0x27
/* INTSTS2: bit2=PKEY_LONG, bit3=PKEY_SHORT (matches Waveshare example masks). */
#define AXP_STS2_PKEY_LONG  0x04
#define AXP_STS2_PKEY_SHORT 0x08

static const char *TAG = "buddy_btn";

static bool s_boot_stable = true;
static bool s_boot_last = true;
static uint32_t s_boot_last_change;
static uint32_t s_boot_press_start;
static bool s_boot_pressed;
static bool s_boot_long_fired;
static bool s_boot_super_fired;

static i2c_master_dev_handle_t s_axp;
static bool s_axp_ok;

static esp_err_t axp_wr(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(s_axp, buf, 2, 50);
}

static esp_err_t axp_rd(uint8_t reg, uint8_t *val)
{
    return i2c_master_transmit_receive(s_axp, &reg, 1, val, 1, 50);
}

bool buddy_button_pmu_read(uint8_t reg,uint8_t *value){return s_axp_ok && value && axp_rd(reg,value)==ESP_OK;}

static void axp_init(void)
{
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    if (!bus) {
        ESP_LOGW(TAG, "no I2C bus; PWR button unavailable");
        return;
    }
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AXP_ADDR,
        .scl_speed_hz = 100000,
    };
    if (i2c_master_bus_add_device(bus, &cfg, &s_axp) != ESP_OK) {
        ESP_LOGW(TAG, "AXP2101 add device failed");
        return;
    }
    /* Soft power-off only on long PEK; keep short/long IRQs for UI. */
    (void)axp_wr(AXP_REG_IRQ_OFF, 0x06); /* enable PEK as power-off source */
    (void)axp_wr(AXP_REG_PEK_KEY, 0x10);  /* ~4s hold to power off */
    /* Enable PKEY short+long in INTEN2; leave other banks as-is / zero unused. */
    uint8_t inten2 = 0;
    if (axp_rd(AXP_REG_INTEN2, &inten2) == ESP_OK) {
        inten2 |= (AXP_STS2_PKEY_LONG | AXP_STS2_PKEY_SHORT);
        (void)axp_wr(AXP_REG_INTEN2, inten2);
    } else {
        (void)axp_wr(AXP_REG_INTEN2, AXP_STS2_PKEY_LONG | AXP_STS2_PKEY_SHORT);
    }
    (void)axp_wr(AXP_REG_INTSTS1, 0xFF);
    (void)axp_wr(AXP_REG_INTSTS2, 0xFF);
    (void)axp_wr(AXP_REG_INTSTS3, 0xFF);
    s_axp_ok = true;
    ESP_LOGI(TAG, "PWR button via AXP2101 PEK IRQ");
}

static buddy_btn_t poll_pwr(void)
{
    buddy_btn_t out = {.src = BUDDY_BTN_SRC_NONE, .ev = BUDDY_BTN_NONE};
    if (!s_axp_ok) {
        return out;
    }
    uint8_t sts2 = 0;
    if (axp_rd(AXP_REG_INTSTS2, &sts2) != ESP_OK) {
        return out;
    }
    if (sts2 & (AXP_STS2_PKEY_SHORT | AXP_STS2_PKEY_LONG)) {
        /* Prefer short if both somehow set; clear all three status regs. */
        bool is_long = (sts2 & AXP_STS2_PKEY_LONG) != 0;
        bool is_short = (sts2 & AXP_STS2_PKEY_SHORT) != 0;
        (void)axp_wr(AXP_REG_INTSTS1, 0xFF);
        (void)axp_wr(AXP_REG_INTSTS2, 0xFF);
        (void)axp_wr(AXP_REG_INTSTS3, 0xFF);
        out.src = BUDDY_BTN_SRC_PWR;
        if (is_long && !is_short) {
            out.ev = BUDDY_BTN_LONG;
        } else if (is_long && is_short) {
            /* Long press also sets short on some PMUs flows — treat as long. */
            out.ev = BUDDY_BTN_LONG;
        } else {
            out.ev = BUDDY_BTN_SHORT;
        }
    }
    return out;
}

static buddy_btn_t poll_boot(uint32_t now_ms)
{
    buddy_btn_t out = {.src = BUDDY_BTN_SRC_NONE, .ev = BUDDY_BTN_NONE};
    bool raw = gpio_get_level(BOOT_GPIO) != 0;
    if (raw != s_boot_last) {
        s_boot_last = raw;
        s_boot_last_change = now_ms;
    }
    if ((now_ms - s_boot_last_change) < DEBOUNCE_MS) {
        return out;
    }

    if (s_boot_stable == s_boot_last) {
        if (s_boot_pressed && !s_boot_super_fired && (now_ms - s_boot_press_start) >= SUPER_LONG_MS) {
            s_boot_super_fired = true;
            s_boot_long_fired = true;
            out.src = BUDDY_BTN_SRC_BOOT;
            out.ev = BUDDY_BTN_SUPER_LONG;
            return out;
        }
        if (s_boot_pressed && !s_boot_long_fired && (now_ms - s_boot_press_start) >= LONG_MS) {
            s_boot_long_fired = true;
            out.src = BUDDY_BTN_SRC_BOOT;
            out.ev = BUDDY_BTN_LONG;
            return out;
        }
        return out;
    }

    s_boot_stable = s_boot_last;
    if (!s_boot_stable) {
        s_boot_pressed = true;
        s_boot_long_fired = false;
        s_boot_super_fired = false;
        s_boot_press_start = now_ms;
        return out;
    }

    s_boot_pressed = false;
    if (s_boot_long_fired || s_boot_super_fired) {
        return out;
    }
    out.src = BUDDY_BTN_SRC_BOOT;
    out.ev = BUDDY_BTN_SHORT;
    return out;
}

void buddy_button_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BOOT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    ESP_LOGI(TAG, "BOOT button on GPIO%d", BOOT_GPIO);
    /* I2C comes up with display/touch; defer AXP until first poll if needed. */
}

buddy_btn_t buddy_button_poll(uint32_t now_ms)
{
    if (!s_axp_ok) {
        static bool tried;
        if (!tried) {
            tried = true;
            axp_init();
        }
    }

    buddy_btn_t out = poll_pwr();
    if (out.ev == BUDDY_BTN_NONE) {
        out = poll_boot(now_ms);
    }
    if (out.ev != BUDDY_BTN_NONE) {
        const char *src = (out.src == BUDDY_BTN_SRC_BOOT) ? "BOOT"
                        : (out.src == BUDDY_BTN_SRC_PWR) ? "PWR" : "?";
        const char *ev = (out.ev == BUDDY_BTN_SHORT) ? "SHORT"
                       : (out.ev == BUDDY_BTN_LONG) ? "LONG"
                       : (out.ev == BUDDY_BTN_SUPER_LONG) ? "SUPER_LONG" : "?";
        ESP_LOGI(TAG, "btn src=%s ev=%s", src, ev);
    }
    return out;
}
