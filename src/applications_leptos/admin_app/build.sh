#!/bin/bash

# Build script for Admin Leptos WASM app
# This compiles Rust to WebAssembly and generates JS bindings

set -e

echo "🔒 Building Admin WASM app..."

cd "$(dirname "$0")"

# Check if wasm-pack is installed
if ! command -v wasm-pack &> /dev/null; then
    echo "❌ wasm-pack not found. Installing..."
    cargo install wasm-pack
fi

# Build for web target with optimizations
# Output to public/wasm/admin_app/ so Astro can serve the files
wasm-pack build --target web --out-dir ../../../public/wasm/admin_app --release

echo "✅ Admin WASM build complete! Output in public/wasm/admin_app/"
