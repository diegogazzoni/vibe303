#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

//==============================================================================
// Roland 303-style custom look-and-feel
//==============================================================================
class Acid303LookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32_t kBodySilver  = 0xffb8bdc0;
    static constexpr uint32_t kKnobSilver  = 0xffc8cacc;
    static constexpr uint32_t kKnobShine   = 0xffe8eaec;
    static constexpr uint32_t kKnobShadow  = 0xff888a8c;
    static constexpr uint32_t kOrange      = 0xffdd6600;
    static constexpr uint32_t kOrangeDark  = 0xff994400;
    static constexpr uint32_t kBlack       = 0xff1a1a1a;
    static constexpr uint32_t kLabelText   = 0xff222222;

    Acid303LookAndFeel()
    {
        setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (kOrange));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff999999));
        setColour (juce::Slider::thumbColourId,               juce::Colour (kKnobShine));
        setColour (juce::Label::textColourId,                 juce::Colour (kLabelText));
        setColour (juce::TextButton::buttonColourId,          juce::Colour (0xff666666));
        setColour (juce::TextButton::buttonOnColourId,        juce::Colour (kOrange));
        setColour (juce::TextButton::textColourOnId,          juce::Colours::white);
        setColour (juce::TextButton::textColourOffId,         juce::Colour (0xffdddddd));
        setColour (juce::ComboBox::backgroundColourId,        juce::Colour (0xff2a2a2a));
        setColour (juce::ComboBox::textColourId,              juce::Colour (kOrange));
        setColour (juce::ComboBox::outlineColourId,           juce::Colour (0xff555555));
        setColour (juce::ComboBox::arrowColourId,             juce::Colour (kOrange));
        setColour (juce::PopupMenu::backgroundColourId,       juce::Colour (0xff2a2a2a));
        setColour (juce::PopupMenu::textColourId,             juce::Colour (kOrange));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (kOrange));
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colours::white);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& /*slider*/) override
    {
        const float radius  = (float) std::min (width, height) * 0.5f - 5.0f;
        const float centreX = (float) x + (float) width  * 0.5f;
        const float centreY = (float) y + (float) height * 0.5f;
        const float angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Shadow ring
        g.setColour (juce::Colour (0x66000000));
        g.fillEllipse (centreX - radius - 4.0f, centreY - radius - 4.0f,
                       (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);

        // Track ring
        g.setColour (juce::Colour (0xff888888));
        g.fillEllipse (centreX - radius - 2.0f, centreY - radius - 2.0f,
                       (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f);

        // Orange arc (filled portion)
        {
            juce::Path arc;
            arc.addArc (centreX - radius, centreY - radius,
                        radius * 2.0f, radius * 2.0f,
                        rotaryStartAngle, angle, true);
            g.setColour (juce::Colour (kOrange));
            g.strokePath (arc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // Unfilled arc
        {
            juce::Path arc;
            arc.addArc (centreX - radius, centreY - radius,
                        radius * 2.0f, radius * 2.0f,
                        angle, rotaryEndAngle, true);
            g.setColour (juce::Colour (0xff555555));
            g.strokePath (arc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // Knob body — radial metallic gradient
        {
            const float kr = radius - 5.0f;
            juce::ColourGradient grad (juce::Colour (kKnobShine),
                                       centreX - kr * 0.35f, centreY - kr * 0.35f,
                                       juce::Colour (kKnobShadow),
                                       centreX + kr * 0.5f,  centreY + kr * 0.5f, true);
            grad.addColour (0.45, juce::Colour (kKnobSilver));
            g.setGradientFill (grad);
            g.fillEllipse (centreX - kr, centreY - kr, kr * 2.0f, kr * 2.0f);
            g.setColour (juce::Colour (0x55ffffff));
            g.drawEllipse (centreX - kr, centreY - kr, kr * 2.0f, kr * 2.0f, 1.0f);
        }

        // Pointer (shadow + highlight)
        const float pr     = radius - 5.0f;
        const float pOuter = -pr + 2.0f;
        const float pInner = -pr * 0.45f;

        juce::Path shadow;
        shadow.startNewSubPath (0.5f, pOuter + 1.0f);
        shadow.lineTo          (0.5f, pInner);
        shadow.applyTransform  (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (juce::Colour (0xff222222));
        g.strokePath (shadow, juce::PathStrokeType (3.0f, juce::PathStrokeType::mitered,
                                                     juce::PathStrokeType::butt));

        juce::Path pointer;
        pointer.startNewSubPath (0.0f, pOuter);
        pointer.lineTo          (0.0f, pInner);
        pointer.applyTransform  (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (juce::Colour (kKnobShine));
        g.strokePath (pointer, juce::PathStrokeType (2.0f, juce::PathStrokeType::mitered,
                                                      juce::PathStrokeType::butt));
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                const juce::Colour&, bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        const bool isOn = button.getToggleState();

        juce::Colour fill = isOn ? juce::Colour (kOrange) : juce::Colour (0xff555555);
        if (isHighlighted) fill = fill.brighter (0.12f);
        if (isDown)        fill = fill.darker   (0.15f);

        g.setColour (fill);
        g.fillRoundedRectangle (bounds, 3.0f);

        g.setColour (juce::Colour (0x33ffffff));
        g.drawLine (bounds.getX() + 3.0f, bounds.getY() + 1.0f,
                    bounds.getRight() - 3.0f, bounds.getY() + 1.0f, 1.0f);

        g.setColour (isOn ? juce::Colour (kOrangeDark) : juce::Colour (0xff333333));
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
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

    // --- Synth knobs
    juce::Slider tuningKnob, cutoffKnob, resonanceKnob,
                 envModKnob, decayKnob, accentKnob, volumeKnob;

    // --- Delay knobs (no TIME knob — replaced by division selector)
    juce::Slider delayFeedbackKnob, delayMixKnob;

    // --- Delay division selector
    juce::ComboBox delayDivCombo;

    // --- Labels
    juce::Label tuningLabel, cutoffLabel, resonanceLabel,
                envModLabel, decayLabel, accentLabel, volumeLabel, waveformLabel;
    juce::Label delayFeedbackLabel, delayMixLabel, delayDivLabel;
    juce::Label delaySectionLabel;

    // --- Waveform toggle
    juce::TextButton sawButton { "SAW" };
    juce::TextButton sqButton  { "SQ"  };

    // --- APVTS attachments
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> tuningAttach, cutoffAttach, resonanceAttach,
                                      envModAttach, decayAttach, accentAttach, volumeAttach;
    std::unique_ptr<SliderAttachment>   delayFeedbackAttach, delayMixAttach;
    std::unique_ptr<ComboBoxAttachment> delayDivAttach;

    // Waveform combo (hidden — drives the Choice parameter)
    juce::ComboBox               waveformCombo;
    std::unique_ptr<ComboBoxAttachment> waveformAttach;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& labelText,
                    juce::Colour labelColour = juce::Colour (0xff222222));

    // Layout
    static constexpr int kWindowW      = 880;
    static constexpr int kWindowH      = 265;
    static constexpr int kTopBarH      = 40;
    static constexpr int kKnobSize     = 72;
    static constexpr int kColW         = 84;
    static constexpr int kLabelH       = 18;
    static constexpr int kSynthStartX  = 18;
    static constexpr int kDelaySectionX = kSynthStartX + 8 * kColW + 12;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Acid303AudioProcessorEditor)
};
