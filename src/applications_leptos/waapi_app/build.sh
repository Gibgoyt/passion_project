#!/bin/bash

# Build script for WaAPI Leptos WASM app
# This compiles Rust to WebAssembly and generates JS bindings

set -e

echo "💚 Building WaAPI WASM app..."

cd "$(dirname "$0")"

# Check if wasm-pack is installed
if ! command -v wasm-pack &> /dev/null; then
    echo "❌ wasm-pack not found. Installing..."
    cargo install wasm-pack
fi

# Build for web target with optimizations
# Output to public/wasm/waapi_app/ so Astro can serve the files
wasm-pack build --target web --out-dir ../../../public/wasm/waapi_app --release

# Remove auto-generated .gitignore (we want to commit these files)
rm -f ../../../public/wasm/waapi_app/.gitignore

echo "✅ WaAPI WASM build complete! Output in public/wasm/waapi_app/"
echo "📝 Removed auto-generated .gitignore (files are now tracked by git)"
