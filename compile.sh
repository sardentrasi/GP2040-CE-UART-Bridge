#!/bin/bash
# ============================================================================
# GP2040-CE v0.7.12 — Compile Script for Waveshare RP2040 Zero (UART Bridge)
# ============================================================================
set -e

BOARD="WaveshareZero"
BUILD_DIR="build"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Pico SDK path
export PICO_SDK_PATH="/home/pico-sdk"

echo "============================================"
echo " GP2040-CE Firmware Builder"
echo " Board: ${BOARD}"
echo "============================================"
echo ""

# --- Step 1: Check prerequisites ---
echo "[1/4] Checking prerequisites..."

MISSING=""
command -v cmake >/dev/null 2>&1 || MISSING="${MISSING} cmake"
command -v arm-none-eabi-gcc >/dev/null 2>&1 || MISSING="${MISSING} arm-none-eabi-gcc"
command -v python3 >/dev/null 2>&1 || MISSING="${MISSING} python3"

if [ -n "$MISSING" ]; then
    echo ""
    echo "ERROR: Missing required tools:${MISSING}"
    echo ""
    echo "Install with:"
    echo "  sudo apt update"
    echo "  sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \\"
    echo "      build-essential libstdc++-arm-none-eabi-newlib python3 git nodejs npm"
    echo ""
    exit 1
fi

echo "  cmake:              $(cmake --version | head -1)"
echo "  arm-none-eabi-gcc:  $(arm-none-eabi-gcc --version | head -1)"
echo "  python3:            $(python3 --version)"

# Fix for Python 3.12+ (pkg_resources removed from stdlib)
pip3 install setuptools --quiet 2>/dev/null || pip install setuptools --quiet 2>/dev/null || true
echo ""

# --- Step 2: Clean & create build directory ---
echo "[2/4] Preparing build directory..."
cd "$PROJECT_DIR"

if [ -d "$BUILD_DIR" ]; then
    echo "  Removing old build..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
echo "  Created ${BUILD_DIR}/"
echo ""

# --- Step 3: CMake configure ---
echo "[3/4] Running CMake configure..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release \
      -DGP2040_BOARDCONFIG="${BOARD}" \
      ..
echo ""

# --- Step 4: Build ---
NPROC=$(nproc 2>/dev/null || echo 2)
echo "[4/4] Building with ${NPROC} threads..."
make -j"${NPROC}"
echo ""

# --- Done ---
UF2_FILE=$(find . -name "*.uf2" -type f | head -1)
if [ -n "$UF2_FILE" ]; then
    UF2_SIZE=$(du -h "$UF2_FILE" | cut -f1)
    UF2_FULL="$PROJECT_DIR/$BUILD_DIR/$(basename "$UF2_FILE")"
    echo "============================================"
    echo " BUILD SUCCESS!"
    echo "============================================"
    echo ""
    echo " Firmware: ${UF2_FULL}"
    echo " Size:     ${UF2_SIZE}"
    echo ""
    echo " Flash instructions:"
    echo "   1. Hold BOOTSEL on Waveshare RP2040 Zero"
    echo "   2. Plug in USB (device mounts as RPI-RP2)"
    echo "   3. cp ${UF2_FULL} /media/\$USER/RPI-RP2/"
    echo ""
else
    echo "ERROR: Build completed but no .uf2 file found!"
    exit 1
fi
