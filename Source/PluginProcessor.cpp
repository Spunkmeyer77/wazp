#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

static juce::NormalisableRange<float> skewed(float lo, float hi, float centre)
{
    juce::NormalisableRange<float> r(lo, hi);
    r.setSkewForCentre(centre);
    return r;
}

static juce::String pct(float v, int) { return juce::String(juce::roundToInt(v * 100.0f)) + " %"; }
static juce::String tenScale(float v, int) { return juce::String(v * 10.0f, 1); }
static juce::String hz(float v, int)
{
    return v >= 1000.0f ? juce::String(v / 1000.0f, 2) + " kHz" : juce::String(v, v < 10.0f ? 2 : 0) + " Hz";
}
static juce::String bipolar(float v, int) { return (v > 0 ? "+" : "") + juce::String(juce::roundToInt(v * 100.0f)); }
static juce::String sustainText(float v, int) { return v <= 0.04f ? juce::String("REPEAT") : juce::String(juce::roundToInt((v - 0.04f) / 0.96f * 100.0f)) + " %"; }
static juce::String delayText(float v, int)
{
    if (v <= 0.04f) return "REPEAT";
    double s = 0.002 * std::pow(1000.0, (v - 0.04) / 0.96);
    return s < 1.0 ? juce::String(juce::roundToInt(s * 1000.0)) + " ms" : juce::String(s, 2) + " s";
}
static juce::String timeText(float v, double lo, double hi)
{
    double s = lo * std::pow(hi / lo, (double) v);
    return s < 1.0 ? juce::String(juce::roundToInt(s * 1000.0)) + " ms" : juce::String(s, 2) + " s";
}

APVTS::ParameterLayout WazpProcessor::createLayout()
{
    using namespace juce;
    APVTS::ParameterLayout l;
    auto fl = [&](const char* id, const char* name, NormalisableRange<float> r, float def,
                  std::function<String(float, int)> txt = nullptr, String label = {})
    {
        l.add(std::make_unique<AudioParameterFloat>(ParameterID { id, 1 }, name, r, def,
              AudioParameterFloatAttributes().withLabel(label).withStringFromValueFunction(std::move(txt))));
    };
    const StringArray ranges { "32'", "16'", "8'", "4'", "2'" };
    const StringArray waves { "Off", "Pulse", "Saw" };

    fl("glide", "Glide", { 0.0f, 1.0f }, 0.0f, [](float v, int) { return v <= 0.001f ? String("OFF") : timeText(v, 0.005, 2.0); });
    l.add(std::make_unique<AudioParameterBool>(ParameterID { "sync", 1 }, "Sync", false));
    l.add(std::make_unique<AudioParameterBool>(ParameterID { "hold", 1 }, "Hold", false));

    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "osc1Range", 1 }, "Osc 1 Range", ranges, 2));
    fl("osc1Width", "Osc 1 Width", { 10.0f, 90.0f }, 50.0f, [](float v, int) { return String(roundToInt(v)) + " %"; });
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "osc1Wave", 1 }, "Osc 1 Wave", waves, 2));
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "osc2Range", 1 }, "Osc 2 Range", ranges, 2));
    fl("osc2Pitch", "Osc 2 Pitch", { -12.0f, 12.0f }, 0.0f, [](float v, int) { return (v > 0 ? "+" : "") + String(v, 2) + " st"; });
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "osc2Wave", 1 }, "Osc 2 Wave", waves, 2));

    fl("lfoRate", "Control Osc Freq", skewed(0.05f, 1000.0f, 6.0f), 2.0f, hz);
    fl("lfoPitch", "Pitch Mod", { 0.0f, 1.0f }, 0.0f, pct);
    fl("noise", "Noise Signal", { 0.0f, 1.0f }, 0.0f, tenScale);
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "lfoWave", 1 }, "Control Osc Wave",
                                                 StringArray { "Sine", "Square", "Noise", "Random" }, 0));

    fl("cutoff", "Filter Freq", skewed(20.0f, 20000.0f, 900.0f), 8000.0f, hz);
    fl("resonance", "Q", { 0.0f, 1.0f }, 0.2f, pct);
    l.add(std::make_unique<AudioParameterChoice>(ParameterID { "filterMode", 1 }, "Filter Control",
                                                 StringArray { "Lo", "Band", "Hi" }, 0));
    fl("filterEnv", "Filter Env Mod", { -1.0f, 1.0f }, 0.0f, bipolar);
    fl("filterLfo", "Filter Osc Mod", { -1.0f, 1.0f }, 0.0f, bipolar);

    fl("vcaAttack", "VCA Attack", { 0.0f, 1.0f }, 0.0f, [](float v, int) { return timeText(v, 0.001, 2.0); });
    fl("vcaDecay", "VCA Decay", { 0.0f, 1.0f }, 0.4f, [](float v, int) { return timeText(v, 0.01, 4.0); });
    fl("vcaSustain", "VCA Repeat/Sustain", { 0.0f, 1.0f }, 0.7f, sustainText);
    fl("envAttack", "Env Attack", { 0.0f, 1.0f }, 0.0f, [](float v, int) { return timeText(v, 0.001, 2.0); });
    fl("envDecay", "Env Decay", { 0.0f, 1.0f }, 0.4f, [](float v, int) { return timeText(v, 0.01, 4.0); });
    fl("envDelay", "Env Repeat/Delay", { 0.0f, 1.0f }, 0.05f, delayText);

    fl("osc1Level", "Osc 1 Level", { 0.0f, 1.0f }, 0.8f, tenScale);
    fl("osc2Level", "Osc 2 Level", { 0.0f, 1.0f }, 0.8f, tenScale);
    fl("volume", "Volume", skewed(-80.0f, 6.0f, -12.0f), -6.0f,
       [](float v, int) { return v <= -79.5f ? String("-inf dB") : String(v, 1) + " dB"; });
    return l;
}

WazpProcessor::WazpProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "WAZP", createLayout())
{
}

bool WazpProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void WazpProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate);
    mono.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
    keyboardState.reset();
}

void WazpProcessor::readParams(wazp::Params& p) const
{
    auto f = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    auto i = [&](const char* id) { return juce::roundToInt(f(id)); };
    p.glide = f("glide");           p.sync = f("sync") > 0.5f;
    p.osc1Range = i("osc1Range");   p.osc1Width = f("osc1Width");   p.osc1Wave = i("osc1Wave");
    p.osc2Range = i("osc2Range");   p.osc2Pitch = f("osc2Pitch");   p.osc2Wave = i("osc2Wave");
    p.lfoRate = f("lfoRate");       p.lfoPitch = f("lfoPitch");     p.noise = f("noise");   p.lfoWave = i("lfoWave");
    p.cutoff = f("cutoff");         p.resonance = f("resonance");   p.filterMode = i("filterMode");
    p.filterEnv = f("filterEnv");   p.filterLfo = f("filterLfo");
    p.vcaAttack = f("vcaAttack");   p.vcaDecay = f("vcaDecay");     p.vcaSustain = f("vcaSustain");
    p.envAttack = f("envAttack");   p.envDecay = f("envDecay");     p.envDelay = f("envDelay");
    p.osc1Level = f("osc1Level");   p.osc2Level = f("osc2Level");   p.volumeDb = f("volume");
}

void WazpProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer(midi, 0, n, true);   // merges the on-screen keys into the stream

    wazp::Params p;
    readParams(p);
    engine.setHold(apvts.getRawParameterValue("hold")->load() > 0.5f);
    if ((int) mono.size() < n) mono.resize((size_t) n);

    int pos = 0;
    auto renderTo = [&](int end)
    {
        if (end > pos) { engine.render(mono.data() + pos, end - pos, p); pos = end; }
    };

    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        renderTo(juce::jlimit(0, n, meta.samplePosition));
        if (m.isNoteOn()) engine.noteOn(m.getNoteNumber());
        else if (m.isNoteOff()) engine.noteOff(m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
        else if (m.isPitchWheel()) engine.setPitchBend((float) (m.getPitchWheelValue() - 8192) / 8192.0f * 2.0f);
        else if (m.isControllerOfType(1)) engine.setModWheel((float) m.getControllerValue() / 127.0f);
    }
    renderTo(n);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, mono.data(), n);
}

// ---- presets -------------------------------------------------------------------------------
juce::Array<juce::File> WazpProcessor::getUserPresets() const
{
    auto files = wazp::userPresetFolder().findChildFiles(juce::File::findFiles, false, "*.wazp");
    files.sort();
    return files;
}

int WazpProcessor::getNumPrograms()
{
    return (int) wazp::factoryPresets().size() + getUserPresets().size();
}

const juce::String WazpProcessor::getProgramName(int index)
{
    const auto& f = wazp::factoryPresets();
    if (index < (int) f.size()) return f[(size_t) index].name;
    auto u = getUserPresets();
    index -= (int) f.size();
    return juce::isPositiveAndBelow(index, u.size()) ? u[index].getFileNameWithoutExtension() : juce::String();
}

void WazpProcessor::applyFactory(int index)
{
    const auto& preset = wazp::factoryPresets()[(size_t) index];
    for (auto* param : getParameters())
        if (auto* r = dynamic_cast<juce::RangedAudioParameter*>(param))
            r->setValueNotifyingHost(r->getDefaultValue());
    for (auto& [id, value] : preset.values)
        if (auto* r = apvts.getParameter(id))
            r->setValueNotifyingHost(r->convertTo0to1(value));
    currentName = preset.name;
}

void WazpProcessor::setCurrentProgram(int index)
{
    const int numFactory = (int) wazp::factoryPresets().size();
    if (index < numFactory) { applyFactory(juce::jmax(0, index)); currentPreset = juce::jmax(0, index); return; }
    auto u = getUserPresets();
    if (juce::isPositiveAndBelow(index - numFactory, u.size()) && loadUserPreset(u[index - numFactory]))
        currentPreset = index;
}

void WazpProcessor::stepPreset(int delta)
{
    const int total = getNumPrograms();
    setCurrentProgram(((currentPreset + delta) % total + total) % total);
}

juce::String WazpProcessor::getPresetDisplayName() const
{
    return juce::String(currentPreset + 1).paddedLeft('0', 3) + "  " + currentName.toUpperCase();
}

bool WazpProcessor::saveUserPreset(const juce::File& file)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr || !xml->writeTo(file)) return false;
    auto u = getUserPresets();
    currentName = file.getFileNameWithoutExtension();
    currentPreset = (int) wazp::factoryPresets().size() + juce::jmax(0, u.indexOf(file));
    return true;
}

bool WazpProcessor::loadUserPreset(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName(apvts.state.getType())) return false;
    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    currentName = file.getFileNameWithoutExtension();
    auto u = getUserPresets();
    int idx = u.indexOf(file);
    currentPreset = idx >= 0 ? (int) wazp::factoryPresets().size() + idx : currentPreset;
    return true;
}

// ---- state ---------------------------------------------------------------------------------
void WazpProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("presetName", currentName, nullptr);
    state.setProperty("presetIndex", currentPreset, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WazpProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        currentName = tree.getProperty("presetName", "INIT").toString();
        currentPreset = (int) tree.getProperty("presetIndex", 0);
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorEditor* WazpProcessor::createEditor() { return new WazpEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WazpProcessor(); }
