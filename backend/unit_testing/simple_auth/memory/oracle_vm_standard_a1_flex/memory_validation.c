#define _DEFAULT_SOURCE
#include "memory_validation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

// Global validation statistics
static memory_validation_stats_t g_validation_stats = {0};

int validate_architecture_runtime(void) {
    int all_passed = 1;

    // Check page size
    size_t actual_page_size = getpagesize();
    if (actual_page_size != ARM64_PAGE_SIZE) {
        printf("❌ Page size mismatch: expected %d, got %zu\n",
               ARM64_PAGE_SIZE, actual_page_size);
        all_passed = 0;
    } else {
        printf("✅ Page size: %zu bytes (ARM64 4KB)\n", actual_page_size);
    }

    // Check pointer size (should be 64-bit on ARM64)
    if (sizeof(void*) != 8) {
        printf("❌ Pointer size: expected 8 bytes (64-bit), got %zu\n", sizeof(void*));
        all_passed = 0;
    } else {
        printf("✅ Pointer size: %zu bytes (64-bit)\n", sizeof(void*));
    }

    // Check cache line alignment assumptions
    void* test_alloc = malloc(ARM64_CACHE_LINE_SIZE);
    if (test_alloc) {
        uintptr_t addr = (uintptr_t)test_alloc;
        printf("ℹ️  Sample malloc alignment: %zu bytes (addr %% %d = %zu)\n",
               addr & (ARM64_CACHE_LINE_SIZE - 1),
               ARM64_CACHE_LINE_SIZE,
               addr % ARM64_CACHE_LINE_SIZE);
        free(test_alloc);
    }

    if (all_passed) {
        g_validation_stats.architecture_checks_passed++;
    } else {
        g_validation_stats.architecture_checks_failed++;
    }

    return all_passed;
}

int validate_jwt_library_compatibility(void) {
    printf("\n=== JWT Library Compatibility ===\n");

    // Check JWT_MAX_LENGTH definition
    printf("✅ JWT_MAX_LENGTH: %d bytes\n", JWT_MAX_LENGTH);

    // Calculate buffer requirements
    size_t required_buffer = JWT_MAX_LENGTH + 16;  // JWT + "Bearer " + margin
    printf("✅ Required auth buffer: %zu bytes\n", required_buffer);

    // Check page requirements
    size_t pages_needed = page_count_for_size(required_buffer);
    printf("✅ Pages needed for JWT: %zu pages (%zu bytes)\n",
           pages_needed, pages_needed * ARM64_PAGE_SIZE);

    return 1;
}

int validate_page_allocator(void) {
    printf("\n=== Page Allocator Validation ===\n");

    g_validation_stats.page_allocations_validated++;

    // Test basic allocation
    void* test_pages = page_allocate(2);
    if (!test_pages) {
        printf("❌ Basic page allocation failed\n");
        g_validation_stats.page_allocation_failures++;
        return 0;
    }
    printf("✅ Basic allocation: %p\n", test_pages);

    // Test alignment
    if (!page_is_aligned(test_pages)) {
        printf("❌ Allocated pages not page-aligned\n");
        page_free(test_pages, 2);
        g_validation_stats.page_allocation_failures++;
        return 0;
    }
    printf("✅ Page alignment: correct\n");

    // Test memory access
    unsigned char* test_data = (unsigned char*)test_pages;
    test_data[0] = 0xAB;
    test_data[ARM64_PAGE_SIZE] = 0xCD;  // Second page
    test_data[ARM64_PAGE_SIZE * 2 - 1] = 0xEF;  // Last byte

    if (test_data[0] != 0xAB || test_data[ARM64_PAGE_SIZE] != 0xCD ||
        test_data[ARM64_PAGE_SIZE * 2 - 1] != 0xEF) {
        printf("❌ Memory access validation failed\n");
        page_free(test_pages, 2);
        g_validation_stats.page_allocation_failures++;
        return 0;
    }
    printf("✅ Memory access: working\n");

    // Test secure clear
    page_secure_clear(test_pages, 2);
    if (test_data[0] != 0 || test_data[ARM64_PAGE_SIZE] != 0 ||
        test_data[ARM64_PAGE_SIZE * 2 - 1] != 0) {
        printf("❌ Secure clear failed\n");
        page_free(test_pages, 2);
        g_validation_stats.page_allocation_failures++;
        return 0;
    }
    printf("✅ Secure clear: working\n");

    // Test cache-aligned allocation
    void* aligned_pages = page_allocate_aligned(1);
    if (aligned_pages) {
        if (!page_is_cache_aligned(aligned_pages)) {
            printf("⚠️  Cache-aligned allocation not cache-aligned\n");
        } else {
            printf("✅ Cache-aligned allocation: working\n");
        }
        page_free(aligned_pages, 1);
    }

    page_free(test_pages, 2);
    return 1;
}

int validate_jwt_storage(void) {
    printf("\n=== JWT Storage Validation ===\n");

    g_validation_stats.jwt_storage_validations++;

    // Test storage creation
    jwt_storage_t* storage = jwt_storage_create_default();
    if (!storage) {
        printf("❌ JWT storage creation failed\n");
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Storage creation: success\n");

    // Test validity check
    if (!jwt_storage_is_valid(storage)) {
        printf("❌ Storage validity check failed\n");
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Storage validity: confirmed\n");

    // Test token storage
    const char* test_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c";

    jwt_storage_result_t result = jwt_storage_store_string(storage, test_token);
    if (result != JWT_STORAGE_SUCCESS) {
        printf("❌ Token storage failed: %s\n", jwt_storage_error_string(result));
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Token storage: success\n");

    // Test token retrieval
    const char* retrieved = jwt_storage_get_string(storage);
    if (!retrieved || strcmp(retrieved, test_token) != 0) {
        printf("❌ Token retrieval failed\n");
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Token retrieval: success\n");

    // Test Bearer token extraction
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", test_token);

    jwt_storage_result_t extract_result = jwt_storage_extract_bearer_token(storage, auth_header);
    if (extract_result != JWT_STORAGE_SUCCESS) {
        printf("❌ Bearer token extraction failed: %s\n",
               jwt_storage_error_string(extract_result));
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Bearer extraction: success\n");

    // Test large token (simulate the actual JWT size from logs)
    char large_token[JWT_MAX_LENGTH];
    memset(large_token, 'A', sizeof(large_token) - 1);
    large_token[sizeof(large_token) - 1] = '\0';

    jwt_storage_result_t large_result = jwt_storage_store_string(storage, large_token);
    if (large_result != JWT_STORAGE_SUCCESS) {
        printf("❌ Large token storage failed: %s\n",
               jwt_storage_error_string(large_result));
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Large token storage: success (%d bytes)\n", JWT_MAX_LENGTH - 1);

    // Test secure clearing
    jwt_storage_clear(storage);
    if (jwt_storage_has_token(storage)) {
        printf("❌ Secure clear failed - token still present\n");
        jwt_storage_destroy(storage);
        g_validation_stats.jwt_storage_failures++;
        return 0;
    }
    printf("✅ Secure clear: working\n");

    jwt_storage_destroy(storage);
    return 1;
}

int validate_memory_system(void) {
    printf("\n======================================\n");
    printf("🔍 Oracle A1.Flex Memory System Validation\n");
    printf("======================================\n");

    int all_passed = 1;

    all_passed &= validate_architecture_runtime();
    all_passed &= validate_jwt_library_compatibility();
    all_passed &= validate_page_allocator();
    all_passed &= validate_jwt_storage();

    printf("\n======================================\n");
    if (all_passed) {
        printf("✅ All validations PASSED\n");
    } else {
        printf("❌ Some validations FAILED\n");
    }
    printf("======================================\n\n");

    return all_passed;
}

size_t calculate_optimal_auth_buffer_size(void) {
    // JWT_MAX_LENGTH + "Bearer " + null terminator + safety margin
    size_t base_size = JWT_MAX_LENGTH + 16;

    // Round up to next cache line boundary for optimal access
    size_t cache_aligned = (base_size + ARM64_CACHE_LINE_SIZE - 1) &
                          ~(ARM64_CACHE_LINE_SIZE - 1);

    // Ensure it fits within reasonable page boundaries
    size_t page_aligned = page_count_for_size(cache_aligned) * ARM64_PAGE_SIZE;

    return page_aligned;
}

int validate_pointer_alignment(const void* ptr, size_t alignment) {
    if (!ptr) return 0;

    g_validation_stats.alignment_checks_performed++;

    // Alignment must be power of 2
    if ((alignment & (alignment - 1)) != 0) {
        return 0;
    }

    uintptr_t addr = (uintptr_t)ptr;
    if ((addr & (alignment - 1)) == 0) {
        return 1;
    } else {
        g_validation_stats.alignment_violations_found++;
        return 0;
    }
}

int validate_page_boundaries(const void* ptr, size_t size) {
    if (!ptr || size == 0) return 0;

    uintptr_t start = (uintptr_t)ptr;
    uintptr_t end = start + size - 1;

    // Check if the memory region crosses page boundaries unsafely
    uintptr_t start_page = start / ARM64_PAGE_SIZE;
    uintptr_t end_page = end / ARM64_PAGE_SIZE;

    // It's okay to span multiple pages if properly allocated
    // This is just checking for reasonable boundaries
    return (end_page - start_page) < 16;  // Reasonable limit
}

int validate_cache_boundaries(const void* ptr, size_t size) {
    if (!ptr || size == 0) return 0;

    g_validation_stats.cache_optimality_checks++;

    uintptr_t addr = (uintptr_t)ptr;

    // Check if start is cache-line aligned
    if ((addr & (ARM64_CACHE_LINE_SIZE - 1)) != 0) {
        g_validation_stats.cache_suboptimal_cases++;
        return 0;  // Not optimal
    }

    // For small objects, check if they fit in single cache line
    if (size <= ARM64_CACHE_LINE_SIZE) {
        uintptr_t end = addr + size - 1;
        uintptr_t start_line = addr / ARM64_CACHE_LINE_SIZE;
        uintptr_t end_line = end / ARM64_CACHE_LINE_SIZE;

        if (start_line != end_line) {
            g_validation_stats.cache_suboptimal_cases++;
            return 0;  // Spans cache lines unnecessarily
        }
    }

    return 1;  // Cache-friendly
}

int validate_memory_integrity(const void* ptr, size_t size) {
    if (!ptr || size == 0) return 1;

    const unsigned char* data = (const unsigned char*)ptr;

    // Look for common corruption patterns
    int zero_count = 0;
    int ff_count = 0;

    for (size_t i = 0; i < size && i < 256; i++) {  // Sample first 256 bytes
        if (data[i] == 0x00) zero_count++;
        if (data[i] == 0xFF) ff_count++;
    }

    size_t sample_size = (size < 256) ? size : 256;

    // Suspicious if more than 90% are same pattern
    if (zero_count > (int)(sample_size * 9 / 10) || ff_count > (int)(sample_size * 9 / 10)) {
        return 0;  // Potential corruption
    }

    return 1;  // Appears okay
}

int validate_jwt_token_format(const char* token, size_t length) {
    if (!token || length == 0) return 0;

    // Basic JWT format: header.payload.signature
    int dot_count = 0;
    int valid_chars = 1;

    for (size_t i = 0; i < length; i++) {
        char c = token[i];
        if (c == '.') {
            dot_count++;
        } else if (!isalnum(c) && c != '-' && c != '_') {
            valid_chars = 0;  // Invalid base64url character
            break;
        }
    }

    // JWT should have exactly 2 dots (3 parts)
    return (dot_count == 2) && valid_chars;
}

void print_validation_report(void) {
    printf("\n=== Memory Validation Report ===\n");
    printf("Architecture checks: %u passed, %u failed\n",
           g_validation_stats.architecture_checks_passed,
           g_validation_stats.architecture_checks_failed);
    printf("Page allocations validated: %u\n",
           g_validation_stats.page_allocations_validated);
    printf("Page allocation failures: %u\n",
           g_validation_stats.page_allocation_failures);
    printf("JWT storage validations: %u\n",
           g_validation_stats.jwt_storage_validations);
    printf("JWT storage failures: %u\n",
           g_validation_stats.jwt_storage_failures);
    printf("Alignment checks: %u performed, %u violations\n",
           g_validation_stats.alignment_checks_performed,
           g_validation_stats.alignment_violations_found);
    printf("Cache optimality checks: %u performed, %u suboptimal\n",
           g_validation_stats.cache_optimality_checks,
           g_validation_stats.cache_suboptimal_cases);
    printf("================================\n");
}

void print_memory_layout(const void* ptr, size_t size) {
    if (!ptr) {
        printf("=== Memory Layout: NULL POINTER ===\n");
        return;
    }

    uintptr_t addr = (uintptr_t)ptr;

    printf("=== Memory Layout Analysis ===\n");
    printf("Address: %p\n", ptr);
    printf("Size: %zu bytes\n", size);
    printf("End address: %p\n", (void*)(addr + size - 1));

    printf("Page alignment: %s (offset %zu)\n",
           page_is_aligned((void*)ptr) ? "aligned" : "unaligned",
           addr % ARM64_PAGE_SIZE);

    printf("Cache alignment: %s (offset %zu)\n",
           page_is_cache_aligned((void*)ptr) ? "aligned" : "unaligned",
           addr % ARM64_CACHE_LINE_SIZE);

    size_t pages_spanned = page_count_for_size(size);
    printf("Pages spanned: %zu\n", pages_spanned);

    size_t cache_lines = ((addr % ARM64_CACHE_LINE_SIZE) + size +
                          ARM64_CACHE_LINE_SIZE - 1) / ARM64_CACHE_LINE_SIZE;
    printf("Cache lines spanned: %zu\n", cache_lines);

    printf("Cache friendly: %s\n",
           validate_cache_boundaries(ptr, size) ? "yes" : "no");
    printf("=============================\n");
}

void get_memory_validation_stats(memory_validation_stats_t* stats) {
    if (stats) {
        *stats = g_validation_stats;
    }
}

void reset_memory_validation_stats(void) {
    memset(&g_validation_stats, 0, sizeof(g_validation_stats));
}

void print_memory_validation_stats(void) {
    print_validation_report();
}