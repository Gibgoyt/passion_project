# Emscripten C WebAssembly Application

A simple dashboard with an interactive counter, built with pure C and compiled to WebAssembly using Emscripten.

## 🏗️ Project Structure

```
src/applications_emscripten_c/
├── main.c           # Main C source code
├── Makefile         # Build configuration
├── build.sh         # Build script
└── README.md        # This file
```

## 🚀 Building

### Prerequisites

Install Emscripten:

```bash
# Clone emsdk
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate latest version
./emsdk install latest
./emsdk activate latest

# Activate PATH (run this in each new terminal session)
source ./emsdk_env.sh
```

### Build the WASM module

```bash
# From this directory
./build.sh

# Or using make directly
make
```

This will output:
- `public/wasm/emscripten_c/app.js` - JavaScript glue code
- `public/wasm/emscripten_c/app.wasm` - WebAssembly binary

## 🎯 Features

- **Dashboard UI** - Clean interface with Tailwind CSS
- **Interactive Counter** - Increment, decrement, and reset
- **Pure C** - All logic written in C, no JavaScript
- **WebAssembly** - Compiled to WASM for browser execution

## 📦 Deployment

Unlike the Leptos app, **we commit the built WASM files** to git for Cloudflare deployment:

```bash
git add public/wasm/emscripten_c/
git commit -m "Build Emscripten C WASM app"
git push
```

## 🌐 Access

Visit: `http://localhost:4321/emscripten-c-spa`

## 🔧 Development

### Rebuild

```bash
make rebuild
```

### Clean

```bash
make clean
```

## 📝 Exported Functions

The following C functions are exported and callable from JavaScript:

- `init_dashboard()` - Initialize the UI
- `increment_counter()` - Increment counter
- `decrement_counter()` - Decrement counter
- `reset_counter()` - Reset counter to 0

## 🎨 Styling

Uses Tailwind CSS classes defined in `src/styles/global.css`.
