#pragma once
#include <lvgl.h>
#include "pet_behavior.hpp"
namespace pet_view {
  // Call all methods from the existing LVGL/UI task (or with its GUI mutex held).
  void create(lv_obj_t* parent = nullptr);
  void destroy();
  void set_time(uint8_t hour, uint8_t minute, uint8_t month, uint8_t day, bool valid);
  void show(const pet_mvp::Frame& frame);
}
