#include <cmath>
#include "../../Modules/Resonators.h"
#include "../../Vendor/DaisySP/Source/Utility/dsp.h"
#include "../../Vendor/libDaisy/src/daisy_versio.h"

daisy::DaisyVersio hw;

planetbosch::FloatFrame* frame;
planetbosch::Resonators module;
planetbosch::ResonatorsParams* params;

void callback(
  daisy::AudioHandle::InterleavingInputBuffer  in,
  daisy::AudioHandle::InterleavingOutputBuffer out, 
  size_t                                       size
) {
  hw.ProcessAnalogControls();

  params->gain_1 = 0.f;
  params->gain_2 = 0.f;
  params->gain_3 = 0.f;
  params->gain_4 = 0.f;
  params->pitch_1 = round(daisysp::fmap(hw.GetKnobValue(daisy::DaisyVersio::KNOB_3), 24.f, 72.f));
  params->pitch_2 = params->pitch_1 + round(daisysp::fmap(hw.GetKnobValue(daisy::DaisyVersio::KNOB_1), -12.f, 12.f));
  params->pitch_3 = params->pitch_1 + round(daisysp::fmap(hw.GetKnobValue(daisy::DaisyVersio::KNOB_2), -12.f, 12.f));
  params->pitch_4 = params->pitch_1 + round(daisysp::fmap(hw.GetKnobValue(daisy::DaisyVersio::KNOB_5), -12.f, 12.f));
  params->color = hw.GetKnobValue(daisy::DaisyVersio::KNOB_6);
  params->decay_time = powf(hw.GetKnobValue(daisy::DaisyVersio::KNOB_4), 2.f) * 10.f;
  params->mix = hw.GetKnobValue(daisy::DaisyVersio::KNOB_0);

  module.Update(params);

  for (size_t i = 0; i < size; i += 2) {
    frame->in[0] = in[i];
    frame->in[1] = in[i + 1];
    module.Process(frame);
    out[i] = frame->out[0];
    out[i + 1] = frame->out[1];
  }
}

int main(void) {
  hw.Init();
  module.Init(hw.AudioSampleRate());
  hw.StartAudio(callback);
  hw.StartAdc();

  while (1) {
    hw.UpdateExample();
    hw.UpdateLeds();
  }
}
