#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Oscillator.h"
#include "AcidFilter.h"
#include "AcidEnvelope.h"
#include "AccentSlide.h"

//==============================================================================
class Acid303AudioProcessor : public juce::AudioProcessor
{
public:
    Acid303AudioProcessor();
    ~Acid303AudioProcessor() override;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return "Acid303"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    //==========================================================================
    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // Parameter layout (APVTS)
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

private:
    //==========================================================================
    // DSP objects
    Oscillator   osc;
    AcidFilter   filter;
    AcidEnvelope ampEnv;
    AcidEnvelope filterEnv;
    AccentSlide  accentSlide;

    // Cached parameter pointers (updated each block for thread-safety)
    std::atomic<float>* pTuning    = nullptr;
    std::atomic<float>* pCutoff    = nullptr;
    std::atomic<float>* pResonance = nullptr;
    std::atomic<float>* pEnvMod    = nullptr;
    std::atomic<float>* pDecay     = nullptr;
    std::atomic<float>* pAccent    = nullptr;
    std::atomic<float>* pVolume    = nullptr;
    std::atomic<float>* pWaveform  = nullptr;

    // Per-block sample processing
    void processIncomingMidi (const juce::MidiBuffer& midi, int samplePos);
    float renderSample();

    int   currentNote     = -1;
    bool  noteIsOn        = false;
    float masterGain      = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Acid303AudioProcessor)
};
