#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
// PresetManager
// Handles factory presets (baked-in XML strings) and user preset file I/O.
// Works on top of the plugin's APVTS.
//==============================================================================
class PresetManager
{
public:
    struct Preset
    {
        juce::String name;
        juce::String xml;   // full APVTS state as XML string
    };

    explicit PresetManager (juce::AudioProcessorValueTreeState& apvtsRef)
        : apvts (apvtsRef)
    {
        buildFactoryPresets();
    }

    //==========================================================================
    // Factory presets
    int getNumFactoryPresets() const { return (int) factoryPresets.size(); }

    const juce::String& getFactoryPresetName (int index) const
    {
        return factoryPresets[(size_t) index].name;
    }

    void loadFactoryPreset (int index)
    {
        if (index < 0 || index >= (int) factoryPresets.size()) return;
        applyXml (factoryPresets[(size_t) index].xml);
        currentPresetName = factoryPresets[(size_t) index].name;
        currentPresetIndex = index;
    }

    //==========================================================================
    // User preset file I/O
    void savePresetToFile (const juce::String& name)
    {
        auto dir = getUserPresetDirectory();
        dir.createDirectory();

        auto file = dir.getChildFile (name + ".acid303");
        auto state = apvts.copyState();
        auto xml = state.createXml();
        if (xml)
            xml->writeTo (file);

        currentPresetName  = name;
        currentPresetIndex = -1;
    }

    bool loadPresetFromFile (const juce::File& file)
    {
        auto xml = juce::XmlDocument::parse (file);
        if (!xml) return false;
        applyXml (xml->toString());
        currentPresetName  = file.getFileNameWithoutExtension();
        currentPresetIndex = -1;
        return true;
    }

    juce::Array<juce::File> getUserPresetFiles() const
    {
        juce::Array<juce::File> results;
        getUserPresetDirectory().findChildFiles (results,
            juce::File::findFiles, false, "*.acid303");
        results.sort();
        return results;
    }

    static juce::File getUserPresetDirectory()
    {
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                          .getChildFile ("Acid303").getChildFile ("Presets");
    }

    //==========================================================================
    const juce::String& getCurrentPresetName() const { return currentPresetName; }
    int  getCurrentPresetIndex()               const { return currentPresetIndex; }

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Preset>                 factoryPresets;
    juce::String                        currentPresetName { "Init" };
    int                                 currentPresetIndex { 0 };

    void applyXml (const juce::String& xmlStr)
    {
        auto xml = juce::XmlDocument::parse (xmlStr);
        if (xml)
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            if (tree.isValid())
                apvts.replaceState (tree);
        }
    }

    //==========================================================================
    // Helper: build XML for a preset from a flat list of param-id/value pairs.
    // Pairs must match the APVTS parameter IDs exactly.
    //==========================================================================
    static juce::String makePresetXml (
        const std::vector<std::pair<juce::String, float>>& floatParams,
        const std::vector<std::pair<juce::String, int>>&   choiceParams)
    {
        // We build a ValueTree that looks exactly like apvts.copyState()
        juce::ValueTree root ("Parameters");

        // Each parameter is stored as a child <PARAM id="..." value="..."/>
        for (auto& [id, val] : floatParams)
        {
            juce::ValueTree p ("PARAM");
            p.setProperty ("id",    id,  nullptr);
            p.setProperty ("value", val, nullptr);
            root.addChild (p, -1, nullptr);
        }
        for (auto& [id, val] : choiceParams)
        {
            juce::ValueTree p ("PARAM");
            p.setProperty ("id",    id,  nullptr);
            p.setProperty ("value", val, nullptr);
            root.addChild (p, -1, nullptr);
        }

        auto xml = root.createXml();
        return xml ? xml->toString() : juce::String();
    }

    //==========================================================================
    void buildFactoryPresets()
    {
        // ------------------------------------------------------------------ //
        // Preset format: { paramId, value }                                   //
        // Float params: tuning, cutoff, resonance, envmod, decay,             //
        //               accent, volume, delayFeedback, delayMix               //
        //               lfoRate, lfoDepth                                     //
        // Choice params: waveform (0=SAW,1=SQ), delayDiv (0-7),              //
        //                lfoShape (0=Sine,1=Triangle,2=Saw,3=Square,4=S&H)   //
        //                lfoTarget (0=None,1=Cutoff,2=Resonance,             //
        //                           3=EnvMod,4=Volume,5=Pitch)               //
        // ------------------------------------------------------------------ //

        // 1. Classic Acid
        factoryPresets.push_back ({
            "Classic Acid",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",600.0f},{"resonance",0.82f},
                 {"envmod",0.78f},{"decay",0.18f},{"accent",0.7f},{"volume",0.8f},
                 {"delayFeedback",0.35f},{"delayMix",0.22f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 2. Squelchy Bass
        factoryPresets.push_back ({
            "Squelchy Bass",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",300.0f},{"resonance",0.92f},
                 {"envmod",0.9f},{"decay",0.08f},{"accent",0.85f},{"volume",0.85f},
                 {"delayFeedback",0.0f},{"delayMix",0.0f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 3. Dark Techno
        factoryPresets.push_back ({
            "Dark Techno",
            makePresetXml (
                {{"tuning",-2.0f},{"cutoff",200.0f},{"resonance",0.75f},
                 {"envmod",0.55f},{"decay",0.38f},{"accent",0.55f},{"volume",0.82f},
                 {"delayFeedback",0.5f},{"delayMix",0.28f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",3},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 4. Hypno Square
        factoryPresets.push_back ({
            "Hypno Square",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",900.0f},{"resonance",0.68f},
                 {"envmod",0.42f},{"decay",0.55f},{"accent",0.45f},{"volume",0.78f},
                 {"delayFeedback",0.6f},{"delayMix",0.38f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",1},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 5. Acid House
        factoryPresets.push_back ({
            "Acid House",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",500.0f},{"resonance",0.88f},
                 {"envmod",0.85f},{"decay",0.12f},{"accent",0.78f},{"volume",0.8f},
                 {"delayFeedback",0.42f},{"delayMix",0.3f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",7},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 6. Deep Sub
        factoryPresets.push_back ({
            "Deep Sub",
            makePresetXml (
                {{"tuning",-5.0f},{"cutoff",120.0f},{"resonance",0.3f},
                 {"envmod",0.2f},{"decay",0.7f},{"accent",0.3f},{"volume",0.9f},
                 {"delayFeedback",0.0f},{"delayMix",0.0f},
                 {"lfoRate",0.5f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",0}})
        });

        // 7. Wobble Bass (LFO on cutoff)
        factoryPresets.push_back ({
            "Wobble Bass",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",800.0f},{"resonance",0.72f},
                 {"envmod",0.4f},{"decay",0.4f},{"accent",0.5f},{"volume",0.82f},
                 {"delayFeedback",0.3f},{"delayMix",0.2f},
                 {"lfoRate",2.0f},{"lfoDepth",0.65f}},
                {{"waveform",0},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",1}})
        });

        // 8. Resonant Madness (LFO on resonance)
        factoryPresets.push_back ({
            "Resonant Madness",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",1200.0f},{"resonance",0.6f},
                 {"envmod",0.5f},{"decay",0.25f},{"accent",0.6f},{"volume",0.8f},
                 {"delayFeedback",0.55f},{"delayMix",0.35f},
                 {"lfoRate",3.5f},{"lfoDepth",0.55f}},
                {{"waveform",1},{"delayDiv",4},{"lfoShape",1},{"lfoTarget",2}})
        });

        // 9. Pitch Vibrato
        factoryPresets.push_back ({
            "Pitch Vibrato",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",2000.0f},{"resonance",0.5f},
                 {"envmod",0.3f},{"decay",0.3f},{"accent",0.4f},{"volume",0.8f},
                 {"delayFeedback",0.4f},{"delayMix",0.25f},
                 {"lfoRate",5.0f},{"lfoDepth",0.4f}},
                {{"waveform",0},{"delayDiv",6},{"lfoShape",0},{"lfoTarget",5}})
        });

        // 10. Init
        factoryPresets.push_back ({
            "Init",
            makePresetXml (
                {{"tuning",0.0f},{"cutoff",800.0f},{"resonance",0.5f},
                 {"envmod",0.5f},{"decay",0.3f},{"accent",0.5f},{"volume",0.8f},
                 {"delayFeedback",0.4f},{"delayMix",0.35f},
                 {"lfoRate",1.0f},{"lfoDepth",0.0f}},
                {{"waveform",0},{"delayDiv",5},{"lfoShape",0},{"lfoTarget",0}})
        });
    }
};
