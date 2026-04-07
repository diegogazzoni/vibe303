#include "PluginEditor.h"

//==============================================================================
Acid303AudioProcessorEditor::Acid303AudioProcessorEditor (Acid303AudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&acidLookAndFeel);

    // Set up each knob + label
    setupKnob (tuningKnob,    tuningLabel,    "TUNING");
    setupKnob (cutoffKnob,    cutoffLabel,    "CUTOFF");
    setupKnob (resonanceKnob, resonanceLabel, "RESONANCE");
    setupKnob (envModKnob,    envModLabel,    "ENV MOD");
    setupKnob (decayKnob,     decayLabel,     "DECAY");
    setupKnob (accentKnob,    accentLabel,    "ACCENT");
    setupKnob (volumeKnob,    volumeLabel,    "VOLUME");

    // Waveform toggle buttons
    sawButton.setClickingTogglesState (true);
    sqButton.setClickingTogglesState  (true);
    sawButton.setRadioGroupId (1);
    sqButton.setRadioGroupId  (1);
    sawButton.setToggleState  (true, juce::dontSendNotification);

    sawButton.setColour (juce::TextButton::buttonOnColourId,  juce::Colour (0xffcc6600));
    sawButton.setColour (juce::TextButton::buttonColourId,    juce::Colour (0xff333333));
    sawButton.setColour (juce::TextButton::textColourOnId,    juce::Colours::white);
    sawButton.setColour (juce::TextButton::textColourOffId,   juce::Colour (0xffaaaaaa));
    sqButton.setColour  (juce::TextButton::buttonOnColourId,  juce::Colour (0xffcc6600));
    sqButton.setColour  (juce::TextButton::buttonColourId,    juce::Colour (0xff333333));
    sqButton.setColour  (juce::TextButton::textColourOnId,    juce::Colours::white);
    sqButton.setColour  (juce::TextButton::textColourOffId,   juce::Colour (0xffaaaaaa));

    addAndMakeVisible (sawButton);
    addAndMakeVisible (sqButton);

    waveformLabel.setText ("WAVE", juce::dontSendNotification);
    waveformLabel.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    waveformLabel.setJustificationType (juce::Justification::centred);
    waveformLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    addAndMakeVisible (waveformLabel);

    // Waveform combo (hidden — drives the parameter; buttons control it via listener)
    waveformCombo.addItem ("Sawtooth", 1);
    waveformCombo.addItem ("Square",   2);
    waveformCombo.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (waveformCombo);
    waveformCombo.setVisible (false);

    // Sync buttons → combo
    sawButton.onClick = [this]
    {
        if (sawButton.getToggleState())
            waveformCombo.setSelectedId (1);
    };
    sqButton.onClick = [this]
    {
        if (sqButton.getToggleState())
            waveformCombo.setSelectedId (2);
    };

    // APVTS attachments
    auto& apvts = processorRef.apvts;
    tuningAttach    = std::make_unique<SliderAttachment> (apvts, "tuning",    tuningKnob);
    cutoffAttach    = std::make_unique<SliderAttachment> (apvts, "cutoff",    cutoffKnob);
    resonanceAttach = std::make_unique<SliderAttachment> (apvts, "resonance", resonanceKnob);
    envModAttach    = std::make_unique<SliderAttachment> (apvts, "envmod",    envModKnob);
    decayAttach     = std::make_unique<SliderAttachment> (apvts, "decay",     decayKnob);
    accentAttach    = std::make_unique<SliderAttachment> (apvts, "accent",    accentKnob);
    volumeAttach    = std::make_unique<SliderAttachment> (apvts, "volume",    volumeKnob);
    waveformAttach  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
                          apvts, "waveform", waveformCombo);

    // Keep button state in sync with the parameter (e.g. after preset load)
    waveformCombo.onChange = [this]
    {
        bool isSaw = (waveformCombo.getSelectedId() == 1);
        sawButton.setToggleState ( isSaw, juce::dontSendNotification);
        sqButton.setToggleState  (!isSaw, juce::dontSendNotification);
    };

    setSize (680, 200);
}

Acid303AudioProcessorEditor::~Acid303AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void Acid303AudioProcessorEditor::paint (juce::Graphics& g)
{
    // 303-style dark panel background
    g.fillAll (juce::Colour (0xff1a1a1a));

    // Top stripe
    g.setColour (juce::Colour (0xff222222));
    g.fillRect (0, 0, getWidth(), 30);

    // Plugin name
    g.setColour (juce::Colour (0xffcc6600));
    g.setFont   (juce::FontOptions (18.0f).withStyle ("Bold"));
    g.drawText  ("ACID 303", 10, 5, 120, 22, juce::Justification::centredLeft);

    // Subtitle
    g.setColour (juce::Colour (0xff888888));
    g.setFont   (juce::FontOptions (10.0f));
    g.drawText  ("Bass Line Synthesizer", 10, 20, 200, 12, juce::Justification::centredLeft);
}

//==============================================================================
void Acid303AudioProcessorEditor::resized()
{
    // Layout: 7 knobs + waveform section in a row
    // Each knob column is 80 px wide; waveform section is 80 px
    const int topY      = 35;
    const int knobSize  = 70;
    const int labelH    = 18;
    const int colWidth  = 80;
    const int startX    = 20;

    auto placeKnob = [&](juce::Slider& knob, juce::Label& label, int col)
    {
        int x = startX + col * colWidth;
        knob.setBounds  (x + 5,  topY,          knobSize, knobSize);
        label.setBounds (x,      topY + knobSize + 2, colWidth, labelH);
    };

    placeKnob (tuningKnob,    tuningLabel,    0);
    placeKnob (cutoffKnob,    cutoffLabel,    1);
    placeKnob (resonanceKnob, resonanceLabel, 2);
    placeKnob (envModKnob,    envModLabel,    3);
    placeKnob (decayKnob,     decayLabel,     4);
    placeKnob (accentKnob,    accentLabel,    5);
    placeKnob (volumeKnob,    volumeLabel,    6);

    // Waveform section (col 7)
    const int wfX = startX + 7 * colWidth;
    waveformLabel.setBounds (wfX, topY,          colWidth, labelH);
    sawButton.setBounds     (wfX, topY + 22,     colWidth, 30);
    sqButton.setBounds      (wfX, topY + 56,     colWidth, 30);
}

//==============================================================================
void Acid303AudioProcessorEditor::setupKnob (juce::Slider& knob,
                                              juce::Label& label,
                                              const juce::String& labelText)
{
    knob.setSliderStyle  (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
    knob.setColour       (juce::Slider::textBoxTextColourId,    juce::Colour (0xffaaaaaa));
    knob.setColour       (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (knob);

    label.setText              (labelText, juce::dontSendNotification);
    label.setFont              (juce::FontOptions (10.0f).withStyle ("Bold"));
    label.setJustificationType (juce::Justification::centred);
    label.setColour            (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    addAndMakeVisible (label);
}
