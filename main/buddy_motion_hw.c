#include "buddy_motion.h"
#include "buddy_imu6.h"
#include "sdkconfig.h"
#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
static i2c_master_dev_handle_t imu;
static buddy_motion_filter_t filter;
static bool read_reg(uint8_t reg,uint8_t *out,size_t n){return imu&&i2c_master_transmit_receive(imu,&reg,1,out,n,20)==ESP_OK;}
static bool write_reg(uint8_t r,uint8_t v){uint8_t b[]={r,v};return i2c_master_transmit(imu,b,2,20)==ESP_OK;}
void buddy_motion_init(void){
    i2c_master_bus_handle_t bus=bsp_i2c_get_handle();if(!bus)return;
    for(uint8_t addr=0x6a;addr<=0x6b;addr++){
        i2c_device_config_t c={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=addr,.scl_speed_hz=100000};
        if(i2c_master_bus_add_device(bus,&c,&imu)!=ESP_OK)continue;
        uint8_t id=0;if(read_reg(0,&id,1)&&id==5){
            /* QMI8658: address increment, 8g / 125Hz accelerometer, accel enable. */
            uint8_t enable=1;
#ifdef CONFIG_BUDDY_PET_V11_IMU
            enable=3;
#endif
            if(write_reg(8,0)&&write_reg(2,0x40)&&write_reg(3,0x26)&&write_reg(4,0x56)&&write_reg(6,0x11)&&write_reg(8,enable)){ESP_LOGI("motion","QMI8658 ready at 0x%02x (acc8g gyro512dps enable=%u)",addr,enable);return;}
        }
        i2c_master_bus_rm_device(imu);imu=NULL;
    }
    ESP_LOGW("motion","QMI8658 unavailable; tilt disabled");
}
bool buddy_motion_poll(uint32_t now,buddy_motion_output_t *o){
    uint8_t b[6];if(!read_reg(0x35,b,6))return false;
    buddy_imu_sample_t s={(int16_t)(b[0]|b[1]<<8)/4096.f,(int16_t)(b[2]|b[3]<<8)/4096.f,(int16_t)(b[4]|b[5]<<8)/4096.f};
    buddy_motion_update(&filter,&s,now,o);return true;
}

bool buddy_imu6_read(pet_imu_sample_t *sample){
    uint8_t status,b[12];
    if(!read_reg(0x2e,&status,1)||(status&3)!=3||!read_reg(0x35,b,sizeof(b)))return false;
    *sample=buddy_imu6_decode(b);return true;
}
