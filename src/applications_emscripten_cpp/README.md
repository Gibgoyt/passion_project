# Emscripten C++ WebAssembly Applications

This directory contains C++ applications compiled to WebAssembly using Emscripten.

## 📁 Structure

```
src/applications_emscripten_cpp/
├── .gitignore          # Ignore build artifacts (*.o, *.bc, etc.)
├── README.md           # This file
└── app/                # Main dashboard app
    ├── main.cpp        # C++ source code
    ├── Makefile        # Build configuration
    ├── build.sh        # Build script
    └── README.md       # App-specific docs
```

## 🚀 Getting Started

### Prerequisites

Install Emscripten:

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Building Apps

Each app has its own build script:

```bash
cd app
./build.sh
```

## 📦 Deployment

**Emscripten builds are committed to git** for Cloudflare deployment.

The compiled WASM files go to `public/wasm/emscripten_cpp/` and are tracked in version control.

## 🔧 Adding New Apps

To add a new Emscripten C++ app:

1. Create a new directory: `mkdir my-cpp-app`
2. Add C++ source files (`.cpp`)
3. Create a Makefile (use `app/Makefile` as template)
4. Use `em++` compiler instead of `emcc`
5. Update output directory in Makefile
6. Build and commit the WASM output

## 📝 Available Apps

- **app** - Simple dashboard with interactive counter
  - Route: `/emscripten-cpp-spa`
  - Output: `public/wasm/emscripten_cpp/`
  - Language: C++17
  - Features: STL, iostream, namespaces
