cmake_minimum_required(VERSION 3.22)
project(Acid303 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Universal Binary for macOS (ARM64 + x86_64)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures")
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")

# Add JUCE
add_subdirectory(JUCE)

# Plugin definition
juce_add_plugin(Acid303
    COMPANY_NAME                "AcidLab"
    PLUGIN_MANUFACTURER_CODE    Alab
    PLUGIN_CODE                 Ac03
    FORMATS                     VST3
    PRODUCT_NAME                "Acid303"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             "Instrument|Synth"
    COPY_PLUGIN_AFTER_BUILD     TRUE
)

# Source files
target_sources(Acid303 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/AcidFilter.cpp
)

# Include path
target_include_directories(Acid303 PRIVATE Source)

# Compile definitions
target_compile_definitions(Acid303 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(Acid303 PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
