#include "memory_validation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <time.h>

// Test performance by allocating and freeing buffers
static int test_allocation_performance(void) {
    printf("🚀 Testing Mac M1 allocation performance...\n");

    const int num_tests = 10;
    const size_t test_sizes[] = {1024, 2048, 4096, 8192, 16384};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);

    struct timespec start, end;

    for (int size_idx = 0; size_idx < num_sizes; size_idx++) {
        size_t test_size = test_sizes[size_idx];

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < num_tests; i++) {
            mac_crypto_buffer_t* buffer = mac_crypto_buffer_alloc(test_size);
            if (!buffer) {
                printf("❌ Allocation failed for size %zu\n", test_size);
                return -1;
            }

            // Set some data
            void* data = mac_crypto_buffer_get_data(buffer);
            if (data) {
                memset(data, 0xAA, test_size);
                mac_crypto_buffer_set_length(buffer, test_size);
            }

            mac_crypto_buffer_free(buffer);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        uint64_t elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000ULL +
                             (end.tv_nsec - start.tv_nsec);
        uint64_t avg_ns = elapsed_ns / num_tests;

        printf("  Size %6zu bytes: %llu ns avg per alloc/free\n",
               test_size, avg_ns);
    }

    return 0;
}

void mac_print_memory_info(void) {
    printf("🍎 Mac M1 Memory System Information\n");
    printf("===================================\n");

    // Get system page size
    size_t page_size = getpagesize();
    printf("System page size: %zu bytes\n", page_size);
    printf("Mac M1 page size: %d bytes\n", MAC_M1_PAGE_SIZE);

    if (page_size == MAC_M1_PAGE_SIZE) {
        printf("✅ Page size matches Mac M1 expectations\n");
    } else {
        printf("⚠️  Page size mismatch - using Mac M1 optimized size anyway\n");
    }

    // Get physical memory info
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t physical_memory = 0;
    size_t length = sizeof(physical_memory);

    if (sysctl(mib, 2, &physical_memory, &length, NULL, 0) == 0) {
        printf("Physical memory: %llu MB\n", physical_memory / (1024 * 1024));
    }

    // CPU info
    mib[1] = HW_NCPU;
    int ncpu = 0;
    length = sizeof(ncpu);
    if (sysctl(mib, 2, &ncpu, &length, NULL, 0) == 0) {
        printf("CPU cores: %d\n", ncpu);
    }

    printf("\n");
}

int mac_validate_page_allocator(void) {
    printf("🔍 Validating Mac M1 Page Allocator\n");
    printf("====================================\n");

    // Test basic allocation
    mac_m1_page_region_t test_region = {0};

    printf("Testing basic page allocation...\n");
    if (mac_m1_page_init(&test_region, 4) != 0) {
        printf("❌ Failed to initialize page region\n");
        return -1;
    }
    printf("✅ Page region initialized\n");

    // Test page allocation
    void* ptr1 = mac_m1_page_alloc(&test_region, 1);
    if (!ptr1) {
        printf("❌ Failed to allocate 1 page\n");
        mac_m1_page_cleanup(&test_region);
        return -1;
    }
    printf("✅ Successfully allocated 1 page\n");

    // Test writing to the page
    memset(ptr1, 0x42, MAC_M1_PAGE_SIZE);
    printf("✅ Successfully wrote to allocated page\n");

    // Test multiple allocations
    void* ptr2 = mac_m1_page_alloc(&test_region, 2);
    if (!ptr2) {
        printf("❌ Failed to allocate 2 more pages\n");
        mac_m1_page_cleanup(&test_region);
        return -1;
    }
    printf("✅ Successfully allocated 2 more pages\n");

    // Test memory locking
    if (mac_m1_page_lock(ptr1, MAC_M1_PAGE_SIZE) == 0) {
        printf("✅ Successfully locked page in memory\n");
        mac_m1_page_unlock(ptr1, MAC_M1_PAGE_SIZE);
        printf("✅ Successfully unlocked page\n");
    } else {
        printf("⚠️  Page locking failed (may require privileges)\n");
    }

    // Cleanup
    mac_m1_page_cleanup(&test_region);
    printf("✅ Page allocator validation complete\n\n");

    return 0;
}

int mac_validate_jwt_storage(void) {
    printf("🔍 Validating Mac M1 JWT Storage\n");
    printf("================================\n");

    // Initialize JWT storage
    if (mac_jwt_storage_init() != 0) {
        printf("❌ Failed to initialize JWT storage\n");
        return -1;
    }
    printf("✅ JWT storage initialized\n");

    // Test crypto buffer allocation
    const char* test_jwt = "{\"iss\":\"test\",\"sub\":\"user123\",\"exp\":1234567890}";
    size_t jwt_len = strlen(test_jwt);

    mac_crypto_buffer_t* buffer = mac_crypto_buffer_alloc(jwt_len + 1);
    if (!buffer) {
        printf("❌ Failed to allocate crypto buffer\n");
        mac_jwt_storage_cleanup();
        return -1;
    }
    printf("✅ Crypto buffer allocated\n");

    // Test buffer operations
    void* data_ptr = mac_crypto_buffer_get_data(buffer);
    if (!data_ptr) {
        printf("❌ Failed to get data pointer\n");
        mac_crypto_buffer_free(buffer);
        mac_jwt_storage_cleanup();
        return -1;
    }

    // Copy test data
    strcpy((char*)data_ptr, test_jwt);
    if (mac_crypto_buffer_set_length(buffer, jwt_len) != 0) {
        printf("❌ Failed to set buffer length\n");
        mac_crypto_buffer_free(buffer);
        mac_jwt_storage_cleanup();
        return -1;
    }
    printf("✅ Successfully stored JWT data\n");

    // Validate buffer integrity
    if (!mac_crypto_buffer_validate(buffer)) {
        printf("❌ Buffer validation failed\n");
        mac_crypto_buffer_free(buffer);
        mac_jwt_storage_cleanup();
        return -1;
    }
    printf("✅ Buffer validation passed\n");

    // Test buffer locking
    if (mac_crypto_buffer_lock(buffer) == 0) {
        printf("✅ Buffer locked in memory\n");
        mac_crypto_buffer_unlock(buffer);
        printf("✅ Buffer unlocked\n");
    } else {
        printf("⚠️  Buffer locking failed (may require privileges)\n");
    }

    // Debug buffer
    mac_crypto_buffer_debug(buffer, "Test JWT Buffer");

    // Cleanup
    mac_crypto_buffer_free(buffer);
    mac_jwt_storage_cleanup();
    printf("✅ JWT storage validation complete\n\n");

    return 0;
}

int mac_test_memory_performance(void) {
    printf("🚀 Testing Mac M1 Memory Performance\n");
    printf("====================================\n");

    // Initialize storage for performance test
    if (mac_jwt_storage_init() != 0) {
        printf("❌ Failed to initialize JWT storage for performance test\n");
        return -1;
    }

    int result = test_allocation_performance();

    mac_jwt_storage_cleanup();

    if (result == 0) {
        printf("✅ Performance test completed successfully\n\n");
    } else {
        printf("❌ Performance test failed\n\n");
    }

    return result;
}

int mac_validate_memory_system(void) {
    printf("🍎 Mac M1 Memory System Validation\n");
    printf("===================================\n\n");

    mac_print_memory_info();

    int failures = 0;

    if (mac_validate_page_allocator() != 0) {
        failures++;
    }

    if (mac_validate_jwt_storage() != 0) {
        failures++;
    }

    if (mac_test_memory_performance() != 0) {
        failures++;
    }

    printf("=======================================\n");
    if (failures == 0) {
        printf("✅ All Mac M1 memory validations PASSED\n");
        printf("✅ System is ready for JWT authentication\n");
    } else {
        printf("❌ %d validation(s) FAILED\n", failures);
        printf("⚠️  System may have compatibility issues\n");
    }
    printf("=======================================\n");

    return (failures == 0) ? 0 : -1;
}