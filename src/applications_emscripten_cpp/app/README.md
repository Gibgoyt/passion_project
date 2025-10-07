# Emscripten C++ WebAssembly Application

A simple dashboard with an interactive counter, built with C++ and compiled to WebAssembly using Emscripten.

## 🏗️ Project Structure

```
src/applications_emscripten_cpp/app/
├── main.cpp         # Main C++ source code
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
- `public/wasm/emscripten_cpp/app.js` - JavaScript glue code
- `public/wasm/emscripten_cpp/app.wasm` - WebAssembly binary

## 🎯 Features

- **Dashboard UI** - Clean interface with Tailwind CSS
- **Interactive Counter** - Increment, decrement, and reset
- **C++ STL** - Uses iostream, namespaces, std::cout
- **C++17 Standard** - Modern C++ features
- **WebAssembly** - Compiled to WASM for browser execution

## 📦 Deployment

**We commit the built WASM files** to git for Cloudflare deployment:

```bash
git add public/wasm/emscripten_cpp/
git commit -m "Build Emscripten C++ WASM app"
git push
```

## 🌐 Access

Visit: `http://localhost:4321/emscripten-cpp-api`

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

The following C++ functions are exported (with `extern "C"`) and callable from JavaScript:

- `init_dashboard()` - Initialize the UI
- `increment_counter()` - Increment counter
- `decrement_counter()` - Decrement counter
- `reset_counter()` - Reset counter to 0

## 🎨 Styling

Uses Tailwind CSS classes defined in `src/styles/global.css`.

## 🔍 C++ Features Used

- **Namespaces** - Anonymous namespace for counter state
- **iostream** - std::cout for logging
- **std::string** - String manipulation
- **extern "C"** - C linkage for JavaScript interop
- **C++17** - Modern standard
