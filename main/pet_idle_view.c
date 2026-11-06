#include "pet_idle_view.h"
#include "pet_frame_layout.h"
#include "pet_motion_clip.h"
#include "sdkconfig.h"

extern const uint32_t frame_offsets[PET_IDLE_FRAME_COUNT + 1] asm("_binary_pet_idle_v3_frames_bin_start");

static lv_obj_t *avatar;
static lv_image_dsc_t still;

const uint8_t *pet_idle_frame_data(unsigned index, size_t *size)
{
    *size = frame_offsets[index + 1] - frame_offsets[index];
    return (const uint8_t *)(frame_offsets + PET_IDLE_FRAME_COUNT + 1) + frame_offsets[index];
}

void pet_idle_view_init(lv_obj_t *image)
{
    avatar = image;
    size_t size;
    const uint8_t *pixels = pet_idle_frame_data(0, &size);
    still = (lv_image_dsc_t){
        .header = {.magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
                   .w = PET_IDLE_W, .h = PET_IDLE_H, .stride = PET_IDLE_W * 2},
        .data_size = size,
        .data = pixels,
    };
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
    pet_motion_clip_init(avatar);
#endif
}

void pet_idle_view_show_still(void)
{
    lv_image_set_src(avatar, &still);
}

void pet_idle_view_set_active(bool active)
{
    if (active) {
        pet_idle_view_show_still();
#ifdef CONFIG_BUDDY_PET_V11_ANIMATION
        pet_motion_clip_begin(PET_CLIP_IDLE);
#endif
    } else {
        pet_motion_clip_pause_idle();
    }
}
