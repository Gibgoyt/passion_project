# Mac M2 Memory System Unit Tests

Comprehensive test suite for the Mac M1/M2 Apple Silicon memory management system.

## Overview

This directory contains unit tests specifically designed for the Mac M1/M2 memory allocator that handles:

- **16KB Page Management**: Optimized for Apple Silicon's 16KB page size
- **Secure JWT Storage**: Crypto buffer allocation with integrity checking
- **Memory Locking**: Page-level memory protection and locking
- **Performance Testing**: Allocation/deallocation performance benchmarks
- **Corruption Detection**: Magic number validation and integrity checks

## Features Tested

### ✅ Core Functionality
- Memory system initialization and cleanup
- Crypto buffer allocation and deallocation
- Data integrity and validation
- Multiple buffer management
- Buffer size limits and validation

### ✅ Performance
- Allocation/deallocation speed benchmarks
- Memory usage efficiency
- Stress testing with rapid allocations

### ✅ Security
- Memory corruption detection
- Magic number validation
- Secure memory clearing
- Buffer integrity checks

### ✅ Platform-Specific
- 16KB page size optimization
- Apple Silicon memory architecture
- macOS-specific memory locking
- Platform detection

## Quick Start

```bash
# Build and run all tests
make test

# Run performance benchmarks
make benchmark

# Check for memory leaks (basic test on macOS)
make memcheck

# Clean build artifacts
make clean

# Show all available commands
make help
```

## Test Categories

### 1. Basic Operations
- Memory system initialization
- Buffer allocation/deallocation
- Data read/write operations

### 2. Multiple Buffer Management
- Concurrent buffer allocation
- Buffer validation across multiple instances
- Proper cleanup of multiple buffers

### 3. Size Limit Testing
- Zero-size allocation rejection
- Maximum size allocation (2048 bytes)
- Oversized allocation rejection

### 4. Performance Benchmarks
- 1000-iteration allocation/deallocation cycle
- Average time per operation measurement
- Performance target validation (< 1ms per cycle)

### 5. Corruption Detection
- Magic number validation
- Buffer integrity verification
- Corruption simulation and detection

### 6. Platform Features
- 16KB page size verification
- Memory locking capabilities
- Platform-specific optimizations

### 7. Stress Testing
- 100+ rapid allocation/deallocation cycles
- Variable buffer sizes
- Success rate measurement

## Expected Results

When all tests pass, you should see:

```
🎉 ALL TESTS PASSED!
✅ Mac M2 memory system is fully functional
🚀 Ready for production deployment
```

## Performance Targets

- **Allocation Speed**: < 1ms per allocation/deallocation cycle
- **Success Rate**: > 80% success rate under stress conditions
- **Memory Efficiency**: Optimal use of 16KB pages
- **Validation Speed**: Instant buffer integrity checking

## Dependencies

- **macOS**: Required (Apple Silicon M1/M2)
- **Xcode Command Line Tools**: For GCC compiler
- **16KB Page Support**: Automatic on Apple Silicon

## Architecture

The memory system uses a platform-detection layer that automatically selects the Mac M1/M2 implementation:

```
Platform Detection Layer
         ↓
Mac M1/M2 Implementation
         ↓
16KB Page Allocator → JWT Storage → Crypto Buffers
```

## Integration

These unit tests validate the memory system used by the main authentication server. The same memory management code is used in production for:

- JWT token storage and processing
- Secure cryptographic operations
- Session management
- OAuth 2.1 PKCE flow data

## Troubleshooting

If tests fail:

1. **Check Platform**: Ensure running on Mac M1/M2
2. **Check Permissions**: Some memory locking may require privileges
3. **Check Memory**: Ensure sufficient available memory
4. **Check Compilation**: Verify Xcode command line tools installed

## Development

To modify or extend tests:

1. Add new test functions following the pattern in `test_mac_m2_memory.c`
2. Use the `TEST_START`, `TEST_ASSERT`, and `TEST_END` macros
3. Update the main function to call new tests
4. Rebuild with `make clean && make test`