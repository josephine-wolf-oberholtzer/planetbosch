#pragma once
#ifndef PLANETBOSCH_RESONATOR
#define PLANETBOSCH_RESONATOR

#include "../Vendor/DaisySP/Source/Filters/atone.h"
#include "../Vendor/DaisySP/Source/Filters/tone.h"
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
    delay_lag_.Init(sample_rate_, 0.01f);
    high_pass_filter_.Init(sample_rate_);
    low_pass_filter_.Init(sample_rate_);
  }

  float Process(float in) {
    // Read the delay line
    float delay_output =
        delay_line_.ReadHermite(delay_lag_.Process(target_delay_));
    // Average with previous delay output
    float filtered_output  = 0.5f * (delay_output + previous_delay_output_);
    previous_delay_output_ = delay_output;
    // Apply feedback
    delay_output = filtered_output * feedback_;
    // Apply filters
    delay_output = low_pass_filter_.Process(delay_output);
    delay_output = high_pass_filter_.Process(delay_output);
    // Write back to the delay line
    delay_line_.Write(in + delay_output);
    return delay_output * amplitude_lag_.Process(target_amplitude_);
  }

  inline void SetAmplitude(float amplitude) {
    target_amplitude_ = daisysp::fclamp(amplitude, 0.0001f, 1.f);
  }

  inline void SetColor(float color) {
    float color_frequency =
        powf(1000.f, 2.f * daisysp::fclamp(color, 0.f, 1.f) - 1.f);
    // 20Hz (-1) to 20,000Hz (0) 20,000,000Hz (1)
    float low_pass_frequency =
        daisysp::fclamp(20000.f * color_frequency, 20.f, sample_rate_ / 3.f);
    // 0.002Hz (-1) to 2Hz (0) to 2,000Hz (1)
    float high_pass_frequency =
        daisysp::fclamp(2.f * color_frequency, 20.f, sample_rate_ / 3.f);
    low_pass_filter_.SetFreq(low_pass_frequency);
    high_pass_filter_.SetFreq(high_pass_frequency);
  }

  inline void SetDecay(float decay) {
    feedback_ = powf(daisysp::fclamp(decay, 0.f, 1.f), 0.2f) * 0.299f + 0.7f;
  }

  inline void SetFrequency(float frequency) {
    target_delay_ = (1.f / frequency) * sample_rate_;
  }

private:
  daisysp::ATone                  high_pass_filter_;
  daisysp::DelayLine<float, 4800> delay_line_;
  daisysp::Port                   amplitude_lag_;
  daisysp::Port                   delay_lag_;
  daisysp::Tone                   low_pass_filter_;
  float                           decay_;
  float                           feedback_;
  float                           previous_delay_output_;
  float                           sample_rate_;
  float                           target_amplitude_;
  float                           target_delay_;
};

} // namespace planetbosch

#endif
