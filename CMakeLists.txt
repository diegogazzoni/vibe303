cmake_minimum_required(VERSION 3.22)
project(Acid303 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Platform-specific settings
# ==============================================================================

if(APPLE)
    # Universal Binary: native on both Apple Silicon and Intel
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
endif()

if(WIN32)
    # Use the static MSVC runtime so the plugin has no external DLL dependencies
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(UNIX AND NOT APPLE)
    # Linux: require position-independent code (needed for shared libs/plugins)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# ==============================================================================
# JUCE
# ==============================================================================
add_subdirectory(JUCE)

# ==============================================================================
# Plugin formats
#   macOS   → VST3
#   Windows → VST3
#   Linux   → VST3
# ==============================================================================
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

# ==============================================================================
# Sources
# ==============================================================================
target_sources(Acid303 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/AcidFilter.cpp
)

target_include_directories(Acid303 PRIVATE Source)

# ==============================================================================
# Compile definitions
# ==============================================================================
target_compile_definitions(Acid303 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    # Linux: disable ALSA/JACK browser windows in plugin context
    $<$<PLATFORM_ID:Linux>:JUCE_JACK=0>
    $<$<PLATFORM_ID:Linux>:JUCE_ALSA=0>
)

# ==============================================================================
# Link libraries
# ==============================================================================
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

# Linux requires explicit linking against system libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(Acid303 PRIVATE
        dl
        pthread
        ${CMAKE_DL_LIBS}
    )
endif()

# ==============================================================================
# Post-build: print install location
# ==============================================================================
if(APPLE)
    set(_INSTALL_HINT "~/Library/Audio/Plug-Ins/VST3/Acid303.vst3")
elseif(WIN32)
    set(_INSTALL_HINT "C:/Program Files/Common Files/VST3/Acid303.vst3")
else()
    set(_INSTALL_HINT "~/.vst3/Acid303.vst3")
endif()

add_custom_command(TARGET Acid303_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo
        "-- Acid303.vst3 installed to: ${_INSTALL_HINT}"
)
