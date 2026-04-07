#!/usr/bin/env bash
# =============================================================================
# build-linux.sh — Build Acid303 VST3 for Linux (x86_64)
# Tested on Ubuntu 22.04 / Debian 12. Adapt package names for other distros.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$ROOT/build"
JUCE_DIR="$ROOT/JUCE"

# ── Colour output ─────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()    { echo -e "${CYAN}[acid303]${NC} $*"; }
success() { echo -e "${GREEN}[acid303]${NC} $*"; }
warn()    { echo -e "${YELLOW}[acid303] WARN:${NC} $*"; }
error()   { echo -e "${RED}[acid303] ERROR:${NC} $*" >&2; exit 1; }

# ── Detect package manager ────────────────────────────────────────────────────
install_deps() {
    if command -v apt-get >/dev/null 2>&1; then
        info "Detected apt (Debian/Ubuntu). Installing dependencies..."
        sudo apt-get update -qq
        sudo apt-get install -y \
            build-essential cmake git pkg-config \
            libasound2-dev \
            libfreetype6-dev \
            libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
            libgl1-mesa-dev \
            libcurl4-openssl-dev \
            webkit2gtk-4.0 libgtk-3-dev
    elif command -v dnf >/dev/null 2>&1; then
        info "Detected dnf (Fedora/RHEL). Installing dependencies..."
        sudo dnf install -y \
            gcc-c++ cmake git pkg-config \
            alsa-lib-devel \
            freetype-devel \
            libX11-devel libXext-devel libXinerama-devel libXrandr-devel libXcursor-devel \
            mesa-libGL-devel \
            libcurl-devel \
            webkit2gtk4.0-devel gtk3-devel
    elif command -v pacman >/dev/null 2>&1; then
        info "Detected pacman (Arch Linux). Installing dependencies..."
        sudo pacman -Sy --noconfirm \
            base-devel cmake git pkg-config \
            alsa-lib \
            freetype2 \
            libx11 libxext libxinerama libxrandr libxcursor \
            mesa \
            curl \
            webkit2gtk gtk3
    else
        warn "Unknown package manager. Please install manually:"
        warn "  cmake git gcc/clang alsa-lib freetype2 libX11 libXext"
        warn "  libXinerama libXrandr libXcursor mesa libcurl webkit2gtk gtk3"
    fi
}

# ── Dependency checks ─────────────────────────────────────────────────────────
MISSING=0
for cmd in cmake git pkg-config; do
    command -v "$cmd" >/dev/null 2>&1 || { warn "Missing: $cmd"; MISSING=1; }
done

if [ "$MISSING" -eq 1 ]; then
    read -rp "Install missing dependencies now? [Y/n] " REPLY
    REPLY="${REPLY:-Y}"
    if [[ "$REPLY" =~ ^[Yy]$ ]]; then
        install_deps
    else
        error "Cannot continue without dependencies."
    fi
fi

info "CMake $(cmake --version | head -1)"
info "$(${CXX:-c++} --version | head -1)"

# ── Clone JUCE if needed ──────────────────────────────────────────────────────
if [ ! -d "$JUCE_DIR/modules" ]; then
    info "Cloning JUCE 8.0.4..."
    git clone --depth=1 --branch 8.0.4 \
        https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
else
    info "JUCE already present — skipping clone."
fi

# ── Configure ─────────────────────────────────────────────────────────────────
info "Configuring..."
cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release

# ── Build ─────────────────────────────────────────────────────────────────────
JOBS=$(nproc 2>/dev/null || echo 4)
info "Building with $JOBS parallel jobs..."
cmake --build "$BUILD_DIR" --config Release --parallel "$JOBS"

# ── Report ────────────────────────────────────────────────────────────────────
ARTEFACT="$BUILD_DIR/Acid303_artefacts/Release/VST3/Acid303.vst3"
INSTALLED="${HOME}/.vst3/Acid303.vst3"

success "Build complete!"
info "  Bundle    : $ARTEFACT"
info "  Installed : $INSTALLED"
info ""
info "In Ableton Live / Bitwig / REAPER: rescan VST3 plugin folders."
