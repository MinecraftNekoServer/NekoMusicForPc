#!/bin/bash
# Neko云音乐 Linux Build & Package Script
# Produces: Neko云音乐-{version}-linux-amd64.deb

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="build-linux"
BUILD_TYPE="${1:-Release}"

echo "========================================="
echo "  Neko云音乐 Linux Build Script"
echo "========================================="
echo "Build type: $BUILD_TYPE"
echo ""

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DLINUX=ON \
    -DCMAKE_INSTALL_PREFIX=/

# Build
echo ""
echo "Building..."
cmake --build . -j"$(nproc)"

# Get version from CMake
VERSION=$(grep -oP 'set\(PROJECT_VERSION\s+\K[^)]+' ../CMakeLists.txt)
SUFFIX=$(grep -oP 'set\(PROJECT_VERSION_SUFFIX\s+"?\K[^"]+' ../CMakeLists.txt | head -1)
FULL_VERSION="${VERSION}${SUFFIX}"

echo ""
echo "Version: $FULL_VERSION"

# Package with CPack
echo ""
echo "Packaging .deb..."
cpack -G DEB

# Find the output .deb file
DEB_FILE=$(find . -name "Neko云音乐-*.deb" -type f | head -1)

if [ -n "$DEB_FILE" ]; then
    DEB_BASENAME=$(basename "$DEB_FILE")
    OUTPUT_DIR="$SCRIPT_DIR/dist"
    mkdir -p "$OUTPUT_DIR"
    cp "$DEB_FILE" "$OUTPUT_DIR/"
    
    echo ""
    echo "========================================="
    echo "  Build Successful!"
    echo "========================================="
    echo "Output: dist/$DEB_BASENAME"
    echo ""
    echo "Install with:"
    echo "  sudo dpkg -i dist/$DEB_BASENAME"
    echo ""
    echo "Or:"
    echo "  sudo apt install ./dist/$DEB_BASENAME"
else
    echo "ERROR: .deb file not found!"
    exit 1
fi
