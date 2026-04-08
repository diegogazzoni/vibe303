# vibe303 — Roland TB-303 Bass Line VST3 Emulator

A faithful software emulation of the Roland TB-303 Bass Line synthesizer, built with C++17 and JUCE 8. Produces the classic acid squelch used in tech house, acid house, and techno.

---

## Controls

| Control | Range | Description |
|---|---|---|
| **Tuning** | ±12 semitones | Global pitch offset |
| **Cutoff** | 20 Hz – 15 kHz | VCF cutoff frequency (log scale) |
| **Resonance** | 0 – 100% | Filter resonance (non-self-oscillating, faithful to hardware) |
| **Env Mod** | 0 – 100% | How much the filter envelope opens the cutoff |
| **Decay** | 2 ms – 2 s | Filter envelope decay time (also sets amp decay) |
| **Accent** | 0 – 100% | Accent intensity — boosts amplitude and filter sweep |
| **Volume** | 0 – 100% | Output level |
| **Wave** | SAW / SQ | Oscillator waveform: sawtooth or square |

### Accent (via velocity)
Notes with MIDI velocity **> 100** trigger accent mode:
- Amplitude boosted up to +6 dB
- Filter envelope decays ~50% faster for a snappier sweep
- Filter env mod amount increased proportionally to the Accent knob

### Slide (via legato)
Play a new note **before releasing the previous one** to engage slide (portamento).  
- Pitch glides from the previous note to the new note over ~60 ms
- The amplitude envelope does not retrigger — the gate stays open
- The filter envelope **does** retrigger for the characteristic slide sweep

---

## Synthesis Architecture

```
MIDI Note
    │
    ▼
VCO  ── Sawtooth or Square (PolyBLEP anti-aliased, with portamento)
    │
    ▼
VCA ◄── AD Amplitude Envelope (instant attack)
    │ ◄── Accent gain boost
    │
    ▼
4-Pole Diode Ladder VCF (24 dB/oct, tanh saturation per stage)
    │ ◄── AD Filter Envelope (Decay knob)
    │ ◄── Env Mod amount
    │ ◄── Resonance
    │ ◄── Accent (faster decay + extra env mod)
    │
    ▼
Output × Volume
```

---

## Project Structure

```
303-vst/
├── CMakeLists.txt           — Build definition (JUCE CMake API)
├── README.md                — This file
├── JUCE/                    — JUCE 8.0.4 (git clone, not committed)
└── Source/
    ├── Oscillator.h         — VCO: sawtooth/square with PolyBLEP + portamento
    ├── AcidEnvelope.h       — AD envelope (instant attack, variable decay)
    ├── AccentSlide.h        — Accent boost and slide/legato detection
    ├── AcidFilter.h         — 4-pole diode ladder filter (header-only)
    ├── AcidFilter.cpp       — Compilation unit for AcidFilter
    ├── PluginProcessor.h    — AudioProcessor declaration
    ├── PluginProcessor.cpp  — Parameter tree, processBlock, MIDI handling
    ├── PluginEditor.h       — GUI declaration (custom LookAndFeel)
    └── PluginEditor.cpp     — GUI implementation (knobs, waveform toggle)
```

---

## Requirements

| Tool | Version | Install |
|---|---|---|
| **macOS** | 12.0 Monterey or later | — |
| **Xcode** | 14 or later (15+ recommended) | Mac App Store |
| **Xcode Command Line Tools** | matching Xcode version | `xcode-select --install` |
| **Homebrew** | any | `https://brew.sh` |
| **CMake** | 3.22 or later | `brew install cmake` |
| **git** | any | bundled with Xcode CLT |

---

## Building from Source

### 1. Clone JUCE

JUCE is not committed to this repository. Clone it once before building:

```bash
git clone --depth=1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git JUCE
```

This must be run inside the `303-vst` project directory so JUCE lands at `303-vst/JUCE/`.

### 2. Configure

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

- `arm64;x86_64` produces a **Universal Binary** that runs natively on both Apple Silicon and Intel Macs.
- Omit the architectures flag to build only for your host machine (faster compile during development).

### 3. Build

```bash
cmake --build build --config Release --parallel
```

Build time is approximately 2–4 minutes on first run (compiles all JUCE modules).  
Subsequent incremental builds take ~5 seconds.

### 4. Output location

The compiled plugin bundle is placed at:

```
build/Acid303_artefacts/Release/VST3/Acid303.vst3
```

---

## Installing the Plugin

JUCE automatically copies the built plugin to your user VST3 folder on every successful build (`COPY_PLUGIN_AFTER_BUILD TRUE` in `CMakeLists.txt`).

**Automatic install path:**
```
~/Library/Audio/Plug-Ins/VST3/Acid303.vst3
```

To install system-wide (available to all users on the machine), copy manually:

```bash
sudo cp -r build/Acid303_artefacts/Release/VST3/Acid303.vst3 \
           /Library/Audio/Plug-Ins/VST3/
```

---

## Loading in Ableton Live

1. Open Ableton Live.
2. Go to **Preferences → Plug-Ins**.
3. Make sure **"Use VST3 Plug-In Custom Folder"** is enabled, or that the user VST3 path (`~/Library/Audio/Plug-Ins/VST3`) is scanned.
4. Click **"Rescan"** (or restart Live).
5. In the browser, go to **Plug-ins → VST3** — you will find **Acid303** under manufacturer **AcidLab**.
6. Drag it onto a MIDI track.

> **Note:** On first load, macOS Gatekeeper may block the plugin because it is not notarized. To allow it:
> ```bash
> xattr -rd com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Acid303.vst3
> ```
> Then rescan in Ableton.

---

## Programming Acid Basslines in Ableton

### Basic sequence
- Draw MIDI notes in a clip on the Acid303 track.
- Keep notes short (16th or 32nd notes) for the classic staccato 303 feel.

### Accent
- Any note with velocity **> 100** will trigger accent mode (louder hit + stronger filter sweep).
- In the MIDI clip editor, select a note and raise its velocity above 100 in the velocity lane.

### Slide
- Overlap two consecutive notes (the second note's start time is before the first note's end time) to engage slide/portamento.
- The pitch glides smoothly between them over ~60 ms without retriggering the amplitude envelope.

### Typical starting patch for tech house acid
| Knob | Starting value |
|---|---|
| Cutoff | ~300 Hz |
| Resonance | ~70% |
| Env Mod | ~60% |
| Decay | ~200 ms |
| Accent | ~60% |
| Waveform | SAW |

Then automate the **Cutoff** and **Env Mod** knobs over the track for movement.

---

## One-liner build + install (copy-paste)

```bash
# Run once from inside the 303-vst directory
git clone --depth=1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git JUCE && \
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" && \
cmake --build build --config Release --parallel
```

After this, `Acid303.vst3` is automatically installed to `~/Library/Audio/Plug-Ins/VST3/`.

---

## Troubleshooting

| Problem | Solution |
|---|---|
| CMake not found | `brew install cmake` |
| JUCE directory missing | Run the `git clone` step above |
| Plugin not visible in Ableton | Rescan VST3 folder in Preferences → Plug-Ins |
| "Plugin blocked by Gatekeeper" | Run the `xattr` command above |
| Build errors about Xcode SDK | Run `xcode-select --install` and accept the license: `sudo xcodebuild -license accept` |
| Only arm64 built (no Universal) | Pass `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` to the cmake configure step |

---

## License

This project is provided for personal, educational, and musical use.  
JUCE is subject to the [JUCE License](https://juce.com/juce-8-licence/).  
"Roland" and "TB-303" are trademarks of Roland Corporation. This plugin is an independent software emulation and is not affiliated with or endorsed by Roland.
