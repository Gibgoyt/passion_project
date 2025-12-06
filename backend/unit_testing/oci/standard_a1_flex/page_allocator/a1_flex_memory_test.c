/*
 * Oracle VM.Standard.A1.Flex Memory Allocation Unit Test
 *
 * This standalone C program validates system assumptions about Oracle A1.Flex instances,
 * specifically focusing on page allocation and memory characteristics.
 *
 * Expected Oracle A1.Flex characteristics:
 * - ARM64 Ampere Altra processor
 * - 4KB (4096 bytes) page size
 * - 64-byte L1 cache line size
 * - DDR4 memory with proper alignment
 * - Oracle Linux with standard mmap behavior
 *
 * Compilation: gcc -o a1_flex_memory_test a1_flex_memory_test.c -std=c99 -D_GNU_SOURCE
 * Usage: ./a1_flex_memory_test
 */

#define _DEFAULT_SOURCE

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/utsname.h>

/* Test result structure */
typedef struct {
    const char* test_name;
    int passed;
    const char* error_msg;
    const char* details;
} test_result_t;

/* Global test statistics */
static int total_tests = 0;
static int passed_tests = 0;

/* Utility function to print test results */
void print_test_result(test_result_t result) {
    const char* status = result.passed ? "PASS" : "FAIL";
    printf("[%s] %s", status, result.test_name);

    if (result.details) {
        printf(" - %s", result.details);
    }

    if (!result.passed && result.error_msg) {
        printf(" - ERROR: %s", result.error_msg);
    }

    printf("\n");

    total_tests++;
    if (result.passed) {
        passed_tests++;
    }
}

/* Test 1: System Architecture Validation */
test_result_t test_system_architecture() {
    test_result_t result = {"System Architecture Validation", 0, NULL, NULL};

    struct utsname system_info;
    if (uname(&system_info) != 0) {
        result.error_msg = "Failed to get system information";
        return result;
    }

    /* Check for ARM64 architecture */
    if (strcmp(system_info.machine, "aarch64") != 0) {
        result.error_msg = "Expected aarch64 architecture";
        return result;
    }

    /* Check for Oracle Linux */
    if (strstr(system_info.release, "el9uek") == NULL) {
        result.error_msg = "Expected Oracle Linux UEK kernel";
        return result;
    }

    result.passed = 1;
    result.details = "ARM64 (aarch64) on Oracle Linux UEK";
    return result;
}

/* Test 2: Processor Information Validation */
test_result_t test_processor_info() {
    test_result_t result = {"Processor Information", 0, NULL, NULL};

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) {
        result.error_msg = "Cannot read /proc/cpuinfo";
        return result;
    }

    char line[256];
    int found_ampere = 0;

    while (fgets(line, sizeof(line), cpuinfo)) {
        /* Look for ARM Neoverse-N1 processor (used in Oracle A1.Flex) */
        if (strstr(line, "CPU implementer") && strstr(line, "0x41")) {
            found_ampere = 1; /* ARM Limited implementer ID */
        }
        if (strstr(line, "CPU part") && strstr(line, "0xd0c")) {
            found_ampere = 1; /* ARM Neoverse-N1 part ID */
        }
        if (strstr(line, "model name") && strstr(line, "Neoverse")) {
            found_ampere = 1; /* Ampere Altra uses ARM Neoverse cores */
        }
    }

    fclose(cpuinfo);

    if (!found_ampere) {
        result.error_msg = "Did not detect Ampere Altra processor";
        return result;
    }

    result.passed = 1;
    result.details = "ARM Neoverse-N1 processor detected (Oracle A1.Flex)";
    return result;
}

/* Test 3: Page Size Validation */
test_result_t test_page_size() {
    test_result_t result = {"Page Size Validation", 0, NULL, NULL};

    /* Test getpagesize() */
    int page_size_getpagesize = getpagesize();

    /* Test sysconf(_SC_PAGESIZE) */
    long page_size_sysconf = sysconf(_SC_PAGESIZE);

    /* Expected page size for ARM64 Linux */
    const int expected_page_size = 4096; /* 4KB */

    if (page_size_getpagesize != expected_page_size) {
        result.error_msg = "getpagesize() returned unexpected value";
        return result;
    }

    if (page_size_sysconf != expected_page_size) {
        result.error_msg = "sysconf(_SC_PAGESIZE) returned unexpected value";
        return result;
    }

    if (page_size_getpagesize != page_size_sysconf) {
        result.error_msg = "getpagesize() and sysconf() values don't match";
        return result;
    }

    result.passed = 1;
    result.details = "4KB (4096 bytes) page size confirmed";
    return result;
}

/* Test 4: Cache Line Size Validation */
test_result_t test_cache_line_size() {
    test_result_t result = {"Cache Line Size Validation", 0, NULL, NULL};

    long cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    /* Expected cache line size for Ampere Altra */
    const long expected_cache_line_size = 64; /* 64 bytes */

    if (cache_line_size == -1) {
        result.error_msg = "Cannot determine L1 cache line size";
        return result;
    }

    if (cache_line_size != expected_cache_line_size) {
        result.error_msg = "Unexpected L1 cache line size";
        return result;
    }

    result.passed = 1;
    result.details = "64-byte L1 cache line size confirmed";
    return result;
}

/* Test 5: Pointer Size and Endianness */
test_result_t test_pointer_characteristics() {
    test_result_t result = {"Pointer Characteristics", 0, NULL, NULL};

    /* Check pointer size (should be 64-bit) */
    if (sizeof(void*) != 8) {
        result.error_msg = "Expected 64-bit pointers";
        return result;
    }

    /* Check endianness (ARM64 is little-endian) */
    uint32_t test_value = 0x12345678;
    uint8_t* bytes = (uint8_t*)&test_value;

    if (bytes[0] != 0x78) {
        result.error_msg = "Expected little-endian byte order";
        return result;
    }

    result.passed = 1;
    result.details = "64-bit pointers, little-endian confirmed";
    return result;
}

/* Test 6: Basic Memory Allocation */
test_result_t test_basic_memory_allocation() {
    test_result_t result = {"Basic Memory Allocation", 0, NULL, NULL};

    size_t page_size = getpagesize();

    /* Allocate one page using mmap */
    void* memory = mmap(NULL, page_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);

    if (memory == MAP_FAILED) {
        result.error_msg = strerror(errno);
        return result;
    }

    /* Test memory access */
    unsigned char* bytes = (unsigned char*)memory;
    bytes[0] = 0xAA;
    bytes[page_size - 1] = 0x55;

    if (bytes[0] != 0xAA || bytes[page_size - 1] != 0x55) {
        result.error_msg = "Memory read/write test failed";
        munmap(memory, page_size);
        return result;
    }

    /* Clean up */
    if (munmap(memory, page_size) != 0) {
        result.error_msg = "munmap failed";
        return result;
    }

    result.passed = 1;
    result.details = "mmap/munmap allocation and access successful";
    return result;
}

/* Test 7: Memory Alignment */
test_result_t test_memory_alignment() {
    test_result_t result = {"Memory Alignment", 0, NULL, NULL};

    size_t page_size = getpagesize();

    void* memory = mmap(NULL, page_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);

    if (memory == MAP_FAILED) {
        result.error_msg = strerror(errno);
        return result;
    }

    uintptr_t addr = (uintptr_t)memory;

    /* Check page alignment */
    if (addr % page_size != 0) {
        result.error_msg = "Memory not page-aligned";
        munmap(memory, page_size);
        return result;
    }

    /* Check cache line alignment */
    if (addr % 64 != 0) {
        result.error_msg = "Memory not cache-line aligned";
        munmap(memory, page_size);
        return result;
    }

    munmap(memory, page_size);

    result.passed = 1;
    result.details = "Page and cache-line alignment confirmed";
    return result;
}

/* Test 8: Memory Protection */
test_result_t test_memory_protection() {
    test_result_t result = {"Memory Protection", 0, NULL, NULL};

    size_t page_size = getpagesize();

    /* Allocate read-only memory */
    void* memory = mmap(NULL, page_size,
                       PROT_READ,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);

    if (memory == MAP_FAILED) {
        result.error_msg = "Read-only allocation failed";
        return result;
    }

    /* Change protection to read-write */
    if (mprotect(memory, page_size, PROT_READ | PROT_WRITE) != 0) {
        result.error_msg = "mprotect failed";
        munmap(memory, page_size);
        return result;
    }

    /* Test write access after mprotect */
    unsigned char* bytes = (unsigned char*)memory;
    bytes[0] = 0xCC;

    if (bytes[0] != 0xCC) {
        result.error_msg = "Write after mprotect failed";
        munmap(memory, page_size);
        return result;
    }

    munmap(memory, page_size);

    result.passed = 1;
    result.details = "Memory protection and mprotect working";
    return result;
}

/* Test 9: System Memory Information */
test_result_t test_system_memory_info() {
    test_result_t result = {"System Memory Information", 0, NULL, NULL};

    long total_pages = sysconf(_SC_PHYS_PAGES);
    long available_pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);

    if (total_pages == -1 || available_pages == -1 || page_size == -1) {
        result.error_msg = "Cannot get system memory information";
        return result;
    }

    size_t total_memory_mb = (size_t)(total_pages * page_size) / (1024 * 1024);
    size_t available_memory_mb = (size_t)(available_pages * page_size) / (1024 * 1024);

    /* Basic sanity checks */
    if (total_memory_mb < 1024) { /* Should have at least 1GB */
        result.error_msg = "Unexpectedly low total memory";
        return result;
    }

    if (available_memory_mb > total_memory_mb) {
        result.error_msg = "Available memory exceeds total memory";
        return result;
    }

    char details_buffer[128];
    snprintf(details_buffer, sizeof(details_buffer),
             "Total: %zu MB, Available: %zu MB",
             total_memory_mb, available_memory_mb);

    result.passed = 1;
    result.details = details_buffer;
    return result;
}

/* Test 10: Huge Page Support Detection */
test_result_t test_huge_page_support() {
    test_result_t result = {"Huge Page Support", 0, NULL, NULL};

    /* Try to allocate a 2MB huge page */
    size_t huge_page_size = 2 * 1024 * 1024;

    void* memory = mmap(NULL, huge_page_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                       -1, 0);

    if (memory == MAP_FAILED) {
        /* This is expected if huge pages are not configured */
        result.passed = 1; /* Not a failure, just not available */
        result.details = "Huge pages not available (normal for default config)";
        return result;
    }

    /* If huge page allocation succeeded */
    munmap(memory, huge_page_size);

    result.passed = 1;
    result.details = "2MB huge pages are available";
    return result;
}

/* Print system information */
void print_system_info() {
    printf("=== ORACLE VM.STANDARD.A1.FLEX SYSTEM INFORMATION ===\n");

    struct utsname info;
    if (uname(&info) == 0) {
        printf("System: %s %s %s\n", info.sysname, info.release, info.machine);
        printf("Hostname: %s\n", info.nodename);
    }

    printf("Page size: %d bytes\n", getpagesize());

    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (cache_line != -1) {
        printf("L1 cache line: %ld bytes\n", cache_line);
    }

    long total_pages = sysconf(_SC_PHYS_PAGES);
    if (total_pages != -1) {
        size_t total_memory_mb = (size_t)(total_pages * getpagesize()) / (1024 * 1024);
        printf("Total memory: %zu MB\n", total_memory_mb);
    }

    printf("Pointer size: %zu bytes\n", sizeof(void*));
    printf("\n");
}

/* Main test runner */
int main() {
    printf("ORACLE VM.STANDARD.A1.FLEX MEMORY ALLOCATION UNIT TEST\n");
    printf("====================================================\n\n");

    print_system_info();

    printf("=== RUNNING VALIDATION TESTS ===\n");

    /* Run all validation tests */
    print_test_result(test_system_architecture());
    print_test_result(test_processor_info());
    print_test_result(test_page_size());
    print_test_result(test_cache_line_size());
    print_test_result(test_pointer_characteristics());
    print_test_result(test_basic_memory_allocation());
    print_test_result(test_memory_alignment());
    print_test_result(test_memory_protection());
    print_test_result(test_system_memory_info());
    print_test_result(test_huge_page_support());

    printf("\n=== TEST SUMMARY ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, total_tests);
    printf("Success rate: %.1f%%\n", total_tests > 0 ? (100.0 * passed_tests) / total_tests : 0.0);

    if (passed_tests == total_tests) {
        printf("\n✓ All Oracle A1.Flex system assumptions validated successfully!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. System may not match expected A1.Flex characteristics.\n");
        return 1;
    }
}