#include "../Common/DSP.h"
#include "../Common/FrequencyShifter.h"
#include "../Common/XFade.h"
#include "../Vendor/DaisySP/Source/daisysp.h"

namespace planetbosch {

typedef struct {
  float brightness;
  float damping;
  float frequency_a;
  float frequency_b;
  float fx;
  float mix;
  float nonlinearity;
} HrongirParams;

template <size_t size> class Engine {
public:
  Engine() {}
  ~Engine() {}

  void Init(float sample_rate) {
    for (size_t i = 0; i < size; i++) {
      karplus_[i].Init(sample_rate);
    }
  }

  float Process(float value) {
    float out = 0.f;
    for (size_t i = 0; i < size; i++) {
      out += karplus_[i].Process(value);
    }
    return out / (float)size;
  }

  void SetFrequency(float frequency, size_t index) {
    karplus_[index % size].SetFreq(frequency);
  }

  void SetBrightness(float brightness) {
    for (size_t i = 0; i < size; i++) {
      karplus_[i].SetBrightness(brightness);
    }
  }

  void SetDamping(float damping) {
    for (size_t i = 0; i < size; i++) {
      karplus_[i].SetDamping(damping);
    }
  }

  void SetNonLinearity(float nonlinearity) {
    for (size_t i = 0; i < size; i++) {
      karplus_[i].SetNonLinearity(nonlinearity);
    }
  }

private:
  daisysp::String karplus_[size];
};

class Hrongir {
public:
  Hrongir() {}
  ~Hrongir() {}

  void Init(float sample_rate) {
    xfade_.Init(XFADE_CPOW);
    for (int i = 0; i < 4; i++) {
      engine_[i].Init(sample_rate);
    };
    for (int i = 0; i < 2; i++) {
      frequency_shifters_[i].Init(sample_rate);
    }
  }

  void Process(FloatFrame *frame) {
    for (int i = 0; i < 2; i++) {
      float in  = frame->in[i];
      float out = frequency_shifters_[i].Process(
          (engine_[i].Process(in) + engine_[i + 2].Process(in)) / 2.f
      );
      frame->out[i] = xfade_.Process(in, out);
    };
  }

  void SetFrequencyShifterFrequency(float value) {
    float base      = (value - 0.5) * 2.f;
    int   sign      = (0.f < base) - (base < 0.f);
    float frequency = 1000.f * (float)sign * powf(abs(base), 4.f);
    for (int i = 0; i < 2; i++) {
      frequency_shifters_[i].SetFrequency(frequency);
    }
  }

  void SetKarplusAFrequency(float frequency) {
    engine_[0].SetFrequency(frequency, 0);
    engine_[1].SetFrequency(frequency, 0);
  }

  void SetKarplusBFrequency(float frequency) {
    engine_[2].SetFrequency(frequency, 0);
    engine_[3].SetFrequency(frequency, 0);
  }

  void SetKarplusBrightness(float value) {
    for (int i = 0; i < 4; i++) {
      engine_[i].SetBrightness(value);
    }
  }

  void SetKarplusDamping(float value) {
    for (int i = 0; i < 4; i++) {
      engine_[i].SetDamping(powf(value, 0.25));
    }
  }

  void SetKarplusNonLinearity(float value) {
    for (int i = 0; i < 4; i++) {
      engine_[i].SetNonLinearity(powf(value, 0.25));
    }
  }

  void SetMix(float mix) { xfade_.SetPos(mix); }

  void Update(HrongirParams *params) {
    SetFrequencyShifterFrequency(params->fx);
    SetKarplusAFrequency(daisysp::mtof(params->frequency_a * 64.f));
    SetKarplusBFrequency(daisysp::mtof(params->frequency_b * 64.f));
    SetKarplusBrightness(params->brightness);
    SetKarplusDamping(params->damping);
    SetKarplusNonLinearity(params->nonlinearity);
    SetMix(params->mix);
  }

private:
  Engine<1>                     engine_[4];
  planetbosch::FrequencyShifter frequency_shifters_[2];
  planetbosch::XFade            xfade_;
};

} // namespace planetbosch
