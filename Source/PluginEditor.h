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
// LfoDisplayComponent
// Vital-style animated LFO visualiser. Draws the waveform shape as a filled
// curve, with a scanning vertical cursor that tracks the live LFO phase.
//==============================================================================
class LfoDisplayComponent : public juce::Component,
                             private juce::Timer
{
public:
    LfoDisplayComponent() { startTimerHz (60); }

    // Called each timer tick to update the live cursor position
    // lfoValue: current LFO output in [-1, +1]
    void setLfoValue (float lfoValue)
    {
        // Advance the display phase manually using the rate set externally.
        // We just use the raw value for cursor animation.
        currentLfoValue = lfoValue;
        // Advance normalized phase 0..1 based on display rate
        displayPhase += displayPhaseIncrement;
        if (displayPhase >= 1.0f) displayPhase -= 1.0f;
    }

    void setRate (float rateHz)
    {
        // Clamp display refresh — very fast LFOs just show the shape statically.
        const float displayRate = juce::jlimit (0.05f, 4.0f, rateHz);
        displayPhaseIncrement = displayRate / 60.0f; // 60 Hz timer
    }

    void setShape (int shapeIndex) { shape = shapeIndex; repaint(); }
    void setDepth (float depth)    { currentDepth = depth; }

private:
    void timerCallback() override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        const float w = bounds.getWidth();
        const float h = bounds.getHeight();
        const float cx = bounds.getX();
        const float cy = bounds.getY();
        const float midY = cy + h * 0.5f;

        // Background
        g.setColour (juce::Colour (0xff1a1a1a));
        g.fillRoundedRectangle (bounds, 4.0f);

        // Grid lines
        g.setColour (juce::Colour (0xff2e2e2e));
        g.drawHorizontalLine ((int) midY, cx, cx + w);
        g.drawHorizontalLine ((int) (cy + h * 0.25f), cx, cx + w);
        g.drawHorizontalLine ((int) (cy + h * 0.75f), cx, cx + w);
        for (int col = 1; col < 4; ++col)
            g.drawVerticalLine ((int) (cx + w * col / 4.0f), cy, cy + h);

        // Build waveform path (1 full cycle displayed)
        const int numPts = (int) w;
        juce::Path wavePath;
        bool first = true;

        for (int px = 0; px < numPts; ++px)
        {
            const float t  = (float) px / (float) numPts; // 0..1 across display
            const float sv = sampleShape (t);
            const float py = midY - sv * (h * 0.43f);

            if (first) { wavePath.startNewSubPath (cx + (float) px, py); first = false; }
            else        wavePath.lineTo            (cx + (float) px, py);
        }

        // Fill area under the wave
        juce::Path fillPath = wavePath;
        fillPath.lineTo (cx + w, midY);
        fillPath.lineTo (cx,     midY);
        fillPath.closeSubPath();

        juce::ColourGradient grad (juce::Colour (0x55dd6600), cx, cy,
                                   juce::Colour (0x00dd6600), cx, cy + h, false);
        g.setGradientFill (grad);
        g.fillPath (fillPath);

        // Wave outline
        g.setColour (juce::Colour (currentDepth > 0.01f ? 0xffdd6600u : 0xff555555u));
        g.strokePath (wavePath, juce::PathStrokeType (1.5f));

        // Scanning cursor (vertical line at current phase)
        const float cursorX = cx + displayPhase * w;
        g.setColour (juce::Colour (0xccffffff));
        g.drawVerticalLine ((int) cursorX, cy, cy + h);

        // Cursor dot on the waveform
        const float cursorVal = sampleShape (displayPhase);
        const float dotY = midY - cursorVal * (h * 0.43f);
        g.setColour (juce::Colour (0xffdd6600));
        g.fillEllipse (cursorX - 3.5f, dotY - 3.5f, 7.0f, 7.0f);

        // Border
        g.setColour (juce::Colour (0xff444444));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    // Returns waveform value [-1,+1] at normalised phase t in [0,1]
    float sampleShape (float t) const
    {
        switch (shape)
        {
            case 0: return std::sin (t * 2.0f * 3.14159265f);           // Sine
            case 1: return (t < 0.5f) ? (4.0f * t - 1.0f)              // Triangle
                                      : (-4.0f * t + 3.0f);
            case 2: return 2.0f * t - 1.0f;                              // Sawtooth
            case 3: return (t < 0.5f) ? 1.0f : -1.0f;                   // Square
            case 4: // S&H — staircase across 8 steps
            {
                const int   steps = 8;
                const int   step  = (int) (t * steps);
                // Use a fixed pseudo-random sequence so it looks consistent
                static const float vals[8] = {0.6f,-0.4f,0.9f,-0.7f,0.3f,-0.9f,0.5f,-0.2f};
                return vals[step % 8];
            }
            default: return 0.0f;
        }
    }

    int   shape               = 0;
    float currentLfoValue     = 0.0f;
    float currentDepth        = 0.0f;
    float displayPhase        = 0.0f;
    float displayPhaseIncrement = 1.0f / 60.0f; // 1 Hz default
};

//==============================================================================
class Acid303AudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit Acid303AudioProcessorEditor (Acid303AudioProcessor&);
    ~Acid303AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

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

    // ---- Preset bar -------------------------------------------------------
    juce::ComboBox presetCombo;
    juce::TextButton presetSaveButton  { "SAVE" };
    juce::TextButton presetLoadButton  { "LOAD" };
    juce::Label      presetSectionLabel;

    // ---- LFO section ------------------------------------------------------
    LfoDisplayComponent lfoDisplay;

    juce::Slider   lfoRateKnob, lfoDepthKnob;
    juce::Label    lfoRateLabel, lfoDepthLabel;
    juce::ComboBox lfoShapeCombo, lfoTargetCombo;
    juce::Label    lfoShapeLabel, lfoTargetLabel;
    juce::Label    lfoSectionLabel;

    // --- APVTS attachments
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> tuningAttach, cutoffAttach, resonanceAttach,
                                      envModAttach, decayAttach, accentAttach, volumeAttach;
    std::unique_ptr<SliderAttachment>   delayFeedbackAttach, delayMixAttach;
    std::unique_ptr<ComboBoxAttachment> delayDivAttach;

    std::unique_ptr<SliderAttachment>   lfoRateAttach, lfoDepthAttach;
    std::unique_ptr<ComboBoxAttachment> lfoShapeAttach, lfoTargetAttach;

    // Waveform combo (hidden — drives the Choice parameter)
    juce::ComboBox               waveformCombo;
    std::unique_ptr<ComboBoxAttachment> waveformAttach;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& labelText,
                    juce::Colour labelColour = juce::Colour (0xff222222));

    void setupSmallKnob (juce::Slider& knob, juce::Label& label,
                         const juce::String& labelText, juce::Colour labelColour);

    void rebuildPresetCombo();
    void onSavePreset();
    void onLoadPreset();

    // Layout — original window extended in height to accommodate LFO row
    static constexpr int kWindowW       = 880;
    static constexpr int kTopBarH       = 40;
    static constexpr int kPresetBarH    = 30;   // preset bar below the top header
    static constexpr int kSynthPanelH   = 210;  // original panel height
    static constexpr int kLfoPanelH     = 140;  // LFO panel below synth panel
    static constexpr int kWindowH       = kTopBarH + kPresetBarH + kSynthPanelH + kLfoPanelH + 8;
    static constexpr int kKnobSize      = 72;
    static constexpr int kSmallKnobSize = 52;
    static constexpr int kColW          = 84;
    static constexpr int kLabelH        = 18;
    static constexpr int kSynthStartX   = 18;
    static constexpr int kDelaySectionX = kSynthStartX + 8 * kColW + 12;

    // Y-offset for the synth knob row (below preset bar)
    int synthRowY() const { return kTopBarH + kPresetBarH + 22; }
    int lfoRowY()   const { return kTopBarH + kPresetBarH + kSynthPanelH + 8; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Acid303AudioProcessorEditor)
};
