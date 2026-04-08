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

    // ---- Preset bar -----------------------------------------------------
    presetSectionLabel.setText ("PRESET", juce::dontSendNotification);
    presetSectionLabel.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    presetSectionLabel.setJustificationType (juce::Justification::centredLeft);
    presetSectionLabel.setColour (juce::Label::textColourId,
                                   juce::Colour (Acid303LookAndFeel::kOrange));
    addAndMakeVisible (presetSectionLabel);

    rebuildPresetCombo();
    presetCombo.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (presetCombo);

    presetSaveButton.onClick = [this] { onSavePreset(); };
    addAndMakeVisible (presetSaveButton);

    presetLoadButton.onClick = [this] { onLoadPreset(); };
    addAndMakeVisible (presetLoadButton);

    // ---- LFO section ----------------------------------------------------
    lfoSectionLabel.setText ("LFO", juce::dontSendNotification);
    lfoSectionLabel.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    lfoSectionLabel.setJustificationType (juce::Justification::centred);
    lfoSectionLabel.setColour (juce::Label::textColourId,
                                juce::Colour (Acid303LookAndFeel::kOrange));
    addAndMakeVisible (lfoSectionLabel);

    addAndMakeVisible (lfoDisplay);

    // Rate knob
    setupSmallKnob (lfoRateKnob,  lfoRateLabel,  "RATE",  juce::Colour (0xffdddddd));
    // Depth knob
    setupSmallKnob (lfoDepthKnob, lfoDepthLabel, "DEPTH", juce::Colour (0xffdddddd));

    // Shape combo
    for (int i = 0; i < Acid303AudioProcessor::kLfoShapeLabels.size(); ++i)
        lfoShapeCombo.addItem (Acid303AudioProcessor::kLfoShapeLabels[i], i + 1);
    lfoShapeCombo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lfoShapeCombo);

    lfoShapeLabel.setText ("SHAPE", juce::dontSendNotification);
    lfoShapeLabel.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    lfoShapeLabel.setJustificationType (juce::Justification::centred);
    lfoShapeLabel.setColour (juce::Label::textColourId, juce::Colour (0xffdddddd));
    addAndMakeVisible (lfoShapeLabel);

    // Target combo
    for (int i = 0; i < Acid303AudioProcessor::kLfoTargetLabels.size(); ++i)
        lfoTargetCombo.addItem (Acid303AudioProcessor::kLfoTargetLabels[i], i + 1);
    lfoTargetCombo.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lfoTargetCombo);

    lfoTargetLabel.setText ("TARGET", juce::dontSendNotification);
    lfoTargetLabel.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    lfoTargetLabel.setJustificationType (juce::Justification::centred);
    lfoTargetLabel.setColour (juce::Label::textColourId, juce::Colour (0xffdddddd));
    addAndMakeVisible (lfoTargetLabel);

    // Keep LFO display shape in sync with combo
    lfoShapeCombo.onChange = [this]
    {
        lfoDisplay.setShape (lfoShapeCombo.getSelectedId() - 1);
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

    lfoRateAttach   = std::make_unique<SliderAttachment>   (apvts, "lfoRate",   lfoRateKnob);
    lfoDepthAttach  = std::make_unique<SliderAttachment>   (apvts, "lfoDepth",  lfoDepthKnob);
    lfoShapeAttach  = std::make_unique<ComboBoxAttachment> (apvts, "lfoShape",  lfoShapeCombo);
    lfoTargetAttach = std::make_unique<ComboBoxAttachment> (apvts, "lfoTarget", lfoTargetCombo);

    // Sync shape display to the loaded value
    lfoDisplay.setShape (lfoShapeCombo.getSelectedId() - 1);

    // 30 Hz timer to update LFO display
    startTimerHz (30);

    setSize (kWindowW, kWindowH);
}

Acid303AudioProcessorEditor::~Acid303AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void Acid303AudioProcessorEditor::timerCallback()
{
    const float lfoVal   = processorRef.lfoOutputValue.load();
    const float lfoRate  = (float) lfoRateKnob.getValue();
    const float lfoDepth = (float) lfoDepthKnob.getValue();

    lfoDisplay.setRate  (lfoRate);
    lfoDisplay.setDepth (lfoDepth);
    lfoDisplay.setLfoValue (lfoVal);
    lfoDisplay.setShape (lfoShapeCombo.getSelectedId() - 1);
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

    // ---- Preset bar background ------------------------------------------
    {
        const int py = kTopBarH;
        g.setColour (juce::Colour (0xff252525));
        g.fillRect (0, py, getWidth(), kPresetBarH);
        g.setColour (juce::Colour (0xff444444));
        g.drawHorizontalLine (py + kPresetBarH - 1, 0.0f, (float) getWidth());
    }

    // ---- Synth section panel --------------------------------------------
    {
        const int px = kSynthStartX - 6;
        const int py = kTopBarH + kPresetBarH + 4;
        const int pw = 8 * kColW + 6;
        const int ph = kSynthPanelH - 8;

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
        const int py = kTopBarH + kPresetBarH + 4;
        const int pw = 3 * kColW + 12;
        const int ph = kSynthPanelH - 8;

        g.setColour (juce::Colour (0xff3a3a3a));
        g.fillRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f);
        g.setColour (juce::Colour (0x44ffffff));
        g.drawRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f, 1.0f);
    }

    // ---- Divider between synth and delay --------------------------------
    {
        const int divX = kDelaySectionX - 10;
        const int divY0 = kTopBarH + kPresetBarH + 8;
        const int divY1 = kTopBarH + kPresetBarH + kSynthPanelH - 8;
        g.setColour (juce::Colour (0xff888888));
        g.drawLine ((float) divX, (float) divY0, (float) divX, (float) divY1, 1.5f);
        g.setColour (juce::Colour (0x55ffffff));
        g.drawLine ((float) (divX + 2), (float) divY0, (float) (divX + 2), (float) divY1, 0.5f);
    }

    // ---- LFO panel ------------------------------------------------------
    {
        const int px = kSynthStartX - 6;
        const int py = lfoRowY() - 4;
        const int pw = getWidth() - px - 8;
        const int ph = kLfoPanelH - 4;

        g.setColour (juce::Colour (0xff2a2a2a));
        g.fillRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f);
        g.setColour (juce::Colour (0x44ffffff));
        g.drawRoundedRectangle ((float) px, (float) py, (float) pw, (float) ph, 5.0f, 1.0f);
    }
}

//==============================================================================
void Acid303AudioProcessorEditor::resized()
{
    const int topY     = synthRowY();
    const int knobSize = kKnobSize;
    const int labelH   = kLabelH;

    auto placeKnob = [&](juce::Slider& knob, juce::Label& label, int startX, int col)
    {
        const int x = startX + col * kColW;
        knob.setBounds  (x + 6,  topY,               knobSize, knobSize);
        label.setBounds (x,      topY + knobSize + 2, kColW,    labelH);
    };

    // ---- Preset bar layout ----------------------------------------------
    {
        const int barY = kTopBarH + 2;
        const int barH = kPresetBarH - 4;

        presetSectionLabel.setBounds (10, barY, 48, barH);
        presetCombo.setBounds        (62, barY, 280, barH);
        presetSaveButton.setBounds   (350, barY, 50, barH);
        presetLoadButton.setBounds   (406, barY, 50, barH);
    }

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
        const int headerY = kTopBarH + kPresetBarH + 6;
        delaySectionLabel.setBounds (kDelaySectionX, headerY, 3 * kColW, 14);

        const int comboX = kDelaySectionX + 4;
        const int comboY = topY;
        delayDivLabel.setBounds (kDelaySectionX, comboY - 14, kColW, labelH);
        delayDivCombo.setBounds (comboX, comboY, kColW - 8, 26);

        placeKnob (delayFeedbackKnob, delayFeedbackLabel, kDelaySectionX, 1);
        placeKnob (delayMixKnob,      delayMixLabel,      kDelaySectionX, 2);
    }

    // ---- LFO section ----------------------------------------------------
    {
        const int lfoY   = lfoRowY();
        // Section label
        lfoSectionLabel.setBounds (kSynthStartX, lfoY - 2, 60, 16);

        // LFO display — large waveform visualiser on the left
        const int displayW = 260;
        const int displayH = kLfoPanelH - 28;
        lfoDisplay.setBounds (kSynthStartX, lfoY + 18, displayW, displayH);

        // Controls to the right of the display
        const int ctrlX = kSynthStartX + displayW + 12;
        const int smallKnobSize = kSmallKnobSize;
        const int colSp = 80;

        // Rate knob
        lfoRateKnob.setBounds   (ctrlX,              lfoY + 18, smallKnobSize, smallKnobSize);
        lfoRateLabel.setBounds  (ctrlX - 4,          lfoY + 18 + smallKnobSize + 2, colSp, labelH);

        // Depth knob
        lfoDepthKnob.setBounds  (ctrlX + colSp,      lfoY + 18, smallKnobSize, smallKnobSize);
        lfoDepthLabel.setBounds (ctrlX + colSp - 4,  lfoY + 18 + smallKnobSize + 2, colSp, labelH);

        // Shape combo
        const int comboY = lfoY + 18;
        const int comboH = 24;
        const int comboW = 90;
        const int shapeX = ctrlX + colSp * 2 + 4;

        lfoShapeLabel.setBounds  (shapeX, comboY,          comboW, labelH);
        lfoShapeCombo.setBounds  (shapeX, comboY + labelH + 2, comboW, comboH);

        // Target combo
        const int targetX = shapeX + comboW + 8;
        lfoTargetLabel.setBounds (targetX, comboY,          comboW, labelH);
        lfoTargetCombo.setBounds (targetX, comboY + labelH + 2, comboW, comboH);
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

void Acid303AudioProcessorEditor::setupSmallKnob (juce::Slider& knob,
                                                   juce::Label& label,
                                                   const juce::String& labelText,
                                                   juce::Colour labelColour)
{
    knob.setSliderStyle  (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 13);
    knob.setColour (juce::Slider::textBoxTextColourId,       juce::Colour (0xffdddddd));
    knob.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    knob.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (knob);

    label.setText              (labelText, juce::dontSendNotification);
    label.setFont              (juce::FontOptions (9.5f).withStyle ("Bold"));
    label.setJustificationType (juce::Justification::centred);
    label.setColour            (juce::Label::textColourId, labelColour);
    addAndMakeVisible (label);
}

//==============================================================================
void Acid303AudioProcessorEditor::rebuildPresetCombo()
{
    presetCombo.clear (juce::dontSendNotification);

    auto& pm = processorRef.presetManager;
    int id = 1;

    // Factory presets group
    presetCombo.addSectionHeading ("Factory");
    for (int i = 0; i < pm.getNumFactoryPresets(); ++i)
        presetCombo.addItem (pm.getFactoryPresetName (i), id++);

    // User presets group
    auto userFiles = pm.getUserPresetFiles();
    if (userFiles.size() > 0)
    {
        presetCombo.addSeparator();
        presetCombo.addSectionHeading ("User");
        for (auto& f : userFiles)
            presetCombo.addItem (f.getFileNameWithoutExtension(), id++);
    }

    presetCombo.onChange = [this]
    {
        const int selectedId = presetCombo.getSelectedId();
        if (selectedId <= 0) return;

        auto& pm2 = processorRef.presetManager;
        const int numFactory = pm2.getNumFactoryPresets();

        // IDs 1..numFactory → factory; above → user files
        const int factoryIdx = selectedId - 1;
        if (factoryIdx < numFactory)
        {
            pm2.loadFactoryPreset (factoryIdx);
        }
        else
        {
            auto userFiles2 = pm2.getUserPresetFiles();
            const int userIdx = factoryIdx - numFactory;
            if (userIdx >= 0 && userIdx < userFiles2.size())
                pm2.loadPresetFromFile (userFiles2[userIdx]);
        }
        // Sync LFO display shape after preset load
        lfoDisplay.setShape (lfoShapeCombo.getSelectedId() - 1);
    };

    // Select whatever is currently loaded
    const int curIdx = pm.getCurrentPresetIndex();
    if (curIdx >= 0)
        presetCombo.setSelectedId (curIdx + 1, juce::dontSendNotification);
}

//==============================================================================
void Acid303AudioProcessorEditor::onSavePreset()
{
    auto* alertWindow = new juce::AlertWindow ("Save Preset",
                                               "Enter a name for the preset:",
                                               juce::MessageBoxIconType::NoIcon);
    alertWindow->addTextEditor ("name", processorRef.presetManager.getCurrentPresetName());
    alertWindow->addButton ("Save",   1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    alertWindow->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, alertWindow] (int result)
        {
            if (result == 1)
            {
                auto name = alertWindow->getTextEditorContents ("name").trim();
                if (name.isNotEmpty())
                {
                    processorRef.presetManager.savePresetToFile (name);
                    rebuildPresetCombo();
                }
            }
        }), true);
}

//==============================================================================
void Acid303AudioProcessorEditor::onLoadPreset()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Load Preset",
        PresetManager::getUserPresetDirectory(),
        "*.acid303");

    chooser->launchAsync (juce::FileBrowserComponent::openMode |
                          juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
            {
                processorRef.presetManager.loadPresetFromFile (result);
                rebuildPresetCombo();
                lfoDisplay.setShape (lfoShapeCombo.getSelectedId() - 1);
            }
        });
}
