#define _DEFAULT_SOURCE
#include "page_allocator.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

// Global statistics tracking
static page_alloc_stats_t g_stats = {0};

// Runtime validation of Oracle A1.Flex architecture assumptions
static int validate_architecture(void) {
    static int validated = 0;
    static int is_valid = 0;

    if (validated) return is_valid;

    size_t actual_page_size = getpagesize();
    if (actual_page_size != ARM64_PAGE_SIZE) {
        fprintf(stderr, "⚠️ Page size mismatch: expected %d, got %zu\n",
                ARM64_PAGE_SIZE, actual_page_size);
        is_valid = 0;
    } else {
        is_valid = 1;
    }

    validated = 1;
    return is_valid;
}

size_t page_get_size(void) {
    return getpagesize();  // Should return 4096 on Oracle A1.Flex
}

void* page_allocate(size_t num_pages) {
    if (!validate_architecture() || num_pages == 0) {
        return NULL;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    void* pages = mmap(
        NULL,                           // Let kernel choose address
        total_size,                     // Size in bytes
        PROT_READ | PROT_WRITE,        // Read/write permissions
        MAP_PRIVATE | MAP_ANONYMOUS,    // Private, not backed by file
        -1,                            // No file descriptor
        0                              // No offset
    );

    if (pages == MAP_FAILED) {
        return NULL;
    }

    // Update statistics
    g_stats.total_pages_allocated += num_pages;
    g_stats.total_bytes_allocated += total_size;
    g_stats.allocation_count++;

    return pages;
}

void* page_allocate_aligned(size_t num_pages) {
    void* pages = page_allocate(num_pages);
    if (!pages) return NULL;

    // Check if already cache-line aligned
    if (page_is_cache_aligned(pages)) {
        return pages;
    }

    // Free and try again with larger allocation to get alignment
    page_free(pages, num_pages);

    // Allocate extra space to ensure we can get aligned address
    size_t extra_pages = 1;  // One extra page should be enough
    void* larger_alloc = page_allocate(num_pages + extra_pages);
    if (!larger_alloc) return NULL;

    // Find cache-line aligned address within the allocation
    uintptr_t addr = (uintptr_t)larger_alloc;
    uintptr_t aligned = (addr + ARM64_CACHE_LINE_SIZE - 1) & ~(ARM64_CACHE_LINE_SIZE - 1);

    // Check if aligned address is still within our allocation
    if (aligned + (num_pages * ARM64_PAGE_SIZE) <=
        (uintptr_t)larger_alloc + ((num_pages + extra_pages) * ARM64_PAGE_SIZE)) {
        return (void*)aligned;
    }

    // Fallback: free larger allocation and return unaligned original
    page_free(larger_alloc, num_pages + extra_pages);
    return page_allocate(num_pages);
}

void* page_allocate_at_address(void* desired_addr, size_t num_pages) {
    if (!validate_architecture() || num_pages == 0 || !desired_addr) {
        return NULL;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    void* pages = mmap(
        desired_addr,                   // Specific address request
        total_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,  // MAP_FIXED forces address
        -1,
        0
    );

    if (pages == MAP_FAILED) {
        return NULL;
    }

    // Update statistics
    g_stats.total_pages_allocated += num_pages;
    g_stats.total_bytes_allocated += total_size;
    g_stats.allocation_count++;

    return pages;
}

page_alloc_result_t page_free(void* pages, size_t num_pages) {
    if (!pages || num_pages == 0) {
        return PAGE_ALLOC_ERROR_INVALID_PARAM;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    if (munmap(pages, total_size) == -1) {
        switch (errno) {
            case EINVAL: return PAGE_ALLOC_ERROR_INVALID_PARAM;
            default: return PAGE_ALLOC_ERROR_OUT_OF_MEMORY;
        }
    }

    // Update statistics
    g_stats.deallocation_count++;

    return PAGE_ALLOC_SUCCESS;
}

page_alloc_result_t page_lock_memory(void* pages, size_t num_pages) {
    if (!pages || num_pages == 0) {
        return PAGE_ALLOC_ERROR_INVALID_PARAM;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    if (mlock(pages, total_size) == -1) {
        switch (errno) {
            case EPERM: return PAGE_ALLOC_ERROR_PERMISSION_DENIED;
            case ENOMEM: return PAGE_ALLOC_ERROR_OUT_OF_MEMORY;
            case EINVAL: return PAGE_ALLOC_ERROR_INVALID_PARAM;
            default: return PAGE_ALLOC_ERROR_LOCK_FAILED;
        }
    }

    // Update statistics
    g_stats.pages_currently_locked += num_pages;
    g_stats.bytes_currently_locked += total_size;

    return PAGE_ALLOC_SUCCESS;
}

page_alloc_result_t page_unlock_memory(void* pages, size_t num_pages) {
    if (!pages || num_pages == 0) {
        return PAGE_ALLOC_ERROR_INVALID_PARAM;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    if (munlock(pages, total_size) == -1) {
        switch (errno) {
            case EPERM: return PAGE_ALLOC_ERROR_PERMISSION_DENIED;
            case ENOMEM: return PAGE_ALLOC_ERROR_OUT_OF_MEMORY;
            case EINVAL: return PAGE_ALLOC_ERROR_INVALID_PARAM;
            default: return PAGE_ALLOC_ERROR_UNLOCK_FAILED;
        }
    }

    // Update statistics
    g_stats.pages_currently_locked -= num_pages;
    g_stats.bytes_currently_locked -= total_size;

    return PAGE_ALLOC_SUCCESS;
}

page_alloc_result_t page_set_permissions(void* pages, size_t num_pages, int permissions) {
    if (!pages || num_pages == 0) {
        return PAGE_ALLOC_ERROR_INVALID_PARAM;
    }

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    if (mprotect(pages, total_size, permissions) == -1) {
        switch (errno) {
            case EACCES: return PAGE_ALLOC_ERROR_PERMISSION_DENIED;
            case EINVAL: return PAGE_ALLOC_ERROR_INVALID_PARAM;
            case ENOMEM: return PAGE_ALLOC_ERROR_OUT_OF_MEMORY;
            default: return PAGE_ALLOC_ERROR_PROTECTION_FAILED;
        }
    }

    return PAGE_ALLOC_SUCCESS;
}

page_alloc_result_t page_make_readonly(void* pages, size_t num_pages) {
    return page_set_permissions(pages, num_pages, PROT_READ);
}

page_alloc_result_t page_make_executable(void* pages, size_t num_pages) {
    return page_set_permissions(pages, num_pages, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void page_secure_clear(void* pages, size_t num_pages) {
    if (!pages || num_pages == 0) return;

    size_t total_size = num_pages * ARM64_PAGE_SIZE;

    // Use volatile pointers to prevent compiler optimization
    volatile unsigned char* volatile_ptr = (volatile unsigned char*)pages;
    for (size_t i = 0; i < total_size; i++) {
        volatile_ptr[i] = 0;
    }

    // Additional security: use explicit_bzero if available
    #ifdef HAVE_EXPLICIT_BZERO
    explicit_bzero(pages, total_size);
    #endif
}

size_t page_count_for_size(size_t size) {
    if (size == 0) return 0;
    return (size + ARM64_PAGE_SIZE - 1) / ARM64_PAGE_SIZE;
}

int page_is_aligned(void* addr) {
    uintptr_t address = (uintptr_t)addr;
    return (address & (ARM64_PAGE_SIZE - 1)) == 0;
}

int page_is_cache_aligned(void* addr) {
    uintptr_t address = (uintptr_t)addr;
    return (address & (ARM64_CACHE_LINE_SIZE - 1)) == 0;
}

void page_get_stats(page_alloc_stats_t* stats) {
    if (stats) {
        *stats = g_stats;
    }
}

void page_reset_stats(void) {
    memset(&g_stats, 0, sizeof(g_stats));
}

const char* page_error_string(page_alloc_result_t error) {
    switch (error) {
        case PAGE_ALLOC_SUCCESS:
            return "Success";
        case PAGE_ALLOC_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case PAGE_ALLOC_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case PAGE_ALLOC_ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case PAGE_ALLOC_ERROR_ADDRESS_IN_USE:
            return "Address already in use";
        case PAGE_ALLOC_ERROR_LOCK_FAILED:
            return "Memory lock failed";
        case PAGE_ALLOC_ERROR_UNLOCK_FAILED:
            return "Memory unlock failed";
        case PAGE_ALLOC_ERROR_PROTECTION_FAILED:
            return "Permission change failed";
        default:
            return "Unknown error";
    }
}

void page_print_info(void* pages, size_t num_pages) {
    if (!pages) {
        printf("=== Page Info: NULL POINTER ===\n");
        return;
    }

    printf("=== Oracle A1.Flex Page Info ===\n");
    printf("Base address: %p\n", pages);
    printf("Pages allocated: %zu\n", num_pages);
    printf("Total bytes: %zu\n", num_pages * ARM64_PAGE_SIZE);
    printf("Page size: %zu bytes\n", page_get_size());
    printf("Cache line size: %d bytes\n", ARM64_CACHE_LINE_SIZE);
    printf("Page aligned: %s\n", page_is_aligned(pages) ? "Yes" : "No");
    printf("Cache aligned: %s\n", page_is_cache_aligned(pages) ? "Yes" : "No");
    printf("Address range: %p - %p\n", pages,
           (char*)pages + (num_pages * ARM64_PAGE_SIZE) - 1);

    // Show cache line information
    uintptr_t addr = (uintptr_t)pages;
    printf("Cache line offset: %zu bytes\n", addr & (ARM64_CACHE_LINE_SIZE - 1));
    printf("Spans %zu cache lines\n",
           ((addr % ARM64_CACHE_LINE_SIZE) + (num_pages * ARM64_PAGE_SIZE) +
            ARM64_CACHE_LINE_SIZE - 1) / ARM64_CACHE_LINE_SIZE);

    printf("================================\n");
}