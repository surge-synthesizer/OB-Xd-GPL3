/*
 * OB-Xf - a continuation of the last open source version of OB-Xd.
 *
 * OB-Xd was originally written by Vadim Filatov, and then a version
 * was released under the GPL3 at https://github.com/reales/OB-Xd.
 * Subsequently, the product was continued by DiscoDSP and the copyright
 * holders as an excellent closed source product. For more info,
 * see "HISTORY.md" in the root of this repository.
 *
 * This repository is a successor to the last open source release,
 * a version marked as 2.11. Copyright 2013-2025 by the authors
 * as indicated in the original release, and subsequent authors
 * per the GitHub transaction log.
 *
 * OB-Xf is released under the GNU General Public Licence v3 or later
 * (GPL-3.0-or-later). The license is found in the file "LICENSE"
 * in the root of this repository or at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Source code is available at https://github.com/surge-synthesizer/OB-Xf
 */

#ifndef OBXF_SRC_ENGINE_FILTER_H
#define OBXF_SRC_ENGINE_FILTER_H

#include "ObxfVoice.h"
#include <math.h>

class Filter
{
  private:
    float s1, s2, s3, s4;
    float R;
    float R24;
    float rcor, rcorInv;
    float rcor24, rcor24Inv;

    // 24 db multimode
    float mmt;
    int mmch;

  public:
    float SampleRate;
    float sampleRateInv;
    bool bandPassSw;
    float mm;
    bool selfOscPush;

    Filter()
    {
        selfOscPush = false;
        bandPassSw = false;
        mm = 0.f;
        s1 = s2 = s3 = s4 = 0.f;
        SampleRate = 44000.f;
        sampleRateInv = 1.f / SampleRate;
        rcor = 500.f / 44000.f;
        rcorInv = 1.f / rcor;
        rcor24 = 970.f / 44000.f;
        rcor24Inv = 1.f / rcor24;
        R = 1.f;
        R24 = 0.f;
    }

    void setMultimode(float m)
    {
        mm = m;
        mmch = (int)(mm * 3);
        mmt = mm * 3 - mmch;
    }

    inline void setSampleRate(float sr)
    {
        SampleRate = sr;
        sampleRateInv = 1.f / SampleRate;

        float rcrate = sqrt((44000.f / SampleRate));

        rcor = (500.f / 44000.f) * rcrate;
        rcor24 = (970.f / 44000.f) * rcrate;
        rcorInv = 1.f / rcor;
        rcor24Inv = 1.f / rcor24;
    }

    inline void setResonance(float res)
    {
        R = 1.f - res;
        R24 = (3.5f * res);
    }

    inline float diodePairResistanceApprox(float x)
    {
        // Taylor approx of slightly mismatched diode pair
        return (((((0.0103592f) * x + 0.00920833f) * x + 0.185f) * x + 0.05f) * x + 1.f);
    }

    // resolve 0-delay feedback
    inline float NR(float sample, float g)
    {
        // calculating feedback non-linear transconducance and compensated for R(-1)
        float tCfb;

        if (!selfOscPush)
            tCfb = diodePairResistanceApprox(s1 * 0.0876f) - 1.f;
        else
            // boosting non-linearity
            tCfb = diodePairResistanceApprox(s1 * 0.0876f) - 1.035f;

        // disable non-linearity == digital filter
        // float tCfb = 0;

        // resolve linear feedback
        float y =
            ((sample - 2.f * (s1 * (R + tCfb)) - g * s1 - s2) / (1.f + g * (2.f * (R + tCfb) + g)));

        // float y = ((sample - 2.f * (s1 * (R + tCfb)) - g2 * s1  - s2) / (1.f + g1 * (2.f * (R +
        // tCfb) + g2)));

        return y;
    }

    inline float Apply2Pole(float sample, float g)
    {

        float gpw = tanf(g * sampleRateInv * juce::MathConstants<float>::pi);

        g = gpw;

        // float v = ((sample - R * s1 * 2 - g2 * s1 - s2) / (1.f + R * g1 * 2.f + g1 * g2));
        float v = NR(sample, g);

        float y1 = v * g + s1;
        s1 = v * g + y1;

        float y2 = y1 * g + s2;
        s2 = y1 * g + y2;

        float mc;

        if (!bandPassSw)
        {
            mc = (1.f - mm) * y2 + (mm)*v;
        }
        else
        {
            mc = 2.f *
                 (mm < 0.5f ? ((0.5f - mm) * y2 + (mm)*y1) : ((1.f - mm) * y1 + (mm - 0.5f) * v));
        }

        return mc;
    }

    inline float NR24(float sample, float g, float lpc)
    {
        float ml = 1.f / (1.f + g);
        float S = (lpc * (lpc * (lpc * s1 + s2) + s3) + s4) * ml;
        float G = lpc * lpc * lpc * lpc;
        float y = (sample - R24 * S) / (1.f + R24 * G);

        return y;
    }

    inline float Apply4Pole(float sample, float g)
    {
        float g1 = (float)tan(g * sampleRateInv * juce::MathConstants<float>::pi);
        g = g1;

        float lpc = g / (1.f + g);
        float y0 = NR24(sample, g, lpc);

        // first low pass in cascade
        double v = (y0 - s1) * lpc;
        double res = v + s1;

        s1 = res + v;

        // damping
        s1 = atan(s1 * rcor24) * rcor24Inv;

        float y1 = res;
        float y2 = tpt_process(s2, y1, g);
        float y3 = tpt_process(s3, y2, g);
        float y4 = tpt_process(s4, y3, g);
        float mc;

        switch (mmch)
        {
        case 0:
            mc = ((1.f - mmt) * y4 + (mmt)*y3);
            break;
        case 1:
            mc = ((1.f - mmt) * y3 + (mmt)*y2);
            break;
        case 2:
            mc = ((1.f - mmt) * y2 + (mmt)*y1);
            break;
        case 3:
            mc = y1;
            break;
        default:
            mc = 0.f;
            break;
        }

        // half volume comp
        return mc * (1.f + R24 * 0.45f);
    }
};

#endif // OBXF_SRC_ENGINE_FILTER_H
