#pragma once
#include <cmath>

//==============================================================================
// Oscillator
// Generates sawtooth or square wave for the TB-303 VCO.
// Supports portamento (slide) between notes.
//==============================================================================
class Oscillator
{
public:
    enum class Waveform { Sawtooth, Square };

    Oscillator() = default;

    void reset()
    {
        phase         = 0.0;
        currentPitch  = -1.0;
        targetPitch   = -1.0;
        slideActive   = false;
        slideSamples  = 0;
        slideDelta    = 0.0;
    }

    void prepare (double sampleRate_)
    {
        sampleRate = sampleRate_;
        reset();
    }

    // Call when a new note starts.
    // If slide==true, pitch glides from currentPitch to newMidiNote over ~60 ms.
    void noteOn (int newMidiNote, bool slide)
    {
        targetPitch = (double) newMidiNote;

        if (slide && currentPitch > 0.0)
        {
            slideActive  = true;
            // 60 ms slide time, faithful to original hardware
            slideSamples = (int) (sampleRate * 0.060);
            slideDelta   = (targetPitch - currentPitch) / (double) slideSamples;
        }
        else
        {
            slideActive  = false;
            currentPitch = targetPitch;
            slideSamples = 0;
            slideDelta   = 0.0;
        }
    }

    // tuningCents: global tuning offset in semitones (±12 from the Tuning knob)
    float process (float tuningCents)
    {
        // Advance slide
        if (slideActive && slideSamples > 0)
        {
            currentPitch += slideDelta;
            --slideSamples;
            if (slideSamples == 0)
            {
                currentPitch = targetPitch;
                slideActive  = false;
            }
        }

        // MIDI note → frequency, apply tuning offset
        double midiNote  = currentPitch + (double) tuningCents;
        double frequency = 440.0 * std::pow (2.0, (midiNote - 69.0) / 12.0);

        // Phase increment
        double increment = frequency / sampleRate;
        phase += increment;
        if (phase >= 1.0) phase -= 1.0;

        // Waveform generation
        if (waveform == Waveform::Sawtooth)
        {
            // Naive sawtooth: phase 0..1 → sample -1..+1
            // Slight PolyBLEP correction for alias reduction
            return (float) (2.0 * phase - 1.0) - (float) polyBlep (phase, increment);
        }
        else
        {
            // Square wave with 50% duty cycle + PolyBLEP
            float sq = (phase < 0.5f) ? 1.0f : -1.0f;
            sq += (float) polyBlep (phase, increment);
            sq -= (float) polyBlep (std::fmod (phase + 0.5, 1.0), increment);
            return sq;
        }
    }

    Waveform waveform = Waveform::Sawtooth;

private:
    // PolyBLEP anti-aliasing correction
    double polyBlep (double t, double dt) const
    {
        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0;
        }
        else if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }
        return 0.0;
    }

    double sampleRate    = 44100.0;
    double phase         = 0.0;
    double currentPitch  = 69.0; // MIDI note number (can be fractional during slide)
    double targetPitch   = 69.0;
    bool   slideActive   = false;
    int    slideSamples  = 0;
    double slideDelta    = 0.0;
};
