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

#ifndef OBXF_SRC_ENGINE_AUDIOUTILS_H
#define OBXF_SRC_ENGINE_AUDIOUTILS_H

#include <Utils.h>
#include "SynthEngine.h"

constexpr float dc = 1e-18f;

inline static float tpt_lp_unwarped(float &state, float inp, float cutoff, float srInv)
{
    cutoff = (cutoff * srInv) * juce::MathConstants<float>::pi;
    double v = (inp - state) * cutoff / (1.f + cutoff);
    double res = v + state;
    state = res + v;
    return res;
}

inline static float tpt_lp(float &state, float inp, float cutoff, float srInv)
{
    cutoff = tan(cutoff * srInv * juce::MathConstants<float>::pi);
    double v = (inp - state) * cutoff / (1.f + cutoff);
    double res = v + state;
    state = res + v;
    return res;
};

inline static float tpt_process(float &state, float inp, float cutoff)
{
    double v = (inp - state) * cutoff / (1.f + cutoff);
    double res = v + state;
    state = res + v;
    return res;
}

#endif // OBXF_SRC_ENGINE_AUDIOUTILS_H
