cmake_minimum_required(VERSION 3.22)

# must be set before project(), otherwise the build machine's own macOS version is used
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

project(WAZP VERSION 1.0.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use a local checkout if -DJUCE_DIR=... is given, otherwise download it
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.9
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
    set(WAZP_FORMATS AU VST3 Standalone)
elseif(WIN32)
    set(WAZP_FORMATS VST3 Standalone)
else()
    set(WAZP_FORMATS VST3 LV2 Standalone)
endif()

option(WAZP_COPY_PLUGINS "Copy plug-ins to the plug-in folders after building" ON)

juce_add_plugin(WAZP
    COMPANY_NAME "Transistor Labs"
    PLUGIN_MANUFACTURER_CODE Tcro
    PLUGIN_CODE Wazp
    FORMATS ${WAZP_FORMATS}
    LV2URI "urn:transistorlabs:wazp"
    PRODUCT_NAME "WAZP"
    BUNDLE_ID com.transistorlabs.wazp
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    VST3_CATEGORIES Instrument Synth Mono
    COPY_PLUGIN_AFTER_BUILD ${WAZP_COPY_PLUGINS})

# panel artwork (rendered by tools/rasterise_ui.py) and the manual (opened from the About window)
juce_add_binary_data(WAZPData SOURCES
    "docs/WAZP Manual.pdf"
    assets/ui/background.jpg
    assets/ui/sync_off.png
    assets/ui/sync_on.png
    assets/brand/tl_icon_yellow.svg)

set(WAZP_SOURCES Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/dsp/WazpEngine.cpp)
target_sources(WAZP PRIVATE ${WAZP_SOURCES})

target_compile_definitions(WAZP PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    _USE_MATH_DEFINES)

target_link_libraries(WAZP
    PRIVATE
        WAZPData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Optional: headless test app (audio render, presets, state, editor snapshot)
option(WAZP_BUILD_TESTS "Build the plugin test app" OFF)
if(WAZP_BUILD_TESTS)
    juce_add_console_app(WAZPTest PRODUCT_NAME "WAZPTest")
    target_sources(WAZPTest PRIVATE tools/plugin_test.cpp ${WAZP_SOURCES})
    target_compile_definitions(WAZPTest PRIVATE
        JucePlugin_Name="WAZP" JucePlugin_VersionString="${PROJECT_VERSION}" JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        _USE_MATH_DEFINES)
    target_link_libraries(WAZPTest PRIVATE WAZPData juce::juce_audio_utils juce::juce_recommended_config_flags)
endif()
