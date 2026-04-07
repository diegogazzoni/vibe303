@echo off
REM =============================================================================
REM build-windows.bat — Build Acid303 VST3 for Windows (x64)
REM Requires: Visual Studio 2019/2022 (with C++ Desktop workload), CMake, git
REM =============================================================================
setlocal EnableDelayedExpansion

set "ROOT=%~dp0.."
set "BUILD_DIR=%ROOT%\build"
set "JUCE_DIR=%ROOT%\JUCE"

echo [acid303] Checking dependencies...

where cmake >nul 2>&1
if errorlevel 1 (
    echo [acid303] ERROR: CMake not found.
    echo           Install from https://cmake.org/download/ and add to PATH.
    exit /b 1
)

where git >nul 2>&1
if errorlevel 1 (
    echo [acid303] ERROR: git not found.
    echo           Install from https://git-scm.com/
    exit /b 1
)

cmake --version | findstr /i "cmake version"

REM ── Clone JUCE if needed ────────────────────────────────────────────────────
if not exist "%JUCE_DIR%\modules" (
    echo [acid303] Cloning JUCE 8.0.4...
    git clone --depth=1 --branch 8.0.4 ^
        https://github.com/juce-framework/JUCE.git "%JUCE_DIR%"
    if errorlevel 1 (
        echo [acid303] ERROR: Failed to clone JUCE.
        exit /b 1
    )
) else (
    echo [acid303] JUCE already present -- skipping clone.
)

REM ── Detect Visual Studio ────────────────────────────────────────────────────
REM Try VS 2022 first, then VS 2019
set "VS_GENERATOR="
for /f "tokens=*" %%i in ('cmake --help 2^>^&1 ^| findstr /i "Visual Studio 17"') do (
    set "VS_GENERATOR=Visual Studio 17 2022"
)
if "!VS_GENERATOR!"=="" (
    for /f "tokens=*" %%i in ('cmake --help 2^>^&1 ^| findstr /i "Visual Studio 16"') do (
        set "VS_GENERATOR=Visual Studio 16 2019"
    )
)
if "!VS_GENERATOR!"=="" (
    echo [acid303] ERROR: Visual Studio 2019 or 2022 not found.
    echo           Install from https://visualstudio.microsoft.com/
    echo           Make sure "Desktop development with C++" workload is selected.
    exit /b 1
)

echo [acid303] Using generator: !VS_GENERATOR!

REM ── Configure ───────────────────────────────────────────────────────────────
echo [acid303] Configuring...
cmake -S "%ROOT%" -B "%BUILD_DIR%" ^
    -G "!VS_GENERATOR!" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [acid303] ERROR: CMake configure failed.
    exit /b 1
)

REM ── Build ───────────────────────────────────────────────────────────────────
echo [acid303] Building...
cmake --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 (
    echo [acid303] ERROR: Build failed.
    exit /b 1
)

REM ── Report ──────────────────────────────────────────────────────────────────
set "ARTEFACT=%BUILD_DIR%\Acid303_artefacts\Release\VST3\Acid303.vst3"
set "INSTALLED=C:\Program Files\Common Files\VST3\Acid303.vst3"

echo.
echo [acid303] Build complete!
echo           Bundle    : %ARTEFACT%
echo           Installed : %INSTALLED%
echo.
echo [acid303] In Ableton Live: Preferences ^> Plug-Ins ^> Rescan VST3 folder.

endlocal
