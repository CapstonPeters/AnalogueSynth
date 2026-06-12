cmake_minimum_required(VERSION 3.22)
project(AnalogSynth VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JUCE source directory
set(JUCE_DIR "/home/jo/JUCE" CACHE PATH "Path to JUCE source")

# Include JUCE's CMake support - this provides juce_add_plugin etc.
add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/juce-build)

# Our plugin - this is the modern JUCE way
juce_add_plugin(AnalogSynth
    COMPANY_NAME "AnalogSynth"
    PRODUCT_NAME "AnalogSynth"
    VERSION "1.0.0"
    IS_SYNTH
    WANTS_MIDI_INPUT
    WANTS_MIDI_OUTPUT
    PLUGIN_MANUFACTURER "AnalogSynth"
    PLUGIN_NAME "AnalogSynth"
    PLUGIN_DESCRIPTION "3-oscillator analog-style VST3 synthesizer"
    FORMATS VST3
    CXX_STANDARD 20
)

# Add our source files to the plugin target
target_sources(AnalogSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Main.cpp
)

# Link JUCE modules that our plugin needs
target_link_libraries(AnalogSynth PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_formats
)

# Linux: link browser dependencies (webkit2gtk-4.1 + gtk4) for juce_gui_extra
# Linux: link curl for juce_core URL handling
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(AnalogSynth PRIVATE
        juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS
        juce::pkgconfig_JUCE_CURL_LINUX_DEPS
    )
endif()

# VST3-specific settings for Linux
target_compile_definitions(AnalogSynth PRIVATE
    JUCE_VST3_CUSTOM_BUNDLE_SUFFIX="\"\""
    JUCE_PLUGIN_NAME="AnalogSynth"
    JUCE_VST3_CAN_REPLACE_VST2=0
)