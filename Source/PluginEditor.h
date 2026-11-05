#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

namespace wazp
{
// A knob drawn on top of the panel artwork: only the pointer line moves.
class Knob : public juce::Slider
{
public:
    Knob(float scale, std::vector<float> stepAnglesDeg = {}, float sweepDeg = 135.0f);
    void paint(juce::Graphics&) override;
    std::function<void()> onTouched;
    void mouseDown(const juce::MouseEvent& e) override { Slider::mouseDown(e); if (onTouched) onTouched(); }
    void mouseDrag(const juce::MouseEvent& e) override { Slider::mouseDrag(e); if (onTouched) onTouched(); }
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override { Slider::mouseWheelMove(e, w); if (onTouched) onTouched(); }
private:
    float scale;
    std::vector<float> stepAngles;
    float sweep;
};

class SyncSwitch : public juce::Button
{
public:
    SyncSwitch();
    void paintButton(juce::Graphics&, bool, bool) override;
private:
    juce::Image off, on;
};

// 3-position slide switch (pitch mod destination): click a position or drag the lever
class SlideSwitch : public juce::Slider
{
public:
    explicit SlideSwitch(std::vector<float> leverX) : Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox), xs(std::move(leverX)) {}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e) override { setFromX(e.position.x); if (onTouched) onTouched(); }
    void mouseDrag(const juce::MouseEvent& e) override { setFromX(e.position.x); if (onTouched) onTouched(); }
    void mouseUp(const juce::MouseEvent&) override {}
    std::function<void()> onTouched;
private:
    void setFromX(float x);
    std::vector<float> xs;   // lever centres, local x
};

class PanelButton : public juce::Button
{
public:
    explicit PanelButton(const juce::String& name) : Button(name) {}
    void paintButton(juce::Graphics&, bool over, bool down) override;
};

class Keys : public juce::Component, private juce::Timer
{
public:
    explicit Keys(juce::MidiKeyboardState& s);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    bool hitTest(int x, int y) override { return noteAt({ (float) x, (float) y }) >= 0; }
    static constexpr int lowestNote = 48;   // C3
private:
    int noteAt(juce::Point<float>) const;
    juce::Rectangle<float> keyRect(int note) const;
    bool isBlack(int note) const;
    void timerCallback() override { repaint(); }
    juce::MidiKeyboardState& state;
    int mouseNote = -1;
};

class AboutOverlay : public juce::Component
{
public:
    AboutOverlay();
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override { setVisible(false); }
private:
    juce::HyperlinkButton store { "ticcoross.gumroad.com", juce::URL("https://ticcoross.gumroad.com/") };
    juce::HyperlinkButton mail { "ticcoross@gmail.com", juce::URL("mailto:ticcoross@gmail.com") };
    PanelButton manual { "USER MANUAL" };
};
} // namespace wazp

class WazpEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit WazpEditor(WazpProcessor&);
    ~WazpEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    struct Panel : public juce::Component
    {
        juce::Image background;
        void paint(juce::Graphics& g) override;
        std::function<void(juce::Point<float>)> onClick;
        void mouseDown(const juce::MouseEvent& e) override { if (onClick) onClick(e.position); }
    };

    void timerCallback() override;
    void showValue(const juce::String& paramId);
    void savePreset();
    void loadPreset();
    void showPresetList();

    WazpProcessor& proc;
    Panel panel;
    juce::OwnedArray<wazp::Knob> knobs;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;
    wazp::SyncSwitch sync, hold;
    wazp::SlideSwitch pitchDest { { 13.0f, 47.0f, 81.0f } }, lfoSync { { 17.0f, 51.0f } };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchDestAttachment, lfoSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment, holdAttachment;
    wazp::PanelButton prev { "<" }, next { ">" }, save { "SAVE" }, load { "LOAD" };
    wazp::Keys keys;
    wazp::AboutOverlay about;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::String displayText, valueText;
    double valueUntil = 0.0;
    struct Display : public juce::Component
    {
        juce::String text;
        std::function<void()> onClick;
        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent&) override { if (onClick) onClick(); }
    } display;

    struct MenuLook : public juce::LookAndFeel_V4
    {
        MenuLook();
        juce::Font getPopupMenuFont() override;
        void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;
    } menuLook;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WazpEditor)
};
