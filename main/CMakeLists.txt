if(CONFIG_BUDDY_PET_V11)
    set(BUDDY_UI_SOURCES "buddy_display_buffer.c" "buddy_pet_ui.c" "pet_view_lvgl9.c" "pet_idle_view.c" "pet_motion_clip.c" "pet_presentation.c" "pet_animation.c" "pet_motion.cpp")
else()
    set(BUDDY_UI_SOURCES "buddy_companion_ui.c" "generated/rem_assets.c" "rem_scene.c" "rem_bg_cache.c")
endif()

# Voice bridge, Wi-Fi provisioning and the speaker path are Rem-companion
# features. The pet stage never calls them; compiling them in only costs
# flash and internal RAM. See CONFIG_BUDDY_VOICE_BRIDGE.
if(CONFIG_BUDDY_VOICE_BRIDGE)
    set(BUDDY_NET_SOURCES "buddy_voice.c" "buddy_network.c")
else()
    set(BUDDY_NET_SOURCES "")
endif()
idf_component_register(
    SRCS
        "main.c"
        "buddy_ble.c"
        "buddy_state_evidence.c"
        ${BUDDY_UI_SOURCES}
        "buddy_scene.c"
        "buddy_requests.c"
        "buddy_power.c"
        "buddy_motion.c"
        "buddy_motion_hw.c"
        ${BUDDY_NET_SOURCES}
        "buddy_button.c"
        "buddy_rtc.c"
        "buddy_settings.c"
        "fonts/font_buddy_cn_16.c"
        "fonts/font_buddy_cn_20.c"
        "fonts/font_buddy_clock_64.c"
        "fonts/buddy_dynamic_fonts.c"
    INCLUDE_DIRS "." "generated" "fonts" "../vendor/codex_pet_device_mvp_v11_1/include"
)
spiffs_create_partition_image(storage ../assets/fontfs FLASH_IN_PROJECT)


if(CONFIG_BUDDY_PET_V11)
    target_link_options(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_lv_adapter_register_display" "-Wl,-u,__wrap_esp_lv_adapter_register_display")
    target_add_binary_data(${COMPONENT_LIB} "generated/pet_idle_v3_frames.bin" BINARY)
    target_add_binary_data(${COMPONENT_LIB} "generated/pet_v41_layers.bin" BINARY)
endif()
