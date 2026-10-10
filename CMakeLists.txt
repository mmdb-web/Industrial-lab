cmake_minimum_required(VERSION 3.22)
project(IndustrialLab VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(IndustrialLab
    COMPANY_NAME "IndustrialLab"
    BUNDLE_ID "com.industriallab.kick"
    PLUGIN_MANUFACTURER_CODE IndL
    PLUGIN_CODE ILab
    FORMATS VST3
    PRODUCT_NAME "Industrial Lab"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(IndustrialLab)
target_sources(IndustrialLab PRIVATE Plugin.cpp)
target_compile_definitions(IndustrialLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(IndustrialLab PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_audio_processors PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
