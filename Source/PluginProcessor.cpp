#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Acid303AudioProcessor::Acid303AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    pTuning    = apvts.getRawParameterValue ("tuning");
    pCutoff    = apvts.getRawParameterValue ("cutoff");
    pResonance = apvts.getRawParameterValue ("resonance");
    pEnvMod    = apvts.getRawParameterValue ("envmod");
    pDecay     = apvts.getRawParameterValue ("decay");
    pAccent    = apvts.getRawParameterValue ("accent");
    pVolume    = apvts.getRawParameterValue ("volume");
    pWaveform  = apvts.getRawParameterValue ("waveform");
}

Acid303AudioProcessor::~Acid303AudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
Acid303AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Tuning: ±12 semitones (displayed as semitones on knob)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "tuning", "Tuning",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));

    // Cutoff: 20 Hz – 15 kHz, skewed toward lower values (log scale)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "cutoff", "Cutoff",
        juce::NormalisableRange<float> (20.0f, 15000.0f, 0.1f, 0.3f), 800.0f));

    // Resonance: 0 – 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "resonance", "Resonance",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    // Env Mod: 0 – 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "envmod", "Env Mod",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    // Decay: 2 ms – 2000 ms, skewed
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "decay", "Decay",
        juce::NormalisableRange<float> (0.002f, 2.0f, 0.001f, 0.4f), 0.3f));

    // Accent: 0 – 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "accent", "Accent",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    // Volume: 0 – 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "volume", "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.8f));

    // Waveform: 0 = Sawtooth, 1 = Square
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "waveform", "Waveform",
        juce::StringArray { "Sawtooth", "Square" }, 0));

    return { params.begin(), params.end() };
}

//==============================================================================
void Acid303AudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    osc.prepare (sampleRate);
    filter.prepare (sampleRate);
    ampEnv.prepare (sampleRate);
    filterEnv.prepare (sampleRate);
    accentSlide.prepare (sampleRate);

    // Amplitude envelope: virtually instant attack
    ampEnv.setAttack (0.0001f);

    // Filter envelope: slightly slower attack (~2 ms) for smoother sweep
    filterEnv.setAttack (0.002f);
}

void Acid303AudioProcessor::releaseResources()
{
    osc.reset();
    filter.reset();
    accentSlide.reset();
}

//==============================================================================
void Acid303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    // Read parameters
    const float tuning    = pTuning->load();
    const float cutoff    = pCutoff->load();
    const float resonance = pResonance->load();
    const float envMod    = pEnvMod->load();
    const float decay     = pDecay->load();
    const float accent    = pAccent->load();
    const float volume    = pVolume->load();
    const int   wfChoice  = (int) pWaveform->load();

    // Update decay on envelopes
    // When accent is active, filter decay is ~50% faster
    ampEnv.setDecay (decay);
    filterEnv.setDecay (decay);

    // Set waveform
    osc.waveform = (wfChoice == 0) ? Oscillator::Waveform::Sawtooth
                                   : Oscillator::Waveform::Square;

    // Get a write pointer to the output (we write mono then copy to stereo)
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    auto midiIterator = midiMessages.begin();
    int  nextMidiPos  = (midiIterator != midiMessages.end())
                            ? (*midiIterator).samplePosition : numSamples;

    for (int i = 0; i < numSamples; ++i)
    {
        // Handle all MIDI events at this sample position
        while (i >= nextMidiPos && midiIterator != midiMessages.end())
        {
            const auto metadata = *midiIterator;
            const auto msg      = metadata.getMessage();

            if (msg.isNoteOn() && msg.getVelocity() > 0)
            {
                const int  note     = msg.getNoteNumber();
                const int  vel      = msg.getVelocity();
                const bool doSlide  = accentSlide.noteOn (note, vel);
                const bool isAccent = accentSlide.isAccentActive();

                osc.noteOn (note, doSlide);

                if (doSlide)
                {
                    // Slide: do NOT retrigger envelope — keep gate open
                    // Just retrigger filter env for the sweep
                    filterEnv.noteOn();
                }
                else
                {
                    ampEnv.noteOn();
                    filterEnv.noteOn();
                }

                // If accent, shorten filter decay for snappier sweep
                if (isAccent)
                    filterEnv.setDecay (decay * 0.5f);
                else
                    filterEnv.setDecay (decay);

                currentNote = note;
                noteIsOn    = true;
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            {
                accentSlide.noteOff (msg.getNoteNumber());
                if (msg.getNoteNumber() == currentNote)
                {
                    ampEnv.noteOff();
                    filterEnv.noteOff();
                    noteIsOn = false;
                }
            }

            ++midiIterator;
            nextMidiPos = (midiIterator != midiMessages.end())
                              ? (*midiIterator).samplePosition : numSamples;
        }

        // Generate oscillator sample
        float sample = osc.process (tuning);

        // Amplitude envelope
        float ampLevel    = ampEnv.process();
        float filterLevel = filterEnv.process();

        // Accent gain boost
        float accentGain = accentSlide.processAccentGain (accent);
        float extraEnvMod = accentSlide.accentEnvModBoost (accent);

        sample *= ampLevel * accentGain;

        // Filter
        sample = filter.processSample (sample,
                                       cutoff,
                                       resonance,
                                       envMod + extraEnvMod,
                                       filterLevel);

        // Master volume
        sample *= volume;

        outL[i] = sample;
        if (outR != nullptr)
            outR[i] = sample;
    }
}

//==============================================================================
juce::AudioProcessorEditor* Acid303AudioProcessor::createEditor()
{
    return new Acid303AudioProcessorEditor (*this);
}

//==============================================================================
void Acid303AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Acid303AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Acid303AudioProcessor();
}
