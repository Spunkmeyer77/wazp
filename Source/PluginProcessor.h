#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/WazpEngine.h"
#include "Presets.h"

class WazpProcessor : public juce::AudioProcessor
{
public:
    WazpProcessor();
    ~WazpProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "WAZP"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    // presets: factory presets first, then the .wazp files in the user folder
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;     // shared with the on-screen keys

    // preset helpers for the editor
    juce::String getPresetDisplayName() const;
    void stepPreset(int delta);
    bool saveUserPreset(const juce::File& file);
    bool loadUserPreset(const juce::File& file);
    juce::Array<juce::File> getUserPresets() const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    void readParams(wazp::Params& p) const;
    void applyFactory(int index);

    wazp::Engine engine;
    int currentPreset = 0;
    juce::String currentName { "INIT" };
    std::vector<float> mono;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WazpProcessor)
};
