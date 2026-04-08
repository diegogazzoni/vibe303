#pragma once
#include <cmath>
#include <cstdlib>

//==============================================================================
// LFO — Low-Frequency Oscillator for modulation
//
// Shapes: Sine, Triangle, Sawtooth, Square, Sample & Hold
// Rate:   0.05 – 20 Hz (free-running, not tempo-synced in this implementation)
// Depth:  0.0 – 1.0
//==============================================================================
class LFO
{
public:
    enum class Shape { Sine = 0, Triangle, Sawtooth, Square, SampleHold };

    LFO() = default;

    void prepare (double sampleRate_)
    {
        sampleRate = sampleRate_;
        phase      = 0.0;
        shValue    = 0.0f;
        shPhase    = 0.0;
    }

    void reset()
    {
        phase   = 0.0;
        shValue = 0.0f;
        shPhase = 0.0;
    }

    // Returns a modulation value in [-1, +1]
    float process (float rateHz, Shape shape)
    {
        const double increment = rateHz / sampleRate;

        // Advance phase
        phase += increment;
        if (phase >= 1.0) phase -= 1.0;

        float out = 0.0f;

        switch (shape)
        {
            case Shape::Sine:
                out = (float) std::sin (phase * 2.0 * 3.14159265358979);
                break;

            case Shape::Triangle:
                out = (float) (phase < 0.5
                    ?  4.0 * phase - 1.0
                    : -4.0 * phase + 3.0);
                break;

            case Shape::Sawtooth:
                out = (float) (2.0 * phase - 1.0);
                break;

            case Shape::Square:
                out = (phase < 0.5) ? 1.0f : -1.0f;
                break;

            case Shape::SampleHold:
            {
                // A new random value is latched at the start of each cycle.
                // We track an internal phase separately so it always latches
                // exactly once per period regardless of rate changes.
                shPhase += increment;
                if (shPhase >= 1.0)
                {
                    shPhase -= 1.0;
                    // rand() is fine here — audio-thread but LFO is infrequent
                    shValue = (float) std::rand() / (float) RAND_MAX * 2.0f - 1.0f;
                }
                out = shValue;
                break;
            }
        }

        return out;
    }

    // Convenience: returns a unipolar value [0, 1]
    float processUnipolar (float rateHz, Shape shape)
    {
        return (process (rateHz, shape) + 1.0f) * 0.5f;
    }

private:
    double sampleRate = 44100.0;
    double phase      = 0.0;
    float  shValue    = 0.0f;
    double shPhase    = 0.0;
};
