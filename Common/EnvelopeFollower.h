#include <cmath>

#ifndef PLANETBOSCH_ENVELOPE_FOLLOWER
#define PLANETBOSCH_ENVELOPE_FOLLOWER

namespace planetbosch {

class EnvelopeFollower {
public:
  EnvelopeFollower() {}
  ~EnvelopeFollower() {}

  inline void Init(float sample_rate) {
    previous_in_ = 0.f;
    rising_      = false;
    sample_rate_ = sample_rate;
    slewed_out_  = 0.f;
    SetAttackTime(0.01f);
    SetDecayTime(0.01f);
  }

  float Process(float in) {
    float previous_in = previous_in_;
    previous_in_      = fabsf(in);
    if (in > previous_in) {
      rising_ = true;
    } else if (in < previous_in) {
      rising_ = false;
    }
    if (rising_) {
      slewed_out_ += (in - slewed_out_) * attack_time_;
    } else {
      slewed_out_ += (in - slewed_out_) * decay_time_;
    }
    return slewed_out_;
  }

  void SetAttackTime(float attack_time) {
    attack_time_ = 1.f / attack_time / sample_rate_;
  }

  void SetDecayTime(float decay_time) {
    decay_time_ = 1.f / decay_time / sample_rate_;
  }

private:
  bool  rising_;
  float attack_time_;
  float decay_time_;
  float previous_in_;
  float sample_rate_;
  float slewed_out_;
};

} // namespace planetbosch

#endif
