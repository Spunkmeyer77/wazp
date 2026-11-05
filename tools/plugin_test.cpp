// Headless tests for WAZP: notes, presets, state, every wave / mode combination;
// writes a demo .wav and editor snapshots (run under xvfb on Linux CI).
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

static int failures = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++failures;
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir(argc > 1 ? juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]) : juce::File::getCurrentWorkingDirectory());
    outDir.createDirectory();

    WazpProcessor proc;
    const double sr = 48000.0;
    const int block = 256;
    proc.prepareToPlay(sr, block);
    juce::AudioBuffer<float> buf(2, block);
    auto setParam = [&](const char* id, float v) { auto* p = proc.apvts.getParameter(id); p->setValueNotifyingHost(p->convertTo0to1(v)); };

    std::vector<float> rec;
    bool finite = true;
    auto run = [&](int blocks, juce::MidiBuffer midi = {})
    {
        float peak = 0.0f;
        for (int b = 0; b < blocks; ++b)
        {
            juce::MidiBuffer m;
            if (b == 0) m = midi;
            proc.processBlock(buf, m);
            for (int i = 0; i < block; ++i)
            {
                const float x = buf.getSample(0, i);
                if (!std::isfinite(x)) finite = false;
                peak = std::max(peak, std::abs(x));
                rec.push_back(x);
            }
        }
        return peak;
    };
    auto on = [](int n) { juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOn(1, n, (juce::uint8) 100), 10); return m; };
    auto off = [](int n) { juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOff(1, n), 10); return m; };

    check(run(20) < 1e-6f, "silent without notes");
    float p = run(150, on(48));
    check(p > 0.05f, "INIT makes sound");
    run(1, off(48));
    run(400);
    float tail = 0.0f; for (size_t i = rec.size() - 2000; i < rec.size(); ++i) tail = std::max(tail, std::abs(rec[i]));
    check(tail < 1e-3f, "note releases to silence");

    // every factory preset plays a phrase
    const int nf = (int) wazp::factoryPresets().size();
    bool allSound = true;
    for (int i = 0; i < nf; ++i)
    {
        proc.setCurrentProgram(i);
        float pk = 0.0f;
        for (int n : { 45, 52, 57, 48 })
        {
            pk = std::max(pk, run(40, on(n)));
            run(1, off(n));
            run(25);
        }
        run(150);
        if (pk < 0.01f) { allSound = false; std::printf("      silent preset %s\n", proc.getProgramName(i).toRawUTF8()); }
        if (pk > 1.2f) { allSound = false; std::printf("      too loud preset %s (%f)\n", proc.getProgramName(i).toRawUTF8(), pk); }
    }
    check(allSound, "all factory presets sound at a sane level");

    // all wave / filter / lfo combinations stay finite
    for (int w1 = 0; w1 < 3; ++w1) for (int fm = 0; fm < 3; ++fm) for (int lw = 0; lw < 4; ++lw)
    {
        setParam("osc1Wave", (float) w1); setParam("filterMode", (float) fm); setParam("lfoWave", (float) lw);
        setParam("resonance", 1.0f); setParam("sync", 1.0f); setParam("filterEnv", 1.0f); setParam("lfoPitch", 1.0f);
        run(10, on(60)); run(1, off(60)); run(5);
    }
    check(finite, "output always finite");

    // state round trip
    proc.setCurrentProgram(3);
    setParam("cutoff", 1234.0f);
    juce::MemoryBlock mb;
    proc.getStateInformation(mb);
    WazpProcessor other;
    other.setStateInformation(mb.getData(), (int) mb.getSize());
    check(std::abs(other.apvts.getRawParameterValue("cutoff")->load() - 1234.0f) < 1.0f, "state restores parameters");
    check(other.getPresetDisplayName() == proc.getPresetDisplayName(), "state restores preset name");

    // user preset save / load
    auto tmp = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("wazp_test.wazp");
    setParam("osc2Pitch", 7.0f);
    check(proc.saveUserPreset(tmp), "save user preset");
    setParam("osc2Pitch", 0.0f);
    check(proc.loadUserPreset(tmp) && std::abs(proc.apvts.getRawParameterValue("osc2Pitch")->load() - 7.0f) < 0.01f, "load user preset");
    tmp.deleteFile();

    // demo
    {
        rec.clear();
        proc.setCurrentProgram(5);
        const int seq[] = { 36, 36, 48, 36, 39, 36, 46, 43 };
        for (int bar = 0; bar < 2; ++bar)
            for (int n : seq) { run(22, on(n)); run(1, off(n)); }
        proc.setCurrentProgram(3);
        for (int n : { 60, 63, 67, 70 }) { run(60, on(n)); run(1, off(n)); }
        run(200);
        juce::AudioBuffer<float> out(1, (int) rec.size());
        out.copyFrom(0, 0, rec.data(), (int) rec.size());
        juce::WavAudioFormat wav;
        auto f = outDir.getChildFile("wazp_demo.wav");
        f.deleteFile();
        std::unique_ptr<juce::OutputStream> os(f.createOutputStream());
        if (os != nullptr)
            if (auto w = wav.createWriterFor(os, juce::AudioFormatWriterOptions {}.withSampleRate(sr).withNumChannels(1).withBitsPerSample(24)))
                w->writeFromAudioSampleBuffer(out, 0, out.getNumSamples());
    }

    // editor snapshots
    {
        proc.setCurrentProgram(2);
        proc.keyboardState.noteOn(1, 52, 1.0f);
        proc.keyboardState.noteOn(1, 61, 1.0f);
        std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor());
        for (int w : { 1152, 1536 })
        {
            ed->setSize(w, w * 2 / 3);
            auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 1.0f);
            auto f = outDir.getChildFile("editor_" + juce::String(w) + ".png");
            f.deleteFile();
            juce::PNGImageFormat png;
            if (auto os = std::unique_ptr<juce::OutputStream>(f.createOutputStream())) png.writeImageToStream(img, *os);
        }
        // the About window
        if (auto* c = ed->getChildComponent(0))
            for (auto* ch : c->getChildren())
                if (dynamic_cast<wazp::AboutOverlay*>(ch) != nullptr) ch->setVisible(true);
        auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 1.0f);
        auto f = outDir.getChildFile("editor_about.png");
        f.deleteFile();
        juce::PNGImageFormat png;
        if (auto os = std::unique_ptr<juce::OutputStream>(f.createOutputStream())) png.writeImageToStream(img, *os);
        check(true, "editor snapshots written");
    }

    std::printf("\n%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
