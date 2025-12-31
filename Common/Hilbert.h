#ifndef PLANETBOSCH_HILBERT
#define PLANETBOSCH_HILBERT

#include <math.h>

namespace planetbosch {

class Hilbert {
public:
  Hilbert() {}
  ~Hilbert() {}

  void Init(float sample_rate) {
    /* pole values taken from Bernie Hutchins, "Musical Engineer's Handbook" */
    double poles[12] = {
        0.3609,
        2.7412,
        11.1573,
        44.7581,
        179.6242,
        798.4578,
        1.2524,
        5.5671,
        22.3423,
        89.6271,
        364.7914,
        2770.1114,
    };
    double polefreq, rc, alpha, beta;
    /* calculate coefficients for allpass filters, based on sampling rate */
    for (int j = 0; j < 12; j++) {
      /*      p->coef[j] = (1 - (15 * PI * pole[j]) / sr) /
              (1 + (15 * PI * pole[j]) / sr); */
      polefreq = poles[j] * 15.0;
      rc       = 1.0 / (2.0 * PI_F * polefreq);
      alpha    = 1.0 / rc;
      alpha    = alpha * 0.5 * (1.0f / sample_rate);
      beta     = (1.0 - alpha) / (1.0 + alpha);
      xnm1_[j] = ynm1_[j] = 0.0f;
      coef_[j]            = -(float)beta;
    }
  }

  void Process(float in, float *out) {
    float xn1, yn1, xn2, yn2;
    xn1 = in;
    // 6th order allpass filter
    for (int j = 0; j < 6; j++) {
      yn1      = coef_[j] * (xn1 - ynm1_[j]) + xnm1_[j];
      xnm1_[j] = xn1;
      ynm1_[j] = yn1;
      xn1      = yn1;
    }
    xn2 = in;
    // 6th order allpass filter
    for (int j = 6; j < 12; j++) {
      yn2      = coef_[j] * (xn2 - ynm1_[j]) + xnm1_[j];
      xnm1_[j] = xn2;
      ynm1_[j] = yn2;
      xn2      = yn2;
    }
    out[0] = yn2;
    out[1] = yn1;
  }

private:
  float coef_[12];
  float xnm1_[12];
  float ynm1_[12];
};

} // namespace planetbosch

#endif
