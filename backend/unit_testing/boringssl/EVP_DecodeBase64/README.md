# BoringSSL Base64 Unit Test Suite

A comprehensive test suite for debugging JWT signature base64 decoding issues with BoringSSL.

## 🎯 Objective

This test suite is designed to isolate and debug why JWT signature base64url decoding fails with BoringSSL while JWT header decoding works correctly. The tests progressively analyze from simple to complex cases to pinpoint the exact failure point.

## 🏗️ Project Structure

```
base64_url/
├── src/
│   ├── base64url.h           # Base64URL conversion interface
│   ├── base64url.c           # Base64URL implementation
│   └── test_boringssl.c      # Comprehensive test suite
├── results/                  # Test output directory (created automatically)
├── Makefile                  # Standalone build system
└── README.md                 # This documentation
```

## 🚀 Quick Start

### Prerequisites

1. **BoringSSL**: Must be available in one of these locations:
   - `../../uSockets/boringssl_safe/`
   - `../../uSockets/boringssl/`
   - `../../../uSockets/boringssl_safe/`
   - `../../../uSockets/boringssl/`
   - `/usr/local/include/boringssl`
   - `/opt/boringssl`

2. **Compiler**: GCC with C99 support
3. **Optional**: Valgrind for memory testing

### Build and Run Tests

```bash
# Check dependencies and build
make setup

# Run comprehensive test suite
make test

# Analyze results
make analyze

# Generate report
make report
```

## 🧪 Test Categories

### 1. Known Good Base64 Strings
Tests simple, verified base64 strings to establish baseline BoringSSL functionality:
- Empty strings
- Single characters
- Short strings ("Hello", "Hello World")
- Medium length strings (100+ chars)

### 2. JWT Header Case (Working)
Tests the JWT header that successfully decodes:
```
Original: eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjJha0tZVzlNdFFubUJiQ2k1UUx2alEifQ
Converts to base64 and tests with BoringSSL
```

### 3. JWT Signature Case (Failing)
Tests the JWT signature that fails to decode:
```
Original: pNsFdzW3xTjyM-PC9_NrfClY2VQMie2y84MqYwXQ_EZVnGeL2WB8WgaLb3STo92IA1_WP4x_...
Converts to base64 and tests with BoringSSL
```

### 4. Progressive Length Testing
Tests substrings of increasing length to find the exact breaking point:
- 50, 100, 150, 200, 250, 300, full length
- Identifies at what length BoringSSL starts failing

### 5. API Variations and Edge Cases
Tests different BoringSSL API usage patterns:
- Different buffer sizes (exact, too small, too large)
- NULL pointer handling
- Zero-length inputs
- Boundary conditions

## 🔍 Test Output Analysis

Each test provides comprehensive diagnostics:

### ✅ Success Indicators
- `return_code: 1 ✅ SUCCESS`
- Decoded data displayed in hex and ASCII
- Output length reported

### ❌ Failure Indicators
- `return_code: 0 ❌ FAILED`
- Character validation failures
- Buffer size issues

### 📊 Analysis Data
- Character distribution (uppercase, lowercase, digits, special chars)
- Length analysis (total length, mod 4, padding)
- Invalid character detection with positions

## 🛠️ Available Make Targets

### Build Targets
- `make debug` - Build debug version (default)
- `make release` - Build optimized release version
- `make clean` - Remove all build artifacts

### Test Targets
- `make test` - Run comprehensive test suite with logging
- `make test-quick` - Run tests without logging
- `make memtest` - Run with Valgrind memory checking
- `make perftest` - Run performance/timing test

### Analysis Targets
- `make analyze` - Analyze test results and show summary
- `make failures` - Show detailed failure information
- `make successes` - Show successful test cases
- `make report` - Generate comprehensive markdown report

### Info Targets
- `make info` - Show build configuration
- `make help` - Show all available targets
- `make check-deps` - Verify all dependencies

## 🔧 Build Configuration

The Makefile automatically detects:
- BoringSSL installation path
- Symbol prefix configuration (for uSockets integration)
- Available libraries and headers

### Manual Configuration

If automatic detection fails, you can set paths manually:

```bash
# Set BoringSSL root path
export BORINGSSL_ROOT=/path/to/boringssl

# Build with custom path
make debug BORINGSSL_ROOT=/custom/path
```

## 📋 Expected Test Results

### What This Test Will Reveal

1. **Length Limits**: Maximum string length BoringSSL can handle
2. **Character Issues**: Specific problematic character combinations
3. **Buffer Problems**: Memory alignment or sizing issues
4. **API Usage**: Correct vs incorrect BoringSSL API usage
5. **Root Cause**: Exact point and reason for JWT signature failure

### Typical Workflow

1. **Run Tests**: `make test`
2. **Check Summary**: `make analyze`
3. **Examine Failures**: `make failures`
4. **Generate Report**: `make report`
5. **Review Details**: Open `results/test_results.txt`

## 📁 Output Files

All test output is saved in the `results/` directory:

- `test_results.txt` - Complete test output
- `analysis.txt` - Extracted analysis data
- `report.md` - Comprehensive markdown report
- `memtest_results.txt` - Valgrind output (if run)
- `performance_results.txt` - Timing data (if run)

## 🐛 Debugging Tips

### If Compilation Fails

1. Check BoringSSL path: `make info`
2. Verify dependencies: `make check-deps`
3. Try building BoringSSL: `make build-boringssl`

### If Tests Fail

1. Start with simple cases: Look at "Known Good Base64" results
2. Compare working vs failing: JWT Header vs JWT Signature results
3. Find breaking point: Check progressive length test results
4. Memory issues: Run `make memtest`

### Common Issues

1. **BoringSSL not found**: Install or build BoringSSL in expected location
2. **Symbol conflicts**: Check if BORINGSSL_PREFIX is needed
3. **Memory errors**: Buffer size calculations may be incorrect
4. **Length issues**: BoringSSL may have internal length limits

## 🔗 Integration Notes

This is a completely standalone test suite, designed to be:
- **Independent**: No dependencies on other project components
- **Self-contained**: Own build system and documentation
- **Portable**: Works with different BoringSSL installations
- **Comprehensive**: Tests all relevant failure scenarios

## 📞 Support

The test suite is designed to be self-diagnosing. If you encounter issues:

1. Run `make help` for available options
2. Check `make info` for configuration details
3. Review `results/test_results.txt` for detailed output
4. Use `make analyze` for automated problem detection

## 🎓 Understanding the Results

The test output uses emojis for quick visual scanning:
- 🧪 Test sections
- 📝 Input data
- 📊 Parameters and results
- ✅ Success indicators
- ❌ Failure indicators
- 🔍 Analysis sections
- 📋 Summaries and next steps

This comprehensive test suite will definitively identify the root cause of your JWT signature decoding issues and provide clear guidance for implementing a fix.