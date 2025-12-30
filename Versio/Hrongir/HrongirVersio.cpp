#include "../../Common/DSP.h"
#include "../../Modules/Hrongir.h"
#include "../../Vendor/libDaisy/src/daisy_versio.h"

daisy::DaisyVersio hw;

planetbosch::FloatFrame* frame;
planetbosch::Hrongir hrongir;
planetbosch::HrongirParams* params;

void callback(
  daisy::AudioHandle::InterleavingInputBuffer  in,
  daisy::AudioHandle::InterleavingOutputBuffer out, 
  size_t                                       size
) {
  hw.ProcessAnalogControls();

  params->mix = hw.GetKnobValue(daisy::DaisyVersio::KNOB_0);
  params->frequency_a = hw.GetKnobValue(daisy::DaisyVersio::KNOB_1);
  params->brightness = hw.GetKnobValue(daisy::DaisyVersio::KNOB_2);
  params->nonlinearity = hw.GetKnobValue(daisy::DaisyVersio::KNOB_3);
  params->damping = hw.GetKnobValue(daisy::DaisyVersio::KNOB_4);
  params->frequency_b = hw.GetKnobValue(daisy::DaisyVersio::KNOB_5);
  params->fx = hw.GetKnobValue(daisy::DaisyVersio::KNOB_6);
  hrongir.Update(params);

  for (size_t i = 0; i < size; i += 2) {
    frame->in[0] = in[i];
    frame->in[1] = in[i + 1];
    hrongir.Process(frame);
    out[i] = frame->out[0];
    out[i + 1] = frame->out[1];
  }
}

int main(void) {
  hw.Init();
  hrongir.Init(hw.AudioSampleRate());
  hw.StartAudio(callback);
  hw.StartAdc();

  while (1) {
    hw.UpdateExample();
    hw.UpdateLeds();
  }
}
