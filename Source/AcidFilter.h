#pragma once
#include <cmath>
#include <algorithm>

//==============================================================================
// AcidFilter
// Custom 4-pole diode ladder low-pass filter (24 dB/oct).
// Modelled after the TB-303's filter circuit:
//   - Non-self-oscillating (resonance capped below oscillation threshold)
//   - tanh saturation applied at each stage for nonlinear warmth
//   - Cutoff modulated per-sample by the filter envelope + env mod amount
//==============================================================================
class AcidFilter
{
public:
    AcidFilter() = default;

    void prepare (double sampleRate_)
    {
        sampleRate = sampleRate_;
        reset();
    }

    void reset()
    {
        for (int i = 0; i < 4; ++i)
            stage[i] = 0.0f;
        feedback = 0.0f;
    }

    // cutoffHz  : base cutoff frequency in Hz (from Cutoff knob)
    // resonance : 0.0 – 1.0  (mapped internally to 0 – ~3.8, just below self-osc)
    // envMod    : 0.0 – 1.0  (from Env Mod knob)
    // envLevel  : 0.0 – 1.0  (output of the filter AD envelope)
    // Returns filtered sample.
    float processSample (float input,
                         float cutoffHz,
                         float resonance,
                         float envMod,
                         float envLevel)
    {
        // Modulate cutoff upward by envelope (up to ~4 octaves at full env mod)
        float modulated = cutoffHz * (1.0f + envMod * envLevel * 4.0f);
        modulated = std::max (20.0f, std::min (modulated, 18000.0f));

        // Normalised cutoff coefficient
        const float pi = 3.14159265358979f;
        float f = 2.0f * std::sin (pi * modulated / (float) sampleRate);
        f = std::max (0.0f, std::min (f, 0.85f));

        // Resonance: max 3.8 to stay below self-oscillation threshold
        float q = resonance * 3.8f;

        // 4-pole diode ladder with tanh per-stage saturation
        float x = input - q * feedback;
        x = std::tanh (x);

        stage[0] = stage[0] + f * (std::tanh (x)        - std::tanh (stage[0]));
        stage[1] = stage[1] + f * (std::tanh (stage[0]) - std::tanh (stage[1]));
        stage[2] = stage[2] + f * (std::tanh (stage[1]) - std::tanh (stage[2]));
        stage[3] = stage[3] + f * (std::tanh (stage[2]) - std::tanh (stage[3]));

        // Update feedback from output stage
        feedback = stage[3];

        return stage[3];
    }

private:
    double sampleRate = 44100.0;
    float  stage[4]   = { 0.f, 0.f, 0.f, 0.f };
    float  feedback   = 0.0f;
};
