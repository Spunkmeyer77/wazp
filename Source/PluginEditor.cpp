#include "PluginEditor.h"
#include "Layout.h"
#include "BinaryData.h"

using namespace wazp;

static const juce::Colour kYellow { 0xffffdc12 };
static const juce::Colour kPointer { 0xff393421 };

static juce::Image loadImage(const char* data, int size) { return juce::ImageCache::getFromMemory(data, size); }

// ============================================================================ Knob
Knob::Knob(float s, std::vector<float> steps, float sweepDeg)
    : Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox), scale(s), stepAngles(std::move(steps)), sweep(sweepDeg)
{
    setMouseDragSensitivity(stepAngles.empty() ? 220 : 60 * (int) stepAngles.size());
    setVelocityBasedMode(false);
    setScrollWheelEnabled(true);
}

void Knob::paint(juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const juce::Point<float> c { b.getCentreX(), b.getCentreY() - 3.0f * scale };
    float angle;
    const double norm = valueToProportionOfLength(getValue());
    if (!stepAngles.empty())
    {
        int idx = juce::jlimit(0, (int) stepAngles.size() - 1, juce::roundToInt(getValue() - getMinimum()));
        angle = stepAngles[(size_t) idx];
    }
    else
        angle = (float) (-sweep + norm * 2.0 * sweep);

    const float rad = juce::degreesToRadians(angle);
    const juce::Point<float> dir { std::sin(rad), -std::cos(rad) };
    g.setColour(kPointer);
    g.drawLine({ c + dir * 5.0f * scale, c + dir * 19.0f * scale }, 1.9f * scale);
    if (isMouseOverOrDragging())
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillEllipse(juce::Rectangle<float>(48.0f * scale, 48.0f * scale).withCentre(c));
    }
}

// ============================================================================ Sync switch
SyncSwitch::SyncSwitch() : Button("SYNC")
{
    off = loadImage(BinaryData::sync_off_png, BinaryData::sync_off_pngSize);
    on = loadImage(BinaryData::sync_on_png, BinaryData::sync_on_pngSize);
    setClickingTogglesState(true);
}

void SyncSwitch::paintButton(juce::Graphics& g, bool, bool)
{
    g.drawImage(getToggleState() ? on : off, getLocalBounds().toFloat());
}

// ============================================================================ Slide switch
void SlideSwitch::setFromX(float x)
{
    const float third = (float) getWidth() / 3.0f;
    setValue(juce::jlimit(0.0, 2.0, (double) (int) (x / third)), juce::sendNotificationSync);
}

void SlideSwitch::paint(juce::Graphics& g)
{
    const int pos = juce::jlimit(0, 2, juce::roundToInt(getValue()));
    const float cx = layout::pitchDestX[pos] - layout::pitchDestSwitch.getX();
    auto lever = juce::Rectangle<float>(30.0f, 14.0f).withCentre({ cx, (float) getHeight() * 0.5f });
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(lever.translated(0.0f, 1.5f), 3.5f);
    juce::ColourGradient grad(juce::Colour(0xfffff36a), lever.getX(), lever.getY(), juce::Colour(0xffe0b400), lever.getX(), lever.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(lever, 3.5f);
    g.setColour(juce::Colour(0xffffed70));
    g.drawRoundedRectangle(lever, 3.5f, 1.0f);
    g.setColour(juce::Colour(0xff8a6c00).withAlpha(0.7f));
    for (int i = -1; i <= 1; ++i)                                   // grip lines
        g.drawVerticalLine(juce::roundToInt(lever.getCentreX() + (float) i * 5.0f), lever.getY() + 3.5f, lever.getBottom() - 3.5f);
}

// ============================================================================ Panel buttons
void PanelButton::paintButton(juce::Graphics& g, bool over, bool down)
{
    auto b = getLocalBounds().toFloat();
    if (getName() == "USER MANUAL")
    {
        g.setColour(juce::Colour(0xff111314));
        g.fillRoundedRectangle(b, 5.0f);
        g.setColour(kYellow.withAlpha(over ? 1.0f : 0.7f));
        g.drawRoundedRectangle(b.reduced(1.0f), 5.0f, 1.5f);
        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        g.drawText(getName(), b, juce::Justification::centred);
        return;
    }
    // the artwork already shows the button; just add feedback
    if (down) { g.setColour(juce::Colours::black.withAlpha(0.35f)); g.fillRoundedRectangle(b, 5.0f); }
    else if (over) { g.setColour(kYellow.withAlpha(0.10f)); g.fillRoundedRectangle(b, 5.0f); }
}

// ============================================================================ Keys
Keys::Keys(juce::MidiKeyboardState& s) : state(s) { startTimerHz(30); }

bool Keys::isBlack(int note) const
{
    const int pc = note % 12;
    return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
}

juce::Rectangle<float> Keys::keyRect(int note) const
{
    static const int whiteIndexOfPc[12] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 };
    const int rel = note - lowestNote;
    const int octave = rel / 12, pc = rel % 12;
    const int white = octave * 7 + whiteIndexOfPc[pc];
    if (!isBlack(note))
        return juce::isPositiveAndBelow(white, 15) ? layout::whiteKeys[white] : juce::Rectangle<float>();
    for (auto& bk : layout::blackKeys)
        if (bk.after == white) return bk.r;
    return {};
}

int Keys::noteAt(juce::Point<float> p) const
{
    for (int n = lowestNote; n <= lowestNote + 24; ++n)
        if (isBlack(n) && keyRect(n).contains(p)) return n;
    for (int n = lowestNote; n <= lowestNote + 24; ++n)
        if (!isBlack(n) && keyRect(n).contains(p)) return n;
    return -1;
}

void Keys::paint(juce::Graphics& g)
{
    for (int n = lowestNote; n <= lowestNote + 24; ++n)
        if (state.isNoteOnForChannels(0xffff, n))
        {
            auto r = keyRect(n);
            g.setColour(isBlack(n) ? kYellow.withAlpha(0.35f) : juce::Colour(0xff8c5a00).withAlpha(0.35f));
            g.fillRoundedRectangle(r.reduced(1.0f), 4.0f);
        }
}

void Keys::mouseDown(const juce::MouseEvent& e)
{
    mouseNote = noteAt(e.position);
    if (mouseNote >= 0) state.noteOn(1, mouseNote, 0.8f);
}

void Keys::mouseDrag(const juce::MouseEvent& e)
{
    const int n = noteAt(e.position);
    if (n != mouseNote)
    {
        if (mouseNote >= 0) state.noteOff(1, mouseNote, 0.0f);
        mouseNote = n;
        if (n >= 0) state.noteOn(1, n, 0.8f);
    }
}

void Keys::mouseUp(const juce::MouseEvent&)
{
    if (mouseNote >= 0) state.noteOff(1, mouseNote, 0.0f);
    mouseNote = -1;
}

// ============================================================================ About
AboutOverlay::AboutOverlay()
{
    for (auto* l : { &store, &mail })
    {
        l->setFont(juce::FontOptions(17.0f, juce::Font::bold), false);
        l->setColour(juce::HyperlinkButton::textColourId, kYellow);
        addAndMakeVisible(l);
    }
    addAndMakeVisible(manual);
    manual.onClick = []
    {
        auto f = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("WAZP Manual.pdf");
        f.replaceWithData(BinaryData::WAZP_Manual_pdf, (size_t) BinaryData::WAZP_Manual_pdfSize);
        f.startAsProcess();
    };
}

static juce::Rectangle<float> aboutCard(juce::Rectangle<float> b) { return juce::Rectangle<float>(620.0f, 380.0f).withCentre(b.getCentre()); }

void AboutOverlay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.6f));
    auto card = aboutCard(getLocalBounds().toFloat());
    juce::ColourGradient grad(juce::Colour(0xff1d1f21), card.getX(), card.getY(), juce::Colour(0xff060708), card.getRight(), card.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(card, 18.0f);
    g.setColour(juce::Colour(0xffeec600));
    g.drawRoundedRectangle(card, 18.0f, 7.0f);
    g.setColour(juce::Colour(0xfffff26c));
    g.drawRoundedRectangle(card.reduced(9.0f), 10.0f, 1.0f);

    auto t = card.reduced(30.0f);
    g.setColour(kYellow);
    g.setFont(juce::FontOptions(84.0f, juce::Font::bold));
    g.drawText("WAZP", t.removeFromTop(96.0f), juce::Justification::centred);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("MONOPHONIC SYNTHESIZER", t.removeFromTop(26.0f), juce::Justification::centred);
    g.setColour(kYellow.withAlpha(0.75f));
    g.setFont(juce::FontOptions(15.0f));
    g.drawText(juce::String("Version ") + JucePlugin_VersionString + "   |   Ticco Ross 2026", t.removeFromTop(26.0f), juce::Justification::centred);
    g.setColour(kYellow.withAlpha(0.4f));
    g.drawHorizontalLine((int) t.getY() + 10, t.getX() + 60.0f, t.getRight() - 60.0f);
    g.setFont(juce::FontOptions(12.0f));
    g.setColour(kYellow.withAlpha(0.5f));
    g.drawText("click anywhere to close", card.removeFromBottom(30.0f), juce::Justification::centred);
}

void AboutOverlay::resized()
{
    auto card = aboutCard(getLocalBounds().toFloat()).toNearestInt();
    store.setBounds(card.getCentreX() - 150, card.getY() + 200, 300, 30);
    mail.setBounds(card.getCentreX() - 150, card.getY() + 232, 300, 30);
    manual.setBounds(card.getCentreX() - 100, card.getY() + 280, 200, 40);
}

// ============================================================================ Editor
void WazpEditor::Panel::paint(juce::Graphics& g)
{
    g.drawImage(background, getLocalBounds().toFloat());
}

void WazpEditor::Display::paint(juce::Graphics& g)
{
    g.setColour(kYellow);
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 27.0f, juce::Font::bold));
    g.drawFittedText(text, getLocalBounds().reduced(8, 0), juce::Justification::centred, 1, 0.6f);
}

namespace
{
struct KnobDef { const char* artId; const char* param; std::vector<float> steps; float sweep; };
const std::vector<KnobDef>& knobDefs()
{
    static const std::vector<KnobDef> d = {
        { "glide", "glide", {}, 135 },
        { "osc1-range", "osc1Range", { -66, -35, 0, 35, 66 }, 0 },
        { "osc1-width", "osc1Width", {}, 135 },
        { "osc1-wave", "osc1Wave", { -90, 0, 90 }, 0 },
        { "osc2-range", "osc2Range", { -66, -35, 0, 35, 66 }, 0 },
        { "osc2-pitch", "osc2Pitch", {}, 135 },
        { "osc2-wave", "osc2Wave", { -90, 0, 90 }, 0 },
        { "lfo-frequency", "lfoRate", {}, 135 },
        { "pitch-mod", "lfoPitch", {}, 135 },
        { "noise", "noise", {}, 135 },
        { "lfo-wave", "lfoWave", { -45, -13, 36, 57 }, 0 },
        { "filter-frequency", "cutoff", {}, 135 },
        { "resonance", "resonance", {}, 135 },
        { "filter-mode", "filterMode", { -70, 0, 70, 115 }, 0 },
        { "filter-mod1", "filterLfo", {}, 135 },   // left: control osc -> filter (as on the panel arrows)
        { "filter-mod2", "filterEnv", {}, 135 },   // right: control env -> filter
        { "vca-attack", "vcaAttack", {}, 135 },
        { "vca-decay", "vcaDecay", {}, 135 },
        { "vca-sustain", "vcaSustain", {}, 135 },
        { "env-attack", "envAttack", {}, 135 },
        { "env-decay", "envDecay", {}, 135 },
        { "env-delay", "envDelay", {}, 135 },
        { "volume", "volume", {}, 135 },
        { "knob_osc1_level", "osc1Level", {}, 130 },
        { "knob_osc2_level", "osc2Level", {}, 130 },
    };
    return d;
}
}

WazpEditor::WazpEditor(WazpProcessor& p)
    : AudioProcessorEditor(p), proc(p), keys(p.keyboardState)
{
    panel.background = loadImage(BinaryData::background_jpg, BinaryData::background_jpgSize);
    addAndMakeVisible(panel);
    panel.setBounds(0, 0, (int) layout::panelW, (int) layout::panelH);
    panel.onClick = [this](juce::Point<float> pt)
    {
        if (layout::badge.contains(pt)) { about.setVisible(true); about.toFront(false); }
    };

    for (auto& def : knobDefs())
    {
        const layout::KnobPos* pos = nullptr;
        for (auto& k : layout::knobs) if (juce::String(k.artId) == def.artId) pos = &k;
        jassert(pos != nullptr);
        auto* k = knobs.add(new Knob(pos->s, def.steps, def.sweep));
        const float size = 64.0f * pos->s;
        k->setBounds(juce::Rectangle<float>(size, size).withCentre({ pos->x, pos->y }).toNearestInt());
        panel.addAndMakeVisible(k);
        attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(proc.apvts, def.param, *k));
        if (auto* param = proc.apvts.getParameter(def.param))
            k->setDoubleClickReturnValue(true, (double) param->convertFrom0to1(param->getDefaultValue()));
        const juce::String id = def.param;
        k->onTouched = [this, id] { showValue(id); };
    }

    panel.addAndMakeVisible(sync);
    sync.setBounds(juce::Rectangle<float>(68.0f, 46.0f).withCentre(layout::syncSwitch).toNearestInt());
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, "sync", sync);
    sync.onClick = [this] { showValue("sync"); };

    panel.addAndMakeVisible(hold);
    hold.setBounds(juce::Rectangle<float>(68.0f, 46.0f).withCentre(layout::holdSwitch).toNearestInt());
    holdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, "hold", hold);
    hold.onClick = [this] { showValue("hold"); };

    panel.addAndMakeVisible(pitchDest);
    pitchDest.setBounds(layout::pitchDestSwitch.toNearestInt());
    pitchDestAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, "pitchDest", pitchDest);
    pitchDest.onTouched = [this] { showValue("pitchDest"); };

    for (auto [b, r] : { std::pair { &prev, layout::presetprev }, { &next, layout::presetnext },
                          { &save, layout::presetsave }, { &load, layout::presetload } })
    {
        panel.addAndMakeVisible(b);
        b->setBounds(r.toNearestInt());
    }
    prev.onClick = [this] { proc.stepPreset(-1); valueUntil = 0; };
    next.onClick = [this] { proc.stepPreset(+1); valueUntil = 0; };
    save.onClick = [this] { savePreset(); };
    load.onClick = [this] { loadPreset(); };

    panel.addAndMakeVisible(display);
    display.setBounds(layout::presetdisplay.reduced(4.0f).toNearestInt());
    display.onClick = [this] { showPresetList(); };
    display.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    display.text = proc.getPresetDisplayName();

    panel.addAndMakeVisible(keys);
    keys.setBounds(0, 0, (int) layout::panelW, (int) layout::panelH);
    keys.toBack();
    keys.setInterceptsMouseClicks(true, false);
    // keys covers the whole panel but only reacts on the keyboard area
    keys.setMouseClickGrabsKeyboardFocus(false);

    panel.addChildComponent(about);
    about.setBounds(panel.getLocalBounds());

    setResizable(true, true);
    const double ratio = layout::panelW / layout::panelH;
    getConstrainer()->setFixedAspectRatio(ratio);
    setResizeLimits(768, 512, 2304, 1536);
    setSize(1152, 768);
    startTimerHz(20);
}

WazpEditor::~WazpEditor() { stopTimer(); }

void WazpEditor::resized()
{
    panel.setTransform(juce::AffineTransform::scale((float) getWidth() / layout::panelW));
}

void WazpEditor::showValue(const juce::String& paramId)
{
    if (auto* param = proc.apvts.getParameter(paramId))
    {
        valueText = param->getName(24).toUpperCase() + "  " + param->getCurrentValueAsText().toUpperCase();
        valueUntil = juce::Time::getMillisecondCounterHiRes() + 1500.0;
        display.text = valueText;
        display.repaint();
    }
}

void WazpEditor::timerCallback()
{
    juce::String t = juce::Time::getMillisecondCounterHiRes() < valueUntil ? valueText : proc.getPresetDisplayName();
    if (t != display.text) { display.text = t; display.repaint(); }
}

// ---- the full preset list (click the preset display) ----------------------------------------
WazpEditor::MenuLook::MenuLook()
{
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff0b0c0d));
    setColour(juce::PopupMenu::textColourId, kYellow);
    setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xfffff26c));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, kYellow);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xff0b0c0d));
}

juce::Font WazpEditor::MenuLook::getPopupMenuFont()
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 22.0f, juce::Font::bold));
}

void WazpEditor::MenuLook::drawPopupMenuBackground(juce::Graphics& g, int w, int h)
{
    g.fillAll(juce::Colour(0xff0b0c0d));
    g.setColour(juce::Colour(0xffeec600));
    g.drawRect(0, 0, w, h, 2);
}

void WazpEditor::showPresetList()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&menuLook);
    const int numFactory = (int) factoryPresets().size();
    const int current = proc.getCurrentProgram();
    auto label = [](int i, const juce::String& name) { return juce::String(i + 1).paddedLeft('0', 3) + "  " + name.toUpperCase(); };

    menu.addSectionHeader("FACTORY PRESETS");
    for (int i = 0; i < numFactory; ++i)
        menu.addItem(i + 1, label(i, factoryPresets()[(size_t) i].name), true, i == current);

    auto user = proc.getUserPresets();
    menu.addSectionHeader("MY PRESETS");
    if (user.isEmpty())
        menu.addItem(-1, "(none yet - use SAVE)", false, false);
    for (int i = 0; i < user.size(); ++i)
        menu.addItem(numFactory + i + 1, label(numFactory + i, user[i].getFileNameWithoutExtension()), true, numFactory + i == current);

    menu.addSeparator();
    menu.addItem(10000, "OPEN PRESET FOLDER");

    juce::Component::SafePointer<WazpEditor> safe(this);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&display).withMinimumWidth(juce::jmax(360, display.getScreenBounds().getWidth()))
                           .withStandardItemHeight(34)
                           .withMaximumNumColumns(1),
                       [safe](int result)
                       {
                           if (safe == nullptr || result <= 0) return;
                           if (result == 10000) { userPresetFolder().startAsProcess(); return; }
                           safe->proc.setCurrentProgram(result - 1);
                           safe->valueUntil = 0;
                       });
}

void WazpEditor::savePreset()
{
    chooser = std::make_unique<juce::FileChooser>("Save WAZP preset", userPresetFolder().getChildFile("My Sound.wazp"), "*.wazp");
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                             | juce::FileBrowserComponent::warnAboutOverwriting,
                         [this](const juce::FileChooser& fc)
                         {
                             auto f = fc.getResult();
                             if (f == juce::File()) return;
                             proc.saveUserPreset(f.withFileExtension("wazp"));
                             valueUntil = 0;
                         });
}

void WazpEditor::loadPreset()
{
    chooser = std::make_unique<juce::FileChooser>("Load WAZP preset", userPresetFolder(), "*.wazp");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [this](const juce::FileChooser& fc)
                         {
                             auto f = fc.getResult();
                             if (f.existsAsFile()) proc.loadUserPreset(f);
                             valueUntil = 0;
                         });
}
