# Mac M2 Page Allocator Unit Testing

This directory contains standalone unit tests for the Apple Silicon M1/M2 memory management system used in the authentication server.

## Features Tested

- **16KB Page Allocation**: Apple Silicon optimized page management
- **Secure Memory Operations**: Memory locking and secure clearing
- **JWT Storage**: Cryptographic buffer management for JWT tokens
- **Platform Detection**: Automatic Mac M1/M2 detection and optimization
- **Performance Testing**: Allocation/deallocation performance benchmarks

## Quick Start

```bash
# Clean any previous builds
make clean

# Build and run tests
make test

# Or build manually and run
make
./test_mac_memory
```

## Build Targets

- `make` or `make all` - Build the test program
- `make clean` - Remove build artifacts
- `make test` - Build and run all tests
- `make debug` - Build with debug symbols
- `make info` - Show build information
- `make install` - Install to /usr/local/bin (requires sudo)

## Expected Output

The test should show:
- ✅ Memory system validation passed
- ✅ Page allocator validation complete
- ✅ JWT storage validation complete
- ✅ Performance tests passed

## Current Known Issues

1. **Memory Deallocation**: The allocator doesn't properly track freed pages, causing allocation failures after multiple alloc/free cycles
2. **Page Reuse**: Need to implement proper free page tracking for memory reuse

## System Requirements

- Apple Silicon Mac (M1/M2/M3)
- macOS with 16KB page size
- GCC or Clang compiler
- Memory locking privileges (for mlock tests)

## Architecture

- **page_allocator.c/h**: Core 16KB page management
- **jwt_storage.c/h**: Secure JWT token storage
- **memory_validation.c/h**: Comprehensive system validation
- **platform_detection.c/h**: Cross-platform compatibility layer
- **test_mac_memory.c**: Main test program