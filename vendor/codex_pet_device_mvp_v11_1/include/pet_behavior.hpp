#pragma once
// No Arduino, LVGL, or I2C dependencies. Keep this logic in the existing UI task.
#include <cmath>
#include <cstdint>
#include "imu_motion_mapper.hpp"

namespace pet_mvp {
enum class Mode : uint8_t { Idle, Thinking, Coding, Success, Error };
struct Frame {
  Mode mode;
  bool connected;
  bool blink;
  bool surprise;
  float gaze_x;      // 240-canvas pixel units, advisory until iris artwork exists
  float gaze_y;
  float head_tilt_deg; // advisory; avoid large software rotation on first firmware
  float led_intensity;
  float body_y;       // one-pixel breathing only, only if enabled
  bool question;
};
class Behavior {
public:
  void connect(bool value) { connected_ = value; }
  void set_mode(Mode m, uint32_t now_ms) {
    if (m != mode_) { mode_ = m; mode_since_ = now_ms; }
  }
  Mode mode() const { return mode_; }
  void calibrate(float roll, float pitch) { motion_.calibrate(roll,pitch); imu_ready_=true; }
  void set_imu_ready(bool ready) {imu_ready_=ready;}
  // Call on each valid calibrated IMU update. Pass a combined gyro magnitude in dps.
  void sense(float roll, float pitch, float acc_g, float gyro_dps, float dt_sec, uint32_t now_ms) {
    if (!imu_ready_ || !std::isfinite(roll) || !std::isfinite(pitch) ||
        !std::isfinite(acc_g) || !std::isfinite(gyro_dps))return;
    last_expr_ = motion_.step({roll,pitch,acc_g,gyro_dps},dt_sec,now_ms);
    if (last_expr_.surprise) surprise_until_ = now_ms + 420;
  }
  Frame at(uint32_t now_ms) {
    if (next_blink_ms_ == 0) next_blink_ms_ = now_ms + 4200;
    // Time arithmetic is safe for intervals below 2^31 ms.
    if (!is_blinking_ && static_cast<int32_t>(now_ms - next_blink_ms_) >= 0) {
      is_blinking_=true;
      blink_end_ms_=now_ms+120;
    }
    if (is_blinking_ && static_cast<int32_t>(now_ms - blink_end_ms_) >= 0) {
      is_blinking_=false;
      next_blink_ms_=now_ms+3300+(random_()%3500);
    }
    const float t=static_cast<float>(now_ms - mode_since_) / 1000.f;
    const float start = smooth(t/0.75f);
    const float thinking_tilt = (mode_==Mode::Thinking ? -5.f * start : 0.f);
    const float eye_weight = mode_==Mode::Idle?1.f:0.3f;
    const float led=.53f + .47f * (0.5f+0.5f*std::sin(now_ms / 450.f));
    return {mode_,connected_,is_blinking_,
       static_cast<int32_t>(surprise_until_-now_ms)>0,
       imu_ready_?last_expr_.gaze_x_px*eye_weight:0.f,
       imu_ready_?last_expr_.gaze_y_px*eye_weight:0.f,
       thinking_tilt+(imu_ready_?last_expr_.head_roll_deg*eye_weight:0.f),
       led, (mode_==Mode::Idle ? 0.7f*std::sin(now_ms/1300.f):0.f),
       mode_==Mode::Thinking};
  }
private:
  static float smooth(float x){if(x<0)return 0;if(x>1)return 1;return x*x*(3.f-2.f*x);}
  uint32_t random_(){rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;return rng_;}
  Mode mode_=Mode::Idle;
  bool connected_=false,imu_ready_=false,is_blinking_=false;
  uint32_t mode_since_=0,next_blink_ms_=0,blink_end_ms_=0,surprise_until_=0;
  uint32_t rng_=0x9e3779b9u;
  pet_motion::ImuExpressionMapper motion_;
  pet_motion::Expression last_expr_{};
};
} // namespace pet_mvp
