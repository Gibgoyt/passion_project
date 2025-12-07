# Mac M2 Page Allocator Unit Testing Makefile
# Standalone testing for Apple Silicon M1/M2 memory management system

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=native -flto

# Source files for Mac M2 memory system
SOURCES = platform_detection.c \
          page_allocator.c \
          jwt_storage.c \
          memory_validation.c \
          test_mac_memory.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TARGET = test_mac_memory

# Default target
all: $(TARGET)
	@echo "✅ Mac M2 page allocator test compiled successfully"
	@echo "🚀 Run './$(TARGET)' to execute tests"

# Build the test program
$(TARGET): $(OBJECTS)
	@echo "🔗 Linking Mac M2 page allocator test..."
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule for object files
%.o: %.c
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "🧹 Cleaning Mac M2 page allocator build artifacts..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✅ Clean complete"

# Run the test
test: $(TARGET)
	@echo "🧪 Running Mac M2 page allocator tests..."
	@echo "========================================"
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG -O0
debug: clean all

# Print information
info:
	@echo "Mac M2 Page Allocator Unit Testing"
	@echo "=================================="
	@echo "Target: $(TARGET)"
	@echo "Sources: $(SOURCES)"
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo ""
	@echo "Available targets:"
	@echo "  all    - Build the test program"
	@echo "  clean  - Remove build artifacts"
	@echo "  test   - Build and run tests"
	@echo "  debug  - Build with debug symbols"
	@echo "  info   - Show this information"

# Install (for system-wide testing)
install: $(TARGET)
	@echo "📦 Installing Mac M2 page allocator test..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "✅ Installed to /usr/local/bin/$(TARGET)"

# Phony targets
.PHONY: all clean test debug info install

# Dependencies (simplified - in a real project you'd use makedepend or similar)
platform_detection.o: platform_detection.c platform_detection.h
page_allocator.o: page_allocator.c page_allocator.h
jwt_storage.o: jwt_storage.c jwt_storage.h page_allocator.h
memory_validation.o: memory_validation.c memory_validation.h page_allocator.h jwt_storage.h
test_mac_memory.o: test_mac_memory.c platform_detection.h