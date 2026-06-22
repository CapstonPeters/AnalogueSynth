cmake_minimum_required(VERSION 3.22)
project(AnalogSynth VERSION 1.0.0 LANGUAGES CXX)

# Disable Direct2D on Windows (not available in MinGW headers) - must be before FetchContent
option(JUCE_DIRECT2D "Enable Direct2D renderer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option 1: Use local JUCE if JUCE_DIR is set and exists
# Option 2: Fetch JUCE from GitHub via FetchContent (for CI)
if(DEFINED ENV{JUCE_DIR} AND EXISTS "$ENV{JUCE_DIR}/CMakeLists.txt")
    set(JUCE_DIR "$ENV{JUCE_DIR}" CACHE PATH "Path to JUCE source" FORCE)
    message(STATUS "Using local JUCE at: ${JUCE_DIR}")
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/juce-build)
elseif(DEFINED ENV{JUCE_SOURCE_DIR} AND EXISTS "$ENV{JUCE_SOURCE_DIR}/CMakeLists.txt")
    set(JUCE_DIR "$ENV{JUCE_SOURCE_DIR}" CACHE PATH "Path to JUCE source" FORCE)
    message(STATUS "Using local JUCE at: ${JUCE_DIR}")
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/juce-build)
else()
    message(STATUS "Fetching JUCE 8.0.0 via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(juce)
    set(JUCE_DIR ${juce_SOURCE_DIR})
    message(STATUS "Using fetched JUCE at: ${JUCE_DIR}")
endif()

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
    VST3_CATEGORIES "Instrument|Synth"
    PLUGIN_UID "E3A8F2B1-4C7D-4A9E-8B3F-1D2C5E7F9A0B"
    PLUGIN_CLASS "AnalogSynthAudioProcessor"
    PLUGIN_CODE "AnSy"
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
        juce::pkgconfig_JUCE_CURL_LINUX_DEPS
    )
endif()

# VST3-specific settings
target_compile_definitions(AnalogSynth PRIVATE
    JUCE_VST3_CUSTOM_BUNDLE_SUFFIX=\"\"\"
    JUCE_PLUGIN_NAME=\"AnalogSynth\"
    JUCE_VST3_CAN_REPLACE_VST2=0
    # Explicit MIDI for VST3 on Windows
    JucePlugin_WantsMidiInput=1
    JucePlugin_WantsMidiOutput=0
    VST3_NUM_MIDI_INPUTS=1
    VST3_NUM_MIDI_OUTPUTS=0
    # Disable web browser (not needed for VST3 plugin, avoids GTK dependency)
    JUCE_WEB_BROWSER=0
)