#pragma once
// Hardware-agnostic, deterministic visual motion mapper (no I2C). Intended as
// a starting point for Codex integration after calibrated roll/pitch estimation.
#include <cmath>
#include <algorithm>
#include <cstdint>
namespace pet_motion {
struct OrientationInput {float roll_deg, pitch_deg, acc_norm_g, gyro_dps;};
struct Expression {float gaze_x_px, gaze_y_px, head_roll_deg; bool surprise;};
class ImuExpressionMapper {
public:
  void calibrate(float roll, float pitch) {neutral_roll_=roll; neutral_pitch_=pitch;gx_=gy_=head_=0;}
  Expression step(const OrientationInput& in, float dt_sec, uint32_t now_ms) {
    const auto sat = [](float v,float lo,float hi){return std::max(lo,std::min(v,hi));};
    auto dead = [](float x,float d){return x>d?x-d:x< -d?x+d:0.f;};
    dt_sec=sat(dt_sec,0.001f,0.12f);
    float r=sat(dead(in.roll_deg-neutral_roll_,3.5f),-16.5f,16.5f);
    float p=sat(dead(in.pitch_deg-neutral_pitch_,3.5f),-16.5f,16.5f);
    float tx=sat(r/16.5f*3.0f,-3.0f,3.0f), ty=sat(-p/16.5f*2.0f,-2.0f,2.0f);
    float th=sat(-r/16.5f*4.0f,-4.0f,4.0f);
    const float smooth=1.0f-std::exp(-dt_sec/0.12f);
    gx_+=(tx-gx_)*smooth; gy_+=(ty-gy_)*smooth; head_+=(th-head_)*smooth;
    bool shake = (std::fabs(in.acc_norm_g-1.0f)>0.38f &&
                  std::fabs(in.gyro_dps)>80.0f &&
                  (!has_shaken_ || uint32_t(now_ms-last_shake_ms_)>=2000U));
    if(shake){last_shake_ms_=now_ms;has_shaken_=true;}
    return {gx_,gy_,head_,shake};
  }
private:
  float neutral_roll_=0,neutral_pitch_=0, gx_=0,gy_=0,head_=0;
  uint32_t last_shake_ms_=0; bool has_shaken_=false;
};
} // namespace pet_motion
