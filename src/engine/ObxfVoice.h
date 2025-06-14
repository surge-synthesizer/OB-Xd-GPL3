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

#ifndef OBXF_SRC_ENGINE_OBXDVOICE_H
#define OBXF_SRC_ENGINE_OBXDVOICE_H

#include "ObxfOscillatorB.h"
#include "AdsrEnvelope.h"
#include "Filter.h"
#include "Decimator.h"
#include "APInterpolator.h"
#include "Tuning.h"

class ObxfVoice
{
  private:
    float SampleRate;
    float sampleRateInv;
    // float Volume;
    // float port;
    float velocityValue;

    float d1, d2;
    float c1, c2;

    Tuning *tuning;

    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ObxfVoice)

  public:
    bool sustainHold;
    // bool resetAdsrsOnAttack;

    AdsrEnvelope env;
    AdsrEnvelope fenv;
    ObxfOscillatorB osc;
    Filter flt;

    juce::Random ng;

    float vamp, vflt;

    float cutoff;
    float fenvamt;

    float EnvSlop;
    float FenvSlop;

    float FltSlop;
    float FltSlopAmt;

    float PortaSlop;
    float PortaSlopAmt;

    float levelSlop;
    float levelSlopAmt;

    float brightCoef;

    int midiIndx;

    bool Active;
    bool shouldProcessed;

    float fltKF;

    float porta;
    float prtst;

    float cutoffwas, envelopewas;

    float lfoIn;
    float lfoVibratoIn;

    float pitchWheel;
    float pitchWheelUpAmt;
    float pitchWheelDownAmt;
    bool pitchWheelOsc2Only;

    float lfoa1, lfoa2;
    bool lfoo1, lfoo2, lfof;
    bool lfopw1, lfopw2;

    bool oversample;
    bool selfOscPush;

    float envpitchmod;
    float pwenvmod;

    float pwOfs;
    bool pwEnvBoth;
    bool pitchModBoth;

    bool invertFenv;

    bool fourpole;

    DelayLine<Samples * 2> lenvd, fenvd, lfod;

    ApInterpolator ap;
    float oscpsw;
    int legatoMode;
    float briHold;

    ObxfVoice() : ap()
    {
        selfOscPush = false;
        pitchModBoth = false;
        pwOfs = 0.f;
        invertFenv = false;
        pwEnvBoth = false;
        ng = juce::Random(juce::Random::getSystemRandom().nextInt64());
        sustainHold = false;
        shouldProcessed = false;
        vamp = vflt = 0.f;
        velocityValue = 0.f;
        lfoVibratoIn = 0.f;
        fourpole = false;
        legatoMode = 0;
        brightCoef = briHold = 1.f;
        envpitchmod = 0.f;
        pwenvmod = 0.f;
        oscpsw = 0.f;
        cutoffwas = envelopewas = 0.f;
        oversample = false;
        c1 = c2 = d1 = d2 = 0.f;
        pitchWheel = pitchWheelUpAmt = pitchWheelDownAmt = 0.f;
        lfoIn = 0.f;
        PortaSlopAmt = 0.f;
        FltSlopAmt = 0.f;
        levelSlopAmt = 0.f;
        porta = 0.f;
        prtst = 0.f;
        fltKF = 0.f;
        cutoff = 0.f;
        fenvamt = 0.f;
        Active = false;
        midiIndx = 30;
        levelSlop = juce::Random::getSystemRandom().nextFloat() - 0.5f;
        EnvSlop = juce::Random::getSystemRandom().nextFloat() - 0.5f;
        FenvSlop = juce::Random::getSystemRandom().nextFloat() - 0.5f;
        FltSlop = juce::Random::getSystemRandom().nextFloat() - 0.5f;
        PortaSlop = juce::Random::getSystemRandom().nextFloat() - 0.5f;
    }

    ~ObxfVoice() {}

    void initTuning(Tuning *t) { tuning = t; }

    inline float ProcessSample()
    {
        double tunedMidiNote = tuning->tunedMidiNote(midiIndx);

        // portamento on osc input voltage
        // implements rc circuit
        float ptNote = tpt_lp_unwarped(prtst, tunedMidiNote - 93,
                                       porta * (1 + PortaSlop * PortaSlopAmt), sampleRateInv);
        float pitchwheelcalc =
            (pitchWheel < 0.f) ? (pitchWheel * pitchWheelDownAmt) : (pitchWheel * pitchWheelUpAmt);

        osc.notePlaying = ptNote;

        // both envelopes and filter cv need a delay equal to osc internal delay
        float lfoDelayed = lfod.feedReturn(lfoIn);

        // filter envelope undelayed
        float envm = fenv.processSample() * (1 - (1 - velocityValue) * vflt);

        if (invertFenv)
            envm = -envm;

        // filter cutoff calculation
        float cutoffcalc =
            juce::jmin(getPitch((lfof ? lfoDelayed * lfoa1 : 0) + cutoff + FltSlop * FltSlopAmt +
                                fenvamt * fenvd.feedReturn(envm) - 45 +
                                (fltKF * (pitchwheelcalc + ptNote + 40)))
                           // noisy filter cutoff
                           + (ng.nextFloat() - 0.5f) * 3.5f,
                       (flt.SampleRate * 0.5f - 120.0f)); // for numerical stability purposes

        // limit our max cutoff on self osc to prevent aliasing
        if (selfOscPush)
            cutoffcalc = juce::jmin(cutoffcalc, oversample ? 24000.f : 19000.f);

        // pulsewidth modulation
        float pwenv = envm * (osc.pwenvinv ? -1 : 1);

        osc.pw1 = (lfopw1 ? (lfoIn * lfoa2) : 0) + (pwEnvBoth ? (pwenvmod * pwenv) : 0);
        osc.pw2 = (lfopw2 ? (lfoIn * lfoa2) : 0) + (pwenvmod * pwenv) + pwOfs;

        // pitch modulation
        float penv = envm * (osc.penvinv ? -1 : 1);

        osc.pto1 = (!pitchWheelOsc2Only ? pitchwheelcalc : 0) + (lfoo1 ? (lfoIn * lfoa1) : 0) +
                   (pitchModBoth ? (envpitchmod * penv) : 0) + lfoVibratoIn;
        osc.pto2 =
            pitchwheelcalc + (lfoo2 ? lfoIn * lfoa1 : 0) + (envpitchmod * penv) + lfoVibratoIn;

        // variable sort magic - upsample trick
        float envVal = lenvd.feedReturn(env.processSample() * (1 - (1 - velocityValue) * vamp));

        float oscps = osc.ProcessSample() * (1 - levelSlopAmt * levelSlop);

        oscps = oscps - tpt_lp_unwarped(c1, oscps, 12, sampleRateInv);

        float x1 = oscps;

        x1 = tpt_process(d2, x1, brightCoef);
        x1 = fourpole ? flt.Apply4Pole(x1, cutoffcalc) : flt.Apply2Pole(x1, cutoffcalc);

        x1 *= envVal;

        return x1;
    }

    void setBrightness(float val)
    {
        briHold = val;
        brightCoef = tan(juce::jmin(val, flt.SampleRate * 0.5f - 10) *
                         (juce::MathConstants<float>::pi) * flt.sampleRateInv);
    }

    void setEnvTimingOffset(float d)
    {
        env.setUniqueOffset(1 + EnvSlop * d);
        fenv.setUniqueOffset(1 + FenvSlop * d);
    }

    void setHQ(bool hq)
    {
        if (hq)
        {
            osc.setDecimation();
        }
        else
        {
            osc.removeDecimation();
        }

        oversample = hq;
    }

    void setSampleRate(float sr)
    {
        SampleRate = sr;
        sampleRateInv = 1 / sr;

        flt.setSampleRate(sr);
        osc.setSampleRate(sr);
        env.setSampleRate(sr);
        fenv.setSampleRate(sr);

        brightCoef = tan(juce::jmin(briHold, flt.SampleRate * 0.5f - 10) *
                         (juce::MathConstants<float>::pi) * flt.sampleRateInv);
    }

    void checkAdsrState() { shouldProcessed = env.isActive(); }

    void ResetEnvelope()
    {
        env.ResetEnvelopeState();
        fenv.ResetEnvelopeState();
    }

    void NoteOn(int mididx, float velocity)
    {
        if (!shouldProcessed)
        {
            // When your processing is paused we need to clear delay lines and envelopes
            // Not doing this will cause clicks or glitches
            lenvd.fillZeroes();
            fenvd.fillZeroes();
            ResetEnvelope();
        }

        shouldProcessed = true;

        if (velocity != -0.5)
            velocityValue = velocity;

        midiIndx = mididx;

        if ((!Active) || (legatoMode & 1))
            env.triggerAttack();

        if ((!Active) || (legatoMode & 2))
            fenv.triggerAttack();

        Active = true;
    }

    void NoteOff()
    {
        if (!sustainHold)
        {
            env.triggerRelease();
            fenv.triggerRelease();
        }

        Active = false;
    }

    void sustOn() { sustainHold = true; }

    void sustOff()
    {
        sustainHold = false;

        if (!Active)
        {
            env.triggerRelease();
            fenv.triggerRelease();
        }
    }
};

#endif // OBXF_SRC_ENGINE_OBXDVOICE_H
