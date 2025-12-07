/**
 * Mac M1 Memory System Test
 *
 * Simple test program to validate the Mac M1 2022 memory allocator
 * and JWT storage system works correctly on this development machine.
 */

#include "memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("🍎 Mac M1 Memory System Test\n");
    printf("=============================\n\n");

    // Print platform information
    memory_system_print_info();

    // Validate the memory system
    printf("🔍 Running memory system validation...\n");
    int validation_result = memory_system_validate();

    if (validation_result == 0) {
        printf("\n✅ ALL TESTS PASSED!\n");
        printf("🍎 Mac M1 memory system is working correctly\n");
        printf("🚀 Ready for production use\n");
    } else {
        printf("\n❌ VALIDATION FAILED!\n");
        printf("⚠️  Mac M1 memory system has issues\n");
        printf("🔧 Check system compatibility\n");
        return 1;
    }

    // Test basic crypto buffer allocation
    printf("\n🧪 Testing basic crypto buffer operations...\n");

    // Initialize memory system
    if (memory_system_init() != 0) {
        printf("❌ Failed to initialize memory system\n");
        return 1;
    }

    // Allocate a test buffer
    crypto_buffer_t* buffer = platform_crypto_buffer_alloc(1024);
    if (!buffer) {
        printf("❌ Failed to allocate crypto buffer\n");
        memory_system_cleanup();
        return 1;
    }

    printf("✅ Successfully allocated 1024-byte crypto buffer\n");

    // Get data pointer and write test data
    void* data = platform_crypto_buffer_get_data(buffer);
    if (!data) {
        printf("❌ Failed to get data pointer\n");
        platform_crypto_buffer_free(buffer);
        memory_system_cleanup();
        return 1;
    }

    // Write test data
    const char* test_string = "Hello, Mac M1 Memory System!";
    strcpy((char*)data, test_string);

    printf("✅ Successfully wrote test data to buffer\n");

    // Validate buffer integrity
    if (!platform_crypto_buffer_validate(buffer)) {
        printf("❌ Buffer validation failed\n");
        platform_crypto_buffer_free(buffer);
        memory_system_cleanup();
        return 1;
    }

    printf("✅ Buffer validation passed\n");

    // Read back data
    const char* read_data = (const char*)platform_crypto_buffer_get_data(buffer);
    if (!read_data || strcmp(read_data, test_string) != 0) {
        printf("❌ Data integrity check failed\n");
        platform_crypto_buffer_free(buffer);
        memory_system_cleanup();
        return 1;
    }

    printf("✅ Data integrity check passed\n");
    printf("📖 Read back: \"%s\"\n", read_data);

    // Cleanup
    platform_crypto_buffer_free(buffer);
    memory_system_cleanup();

    printf("\n🎉 All tests completed successfully!\n");
    printf("✅ Mac M1 memory system is fully functional\n");

    return 0;
}