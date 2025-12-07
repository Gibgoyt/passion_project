# Mac M2 Memory System Unit Tests Makefile
#
# Comprehensive testing suite for Mac M1/M2 Apple Silicon memory management

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=native
INCLUDES = -I../../../simple_auth/memory

# Test target
TARGET = test_mac_m2_memory

# Source files (using relative paths to main project)
MEMORY_SRCS = ../../../simple_auth/memory/platform_detection.c \
              ../../../simple_auth/memory/mac_m1_2022/page_allocator.c \
              ../../../simple_auth/memory/mac_m1_2022/jwt_storage.c \
              ../../../simple_auth/memory/mac_m1_2022/memory_validation.c

# Object files
MEMORY_OBJS = $(MEMORY_SRCS:.c=.o)
TEST_OBJ = test_mac_m2_memory.o

.PHONY: all clean test run help

# Default target
all: $(TARGET)

# Compile test program
$(TARGET): $(TEST_OBJ) $(MEMORY_OBJS)
	@echo "🔗 Linking Mac M2 memory test..."
	$(CC) $(CFLAGS) $^ -o $@
	@echo "✅ Build complete: $(TARGET)"

# Compile test source
test_mac_m2_memory.o: test_mac_m2_memory.c
	@echo "🔨 Compiling test suite..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile memory management sources
../../../simple_auth/memory/%.o: ../../../simple_auth/memory/%.c
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(TARGET)
	@echo "🚀 Running Mac M2 memory system tests..."
	@echo "========================================"
	./$(TARGET)

# Quick test run
run: test

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(TARGET) $(TEST_OBJ) $(MEMORY_OBJS)
	@echo "✅ Clean complete"

# Performance test with timing
benchmark: $(TARGET)
	@echo "📊 Running performance benchmarks..."
	@echo "===================================="
	time ./$(TARGET)

# Valgrind memory check (if available)
memcheck: $(TARGET)
	@echo "🔍 Running memory leak detection..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --tool=memcheck --leak-check=full --track-origins=yes ./$(TARGET); \
	else \
		echo "⚠️  Valgrind not available on macOS, running basic test instead"; \
		./$(TARGET); \
	fi

# Install dependencies (macOS)
deps:
	@echo "📦 Checking macOS development dependencies..."
	@if ! command -v gcc >/dev/null 2>&1; then \
		echo "❌ GCC not found. Install Xcode command line tools:"; \
		echo "   xcode-select --install"; \
		exit 1; \
	fi
	@echo "✅ All dependencies satisfied"

# Help
help:
	@echo "Mac M2 Memory System Unit Tests"
	@echo "==============================="
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build test program (default)"
	@echo "  test       - Build and run all tests"
	@echo "  run        - Alias for test"
	@echo "  benchmark  - Run with timing information"
	@echo "  memcheck   - Run with memory leak detection"
	@echo "  clean      - Remove build artifacts"
	@echo "  deps       - Check development dependencies"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Platform: Mac M1/M2 (Apple Silicon)"
	@echo "Page size: 16KB"
	@echo "Compiler: $(CC)"