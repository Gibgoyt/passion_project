# Oracle VM.Standard.A1.Flex Memory Allocation Unit Test

This directory contains a comprehensive unit test for validating system assumptions about Oracle VM.Standard.A1.Flex instances, specifically focusing on memory allocation and system characteristics.

## Purpose

This test validates that the Oracle A1.Flex virtual machine environment matches expected specifications:

- **Architecture**: ARM64 (aarch64) with Ampere Altra processors
- **Page Size**: 4KB (4096 bytes) - standard for ARM64 Linux
- **Cache Line**: 64-byte L1 cache line size
- **Memory**: Proper DDR4 memory alignment and allocation behavior
- **OS**: Oracle Linux UEK kernel with standard mmap functionality

## Files

- `a1_flex_memory_test.c` - Main test program
- `README.md` - This documentation file

## Compilation

To compile the test program:

```bash
gcc -o a1_flex_memory_test a1_flex_memory_test.c -std=c99
```

### Alternative compilation options:

For debug build:
```bash
gcc -o a1_flex_memory_test a1_flex_memory_test.c -std=c99 -g -O0 -DDEBUG
```

For optimized build:
```bash
gcc -o a1_flex_memory_test a1_flex_memory_test.c -std=c99 -O2
```

## Usage

Run the compiled test program:

```bash
./a1_flex_memory_test
```

## Test Categories

The program performs the following validation tests:

### 1. System Architecture Validation
- Verifies ARM64 (aarch64) architecture
- Confirms Oracle Linux UEK kernel

### 2. Processor Information
- Detects Ampere Altra processor characteristics
- Validates ARM Neoverse core implementation

### 3. Page Size Validation
- Tests `getpagesize()` returns 4096 bytes
- Verifies `sysconf(_SC_PAGESIZE)` consistency
- Confirms 4KB page size standard

### 4. Cache Line Size Validation
- Checks L1 cache line size is 64 bytes
- Uses `sysconf(_SC_LEVEL1_DCACHE_LINESIZE)`

### 5. Pointer Characteristics
- Validates 64-bit pointer size
- Confirms little-endian byte order

### 6. Basic Memory Allocation
- Tests `mmap()` with anonymous private mapping
- Verifies memory read/write access
- Tests `munmap()` cleanup

### 7. Memory Alignment
- Checks page-aligned memory allocation
- Verifies cache-line alignment (64-byte)

### 8. Memory Protection
- Tests memory protection flags
- Validates `mprotect()` functionality

### 9. System Memory Information
- Reports total and available memory
- Performs sanity checks on memory values

### 10. Huge Page Support Detection
- Tests 2MB huge page availability
- Notes if huge pages are configured

## Expected Output

### Successful Run
```
ORACLE VM.STANDARD.A1.FLEX MEMORY ALLOCATION UNIT TEST
====================================================

=== ORACLE VM.STANDARD.A1.FLEX SYSTEM INFORMATION ===
System: Linux 6.12.0-104.43.4.3.el9uek.aarch64 aarch64
Hostname: [hostname]
Page size: 4096 bytes
L1 cache line: 64 bytes
Total memory: [X] MB
Pointer size: 8 bytes

=== RUNNING VALIDATION TESTS ===
[PASS] System Architecture Validation - ARM64 (aarch64) on Oracle Linux UEK
[PASS] Processor Information - Ampere Altra (ARM Neoverse) processor detected
[PASS] Page Size Validation - 4KB (4096 bytes) page size confirmed
[PASS] Cache Line Size Validation - 64-byte L1 cache line size confirmed
[PASS] Pointer Characteristics - 64-bit pointers, little-endian confirmed
[PASS] Basic Memory Allocation - mmap/munmap allocation and access successful
[PASS] Memory Alignment - Page and cache-line alignment confirmed
[PASS] Memory Protection - Memory protection and mprotect working
[PASS] System Memory Information - Total: [X] MB, Available: [Y] MB
[PASS] Huge Page Support - Huge pages not available (normal for default config)

=== TEST SUMMARY ===
Passed: 10/10 tests
Success rate: 100.0%

✓ All Oracle A1.Flex system assumptions validated successfully!
```

### Exit Codes
- **0**: All tests passed - system matches expected A1.Flex characteristics
- **1**: One or more tests failed - system may not be A1.Flex or configured differently

## Troubleshooting

### Common Issues

**Permission Errors:**
- Ensure you have permission to allocate memory with mmap
- Run with sufficient user privileges

**Compilation Errors:**
- Ensure gcc is installed: `sudo dnf install gcc`
- Use C99 standard: `-std=c99`

**Test Failures:**
- Check if running on actual Oracle A1.Flex instance
- Verify Oracle Linux UEK kernel is installed
- Confirm ARM64 architecture

**Huge Page Test:**
- Huge page failure is normal and expected on default configurations
- To enable huge pages: `echo 10 > /proc/sys/vm/nr_hugepages`

## System Requirements

- **OS**: Oracle Linux with UEK kernel
- **Architecture**: ARM64 (aarch64)
- **Processor**: Ampere Altra
- **Compiler**: GCC with C99 support
- **Memory**: Sufficient RAM for basic allocations (test uses minimal memory)

## Notes

- This test is designed specifically for Oracle Cloud Infrastructure A1.Flex shapes
- The test validates your research assumptions about the physical hardware
- All tests are read-only system queries except for basic memory allocation tests
- No system configuration is modified by this test
- Test results confirm the predictable nature of Oracle A1.Flex instances

## Integration

This test can be integrated into larger test suites or CI/CD pipelines:

```bash
# Run test and capture exit code
./a1_flex_memory_test
if [ $? -eq 0 ]; then
    echo "Oracle A1.Flex validation passed"
else
    echo "Oracle A1.Flex validation failed"
    exit 1
fi
```