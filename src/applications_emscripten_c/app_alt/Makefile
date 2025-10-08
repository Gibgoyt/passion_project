# Emscripten C to WASM Build Configuration

# Compiler
EMCC = emcc

# Source files
SOURCES = main.c

# Output files
OUTPUT_DIR = ../../../public/wasm/emscripten_c/app_alt
OUTPUT_JS = $(OUTPUT_DIR)/app.js
OUTPUT_WASM = $(OUTPUT_DIR)/app.wasm

# Compiler flags
CFLAGS = -O3 \
	-s WASM=1 \
	-s EXPORTED_FUNCTIONS='["_main","_increment_counter","_decrement_counter","_reset_counter","_handle_route_change","_navigate_to"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
	-s MODULARIZE=1 \
	-s EXPORT_NAME='createEmscriptenModule' \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ENVIRONMENT='web' \
	-s INVOKE_RUN=0

# Default target
all: $(OUTPUT_JS)

# Build the WASM module
$(OUTPUT_JS): $(SOURCES)
	@echo "🔨 Compiling C to WebAssembly with Emscripten..."
	@mkdir -p $(OUTPUT_DIR)
	$(EMCC) $(SOURCES) $(CFLAGS) -o $(OUTPUT_JS)
	@echo "✅ Build complete!"
	@echo "   JS:   $(OUTPUT_JS)"
	@echo "   WASM: $(OUTPUT_WASM)"

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(OUTPUT_JS) $(OUTPUT_WASM) $(OUTPUT_DIR)/*.data
	@echo "✅ Clean complete!"

# Rebuild from scratch
rebuild: clean all

# Help target
help:
	@echo "Emscripten C WASM Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make         - Build the WASM module"
	@echo "  make clean   - Remove build artifacts"
	@echo "  make rebuild - Clean and rebuild"
	@echo "  make help    - Show this help message"

.PHONY: all clean rebuild help
