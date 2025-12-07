#include "platform_detection.h"
#include <stdio.h>

int memory_system_init(void) {
    printf("🚀 Initializing Memory System\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    printf("=============================\n");

#ifdef PLATFORM_MAC_M1
    return platform_jwt_storage_init();
#elif defined(PLATFORM_ORACLE_A1_FLEX)
    // This would call the Oracle A1.Flex initialization
    printf("❌ Oracle A1.Flex support not yet implemented in platform wrapper\n");
    return -1;
#else
    printf("❌ Unsupported platform\n");
    return -1;
#endif
}

void memory_system_cleanup(void) {
    printf("🧹 Cleaning up Memory System (%s)\n", PLATFORM_NAME);

#ifdef PLATFORM_MAC_M1
    platform_jwt_storage_cleanup();
#elif defined(PLATFORM_ORACLE_A1_FLEX)
    // This would call the Oracle A1.Flex cleanup
#endif
}

int memory_system_validate(void) {
    printf("🔍 Validating Memory System (%s)\n", PLATFORM_NAME);
    printf("===============================================\n");

#ifdef PLATFORM_MAC_M1
    return platform_validate_memory_system();
#elif defined(PLATFORM_ORACLE_A1_FLEX)
    // This would call the Oracle A1.Flex validation
    printf("❌ Oracle A1.Flex validation not yet implemented in platform wrapper\n");
    return -1;
#else
    printf("❌ Unsupported platform\n");
    return -1;
#endif
}

void memory_system_print_info(void) {
    printf("📋 Memory System Information\n");
    printf("============================\n");
    printf("Platform: %s\n", PLATFORM_NAME);

#ifdef PLATFORM_MAC_M1
    printf("Implementation: Mac M1/M2 Apple Silicon\n");
    printf("Page size: %zu bytes\n", platform_get_page_size());
    printf("Features:\n");
    printf("  - 16KB page optimization\n");
    printf("  - Apple Silicon memory management\n");
    printf("  - Secure buffer allocation\n");
    printf("  - Memory locking support\n");
#elif defined(PLATFORM_ORACLE_A1_FLEX)
    printf("Implementation: Oracle VM Standard A1.Flex\n");
    printf("Page size: 4096 bytes\n");
    printf("Features:\n");
    printf("  - 4KB page optimization\n");
    printf("  - ARM64 performance tuning\n");
    printf("  - Cloud-optimized allocation\n");
#else
    printf("Implementation: Unknown/Unsupported\n");
#endif

    printf("\n");
}