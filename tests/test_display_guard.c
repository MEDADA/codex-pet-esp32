#include <assert.h>
#include <stdio.h>
#include "buddy_display_guard.h"
static int result;static uint32_t timeout;
int bsp_display_lock(uint32_t milliseconds){timeout=milliseconds;return result;}
int main(void){
 result=ESP_OK;assert(buddy_display_try_lock(50));assert(timeout==50);
 result=0x107;assert(!buddy_display_try_lock(50));
 result=-1;assert(!buddy_display_try_lock(0));assert(timeout==0);
 puts("PASS real BSP esp_err_t lock convention: only ESP_OK permits UI access");
}
