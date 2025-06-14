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

#ifndef OBXF_SRC_ENGINE_ADSRENVELOPE_H
#define OBXF_SRC_ENGINE_ADSRENVELOPE_H

#include "ObxfVoice.h"

class AdsrEnvelope
{
  private:
    enum State
    {
        Attack = 1,
        Decay = 2,
        Sustain = 3,
        Release = 4,
        Silent = 5
    };

    float Value;
    float attack, decay, sustain, release;
    float ua, ud, us, ur;
    float coef;
    float uf;
    float SampleRate;
    State state;

  public:
    AdsrEnvelope()
    {
        Value = 0.0;
        attack = decay = sustain = release = 0.0001f;
        ua = ud = us = ur = 0.0001f;
        coef = 0.f;
        uf = 1;
        SampleRate = 44000.f;
        state = State::Attack;
    }

    void ResetEnvelopeState()
    {
        Value = 0.0f;
        state = State::Silent;
    }

    void setSampleRate(float sr) { SampleRate = sr; }

    void setUniqueOffset(float der)
    {
        uf = der;
        setAttack(ua);
        setDecay(ud);
        setSustain(us);
        setRelease(ur);
    }

    void setAttack(float atk)
    {
        ua = atk;
        attack = atk * uf;
        if (state == State::Attack)
            coef = (float)((log(0.001) - log(1.3)) / (SampleRate * (atk) / 1000.f));
    }

    void setDecay(float dec)
    {
        ud = dec;
        decay = dec * uf;
        if (state == State::Decay)
            coef = (float)((log(juce::jmin(sustain + 0.0001, 0.99)) - log(1.0)) /
                           (SampleRate * (dec) / 1000.f));
    }

    void setSustain(float sust)
    {
        us = sust;
        sustain = sust;
        if (state == State::Decay)
            coef = (float)((log(juce::jmin(sustain + 0.0001, 0.99)) - log(1.0)) /
                           (SampleRate * (decay) / 1000.f));
    }

    void setRelease(float rel)
    {
        ur = rel;
        release = rel * uf;
        if (state == State::Release)
            coef = (float)((log(0.00001) - log(Value + 0.0001)) / (SampleRate * (rel) / 1000.f));
    }

    void triggerAttack()
    {
        state = State::Attack;
        coef = (float)((log(0.001) - log(1.3)) / (SampleRate * (attack) / 1000.f));
    }

    void triggerRelease()
    {
        if (state != State::Release)
            coef =
                (float)((log(0.00001) - log(Value + 0.0001)) / (SampleRate * (release) / 1000.f));
        state = State::Release;
    }

    inline bool isActive() { return state != State::Silent; }

    inline float processSample()
    {
        switch (state)
        {
        case State::Attack:
            if (Value - 1.f > -0.1f)
            {
                Value = juce::jmin(Value, 0.99f);
                state = State::Decay;
                coef = (float)((log(juce::jmin(sustain + 0.0001, 0.99)) - log(1.0)) /
                               (SampleRate * (decay) / 1000.f));
                goto dec;
            }
            else
            {
                Value = Value - (1.f - Value) * (coef);
            }
            break;
        case State::Decay:
        dec:
            if (Value - sustain < 10e-6f)
            {
                state = State::Sustain;
            }
            else
            {
                Value = Value + Value * coef;
            }
            break;
        case State::Sustain:
            Value = juce::jmin(sustain, 0.9f);
            break;
        case State::Release:
            if (Value > 20e-6f)
                Value = Value + Value * coef + dc;
            else
                state = State::Silent;
            break;
        case State::Silent:
            Value = 0.f;
            break;
        }

        return Value;
    }
};

#endif // OBXF_SRC_ENGINE_ADSRENVELOPE_H
