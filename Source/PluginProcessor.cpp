#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Note-division labels shown in the UI
const juce::StringArray Acid303AudioProcessor::kDivisionLabels
{
    "1/32", "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1/1"
};

// Each value is the number of quarter-note beats for that division
// (1/8T = triplet eighth = 1/3 of a quarter note, etc.)
const float Acid303AudioProcessor::kDivisionBeats[]
{
    0.125f,          // 1/32
    0.25f,           // 1/16
    1.0f / 3.0f,     // 1/8 triplet
    0.5f,            // 1/8
    2.0f / 3.0f,     // 1/4 triplet
    1.0f,            // 1/4
    2.0f,            // 1/2
    4.0f             // 1/1
};

//==============================================================================
Acid303AudioProcessor::Acid303AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    pTuning        = apvts.getRawParameterValue ("tuning");
    pCutoff        = apvts.getRawParameterValue ("cutoff");
    pResonance     = apvts.getRawParameterValue ("resonance");
    pEnvMod        = apvts.getRawParameterValue ("envmod");
    pDecay         = apvts.getRawParameterValue ("decay");
    pAccent        = apvts.getRawParameterValue ("accent");
    pVolume        = apvts.getRawParameterValue ("volume");
    pWaveform      = apvts.getRawParameterValue ("waveform");
    pDelayDiv      = apvts.getRawParameterValue ("delayDiv");
    pDelayFeedback = apvts.getRawParameterValue ("delayFeedback");
    pDelayMix      = apvts.getRawParameterValue ("delayMix");
}

Acid303AudioProcessor::~Acid303AudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
Acid303AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "tuning", "Tuning",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "cutoff", "Cutoff",
        juce::NormalisableRange<float> (20.0f, 15000.0f, 0.1f, 0.3f), 800.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "resonance", "Resonance",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "envmod", "Env Mod",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "decay", "Decay",
        juce::NormalisableRange<float> (0.002f, 2.0f, 0.001f, 0.4f), 0.3f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "accent", "Accent",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "volume", "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.8f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "waveform", "Waveform",
        juce::StringArray { "Sawtooth", "Square" }, 0));

    // Delay tempo division (index into kDivisionLabels)
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "delayDiv", "Delay Division",
        kDivisionLabels, 5));   // default = 1/4

    // Delay feedback: 0 – 0.93
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float> (0.0f, 0.93f, 0.001f), 0.4f));

    // Delay mix: 0 – 1 (default 0.35 so it's audible from the start)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "delayMix", "Delay Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));

    return { params.begin(), params.end() };
}

//==============================================================================
void Acid303AudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;

    osc.prepare (sampleRate);
    filter.prepare (sampleRate);
    ampEnv.prepare (sampleRate);
    filterEnv.prepare (sampleRate);
    accentSlide.prepare (sampleRate);

    ampEnv.setAttack (0.0001f);
    filterEnv.setAttack (0.002f);

    // Allocate delay buffers: max 2 s at current sample rate
    const size_t maxSamples = (size_t) (sampleRate * 2.0) + 2u;
    delayBufferL.assign (maxSamples, 0.0f);
    delayBufferR.assign (maxSamples, 0.0f);
    delayWritePos = 0;
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

    // ---- Read parameters ------------------------------------------------
    const float tuning        = pTuning->load();
    const float cutoff        = pCutoff->load();
    const float resonance     = pResonance->load();
    const float envMod        = pEnvMod->load();
    const float decay         = pDecay->load();
    const float accent        = pAccent->load();
    const float volume        = pVolume->load();
    const int   wfChoice      = (int) pWaveform->load();
    const int   divIndex      = juce::jlimit (0, 7, (int) pDelayDiv->load());
    const float delayFeedback = juce::jlimit (0.0f, 0.93f, pDelayFeedback->load());
    const float delayMix      = juce::jlimit (0.0f, 1.0f,  pDelayMix->load());

    ampEnv.setDecay (decay);
    filterEnv.setDecay (decay);

    osc.waveform = (wfChoice == 0) ? Oscillator::Waveform::Sawtooth
                                   : Oscillator::Waveform::Square;

    // ---- Compute delay time in samples from BPM + division --------------
    double bpm = 120.0; // fallback
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (pos->getBpm().hasValue())
                bpm = *pos->getBpm();
    }

    const double beatsPerSecond = bpm / 60.0;
    const double divBeats       = (double) kDivisionBeats[divIndex];
    const double delayTimeSec   = divBeats / beatsPerSecond;

    const int delayBufSize = (int) delayBufferL.size();
    const int delaySamples = juce::jlimit (1, delayBufSize - 1,
                                           (int) (delayTimeSec * currentSampleRate));

    // ---- Output pointers ------------------------------------------------
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    // ---- MIDI iterator --------------------------------------------------
    auto midiIterator = midiMessages.begin();
    int  nextMidiPos  = (midiIterator != midiMessages.end())
                            ? (*midiIterator).samplePosition : numSamples;

    // ---- Sample loop ----------------------------------------------------
    for (int i = 0; i < numSamples; ++i)
    {
        // Handle MIDI events at this sample position
        while (i >= nextMidiPos && midiIterator != midiMessages.end())
        {
            const auto msg = (*midiIterator).getMessage();

            if (msg.isNoteOn() && msg.getVelocity() > 0)
            {
                const int  note     = msg.getNoteNumber();
                const int  vel      = msg.getVelocity();
                const bool doSlide  = accentSlide.noteOn (note, vel);
                const bool isAccent = accentSlide.isAccentActive();

                osc.noteOn (note, doSlide);

                if (doSlide)
                    filterEnv.noteOn();
                else
                {
                    ampEnv.noteOn();
                    filterEnv.noteOn();
                }

                filterEnv.setDecay (isAccent ? decay * 0.5f : decay);

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

        // --- Synth chain -------------------------------------------------
        float sample      = osc.process (tuning);
        float ampLevel    = ampEnv.process();
        float filterLevel = filterEnv.process();
        float accentGain  = accentSlide.processAccentGain (accent);
        float extraEnvMod = accentSlide.accentEnvModBoost (accent);

        sample *= ampLevel * accentGain;
        sample  = filter.processSample (sample, cutoff, resonance,
                                        envMod + extraEnvMod, filterLevel);
        sample *= volume;

        // --- Stereo ping-pong delay ---------------------------------------
        //
        // Read the two taps from the circular buffer
        const int readPos = (delayWritePos - delaySamples + delayBufSize) % delayBufSize;
        const float tapL  = delayBufferL[(size_t) readPos];
        const float tapR  = delayBufferR[(size_t) readPos];

        // Write: ping-pong — current dry feeds the OPPOSITE channel's buffer,
        // so the wet signal bounces L→R→L→R each repeat.
        // Crucially we do NOT add the dry signal here; it enters the mix below.
        delayBufferL[(size_t) delayWritePos] = sample + tapR * delayFeedback;
        delayBufferR[(size_t) delayWritePos] = sample + tapL * delayFeedback;

        delayWritePos = (delayWritePos + 1) % delayBufSize;

        // Dry/wet mix
        outL[i] = sample * (1.0f - delayMix) + tapL * delayMix;
        if (outR != nullptr)
            outR[i] = sample * (1.0f - delayMix) + tapR * delayMix;
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
