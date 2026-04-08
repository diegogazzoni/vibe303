#include "PluginEditor.h"

//==============================================================================
Acid303AudioProcessorEditor::Acid303AudioProcessorEditor (Acid303AudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&acidLookAndFeel);

    // ---- Synth knobs ----------------------------------------------------
    setupKnob (tuningKnob,    tuningLabel,    "TUNING");
    setupKnob (cutoffKnob,    cutoffLabel,    "CUTOFF");
    setupKnob (resonanceKnob, resonanceLabel, "RESONANCE");
    setupKnob (envModKnob,    envModLabel,    "ENV MOD");
    setupKnob (decayKnob,     decayLabel,     "DECAY");
    setupKnob (accentKnob,    accentLabel,    "ACCENT");
    setupKnob (volumeKnob,    volumeLabel,    "VOLUME");

    // ---- Delay knobs ----------------------------------------------------
    setupKnob (delayFeedbackKnob, delayFeedbackLabel, "FEEDBACK",
               juce::Colour (0xffdddddd));
    setupKnob (delayMixKnob, delayMixLabel, "MIX",
               juce::Colour (0xffdddddd));

    // ---- Delay division combo -------------------------------------------
    for (int i = 0; i < Acid303AudioProcessor::kDivisionLabels.size(); ++i)
        delayDivCombo.addItem (Acid303AudioProcessor::kDivisionLabels[i], i + 1);
    delayDivCombo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (delayDivCombo);

    delayDivLabel.setText ("DIVISION", juce::dontSendNotification);
    delayDivLabel.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    delayDivLabel.setJustificationType (juce::Justification::centred);
    delayDivLabel.setColour (juce::Label::textColourId, juce::Colour (0xffdddddd));
    addAndMakeVisible (delayDivLabel);

    // ---- Delay section header -------------------------------------------
    delaySectionLabel.setText ("DELAY", juce::dontSendNotification);
    delaySectionLabel.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    delaySectionLabel.setJustificationType (juce::Justification::centred);
    delaySectionLabel.setColour (juce::Label::textColourId,
                                  juce::Colour (Acid303LookAndFeel::kOrange));
    addAndMakeVisible (delaySectionLabel);

    // ---- Waveform toggle ------------------------------------------------
    sawButton.setClickingTogglesState (true);
    sqButton.setClickingTogglesState  (true);
    sawButton.setRadioGroupId (1);
    sqButton.setRadioGroupId  (1);
    sawButton.setToggleState  (true, juce::dontSendNotification);
    addAndMakeVisible (sawButton);
    addAndMakeVisible (sqButton);

    waveformLabel.setText ("WAVE", juce::dontSendNotification);
    waveformLabel.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    waveformLabel.setJustificationType (juce::Justification::centred);
    waveformLabel.setColour (juce::Label::textColourId, juce::Colour (0xff222222));
    addAndMakeVisible (waveformLabel);

    // Hidden waveform combo for the Choice parameter
    waveformCombo.addItem ("Sawtooth", 1);
    waveformCombo.addItem ("Square",   2);
    waveformCombo.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (waveformCombo);
    waveformCombo.setVisible (false);

    sawButton.onClick = [this] { if (sawButton.getToggleState()) waveformCombo.setSelectedId (1); };
    sqButton.onClick  = [this] { if (sqButton.getToggleState())  waveformCombo.setSelectedId (2); };
    waveformCombo.onChange = [this]
    {
        bool isSaw = (waveformCombo.getSelectedId() == 1);
        sawButton.setToggleState ( isSaw, juce::dontSendNotification);
        sqButton.setToggleState  (!isSaw, juce::dontSendNotification);
    };

    // ---- APVTS attachments ----------------------------------------------
    auto& apvts = processorRef.apvts;
    tuningAttach        = std::make_unique<SliderAttachment>   (apvts, "tuning",        tuningKnob);
    cutoffAttach        = std::make_unique<SliderAttachment>   (apvts, "cutoff",        cutoffKnob);
    resonanceAttach     = std::make_unique<SliderAttachment>   (apvts, "resonance",     resonanceKnob);
    envModAttach        = std::make_unique<SliderAttachment>   (apvts, "envmod",        envModKnob);
    decayAttach         = std::make_unique<SliderAttachment>   (apvts, "decay",         decayKnob);
    accentAttach        = std::make_unique<SliderAttachment>   (apvts, "accent",        accentKnob);
    volumeAttach        = std::make_unique<SliderAttachment>   (apvts, "volume",        volumeKnob);
    delayFeedbackAttach = std::make_unique<SliderAttachment>   (apvts, "delayFeedback", delayFeedbackKnob);
    delayMixAttach      = std::make_unique<SliderAttachment>   (apvts, "delayMix",      delayMixKnob);
    delayDivAttach      = std::make_unique<ComboBoxAttachment> (apvts, "delayDiv",      delayDivCombo);
    waveformAttach      = std::make_unique<ComboBoxAttachment> (apvts, "waveform",      waveformCombo);

    setSize (kWindowW, kWindowH);
}

Acid303AudioProcessorEditor::~Acid303AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void Acid303AudioProcessorEditor::paint (juce::Graphics& g)
{
    // ---- Background: warm silver panel ----------------------------------
    g.fillAll (juce::Colour (0xffb0b4b8));

    // Brushed-metal texture
    g.setColour (juce::Colour (0x14ffffff));
    for (int y = 0; y < getHeight(); y += 2)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    // ---- Top header bar -------------------------------------------------
    g.setColour (juce::Colour (0xff1e1e1e));
    g.fillRect (0, 0, getWidth(), kTopBarH);

    // Orange accent line
    g.setColour (juce::Colour (Acid303LookAndFeel::kOrange));
    g.fillRect (0.0f, (float) kTopBarH - 2.0f, (float) getWidth(), 2.0f);

    // ---- Brand name -----------------------------------------------------
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f).withStyle ("Bold"));
    g.drawText ("ACID 303", 14, 4, 160, 30, juce::Justification::centredLeft);

    g.setColour (juce::Colour (Acid303LookAndFeel::kOrange));
    g.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    g.drawText ("BASS LINE SYNTHESIZER", 14, 26, 200, 12,
                juce::Justification::centredLeft);

    // ---- "R" badge (top right) ------------------------------------------
    {
        const float rSize = 26.0f;
        const float rX    = (float) getWidth() - rSize - 10.0f;
        const float rY    = 7.0f;
        g.setColour (juce::Colour (Acid303LookAndFeel::kOrange));
        g.fillRoundedRectangle (rX, rY, rSize, rSize, 4.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
        g.drawText ("R", (int) rX, (int) rY, (int) rSize, (int) rSize,
                    juce::Justification::centred);
    }

    // ---- Synth section panel --------------------------------------------
    {
        const int px = kSynthStartX - 6;
        const int py = kTopBarH + 4;
        const int pw = 8 * kColW + 6;
        const int ph = kWindowH - py - 8;

        g.setColour (juce::Colour (0xffadb1b5));
        g.fillRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f);
        g.setColour (juce::Colour (0x44ffffff));
        g.drawRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f, 1.0f);
        g.setColour (juce::Colour (0x33000000));
        g.drawLine ((float) px + 5.0f, (float) (py + ph - 1),
                    (float) (px + pw - 5), (float) (py + ph - 1), 1.0f);

        g.setColour (juce::Colour (0xff444444));
        g.setFont (juce::FontOptions (8.0f).withStyle ("Bold"));
        g.drawText ("VCO  /  VCF  /  VCA", px + 4, py + 3, pw - 8, 12,
                    juce::Justification::centredLeft);
    }

    // ---- Delay section panel --------------------------------------------
    {
        const int px = kDelaySectionX - 6;
        const int py = kTopBarH + 4;
        const int pw = 3 * kColW + 12;
        const int ph = kWindowH - py - 8;

        g.setColour (juce::Colour (0xff3a3a3a));
        g.fillRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f);
        g.setColour (juce::Colour (0x44ffffff));
        g.drawRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f, 1.0f);
    }

    // ---- Divider --------------------------------------------------------
    {
        const int divX = kDelaySectionX - 10;
        g.setColour (juce::Colour (0xff888888));
        g.drawLine ((float) divX, (float) (kTopBarH + 8),
                    (float) divX, (float) (kWindowH - 8), 1.5f);
        g.setColour (juce::Colour (0x55ffffff));
        g.drawLine ((float) (divX + 2), (float) (kTopBarH + 8),
                    (float) (divX + 2), (float) (kWindowH - 8), 0.5f);
    }
}

//==============================================================================
void Acid303AudioProcessorEditor::resized()
{
    const int topY     = kTopBarH + 22;
    const int knobSize = kKnobSize;
    const int labelH   = kLabelH;

    auto placeKnob = [&](juce::Slider& knob, juce::Label& label, int startX, int col)
    {
        const int x = startX + col * kColW;
        knob.setBounds  (x + 6,  topY,               knobSize, knobSize);
        label.setBounds (x,      topY + knobSize + 2, kColW,    labelH);
    };

    // ---- Synth section --------------------------------------------------
    placeKnob (tuningKnob,    tuningLabel,    kSynthStartX, 0);
    placeKnob (cutoffKnob,    cutoffLabel,    kSynthStartX, 1);
    placeKnob (resonanceKnob, resonanceLabel, kSynthStartX, 2);
    placeKnob (envModKnob,    envModLabel,    kSynthStartX, 3);
    placeKnob (decayKnob,     decayLabel,     kSynthStartX, 4);
    placeKnob (accentKnob,    accentLabel,    kSynthStartX, 5);
    placeKnob (volumeKnob,    volumeLabel,    kSynthStartX, 6);

    // Waveform (col 7)
    {
        const int wfX = kSynthStartX + 7 * kColW;
        waveformLabel.setBounds (wfX,      topY,       kColW, labelH);
        sawButton.setBounds     (wfX + 4,  topY + 20,  kColW - 8, 28);
        sqButton.setBounds      (wfX + 4,  topY + 52,  kColW - 8, 28);
    }

    // ---- Delay section --------------------------------------------------
    {
        const int headerY = kTopBarH + 6;
        // "DELAY" title spanning all 3 columns
        delaySectionLabel.setBounds (kDelaySectionX, headerY, 3 * kColW, 14);

        // Division combo box (col 0 of delay section, below header)
        const int comboX = kDelaySectionX + 4;
        const int comboY = topY;
        delayDivLabel.setBounds (kDelaySectionX, comboY - 14, kColW, labelH);
        delayDivCombo.setBounds (comboX, comboY, kColW - 8, 26);

        // Feedback + Mix knobs (cols 1 and 2)
        placeKnob (delayFeedbackKnob, delayFeedbackLabel, kDelaySectionX, 1);
        placeKnob (delayMixKnob,      delayMixLabel,      kDelaySectionX, 2);
    }
}

//==============================================================================
void Acid303AudioProcessorEditor::setupKnob (juce::Slider& knob,
                                              juce::Label& label,
                                              const juce::String& labelText,
                                              juce::Colour labelColour)
{
    knob.setSliderStyle  (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 15);
    knob.setColour (juce::Slider::textBoxTextColourId,       juce::Colour (0xff222222));
    knob.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    knob.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (knob);

    label.setText              (labelText, juce::dontSendNotification);
    label.setFont              (juce::FontOptions (9.5f).withStyle ("Bold"));
    label.setJustificationType (juce::Justification::centred);
    label.setColour            (juce::Label::textColourId, labelColour);
    addAndMakeVisible (label);
}
