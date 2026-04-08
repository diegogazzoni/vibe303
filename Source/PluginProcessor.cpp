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

// LFO shape names (index matches lfoShape Choice parameter)
const juce::StringArray Acid303AudioProcessor::kLfoShapeLabels
{
    "Sine", "Triangle", "Sawtooth", "Square", "S&H"
};

// LFO target names (index matches lfoTarget Choice parameter)
const juce::StringArray Acid303AudioProcessor::kLfoTargetLabels
{
    "None", "Cutoff", "Resonance", "Env Mod", "Volume", "Pitch"
};

//==============================================================================
Acid303AudioProcessor::Acid303AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout()),
      presetManager (apvts)
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
    pLfoRate       = apvts.getRawParameterValue ("lfoRate");
    pLfoDepth      = apvts.getRawParameterValue ("lfoDepth");
    pLfoShape      = apvts.getRawParameterValue ("lfoShape");
    pLfoTarget     = apvts.getRawParameterValue ("lfoTarget");
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

    // ---- LFO parameters -------------------------------------------------
    // Rate: 0.05 – 20 Hz
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "lfoRate", "LFO Rate",
        juce::NormalisableRange<float> (0.05f, 20.0f, 0.01f, 0.4f), 1.0f));

    // Depth: 0 – 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "lfoDepth", "LFO Depth",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

    // Shape: Sine / Triangle / Sawtooth / Square / S&H
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "lfoShape", "LFO Shape",
        kLfoShapeLabels, 0));

    // Target: None / Cutoff / Resonance / Env Mod / Volume / Pitch
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "lfoTarget", "LFO Target",
        kLfoTargetLabels, 0));

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
    lfo.prepare (sampleRate);

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
    lfo.reset();
}

//==============================================================================
void Acid303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    // ---- Read parameters ------------------------------------------------
    float tuning        = pTuning->load();
    float cutoff        = pCutoff->load();
    float resonance     = pResonance->load();
    float envMod        = pEnvMod->load();
    const float decay         = pDecay->load();
    const float accent        = pAccent->load();
    float volume        = pVolume->load();
    const int   wfChoice      = (int) pWaveform->load();
    const int   divIndex      = juce::jlimit (0, 7, (int) pDelayDiv->load());
    const float delayFeedback = juce::jlimit (0.0f, 0.93f, pDelayFeedback->load());
    const float delayMix      = juce::jlimit (0.0f, 1.0f,  pDelayMix->load());

    // LFO params
    const float     lfoRate   = pLfoRate->load();
    const float     lfoDepth  = pLfoDepth->load();
    const int       lfoShape  = juce::jlimit (0, 4, (int) pLfoShape->load());
    const int       lfoTarget = juce::jlimit (0, 5, (int) pLfoTarget->load());

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

    // LFO shape enum
    const LFO::Shape lfoShapeEnum = static_cast<LFO::Shape> (lfoShape);

    // Last LFO value for editor display
    float lastLfoVal = 0.0f;

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

        // --- LFO tick ----------------------------------------------------
        const float lfoRaw = lfo.process (lfoRate, lfoShapeEnum); // [-1, +1]
        lastLfoVal = lfoRaw;

        // Apply modulation to the base parameter values
        float modCutoff    = cutoff;
        float modResonance = resonance;
        float modEnvMod    = envMod;
        float modVolume    = volume;
        float modTuning    = tuning;

        if (lfoTarget > 0 && lfoDepth > 0.0f)
        {
            const float mod = lfoRaw * lfoDepth;

            switch (lfoTarget)
            {
                case 1: // Cutoff — modulate ±5 octaves on log scale
                {
                    // Map mod [-1,+1] → multiply cutoff by 2^(mod * 3)  (±3 octaves)
                    float factor = std::pow (2.0f, mod * 3.0f);
                    modCutoff = juce::jlimit (20.0f, 15000.0f, cutoff * factor);
                    break;
                }
                case 2: // Resonance ±1 (clamped 0–1)
                    modResonance = juce::jlimit (0.0f, 1.0f, resonance + mod);
                    break;

                case 3: // Env Mod ±1 (clamped 0–1)
                    modEnvMod = juce::jlimit (0.0f, 1.0f, envMod + mod);
                    break;

                case 4: // Volume ±0.5 (clamped 0–1)
                    modVolume = juce::jlimit (0.0f, 1.0f, volume + mod * 0.5f);
                    break;

                case 5: // Pitch ±2 semitones
                    modTuning = tuning + mod * 2.0f;
                    break;

                default: break;
            }
        }

        // --- Synth chain -------------------------------------------------
        float sample      = osc.process (modTuning);
        float ampLevel    = ampEnv.process();
        float filterLevel = filterEnv.process();
        float accentGain  = accentSlide.processAccentGain (accent);
        float extraEnvMod = accentSlide.accentEnvModBoost (accent);

        sample *= ampLevel * accentGain;
        sample  = filter.processSample (sample, modCutoff, modResonance,
                                        modEnvMod + extraEnvMod, filterLevel);
        sample *= modVolume;

        // --- Stereo ping-pong delay ---------------------------------------
        const int readPos = (delayWritePos - delaySamples + delayBufSize) % delayBufSize;
        const float tapL  = delayBufferL[(size_t) readPos];
        const float tapR  = delayBufferR[(size_t) readPos];

        delayBufferL[(size_t) delayWritePos] = sample + tapR * delayFeedback;
        delayBufferR[(size_t) delayWritePos] = sample + tapL * delayFeedback;

        delayWritePos = (delayWritePos + 1) % delayBufSize;

        outL[i] = sample * (1.0f - delayMix) + tapL * delayMix;
        if (outR != nullptr)
            outR[i] = sample * (1.0f - delayMix) + tapR * delayMix;
    }

    // Publish last LFO value for editor display (atomic write)
    lfoOutputValue.store (lastLfoVal);
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
