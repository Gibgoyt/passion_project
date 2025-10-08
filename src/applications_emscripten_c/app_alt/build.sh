#!/bin/bash

# Emscripten C WASM Build Script
# This script compiles the C code to WebAssembly using Emscripten

set -e  # Exit on error

echo "🚀 Starting Emscripten C WASM build..."
echo ""

# Check if emcc is installed
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten (emcc) is not installed or not in PATH"
    echo ""
    echo "To install Emscripten:"
    echo "  1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "  2. cd emsdk"
    echo "  3. ./emsdk install latest"
    echo "  4. ./emsdk activate latest"
    echo "  5. source ./emsdk_env.sh"
    echo ""
    exit 1
fi

# Get Emscripten version
EMCC_VERSION=$(emcc --version | head -n 1)
echo "📦 Using $EMCC_VERSION"
echo ""

# Navigate to the source directory
cd "$(dirname "$0")"

# Run make
make clean
make

echo ""
echo "✅ Build successful!"
echo "📁 Output: public/wasm/emscripten_c/"
echo ""
echo "Next steps:"
echo "  1. Navigate to http://localhost:4321/emscripten-c-spa"
echo "  2. Test the counter functionality"
echo "  3. Commit the built files to git"
