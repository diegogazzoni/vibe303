#pragma once
#include <cmath>

//==============================================================================
// AccentSlide
// Manages the accent and slide (portamento/legato) state per note event,
// matching the TB-303 hardware behaviour:
//
//   ACCENT  — triggered when MIDI velocity > 100
//             Effect: amplitude boosted by +6 dB, filter envelope runs faster,
//             filter env mod amount increases by ~50 % on top of the knob value.
//
//   SLIDE   — triggered when a new note arrives before the previous one released
//             (mono legato), OR can be forced via MIDI CC 65 (Portamento On/Off).
//             Effect: pitch glides from previous note to new note over ~60 ms,
//             and the envelope does NOT retrigger (continuous gate).
//==============================================================================
class AccentSlide
{
public:
    AccentSlide() = default;

    void reset()
    {
        accentActive      = false;
        slideActive       = false;
        noteIsHeld        = false;
        currentNoteNumber = -1;
        accentLevel       = 0.0f;
        accentDecayCoeff  = 0.99f;
    }

    void prepare (double sampleRate)
    {
        // Accent fades out over ~30 ms
        accentDecayCoeff = std::exp (-1.0f / (0.030f * (float) sampleRate));
        reset();
    }

    // Call on each MIDI note-on. Returns whether slide should be engaged.
    // velocity > 100 → accent mode
    bool noteOn (int noteNumber, int velocity)
    {
        bool doSlide = noteIsHeld && (currentNoteNumber >= 0);

        accentActive      = (velocity > 100);
        accentLevel       = accentActive ? 1.0f : 0.0f;
        noteIsHeld        = true;
        currentNoteNumber = noteNumber;

        return doSlide;
    }

    void noteOff (int noteNumber)
    {
        if (noteNumber == currentNoteNumber)
        {
            noteIsHeld = false;
            accentActive = false;
        }
    }

    // Per-sample accent envelope tick
    // Returns a gain multiplier: 1.0 (no accent) up to 2.0 (+6 dB with full accent)
    float processAccentGain (float accentKnob)
    {
        if (accentLevel > 0.0f)
            accentLevel *= accentDecayCoeff;

        // accentKnob 0..1 scales the maximum boost (up to +6 dB = factor 2.0)
        return 1.0f + accentLevel * accentKnob;
    }

    // Returns extra filter env-mod multiplier due to accent (0..1 extra on top of knob)
    float accentEnvModBoost (float accentKnob) const
    {
        return accentLevel * accentKnob * 0.5f;
    }

    bool isAccentActive()  const { return accentActive; }
    bool isNoteHeld()      const { return noteIsHeld; }
    int  getCurrentNote()  const { return currentNoteNumber; }

private:
    bool  accentActive      = false;
    bool  slideActive       = false;
    bool  noteIsHeld        = false;
    int   currentNoteNumber = -1;
    float accentLevel       = 0.0f;
    float accentDecayCoeff  = 0.99f;
};
