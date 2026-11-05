# WAZP

**WAZP** is a monophonic synthesizer plugin (AU / VST3 / LV2 + standalone) inspired by the small, buzzy, cult British synths of the late 1970s.
Two oscillators with footage ranges and pulse width, oscillator sync, a mixer, a control oscillator with noise and random modulation,
a low/band/high-pass filter, two envelopes with repeat mode, glide and preset management.
It started life as a Max for Live device (WOSP) and has been rebuilt as a standalone plugin.

By **Ticco Ross** - [ticcoross.gumroad.com](https://ticcoross.gumroad.com/) - ticcoross@gmail.com

## Download

Installers are on the [Releases](../../releases) page: macOS `.pkg`, Windows `Setup.exe`, Linux `.tar.gz`, plus the manual and update history PDFs.

## Building

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE 8 is downloaded automatically. Other scripts:

- `tools/rasterise_ui.py` renders the panel artwork in `assets/ui_src` into `assets/ui`
- `docs/manual/build_docs.py <version>` rebuilds the manual and update history PDFs
- `tools/make_pkg.sh` builds the macOS installer, `installer/windows/WAZP.iss` the Windows one

Pushing to `main` builds everything in GitHub Actions; a commit message containing `[release]` (or a `v*` tag) publishes a release.
