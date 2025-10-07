# Emscripten C WebAssembly Applications

This directory contains C applications compiled to WebAssembly using Emscripten.

## 📁 Structure

```
src/applications_emscripten_c/
├── .gitignore          # Ignore build artifacts (*.o, *.bc, etc.)
├── README.md           # This file
└── app/                # Main dashboard app
    ├── main.c          # C source code
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

Unlike Leptos/Rust apps, **Emscripten builds are committed to git** for Cloudflare deployment.

The compiled WASM files go to `public/wasm/emscripten_c/` and are tracked in version control.

## 🔧 Adding New Apps

To add a new Emscripten C app:

1. Create a new directory: `mkdir my-app`
2. Add C source files
3. Create a Makefile (use `app/Makefile` as template)
4. Update output directory in Makefile
5. Build and commit the WASM output

## 📝 Available Apps

- **app** - Simple dashboard with interactive counter
  - Route: `/emscripten-c-spa`
  - Output: `public/wasm/emscripten_c/`
