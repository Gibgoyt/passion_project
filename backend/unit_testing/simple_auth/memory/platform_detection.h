#ifndef PLATFORM_DETECTION_H
#define PLATFORM_DETECTION_H

/**
 * Platform Detection and Memory System Selection
 *
 * This header automatically detects the platform and includes
 * the appropriate memory management implementation.
 */

// Platform detection macros
#if defined(__APPLE__) && defined(__aarch64__)
    #define PLATFORM_MAC_M1 1
    #define PLATFORM_NAME "Mac M1/M2 (Apple Silicon)"
#elif defined(__linux__) && defined(__aarch64__)
    #define PLATFORM_ORACLE_A1_FLEX 1
    #define PLATFORM_NAME "Oracle VM Standard A1.Flex"
#else
    #define PLATFORM_GENERIC 1
    #define PLATFORM_NAME "Generic/Unknown"
#endif

// Include platform-specific headers
#ifdef PLATFORM_MAC_M1
    #include "mac_m1_2022/page_allocator.h"
    #include "mac_m1_2022/jwt_storage.h"
    #include "mac_m1_2022/memory_validation.h"

    // Create aliases for platform-neutral code
    typedef mac_m1_page_region_t page_region_t;
    typedef mac_crypto_buffer_t crypto_buffer_t;

    #define platform_page_init(region, num_pages) mac_m1_page_init(region, num_pages)
    #define platform_page_alloc(region, num_pages) mac_m1_page_alloc(region, num_pages)
    #define platform_page_free(region, ptr, num_pages) mac_m1_page_free(region, ptr, num_pages)
    #define platform_page_cleanup(region) mac_m1_page_cleanup(region)
    #define platform_get_page_size() mac_m1_get_page_size()

    #define platform_jwt_storage_init() mac_jwt_storage_init()
    #define platform_jwt_storage_cleanup() mac_jwt_storage_cleanup()
    #define platform_crypto_buffer_alloc(size) mac_crypto_buffer_alloc(size)
    #define platform_crypto_buffer_free(buffer) mac_crypto_buffer_free(buffer)
    #define platform_crypto_buffer_get_data(buffer) mac_crypto_buffer_get_data(buffer)
    #define platform_crypto_buffer_validate(buffer) mac_crypto_buffer_validate(buffer)

    #define platform_validate_memory_system() mac_validate_memory_system()

#elif defined(PLATFORM_ORACLE_A1_FLEX)
    #include "oracle_vm_standard_a1_flex/page_allocator.h"
    #include "oracle_vm_standard_a1_flex/jwt_storage.h"
    #include "oracle_vm_standard_a1_flex/memory_validation.h"

    // Create aliases for Oracle A1.Flex (original implementation)
    // These would need to be implemented based on the original code structure

#else
    #error "Unsupported platform - only Mac M1/M2 and Oracle A1.Flex are supported"
#endif

/**
 * Initialize platform-specific memory system
 * @return 0 on success, -1 on failure
 */
int memory_system_init(void);

/**
 * Cleanup platform-specific memory system
 */
void memory_system_cleanup(void);

/**
 * Validate platform memory system
 * @return 0 if all validations pass, -1 on failure
 */
int memory_system_validate(void);

/**
 * Print platform information
 */
void memory_system_print_info(void);

#endif // PLATFORM_DETECTION_H