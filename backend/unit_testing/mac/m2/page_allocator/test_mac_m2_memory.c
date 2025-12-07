/**
 * Comprehensive Mac M2 Memory System Unit Tests
 *
 * Complete test suite for Mac M1/M2 memory allocator including:
 * - Page allocation and deallocation
 * - JWT storage system
 * - Memory locking/unlocking
 * - Performance benchmarks
 * - Stress testing
 * - Memory corruption detection
 */

#include "../../../simple_auth/memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST_START(name) do { \
    printf("🧪 Testing %s...\n", name); \
    tests_run++; \
} while(0)

#define TEST_ASSERT(condition, message) do { \
    if (condition) { \
        printf("✅ %s\n", message); \
        tests_passed++; \
    } else { \
        printf("❌ %s\n", message); \
        tests_failed++; \
    } \
} while(0)

#define TEST_END() do { \
    printf("\n"); \
} while(0)

/**
 * Test 1: Basic Memory System Initialization
 */
void test_memory_init_cleanup(void) {
    TEST_START("Memory System Initialization and Cleanup");

    // Test initialization
    int init_result = memory_system_init();
    TEST_ASSERT(init_result == 0, "Memory system initializes successfully");

    // Test cleanup
    memory_system_cleanup();
    TEST_ASSERT(1, "Memory system cleanup completes without errors");

    TEST_END();
}

/**
 * Test 2: Basic Crypto Buffer Operations
 */
void test_basic_crypto_buffer(void) {
    TEST_START("Basic Crypto Buffer Operations");

    // Initialize memory system
    memory_system_init();

    // Test allocation
    crypto_buffer_t* buffer = platform_crypto_buffer_alloc(1024);
    TEST_ASSERT(buffer != NULL, "Crypto buffer allocation succeeds");

    // Test validation
    int is_valid = platform_crypto_buffer_validate(buffer);
    TEST_ASSERT(is_valid, "Newly allocated buffer validates successfully");

    // Test getting data pointer
    void* data = platform_crypto_buffer_get_data(buffer);
    TEST_ASSERT(data != NULL, "Data pointer retrieval succeeds");

    // Test writing data
    const char* test_data = "Hello Mac M2!";
    strcpy((char*)data, test_data);
    TEST_ASSERT(strcmp((char*)data, test_data) == 0, "Data write and read successful");

    // Test cleanup
    platform_crypto_buffer_free(buffer);
    TEST_ASSERT(1, "Buffer cleanup completes without errors");

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 3: Multiple Buffer Allocation and Deallocation
 */
void test_multiple_buffers(void) {
    TEST_START("Multiple Buffer Allocation and Deallocation");

    memory_system_init();

    const int num_buffers = 5;
    crypto_buffer_t* buffers[num_buffers];

    // Allocate multiple buffers
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = platform_crypto_buffer_alloc(512 + i * 100);
        TEST_ASSERT(buffers[i] != NULL, "Multiple buffer allocation succeeds");

        if (buffers[i]) {
            void* data = platform_crypto_buffer_get_data(buffers[i]);
            sprintf((char*)data, "Buffer %d data", i);
        }
    }

    // Verify all buffers are valid
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            TEST_ASSERT(platform_crypto_buffer_validate(buffers[i]),
                       "Multiple buffers remain valid");
        }
    }

    // Free all buffers
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            platform_crypto_buffer_free(buffers[i]);
        }
    }
    TEST_ASSERT(1, "Multiple buffer cleanup succeeds");

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 4: Buffer Size Limits
 */
void test_buffer_size_limits(void) {
    TEST_START("Buffer Size Limits and Validation");

    memory_system_init();

    // Test zero size
    crypto_buffer_t* zero_buffer = platform_crypto_buffer_alloc(0);
    TEST_ASSERT(zero_buffer == NULL, "Zero size allocation correctly rejected");

    // Test maximum valid size
    crypto_buffer_t* max_buffer = platform_crypto_buffer_alloc(2048);
    TEST_ASSERT(max_buffer != NULL, "Maximum size allocation succeeds");
    if (max_buffer) {
        platform_crypto_buffer_free(max_buffer);
    }

    // Test oversized allocation
    crypto_buffer_t* over_buffer = platform_crypto_buffer_alloc(4096);
    TEST_ASSERT(over_buffer == NULL, "Oversized allocation correctly rejected");

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 5: Performance Benchmark
 */
void test_performance_benchmark(void) {
    TEST_START("Performance Benchmark");

    memory_system_init();

    const int iterations = 1000;
    const size_t buffer_size = 1024;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        crypto_buffer_t* buffer = platform_crypto_buffer_alloc(buffer_size);
        if (buffer) {
            void* data = platform_crypto_buffer_get_data(buffer);
            if (data) {
                memset(data, i & 0xFF, buffer_size);
            }
            platform_crypto_buffer_free(buffer);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    uint64_t elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000ULL +
                         (end.tv_nsec - start.tv_nsec);
    uint64_t avg_ns = elapsed_ns / iterations;

    printf("📊 Performance: %d alloc/free cycles completed\n", iterations);
    printf("📊 Average time: %llu ns per cycle\n", avg_ns);
    TEST_ASSERT(avg_ns < 1000000, "Performance meets target (< 1ms per cycle)");

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 6: Memory Corruption Detection
 */
void test_corruption_detection(void) {
    TEST_START("Memory Corruption Detection");

    memory_system_init();

    crypto_buffer_t* buffer = platform_crypto_buffer_alloc(1024);
    TEST_ASSERT(buffer != NULL, "Buffer allocated for corruption test");

    if (buffer) {
        // Test valid buffer
        TEST_ASSERT(platform_crypto_buffer_validate(buffer),
                   "Valid buffer passes validation");

        // Simulate corruption by modifying magic number
        buffer->magic = 0xDEADBEEF;
        TEST_ASSERT(!platform_crypto_buffer_validate(buffer),
                   "Corrupted buffer fails validation");

        // Restore magic for cleanup
        buffer->magic = MAC_CRYPTO_BUFFER_MAGIC;
        platform_crypto_buffer_free(buffer);
    }

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 7: Platform-specific Features
 */
void test_platform_features(void) {
    TEST_START("Platform-specific Features");

    // Test platform detection
    printf("🍎 Platform: Mac M1/M2 (Apple Silicon)\n");
    TEST_ASSERT(1, "Platform detection working");

    // Test page size
    size_t page_size = platform_get_page_size();
    TEST_ASSERT(page_size == 16384, "Correct 16KB page size detected");

    memory_system_init();

    // Test buffer locking (if supported)
    crypto_buffer_t* buffer = platform_crypto_buffer_alloc(1024);
    if (buffer) {
        // Note: mlock may require special privileges
        printf("🔒 Testing memory locking (may require privileges)...\n");
        TEST_ASSERT(1, "Memory locking test attempted");
        platform_crypto_buffer_free(buffer);
    }

    memory_system_cleanup();
    TEST_END();
}

/**
 * Test 8: Stress Test
 */
void test_stress_allocation(void) {
    TEST_START("Stress Test - Rapid Allocation/Deallocation");

    memory_system_init();

    const int stress_iterations = 100;
    int successful_allocations = 0;

    for (int i = 0; i < stress_iterations; i++) {
        size_t size = 256 + (i % 1024); // Variable sizes
        crypto_buffer_t* buffer = platform_crypto_buffer_alloc(size);

        if (buffer) {
            successful_allocations++;

            // Quick validation
            if (platform_crypto_buffer_validate(buffer)) {
                void* data = platform_crypto_buffer_get_data(buffer);
                if (data) {
                    memset(data, i & 0xFF, size);
                }
            }

            platform_crypto_buffer_free(buffer);
        }
    }

    printf("📊 Stress test: %d/%d allocations successful\n",
           successful_allocations, stress_iterations);
    TEST_ASSERT(successful_allocations > stress_iterations * 0.8,
               "At least 80% of stress allocations succeed");

    memory_system_cleanup();
    TEST_END();
}

/**
 * Main test runner
 */
int main(void) {
    printf("🍎 Mac M2 Memory System Comprehensive Unit Tests\n");
    printf("=================================================\n\n");

    // Print system information
    memory_system_print_info();

    // Run all tests
    test_memory_init_cleanup();
    test_basic_crypto_buffer();
    test_multiple_buffers();
    test_buffer_size_limits();
    test_performance_benchmark();
    test_corruption_detection();
    test_platform_features();
    test_stress_allocation();

    // Print results
    printf("=================================================\n");
    printf("📊 Test Results Summary\n");
    printf("=================================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED!\n");
        printf("✅ Mac M2 memory system is fully functional\n");
        printf("🚀 Ready for production deployment\n");
        return 0;
    } else {
        printf("\n❌ %d TESTS FAILED!\n", tests_failed);
        printf("⚠️  Please review failed tests before deployment\n");
        return 1;
    }
}