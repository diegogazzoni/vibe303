#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

//==============================================================================
// Custom look-and-feel for the 303-style UI
//==============================================================================
class Acid303LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Acid303LookAndFeel()
    {
        // Dark silver/black 303 aesthetic
        setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffcc6600));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff444444));
        setColour (juce::Slider::thumbColourId,               juce::Colour (0xffeeeeee));
        setColour (juce::Label::textColourId,                 juce::Colour (0xffdddddd));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& /*slider*/) override
    {
        const float radius   = (float) std::min (width, height) * 0.5f - 4.0f;
        const float centreX  = (float) x + (float) width  * 0.5f;
        const float centreY  = (float) y + (float) height * 0.5f;
        const float angle    = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Outer ring
        g.setColour (juce::Colour (0xff333333));
        g.fillEllipse (centreX - radius - 2, centreY - radius - 2,
                       (radius + 2) * 2.0f, (radius + 2) * 2.0f);

        // Knob body
        g.setColour (juce::Colour (0xff555555));
        g.fillEllipse (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Filled arc (position indicator)
        juce::Path arc;
        arc.addArc (centreX - radius + 3, centreY - radius + 3,
                    (radius - 3) * 2.0f, (radius - 3) * 2.0f,
                    rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (0xffcc6600));
        g.strokePath (arc, juce::PathStrokeType (3.0f));

        // Pointer line
        juce::Path pointer;
        const float pointerLength = radius * 0.6f;
        pointer.startNewSubPath (0.0f, -radius + 2);
        pointer.lineTo (0.0f, -radius + 2 + pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                    .translated (centreX, centreY));
        g.setColour (juce::Colour (0xffeeeeee));
        g.strokePath (pointer, juce::PathStrokeType (2.5f,
                                                      juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }
};

//==============================================================================
class Acid303AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Acid303AudioProcessorEditor (Acid303AudioProcessor&);
    ~Acid303AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Acid303AudioProcessor& processorRef;
    Acid303LookAndFeel     acidLookAndFeel;

    //==========================================================================
    // Knobs
    juce::Slider tuningKnob, cutoffKnob, resonanceKnob,
                 envModKnob, decayKnob, accentKnob, volumeKnob;

    // Labels
    juce::Label tuningLabel, cutoffLabel, resonanceLabel,
                envModLabel, decayLabel, accentLabel, volumeLabel, waveformLabel;

    // Waveform toggle
    juce::TextButton sawButton { "SAW" };
    juce::TextButton sqButton  { "SQ"  };

    //==========================================================================
    // APVTS attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> tuningAttach, cutoffAttach, resonanceAttach,
                                      envModAttach, decayAttach, accentAttach, volumeAttach;

    // Waveform uses a ComboBox attachment to handle the Choice parameter
    juce::ComboBox               waveformCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttach;

    //==========================================================================
    void setupKnob (juce::Slider& knob, juce::Label& label,
                    const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Acid303AudioProcessorEditor)
};
