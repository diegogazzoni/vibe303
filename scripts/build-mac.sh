#!/usr/bin/env bash
# =============================================================================
# build-mac.sh — Build Acid303 VST3 for macOS (Universal Binary)
# Tested on macOS 12+ with Xcode 14+ and CMake 3.22+
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$ROOT/build"
JUCE_DIR="$ROOT/JUCE"

# ── Colour output ─────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; NC='\033[0m'
info()    { echo -e "${CYAN}[acid303]${NC} $*"; }
success() { echo -e "${GREEN}[acid303]${NC} $*"; }
error()   { echo -e "${RED}[acid303] ERROR:${NC} $*" >&2; exit 1; }

# ── Dependency checks ─────────────────────────────────────────────────────────
command -v cmake  >/dev/null 2>&1 || error "CMake not found. Run: brew install cmake"
command -v git    >/dev/null 2>&1 || error "git not found. Run: xcode-select --install"
xcode-select -p   >/dev/null 2>&1 || error "Xcode Command Line Tools not found. Run: xcode-select --install"

info "CMake $(cmake --version | head -1)"
info "$(clang --version | head -1)"

# ── Clone JUCE if needed ──────────────────────────────────────────────────────
if [ ! -d "$JUCE_DIR/modules" ]; then
    info "Cloning JUCE 8.0.4..."
    git clone --depth=1 --branch 8.0.4 \
        https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
else
    info "JUCE already present — skipping clone."
fi

# ── Configure ─────────────────────────────────────────────────────────────────
info "Configuring (Universal Binary: arm64 + x86_64)..."
cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# ── Build ─────────────────────────────────────────────────────────────────────
JOBS=$(sysctl -n hw.logicalcpu)
info "Building with $JOBS parallel jobs..."
cmake --build "$BUILD_DIR" --config Release --parallel "$JOBS"

# ── Report ────────────────────────────────────────────────────────────────────
ARTEFACT="$BUILD_DIR/Acid303_artefacts/Release/VST3/Acid303.vst3"
INSTALLED=~/Library/Audio/Plug-Ins/VST3/Acid303.vst3

success "Build complete!"
info "  Bundle : $ARTEFACT"
info "  Installed : $INSTALLED"
info ""
info "Architectures:"
lipo -archs "$ARTEFACT/Contents/MacOS/Acid303"
info ""
info "If macOS blocks the plugin (Gatekeeper), run:"
info "  xattr -rd com.apple.quarantine \"$INSTALLED\""
info "Then rescan VST3 plugins in your DAW."
