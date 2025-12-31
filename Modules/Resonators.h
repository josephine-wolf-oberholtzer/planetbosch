#include "../Common/DSP.h"
#include "../Common/EnvelopeFollower.h"
#include "../Common/Resonator.h"
#include "../Common/XFade.h"
#include "../Vendor/DaisySP/Source/Utility/dcblock.h"

namespace planetbosch {

static int CHANNEL_COUNT   = 2;
static int RESONATOR_COUNT = 4;
static int RESONATOR_LAST  = 4;

typedef struct {
  float color;
  float decay;
  float gain_1;
  float gain_2;
  float gain_3;
  float gain_4;
  float mix;
  float pitch_1;
  float pitch_2;
  float pitch_3;
  float pitch_4;
} ResonatorsParams;

class ResonatorsEngine {
public:
  ResonatorsEngine() {}
  ~ResonatorsEngine() {}

  void Init(float sample_rate) {
    dc_block_.Init(sample_rate);
    envelope_follower_.Init(sample_rate);
    xfade_.Init(XFADE_CPOW);
    for (int i = 0; i < RESONATOR_COUNT; i++) {
      resonators_[i].Init(sample_rate);
    }
  }

  float Process(float in) {
    float in_ = dc_block_.Process(in);
    float out = 0.f;
    for (int i = 0; i < RESONATOR_LAST; i++) {
      out += resonators_[i].Process(in_) / 4.f;
    }
    out = planetbosch::SoftClip(out);
    return xfade_.Process(in_, out);
  }

  void SetColor(float color) {
    for (int i = 0; i < RESONATOR_COUNT; i++) {
      resonators_[i].SetColor(color);
    }
  }

  void SetDecay(float decay) {
    for (int i = 0; i < RESONATOR_COUNT; i++) {
      resonators_[i].SetDecay(decay);
    }
  }

  void SetGain(float gain, int index) {
    float amplitude = DecibelsToAmplitude(gain);
    if (gain <= -72) {
      amplitude = 0;
    }
    resonators_[index].SetAmplitude(amplitude);
  }

  void SetMix(float mix) { xfade_.SetPos(mix); }

  void SetPitch(float pitch, int index) {
    resonators_[index].SetFrequency(PitchToFrequency(pitch));
  }

  void Update(ResonatorsParams *params) {
    SetColor(params->color);
    SetDecay(params->decay);
    SetGain(params->gain_1, 0);
    SetGain(params->gain_2, 1);
    SetGain(params->gain_3, 2);
    SetGain(params->gain_4, 3);
    SetMix(params->mix);
    SetPitch(params->pitch_1, 0);
    SetPitch(params->pitch_2, 1);
    SetPitch(params->pitch_3, 2);
    SetPitch(params->pitch_4, 3);
  }

private:
  daisysp::DcBlock              dc_block_;
  planetbosch::EnvelopeFollower envelope_follower_;
  planetbosch::Resonator        resonators_[4];
  planetbosch::XFade            xfade_;
};

class Resonators {
public:
  Resonators() {}
  ~Resonators() {}

  void Init(float sample_rate) {
    for (int i = 0; i < CHANNEL_COUNT; i++) {
      engine_[i].Init(sample_rate);
    }
  }

  void Process(FloatFrame *frame) {
    for (int i = 0; i < CHANNEL_COUNT; i++) {
      frame->out[i] = engine_[i].Process(frame->in[i]);
    }
  }

  void Update(ResonatorsParams *params) {
    for (int i = 0; i < CHANNEL_COUNT; i++) {
      engine_[i].Update(params);
    }
  }

private:
  planetbosch::ResonatorsEngine engine_[2];
};

} // namespace planetbosch
