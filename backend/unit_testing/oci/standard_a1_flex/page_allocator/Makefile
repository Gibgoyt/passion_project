# Simple Makefile for Oracle A1.Flex Memory Test

CC=gcc
CFLAGS=-std=c99 -Wall -D_GNU_SOURCE
TARGET=a1_flex_memory_test
SOURCE=a1_flex_memory_test.c

# Default target
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Clean target
clean:
	rm -f $(TARGET)

# Run test
test: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Optimized build
release: CFLAGS += -O2
release: $(TARGET)

.PHONY: clean test debug release