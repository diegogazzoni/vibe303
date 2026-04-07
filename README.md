# vibe303 — Roland TB-303 Bass Line VST3 Emulator

A faithful software emulation of the Roland TB-303 Bass Line synthesizer, built with C++17 and JUCE 8. Produces the classic acid squelch used in tech house, acid house, and techno.

**Platforms:** macOS 12+ · Windows 10+ · Linux (Ubuntu 22.04+)

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
├── CMakeLists.txt           — Build definition (JUCE CMake API, cross-platform)
├── README.md                — This file
├── .github/
│   └── workflows/
│       └── build.yml        — GitHub Actions CI (macOS + Windows + Linux)
├── scripts/
│   ├── build-mac.sh         — Automated macOS build script
│   ├── build-linux.sh       — Automated Linux build script
│   └── build-windows.bat    — Automated Windows build script
├── JUCE/                    — JUCE 8.0.4 (git clone, not committed)
└── Source/
    ├── Oscillator.h         — VCO: sawtooth/square with PolyBLEP + portamento
    ├── AcidEnvelope.h       — AD envelope (instant attack, variable decay)
    ├── AccentSlide.h        — Accent boost and slide/legato detection
    ├── AcidFilter.h         — 4-pole diode ladder filter (header)
    ├── AcidFilter.cpp       — Filter implementation
    ├── PluginProcessor.h    — AudioProcessor declaration
    ├── PluginProcessor.cpp  — Parameter tree, processBlock, MIDI handling
    ├── PluginEditor.h       — GUI declaration (custom LookAndFeel)
    └── PluginEditor.cpp     — GUI implementation (knobs, waveform toggle)
```

---

## Requirements

### macOS

| Tool | Version | Install |
|---|---|---|
| macOS | 12.0 Monterey or later | — |
| Xcode | 14 or later (15+ recommended) | Mac App Store |
| Xcode Command Line Tools | matching Xcode | `xcode-select --install` |
| CMake | 3.22 or later | `brew install cmake` |
| git | any | bundled with Xcode CLT |

### Windows

| Tool | Version | Install |
|---|---|---|
| Windows | 10 or later (64-bit) | — |
| Visual Studio | 2019 or 2022 (with "Desktop development with C++" workload) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| CMake | 3.22 or later | bundled with VS, or [cmake.org](https://cmake.org/download/) |
| git | any | [git-scm.com](https://git-scm.com) |

### Linux (Ubuntu / Debian)

| Tool | Notes |
|---|---|
| GCC 11+ or Clang 14+ | `sudo apt install build-essential` |
| CMake 3.22+ | `sudo apt install cmake` |
| git | `sudo apt install git` |
| ALSA, X11, FreeType, GL, GTK3, WebKit2GTK | see below |

Install all Linux system dependencies at once:

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake git \
    libasound2-dev libfreetype6-dev \
    libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
    libgl1-mesa-dev libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev libgtk-3-dev
```

For Fedora/RHEL replace `apt-get install` with `dnf install` and adjust package names (`alsa-lib-devel`, `freetype-devel`, `libX11-devel`, `gtk3-devel`, `webkit2gtk3-devel`, etc.).  
For Arch Linux use `pacman -S` (`alsa-lib`, `freetype2`, `libx11`, `gtk3`, `webkit2gtk`, etc.).

---

## Building from Source

### Step 1 — Clone JUCE

JUCE is not committed to this repository. Clone it once inside the project directory:

```bash
git clone --depth=1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git JUCE
```

### Step 2 — Configure

**macOS (Universal Binary — arm64 + x86_64):**
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
```

**macOS (native architecture only — faster dev builds):**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

**Windows (MSVC x64 — run in Developer Command Prompt or PowerShell):**
```bat
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
```

**Linux:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### Step 3 — Build

```bash
cmake --build build --config Release --parallel
```

First build takes 2–4 minutes (compiles all JUCE modules). Subsequent incremental builds take ~5 seconds.

### Step 4 — Output location

```
build/Acid303_artefacts/Release/VST3/Acid303.vst3
```

The plugin is also **automatically copied** to the system VST3 folder on every successful build (`COPY_PLUGIN_AFTER_BUILD TRUE` in `CMakeLists.txt`).

---

## Automated Build Scripts

One-command scripts are provided in `scripts/` for each platform.

**macOS:**
```bash
chmod +x scripts/build-mac.sh && ./scripts/build-mac.sh
```

**Linux:**
```bash
chmod +x scripts/build-linux.sh && ./scripts/build-linux.sh
```

**Windows (run as Administrator or in Developer Command Prompt):**
```bat
scripts\build-windows.bat
```

Each script handles: dependency checks, JUCE clone (if missing), CMake configure, build, and reports the install path.

---

## Install Paths

| Platform | Auto-install path |
|---|---|
| macOS | `~/Library/Audio/Plug-Ins/VST3/Acid303.vst3` |
| Windows | `C:\Program Files\Common Files\VST3\Acid303.vst3` |
| Linux | `~/.vst3/Acid303.vst3` |

### Manual install (macOS system-wide)
```bash
sudo cp -r build/Acid303_artefacts/Release/VST3/Acid303.vst3 \
           /Library/Audio/Plug-Ins/VST3/
```

### Manual install (Linux system-wide)
```bash
sudo cp -r build/Acid303_artefacts/Release/VST3/Acid303.vst3 \
           /usr/lib/vst3/
```

---

## Loading in a DAW

### Ableton Live (macOS / Windows)

1. Open Ableton Live.
2. Go to **Preferences → Plug-Ins**.
3. Enable **"Use VST3 Plug-In Custom Folder"** or confirm the system VST3 path is scanned.
4. Click **"Rescan"** (or restart Live).
5. In the browser, go to **Plug-ins → VST3** — find **Acid303** under manufacturer **AcidLab**.
6. Drag it onto a MIDI track.

### Other DAWs

Any VST3-compatible DAW (Bitwig, Reaper, Cubase, FL Studio, LMMS, etc.) will find the plugin at the standard VST3 path after a rescan.

---

## Gatekeeper (macOS only)

On first load, macOS may block the unsigned plugin. Run this once, then rescan in your DAW:

```bash
xattr -rd com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Acid303.vst3
```

---

## Programming Acid Basslines in Ableton

### Basic sequence
- Draw MIDI notes in a clip on the Acid303 track.
- Keep notes short (16th or 32nd notes) for the classic staccato 303 feel.

### Accent
- Any note with velocity **> 100** triggers accent mode (louder hit + stronger filter sweep).
- In the MIDI clip editor, select a note and raise its velocity above 100 in the velocity lane.

### Slide
- Overlap two consecutive notes (the second note starts before the first note ends) to engage slide.
- The pitch glides smoothly over ~60 ms without retriggering the amplitude envelope.

### Typical starting patch for tech house acid

| Knob | Starting value |
|---|---|
| Cutoff | ~300 Hz |
| Resonance | ~70% |
| Env Mod | ~60% |
| Decay | ~200 ms |
| Accent | ~60% |
| Waveform | SAW |

Automate **Cutoff** and **Env Mod** over the track for movement.

---

## CI / CD

GitHub Actions builds the plugin on all three platforms on every push to `main`/`master` and every pull request. Compiled `.vst3` bundles are uploaded as workflow artifacts (retained 30 days).

See `.github/workflows/build.yml`.

---

## Troubleshooting

| Problem | Solution |
|---|---|
| CMake not found (macOS) | `brew install cmake` |
| CMake not found (Linux) | `sudo apt install cmake` |
| JUCE directory missing | Run the `git clone` step (Step 1 above) |
| Plugin not visible in DAW | Rescan VST3 folder in DAW preferences |
| Gatekeeper blocks plugin (macOS) | Run the `xattr` command above |
| Xcode SDK errors (macOS) | `xcode-select --install` then `sudo xcodebuild -license accept` |
| Only arm64 built, not Universal | Pass `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` to the configure step |
| Missing headers (Linux) | Install all system deps listed in Requirements above |
| MSVC not found (Windows) | Open the VS Developer Command Prompt, or install the "Desktop C++" workload in VS |
| LSP errors in editor (all) | False positives — the editor lacks `compile_commands.json`. The code compiles correctly with CMake. |

---

## License

This project is provided for personal, educational, and musical use.  
JUCE is subject to the [JUCE License](https://juce.com/juce-8-licence/).  
"Roland" and "TB-303" are trademarks of Roland Corporation. This plugin is an independent software emulation and is not affiliated with or endorsed by Roland.
