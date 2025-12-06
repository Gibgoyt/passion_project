# HTTP/1.1 SSL Unit Testing Makefile
# Based on unit_testing/libmdbx and examples/ patterns

# Compiler configuration
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=native -flto

# uSockets paths (corrected to actual location)
USOCKETS_INCLUDE = /Users/ahmed/dev/uWebSockets/uSockets/src
USOCKETS_LIB = /Users/ahmed/dev/uWebSockets/uSockets/uSockets.a

# OpenSSL paths (using Homebrew OpenSSL)
OPENSSL_INCLUDE = /opt/homebrew/include
OPENSSL_LIB_DIR = /opt/homebrew/lib

# Include directories
INCLUDES = -I$(USOCKETS_INCLUDE) \
           -I$(OPENSSL_INCLUDE)

# Library linking (using system OpenSSL instead of BoringSSL)
# Note: liburing and -ldl are Linux-specific, not needed on macOS
# -lstdc++ is needed because uSockets library contains C++ code
LDFLAGS = $(USOCKETS_LIB) \
          -L$(OPENSSL_LIB_DIR) -lssl -lcrypto \
          -lpthread -lstdc++ -lz -lm

# Build targets
TARGETS = server client

# Default target
all: $(TARGETS)

# Pattern rule: compile .c files to executables
server: server.c
	@echo "Building SSL HTTPS server..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

client: client.c
	@echo "Building SSL HTTPS client..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# Test target - run automated tests
test: all
	@echo "Running SSL HTTPS unit tests..."
	@./test_script.sh

# Clean target
clean:
	@echo "Cleaning up..."
	rm -f $(TARGETS)
	rm -rf data/
	rm -f *.log

# Install target (optional)
install: all
	@echo "Installing to /usr/local/bin..."
	sudo cp $(TARGETS) /usr/local/bin/

# Run server in background for testing
run-server: server
	@echo "Starting SSL HTTPS server on port 2053..."
	@echo "Access at: https://localhost:2053"
	@echo "Press Ctrl+C to stop"
	./server

# Run client test against local server
run-client: client
	@echo "Running SSL client validation test..."
	./client localhost 2053

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build both server and client"
	@echo "  server      - Build SSL HTTPS server only"
	@echo "  client      - Build SSL HTTPS client only"
	@echo "  test        - Run automated test suite"
	@echo "  run-server  - Start server on port 2053"
	@echo "  run-client  - Test client against localhost:2053"
	@echo "  clean       - Remove built binaries"
	@echo "  install     - Install binaries to /usr/local/bin"
	@echo "  help        - Show this help message"

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@test -f $(USOCKETS_LIB) || (echo "ERROR: uSockets library not found at $(USOCKETS_LIB)" && exit 1)
	@test -d $(OPENSSL_INCLUDE) || (echo "ERROR: OpenSSL include directory not found at $(OPENSSL_INCLUDE)" && exit 1)
	@test -f $(OPENSSL_LIB_DIR)/libssl.a || (echo "ERROR: OpenSSL libssl.a not found at $(OPENSSL_LIB_DIR)/libssl.a" && exit 1)
	@test -f $(OPENSSL_LIB_DIR)/libcrypto.a || (echo "ERROR: OpenSSL libcrypto.a not found at $(OPENSSL_LIB_DIR)/libcrypto.a" && exit 1)
	@test -f /etc/ssl/splitdo_api/cert.pem || (echo "WARNING: SSL certificate not found at /etc/ssl/splitdo_api/cert.pem" && exit 1)
	@test -f /etc/ssl/splitdo_api/private/key.pem || (echo "WARNING: SSL private key not found at /etc/ssl/splitdo_api/private/key.pem" && exit 1)
	@echo "✅ All dependencies found"

# Build with debug symbols
debug: CFLAGS += -g -DDEBUG -O0
debug: all

# Build with verbose output
verbose: CFLAGS += -v
verbose: all

# Phony targets
.PHONY: all clean test install run-server run-client help check-deps debug verbose