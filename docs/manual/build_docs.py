#!/usr/bin/env python3
"""Builds docs/WAZP Manual.pdf and docs/WAZP Update History.pdf (black and yellow, like the panel).

    python3 docs/manual/build_docs.py 1.0.0        (needs Python Playwright with Chromium)
"""
import base64, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DOCS = HERE.parent
VERSION = sys.argv[1] if len(sys.argv) > 1 else "1.0.4"
PANEL = "data:image/jpeg;base64," + base64.b64encode((HERE / "img" / "panel.jpg").read_bytes()).decode()

CSS = """
@page { size: A4; margin: 0; }
* { box-sizing: border-box; }
body { margin: 0; font-family: 'DejaVu Sans', 'Liberation Sans', Arial, sans-serif; color: #1b1b1b; font-size: 10pt; line-height: 1.45; }
.page { width: 210mm; height: 297mm; position: relative; padding: 22mm 18mm 18mm; page-break-after: always; overflow: hidden; background: #fbfaf5; }
.cover { background: #0b0c0d; color: #ffdc12; padding: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; }
.cover .frame { position: absolute; inset: 12mm; border: 5px solid #eec600; border-radius: 16px; }
.cover .frame2 { position: absolute; inset: 15mm; border: 1px solid #fff26c; border-radius: 10px; }
.cover h1 { font-size: 92pt; letter-spacing: 6px; margin: 0; font-weight: 900; -webkit-text-stroke: 2px #ffdc12; color: transparent; }
.cover .sub { font-size: 15pt; letter-spacing: 4px; margin-top: 4mm; }
.cover .doc { font-size: 22pt; margin-top: 16mm; font-weight: 700; }
.cover .ver { font-size: 11pt; margin-top: 3mm; opacity: .8; }
.cover img { width: 160mm; margin-top: 16mm; border-radius: 6px; }
.cover .by { position: absolute; bottom: 24mm; font-size: 10pt; letter-spacing: 2px; opacity: .85; }
.hd { position: absolute; top: 9mm; left: 18mm; right: 18mm; display: flex; justify-content: space-between; font-size: 8pt;
      color: #6a5d10; border-bottom: 2px solid #eec600; padding-bottom: 2mm; letter-spacing: 1px; }
.pn { position: absolute; bottom: 9mm; right: 18mm; font-size: 8pt; color: #6a5d10; }
h2 { font-size: 17pt; margin: 0 0 4mm; padding: 2mm 4mm; background: #0b0c0d; color: #ffdc12; border-radius: 4px; letter-spacing: 1px; }
h3 { font-size: 11.5pt; margin: 5mm 0 1.5mm; color: #0b0c0d; border-left: 4px solid #eec600; padding-left: 3mm; }
p { margin: 0 0 2.5mm; }
table { width: 100%; border-collapse: collapse; margin: 2mm 0 4mm; font-size: 9.2pt; }
th { background: #0b0c0d; color: #ffdc12; text-align: left; padding: 1.6mm 2.5mm; }
td { border-bottom: 1px solid #ddd6b0; padding: 1.5mm 2.5mm; vertical-align: top; }
td:first-child { font-weight: 700; white-space: nowrap; }
.note { background: #fff6c2; border: 1px solid #eec600; border-radius: 4px; padding: 2.5mm 3.5mm; margin: 3mm 0; font-size: 9.2pt; }
img.panel { width: 100%; border-radius: 5px; margin: 2mm 0 3mm; }
ul { margin: 1mm 0 3mm; padding-left: 5mm; } li { margin-bottom: 1mm; }
"""


class Doc:
    def __init__(self, title):
        self.title, self.pages = title, []

    def cover(self, doc_name):
        self.pages.append(f'''<section class="page cover"><div class="frame"></div><div class="frame2"></div>
<h1>WAZP</h1><div class="sub">MONOPHONIC SYNTHESIZER</div><div class="doc">{doc_name}</div><div class="ver">Version {VERSION}</div>
<img src="{PANEL}"><div class="by">TICCO ROSS &nbsp;|&nbsp; ticcoross.gumroad.com</div></section>''')

    def page(self, body):
        n = len(self.pages) + 1
        self.pages.append(f'<section class="page"><div class="hd"><span>WAZP {self.title.upper()}</span><span>VERSION {VERSION}</span></div>'
                          f'{body}<div class="pn">{n}</div></section>')

    def html(self):
        return f'<!doctype html><html><head><meta charset="utf-8"><style>{CSS}</style></head><body>{"".join(self.pages)}</body></html>'


def rows(items):
    return "<table><tr><th>Control</th><th>What it does</th></tr>" + "".join(f"<tr><td>{a}</td><td>{b}</td></tr>" for a, b in items) + "</table>"


def manual():
    d = Doc("Owner's Manual")
    d.cover("Owner's Manual")
    d.page(f"""<h2>Welcome</h2>
<p>WAZP is a monophonic synthesizer plug-in inspired by the small, buzzy British synths of the late 1970s:
two raw oscillators, a screaming state-variable filter, a control oscillator for wobbles and random bleeps,
and two envelopes that can repeat on their own.</p>
<img class="panel" src="{PANEL}">
<h3>Formats</h3>
<ul><li>macOS: Audio Unit, VST3 and standalone app (Apple Silicon and Intel), one .pkg installer.</li>
<li>Windows: VST3 and standalone app, one Setup.exe installer.</li>
<li>Linux: VST3, LV2 and standalone app (.tar.gz with an install script).</li></ul>
<h3>Installing</h3>
<p><b>macOS:</b> open <i>WAZP-{VERSION}-macOS.pkg</i> and follow the steps. Click <b>Customize</b> to choose formats.</p>
<div class="note"><b>&#9888; macOS security warning.</b> The WAZP installer is not notarised by Apple, so the first time you open it macOS
may say it &ldquo;cannot be opened because it is from an unidentified developer&rdquo; or &ldquo;cannot be verified&rdquo;.
This is expected. To install: <b>right-click</b> (or Control-click) the .pkg and choose <b>Open</b>, then click <b>Open</b> again.
If macOS still blocks it, go to <b>System Settings &rarr; Privacy &amp; Security</b>, scroll down and click <b>Open Anyway</b> next to the WAZP message.</div>
<p><b>Windows:</b> run <i>WAZP-{VERSION}-Windows-Setup.exe</i>. The VST3 goes to <i>C:\\Program Files\\Common Files\\VST3</i>.</p>
<p><b>Linux:</b> unpack the .tar.gz and run <i>./install.sh</i>.</p>
<p>Then rescan plug-ins in your DAW (Ableton Live: Settings &rarr; Plug-Ins &rarr; Rescan) and load WAZP on a MIDI track.</p>
<div class="note">WAZP is monophonic: one note at a time, last-note priority. Hold a note and play another for a legato line; with GLIDE up the pitch slides between them.</div>""")

    d.page("""<h2>The Panel</h2>
<p>Drag a knob up or down to turn it (scroll wheel works too), double-click to return it to its default. While you move a control,
the preset window shows its name and value. The window can be resized from the bottom-right corner.</p>
<h3>Keyboard</h3>""" + rows([
        ("SYNC", "Hard-syncs oscillator 2 to oscillator 1. Set oscillator 2 higher than oscillator 1 (range or PITCH) and sweep PITCH for the classic tearing sync sound."),
        ("GLIDE", "Portamento: how long the pitch takes to slide to a new note. Fully left = off, up to about 2 seconds."),
        ("Keys", "The on-panel keyboard is playable with the mouse (C3 - C5) and lights up with incoming MIDI notes. MIDI pitch bend (+/- 2 semitones) and the mod wheel (adds pitch modulation) also work."),
    ]) + "<h3>Oscillators</h3>" + rows([
        ("FT. (both)", "Footage / octave range: 32&prime;, 16&prime;, 8&prime;, 4&prime;, 2&prime;. 8&prime; plays at the pitch of the key."),
        ("WIDTH %", "Pulse width of oscillator 1, from 10 % to 90 % (50 % is a square wave). Only heard with the pulse waveform."),
        ("Wave (both)", "OFF, pulse (&#8851;) or sawtooth. OFF silences the oscillator."),
        ("PITCH", "Tunes oscillator 2 against oscillator 1, +/- 12 semitones. Small offsets give a fat detuned sound; 7 = a fifth, 12 = an octave."),
    ]) + "<h3>Mixer / Output</h3>" + rows([
        ("VOLUME", "Master output level, -80 dB to +6 dB."),
        ("OSC 1, OSC 2", "Level of each oscillator going into the filter (0 - 10). Push both up for more filter drive and grit."),
    ]))

    d.page("<h2>The Panel (continued)</h2><h3>Control Osc.</h3>" + rows([
        ("FREQ.", "Speed of the control oscillator, 0.05 Hz up to 1 kHz. It restarts with every new note. Above about 30 Hz it stops being a wobble and becomes audio-rate modulation: metallic, growling, FM-like tones."),
        ("PITCH MOD.", "How much the control oscillator moves the pitch of both oscillators (up to +/- 12 semitones). Vibrato at low settings, sirens and bleeps higher up."),
        ("NOISE SIGNAL", "Level of white noise in the mixer (0 - 10). Use it alone with the filter for wind, surf and percussion."),
        ("Wave", "Shape of the control oscillator: sine, square, noise (smoothed random movement) and RND (a new random value every cycle: sample and hold)."),
    ]) + "<h3>Filter</h3>" + rows([
        ("FREQ.", "Filter cutoff frequency, 20 Hz to 20 kHz."),
        ("Q", "Resonance. The filter is two stages in series, so it rings hard and screams; at the top it self-oscillates and whistles by itself, even with the oscillators off."),
        ("CONTROL", "Filter type: LO (low-pass), BAND (band-pass) or HI (high-pass)."),
        ("Left -/+ knob", "Amount of the control oscillator on the cutoff (wah, wobble, random filter steps), up to +/- 8 kHz. With a low FREQ. setting a deep amount can close the filter completely on the low part of the cycle, giving a gated, chopped sound. Centre = no modulation."),
        ("Right -/+ knob", "Amount of the CONTROL ENV. on the cutoff. Right of centre opens the filter, left of centre closes it (inverted envelope)."),
    ]) + "<h3>Envelope Generators</h3>" + rows([
        ("VCA ENV. ATTACK / DECAY", "How fast the sound rises when you press a key and how fast it falls to the sustain level. DECAY also sets the release time after you let go."),
        ("REPEAT / SUSTAIN LEVEL", "Fully left (REPEAT) the envelope keeps re-triggering itself while a key is held: automatic repeating notes. Turned right, it sets the level the sound holds at while the key is down."),
        ("CONTROL ENV. ATTACK / DECAY", "The envelope that sweeps the filter (through the left -/+ knob). It decays to zero."),
        ("REPEAT / DELAY", "Fully left (REPEAT) the control envelope loops while the key is held (rhythmic filter sweeps). Turned right, it delays the start of the control envelope by up to 2 seconds."),
    ]))

    d.page("""<h2>Presets</h2>
<p>The preset window at the top right shows the current preset number and name.</p>""" + rows([
        ("&lt; &nbsp;&gt;", "Step through the factory presets, followed by your own presets."),
        ("Preset display", "Click the name to open the full preset list: all factory presets, then your own presets (MY PRESETS). The current one is ticked; click any name to load it. OPEN PRESET FOLDER at the bottom shows your presets in Finder / Explorer."),
        ("SAVE", "Saves the current sound as a .wazp file. By default it goes to <i>Documents / Ticco Ross / WAZP / Presets</i>; presets saved there appear in the &lt; &gt; list."),
        ("LOAD", "Opens any .wazp file."),
    ]) + """<p>WAZP comes with 16 factory presets: INIT, BUZZ BASS, WASP STING, SYNC SCREAM, REPEAT PERC, DREXY ACID, NOISE WIND,
SIREN, RANDOM BLEEPS, HOLLOW LEAD, HIGH PASS FIZZ, BAND PLUCK, SUB DRONE, WOBBLE, SLOW SWELL and 8-BIT SQUARE.
The presets also appear in your DAW's program list.</p>
<div class="note">Your DAW saves the complete state (every knob and the preset name) with the project, so you don't need to save a preset to keep a sound in a song.</div>
<h2 style="margin-top:8mm">Automation &amp; MIDI</h2>
<p>All controls can be automated and MIDI-mapped from your DAW. WAZP responds to MIDI notes on any channel, pitch bend (+/- 2 semitones)
and the mod wheel (CC 1, adds pitch modulation from the control oscillator). Velocity is ignored, as on the original instrument.</p>
<h2 style="margin-top:8mm">Sound Design Tips</h2>
<ul><li><b>Classic buzz:</b> both oscillators on sawtooth, PITCH slightly off zero, Q around 50 %, a short CONTROL ENV. on the filter.</li>
<li><b>Sync lead:</b> SYNC on, oscillator 2 one range higher, then automate PITCH.</li>
<li><b>Self-playing rhythm:</b> VCA REPEAT fully left with a short DECAY gives repeating notes; add CONTROL ENV. REPEAT for moving filter pulses.</li>
<li><b>Bleeps:</b> control-osc wave on RND, FREQ. around 6 - 10 Hz, PITCH MOD. up.</li>
<li><b>Noise percussion:</b> oscillators OFF, NOISE SIGNAL up, BAND filter with high Q, short VCA decay.</li></ul>
<h2 style="margin-top:8mm">About &amp; Support</h2>
<p>Click the WAZP logo to open the About window, which shows the version, links to the store and contact address, and opens this manual.</p>
<p>Store: <b>ticcoross.gumroad.com</b> &nbsp;&nbsp; Contact: <b>ticcoross@gmail.com</b></p>""")
    return d


HISTORY = [
    ("1.0.4", [
        "A yellow arrow on the panel now shows that SYNC acts on the oscillators.",
        "Fixed: the left -/+ filter knob is now the control oscillator (LFO) amount and the right one the control envelope amount, matching the arrows on the panel.",
    ]),
    ("1.0.3", [
        "The control oscillator now sweeps the filter much harder, as on the original design (up to +/- 8 kHz).",
        "Control oscillator range extended to 1 kHz for audio-rate filter and pitch modulation; it restarts on every note.",
        "NOISE and RND control-oscillator shapes now use raw pink noise.",
        "New two-stage filter: much stronger resonance that screams and self-oscillates at full Q.",
        "Output drive stage after the filter for a louder, grittier sound.",
        "Click the preset display to see the full preset list (factory and your own) and pick any preset directly.",
    ]),
    ("1.0.2", [
        "The manual now has a clear macOS security warning section explaining how to open the installer.",
        "Documentation and repository text updated.",
    ]),
    ("1.0.1", [
        "The SYNC, OFF and ON labels are back on the panel next to the sync switch.",
        "The preset window shows the preset name as soon as the plug-in opens.",
    ]),
    ("1.0.0", [
        "First release of WAZP as an AU / VST3 / LV2 plug-in and standalone app for macOS, Windows and Linux.",
        "Two oscillators with 32' - 2' ranges, OFF / pulse / sawtooth, pulse width 10 - 90 % and oscillator 2 pitch.",
        "New: oscillator SYNC switch, in the place of the old BEND knob.",
        "New: MIXER section with separate OSC 1 and OSC 2 levels.",
        "Control oscillator (sine, square, noise, random) with pitch modulation and noise source.",
        "Resonant LO / BAND / HI filter with bipolar envelope and control-oscillator modulation.",
        "VCA and control envelopes with REPEAT mode, sustain level and delay.",
        "Glide, MIDI pitch bend and mod wheel; playable on-panel keyboard.",
        "Preset window with 16 factory presets, SAVE / LOAD of your own .wazp presets.",
        "Resizable panel, custom About window with store, contact and built-in manual.",
    ]),
]


def history():
    d = Doc("Update History")
    d.cover("Update History")
    body = "<h2>Update History</h2>"
    for ver, items in HISTORY:
        body += f"<h3>Version {ver}</h3><ul>" + "".join(f"<li>{i}</li>" for i in items) + "</ul>"
    d.page(body)
    return d


def to_pdf(html, out):
    from playwright.sync_api import sync_playwright
    with sync_playwright() as p:
        b = p.chromium.launch()
        pg = b.new_page()
        pg.set_content(html, wait_until="load")
        pg.pdf(path=str(out), format="A4", print_background=True, margin={"top": "0", "bottom": "0", "left": "0", "right": "0"})
        b.close()


if __name__ == "__main__":
    to_pdf(manual().html(), DOCS / "WAZP Manual.pdf")
    to_pdf(history().html(), DOCS / "WAZP Update History.pdf")
    print("done")
