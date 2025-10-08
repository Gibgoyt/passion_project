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

# Remove auto-generated .gitignore (we want to commit these files)
rm -f ../../../public/wasm/admin_app/.gitignore

echo "✅ Admin WASM build complete! Output in public/wasm/admin_app/"
echo "📝 Removed auto-generated .gitignore (files are now tracked by git)"
