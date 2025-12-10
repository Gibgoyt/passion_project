#define _DEFAULT_SOURCE
#include "a1_flex_optimizations.h"
#include "memory_validation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_optimizations_initialized = 0;

int a1_flex_init_optimizations(void) {
    if (g_optimizations_initialized) {
        return 0;  // Already initialized
    }

    printf("🚀 Initializing Oracle A1.Flex optimizations...\n");

    // Validate architecture
    if (!validate_architecture_runtime()) {
        printf("❌ Architecture validation failed\n");
        return -1;
    }

    // Check CPU features
    uint32_t features = a1_flex_check_cpu_features();
    printf("💻 A1.Flex CPU features: 0x%08X\n", features);

    if (features & A1_FLEX_FEATURE_PREFETCH) {
        printf("  ✅ Prefetch instructions available\n");
    }

    g_optimizations_initialized = 1;
    printf("✅ A1.Flex optimizations initialized\n");

    return 0;
}

jwt_storage_t* a1_flex_create_optimized_jwt_storage(size_t max_token_size) {
    if (!g_optimizations_initialized) {
        a1_flex_init_optimizations();
    }

    // Use default size if not specified
    if (max_token_size == 0) {
        max_token_size = JWT_MAX_LENGTH;
    }

    // Calculate optimal buffer size for A1.Flex cache hierarchy
    size_t optimal_size = a1_flex_get_optimal_buffer_size(max_token_size);

    // Create JWT storage with optimized size
    jwt_storage_t* storage = jwt_storage_create(optimal_size, JWT_VALIDATE_DEFAULT);
    if (!storage) {
        return NULL;
    }

    // Check if allocation is cache-friendly
    if (a1_flex_is_memory_placement_optimal(storage, storage->num_pages * ARM64_PAGE_SIZE)) {
        printf("✅ Cache-friendly JWT storage allocated\n");
    } else {
        printf("⚠️ Suboptimal JWT storage allocation\n");
    }

    return storage;
}

void a1_flex_prefetch_jwt_token(jwt_storage_t* storage) {
    if (!storage || !jwt_storage_has_token(storage)) {
        return;
    }

    // Get token data pointer
    const unsigned char* token_data = jwt_storage_get_token(storage);
    if (!token_data) {
        return;
    }

    size_t token_length = jwt_storage_get_length(storage);

    // Prefetch token data for read access
    for (size_t offset = 0; offset < token_length; offset += ARM64_CACHE_LINE_SIZE) {
        ARM64_PREFETCH_READ((const char*)token_data + offset);
    }
}

void a1_flex_optimize_jwt_batch_layout(jwt_storage_t** storages, size_t count) {
    if (!storages || count == 0) return;

    printf("🔧 Optimizing layout for %zu JWT storage instances\n", count);

    // Prefetch all storage metadata first
    for (size_t i = 0; i < count; i++) {
        if (storages[i]) {
            ARM64_PREFETCH_READ(storages[i]);
        }
    }

    // Memory barrier to ensure prefetches complete
    ARM64_MEMORY_BARRIER();

    // Now prefetch token data
    for (size_t i = 0; i < count; i++) {
        if (storages[i] && jwt_storage_has_token(storages[i])) {
            a1_flex_prefetch_jwt_token(storages[i]);
        }
    }
}

size_t a1_flex_get_optimal_buffer_size(size_t data_size) {
    // Start with the requested size
    size_t optimal_size = data_size;

    // For small data (< L1 cache), align to cache line
    if (data_size <= A1_FLEX_L1_DCACHE_SIZE / 2) {
        optimal_size = (data_size + ARM64_CACHE_LINE_SIZE - 1) & ~(ARM64_CACHE_LINE_SIZE - 1);
    }
    // For medium data (< L2 cache), align to page boundaries
    else if (data_size <= A1_FLEX_L2_CACHE_SIZE / 2) {
        optimal_size = (data_size + ARM64_PAGE_SIZE - 1) & ~(ARM64_PAGE_SIZE - 1);
    }
    // For large data, use page alignment but consider L3 cache
    else {
        // Round up to page boundary
        optimal_size = (data_size + ARM64_PAGE_SIZE - 1) & ~(ARM64_PAGE_SIZE - 1);

        // If much larger than L3, don't over-allocate
        if (optimal_size > A1_FLEX_L3_CACHE_SIZE * 2) {
            optimal_size = data_size + ARM64_PAGE_SIZE;  // Just add one page of safety
        }
    }

    return optimal_size;
}

void* a1_flex_cache_optimized_alloc(size_t size, int cache_level) {
    void* ptr;
    size_t aligned_size;

    switch (cache_level) {
        case 1: // L1 cache optimized
            aligned_size = (size + ARM64_CACHE_LINE_SIZE - 1) & ~(ARM64_CACHE_LINE_SIZE - 1);
            break;
        case 2: // L2 cache optimized
            aligned_size = (size + ARM64_PAGE_SIZE - 1) & ~(ARM64_PAGE_SIZE - 1);
            break;
        case 3: // L3 cache optimized
        default:
            aligned_size = page_count_for_size(size) * ARM64_PAGE_SIZE;
            break;
    }

    if (cache_level >= 3 || aligned_size >= ARM64_PAGE_SIZE) {
        // Use page allocation for larger sizes
        size_t pages = page_count_for_size(aligned_size);
        ptr = page_allocate_aligned(pages);
    } else {
        // Use regular malloc for smaller allocations
        ptr = malloc(aligned_size);
    }

    return ptr;
}

void a1_flex_cache_optimized_free(void* ptr, size_t size) {
    if (!ptr) return;

    if (size >= ARM64_PAGE_SIZE) {
        // Was allocated with page allocator
        size_t pages = page_count_for_size(size);
        page_free(ptr, pages);
    } else {
        // Was allocated with aligned_alloc
        free(ptr);
    }
}

int a1_flex_is_memory_placement_optimal(void* ptr, size_t size) {
    if (!ptr || size == 0) return 0;

    // Check basic alignment
    if (!page_is_cache_aligned(ptr)) {
        return 0;
    }

    // For small allocations, check if they fit in L1 cache efficiently
    if (size <= A1_FLEX_L1_DCACHE_SIZE / 4) {
        return validate_cache_boundaries(ptr, size);
    }

    // For medium allocations, check page alignment
    if (size <= A1_FLEX_L2_CACHE_SIZE) {
        return page_is_aligned(ptr);
    }

    // For large allocations, ensure reasonable page usage
    size_t pages_needed = page_count_for_size(size);
    size_t pages_allocated = (size + ARM64_PAGE_SIZE - 1) / ARM64_PAGE_SIZE;

    // Acceptable if we're not wasting more than one page
    return (pages_allocated - pages_needed) <= 1;
}

int a1_flex_optimize_jwt_storage_placement(jwt_storage_t* storage) {
    if (!storage || !jwt_storage_is_valid(storage)) {
        return -1;
    }

    // Check current placement
    if (a1_flex_is_memory_placement_optimal(storage->pages,
                                           storage->num_pages * ARM64_PAGE_SIZE)) {
        return 0;  // Already optimal
    }

    // For now, we can't relocate existing storage, but we can prefetch it
    a1_flex_prefetch_jwt_token(storage);

    return 0;
}

uint32_t a1_flex_check_cpu_features(void) {
    uint32_t features = 0;

    #ifdef __aarch64__
    // Check for prefetch support (most ARM64 CPUs support this)
    features |= A1_FLEX_FEATURE_PREFETCH;

    // Check for other features through CPUID equivalents
    #ifdef __ARM_FEATURE_MEMORY_TAGGING
    features |= A1_FLEX_FEATURE_MEMORY_TAGGING;
    #endif

    // LSE (Large System Extensions) detection
    #ifdef __ARM_FEATURE_ATOMICS
    features |= A1_FLEX_FEATURE_LSE;
    #endif

    // SVE (Scalable Vector Extension) detection
    #ifdef __ARM_FEATURE_SVE
    features |= A1_FLEX_FEATURE_SVE;
    #endif
    #endif

    return features;
}