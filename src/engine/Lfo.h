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

#ifndef OBXF_SRC_ENGINE_LFO_H
#define OBXF_SRC_ENGINE_LFO_H

#include <Constants.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthEngine.h"

class Lfo
{
  private:
    float phase;
    float sine, square, saw, tri, samplehold;
    float pw;
    float sum;
    juce::Random rnd;

    float SampleRate;
    float SampleRateInv;

    float syncRate;
    bool synced;

  public:
    float Frequency;
    float phaseInc;
    float frUnsc; // frequency value without sync
    float rawParam;
    int waveForm;

    Lfo()
    {
        phaseInc = 0.f;
        frUnsc = 0.f;
        syncRate = 1.f;
        rawParam = 0.f;
        synced = false;
        sum = 0.f;
        Frequency = 1.f;
        phase = 0.f;
        sine = square = saw = tri = samplehold = 0.f;
        pw = 0.f;
        rnd = juce::Random();
    }

    void setSynced()
    {
        synced = true;
        recalcRate(rawParam);
    }

    void setUnsynced()
    {
        synced = false;
        phaseInc = frUnsc;
    }

    void hostSyncRetrigger(float bpm, float quaters)
    {
        if (synced)
        {
            phaseInc = (bpm / 60.f) * syncRate;
            phase = phaseInc * quaters;
            phase = fmod(phase, 1.f) * twoPi - pi;
        }
    }

    inline float getVal()
    {
        float result = 0.f;

        if ((waveForm & 1) != 0)
            result += sine;

        if ((waveForm & 2) != 0)
            result += square;

        if ((waveForm & 4) != 0)
            result += samplehold;

        return tpt_lp_unwarped(sum, result, 3000.f, SampleRateInv);
    }

    void setSampleRate(float sr)
    {
        SampleRate = sr;
        SampleRateInv = 1.f / SampleRate;
    }

    inline void update()
    {
        phase += ((phaseInc * twoPi * SampleRateInv));

        sine = juce::dsp::FastMathApproximations::sin(phase);
        tri = (twoByPi * abs(phase + halfPi - (phase > halfPi) * twoPi)) - 1.f;
        saw = pi - phase;
        square = (phase > 0.f ? 1.f : -1.f);

        if (phase > pi)
        {
            phase -= twoPi;
            samplehold = rnd.nextFloat() * 2.f - 1.f;
        }
    }

    void setFrequency(float val)
    {
        frUnsc = val;

        if (!synced)
        {
            phaseInc = val;
        }
    }

    void setRawParam(float param) // used for synced rate changes
    {
        rawParam = param;

        if (synced)
        {
            recalcRate(param);
        }
    }

    void recalcRate(float param)
    {
        const int ratesCount = 9;
        int parval = (int)(param * (ratesCount - 1));
        float rt = 1.f;

        switch (parval)
        {
        case 0:
            rt = 1.f / 8.f;
            break;
        case 1:
            rt = 1.f / 4.f;
            break;
        case 2:
            rt = 1.f / 3.f;
            break;
        case 3:
            rt = 1.f / 2.f;
            break;
        case 4:
            rt = 1.f;
            break;
        case 5:
            rt = 3.f / 2.f;
            break;
        case 6:
            rt = 2.f;
            break;
        case 7:
            rt = 3.f;
            break;
        case 8:
            rt = 4.f;
            break;
        default:
            rt = 1.f;
            break;
        }

        syncRate = rt;
    }
};

#endif // OBXF_SRC_ENGINE_LFO_H
