#include "buddy_power.h"
#ifdef ESP_PLATFORM
#include "buddy_button.h"
#endif
buddy_power_snapshot_t buddy_power_decode(uint8_t s1,uint8_t s2,uint8_t percent){
    bool present=(s1&8)!=0;
    return (buddy_power_snapshot_t){present,present&&percent<=100,present&&(s2>>5)==1,percent};
}
buddy_power_snapshot_t buddy_power_read(void){
#ifdef ESP_PLATFORM
    uint8_t a,b,p;
    if(buddy_button_pmu_read(0,&a)&&buddy_button_pmu_read(1,&b)&&buddy_button_pmu_read(0xa4,&p))
        return buddy_power_decode(a,b,p);
#endif
    return (buddy_power_snapshot_t){0};
}
