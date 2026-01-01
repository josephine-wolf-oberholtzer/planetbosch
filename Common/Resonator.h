#pragma once
#ifndef PLANETBOSCH_RESONATOR
#define PLANETBOSCH_RESONATOR

#include "../Vendor/DaisySP/Source/Filters/svf.h"
#include "../Vendor/DaisySP/Source/Utility/delayline.h"
#include "../Vendor/DaisySP/Source/Utility/dsp.h"
#include "../Vendor/DaisySP/Source/Utility/port.h"

namespace planetbosch {

class Resonator {
public:
  Resonator() {}
  ~Resonator() {}

  inline void Init(float sample_rate) {
    sample_rate_ = sample_rate;
    amplitude_lag_.Init(sample_rate_, 0.01f);
    delay_time_lag_.Init(sample_rate_, 0.01f);
    high_pass_filter_.Init(sample_rate_);
    high_pass_filter_.SetDrive(0.f);
    high_pass_filter_.SetRes(0.f);
    low_pass_filter_.Init(sample_rate_);
    low_pass_filter_.SetDrive(0.f);
    low_pass_filter_.SetRes(0.f);
  }

  float Process(float in) {
    amplitude_ = amplitude_lag_.Process(amplitude_target_);
    delay_time_ = delay_time_lag_.Process(delay_time_target_);
    feedback_ = powf(decay_time_, 0.1f)  * 0.2999f + 0.7f;
    // Read the delay line
    float delay_output = delay_line_.ReadHermite(delay_time_);
    // Average with previous delay output
    float filtered_output  = 0.5f * (delay_output + previous_delay_output_);
    previous_delay_output_ = delay_output;
    // Apply feedback
    delay_output = filtered_output * feedback_;
    // Apply filters
    low_pass_filter_.Process(delay_output);
    delay_output = low_pass_filter_.Low();
    high_pass_filter_.Process(delay_output);
    delay_output = high_pass_filter_.High();
    // Write back to the delay line
    delay_line_.Write(in + delay_output);
    return delay_output * amplitude_;
  }

  inline void SetAmplitude(float amplitude) {
    amplitude_target_ = daisysp::fclamp(amplitude, 0.0001f, 1.f);
  }

  inline void SetColor(float color) {
    float factor = 1000.f;
    float max_frequency = sample_rate_ / 3.f;
    float min_frequency = max_frequency / factor;
    float color_coef =
        powf(factor, 2.f * daisysp::fclamp(color, 0.f, 1.f) - 1.f);
    // 16Hz (-1) to 16kHz (0)
    float low_pass_frequency =
        daisysp::fclamp(max_frequency * color_coef, min_frequency, max_frequency);
    // 1.6Hz (0) to 1.6kHz (1)
    float high_pass_frequency =
        daisysp::fclamp(min_frequency * 0.1 * color_coef, min_frequency, max_frequency);
    low_pass_filter_.SetFreq(low_pass_frequency);
    high_pass_filter_.SetFreq(high_pass_frequency);
  }

  inline void SetDecayTime(float decay_time) {
    decay_time_ = daisysp::fclamp(decay_time, 0.f, 1.f);
  }

  inline void SetDelayTime(float delay_time) {
    delay_time_target_ = delay_time * sample_rate_;
  }

  inline void SetFrequency(float frequency) {
    SetDelayTime(1.f / frequency);
  }

private:
  daisysp::DelayLine<float, 4800> delay_line_;
  daisysp::Port                   amplitude_lag_;
  daisysp::Port                   delay_time_lag_;
  daisysp::Svf                    low_pass_filter_;
  daisysp::Svf                    high_pass_filter_;
  float                           amplitude_;
  float                           amplitude_target_;
  float                           decay_time_;
  float                           delay_time_;
  float                           delay_time_target_;
  float                           feedback_;
  float                           previous_delay_output_;
  float                           sample_rate_;
};

} // namespace planetbosch

#endif
