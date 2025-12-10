#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * Direct Page Allocation System for Oracle A1.Flex ARM64
 *
 * Provides low-level memory management with complete control over:
 * - 4KB page allocation directly from kernel via mmap()
 * - Cache-line alignment for ARM64 L1 cache optimization (64-byte lines)
 * - Memory locking to prevent swapping of sensitive data
 * - Page permission control for security
 *
 * Optimized for Oracle A1.Flex specifications:
 * - ARM64 CPU (Ampere Altra)
 * - 4KB page size
 * - 64-byte cache lines
 * - L1: 64KB I-cache + 64KB D-cache per core
 * - L2: 1MB per core
 * - L3: 32MB shared
 */

// Oracle A1.Flex ARM64 architecture constants
#define ARM64_PAGE_SIZE 4096
#define ARM64_CACHE_LINE_SIZE 64
#define HUGE_PAGE_SIZE (2 * 1024 * 1024)  // 2MB huge pages

// Compile-time validations for Oracle A1.Flex architecture
_Static_assert(ARM64_PAGE_SIZE == 4096, "Page size must be 4KB for ARM64");
_Static_assert(ARM64_CACHE_LINE_SIZE == 64, "Cache line must be 64 bytes for ARM64");
_Static_assert((ARM64_CACHE_LINE_SIZE & (ARM64_CACHE_LINE_SIZE - 1)) == 0,
               "Cache line size must be power of 2");

// Error codes for page allocation
typedef enum {
    PAGE_ALLOC_SUCCESS = 0,
    PAGE_ALLOC_ERROR_INVALID_PARAM = -1,
    PAGE_ALLOC_ERROR_OUT_OF_MEMORY = -2,
    PAGE_ALLOC_ERROR_PERMISSION_DENIED = -3,
    PAGE_ALLOC_ERROR_ADDRESS_IN_USE = -4,
    PAGE_ALLOC_ERROR_LOCK_FAILED = -5,
    PAGE_ALLOC_ERROR_UNLOCK_FAILED = -6,
    PAGE_ALLOC_ERROR_PROTECTION_FAILED = -7
} page_alloc_result_t;

// Memory allocation statistics
typedef struct {
    size_t total_pages_allocated;
    size_t total_bytes_allocated;
    size_t pages_currently_locked;
    size_t bytes_currently_locked;
    size_t allocation_count;
    size_t deallocation_count;
} page_alloc_stats_t;

/**
 * Get system page size (should be 4096 on Oracle A1.Flex)
 */
size_t page_get_size(void);

/**
 * Allocate raw pages directly from kernel
 * @param num_pages Number of 4KB pages to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void* page_allocate(size_t num_pages);

/**
 * Allocate pages with cache-line alignment for ARM64 optimization
 * @param num_pages Number of 4KB pages to allocate
 * @return Cache-aligned pointer or NULL on failure
 */
void* page_allocate_aligned(size_t num_pages);

/**
 * Allocate pages at specific address (dangerous - use with caution)
 * @param desired_addr Specific address to allocate at
 * @param num_pages Number of pages to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void* page_allocate_at_address(void* desired_addr, size_t num_pages);

/**
 * Free pages back to kernel
 * @param pages Pointer returned by page_allocate
 * @param num_pages Number of pages to free
 * @return PAGE_ALLOC_SUCCESS or error code
 */
page_alloc_result_t page_free(void* pages, size_t num_pages);

/**
 * Lock pages in RAM (prevent swapping) - critical for sensitive data
 * @param pages Pointer to memory to lock
 * @param num_pages Number of pages to lock
 * @return PAGE_ALLOC_SUCCESS or error code
 */
page_alloc_result_t page_lock_memory(void* pages, size_t num_pages);

/**
 * Unlock pages (allow swapping)
 * @param pages Pointer to memory to unlock
 * @param num_pages Number of pages to unlock
 * @return PAGE_ALLOC_SUCCESS or error code
 */
page_alloc_result_t page_unlock_memory(void* pages, size_t num_pages);

/**
 * Change page permissions
 * @param pages Pointer to memory
 * @param num_pages Number of pages
 * @param permissions PROT_READ | PROT_WRITE | PROT_EXEC combinations
 * @return PAGE_ALLOC_SUCCESS or error code
 */
page_alloc_result_t page_set_permissions(void* pages, size_t num_pages, int permissions);

/**
 * Make pages read-only (security hardening)
 */
page_alloc_result_t page_make_readonly(void* pages, size_t num_pages);

/**
 * Make pages executable (for code pages)
 */
page_alloc_result_t page_make_executable(void* pages, size_t num_pages);

/**
 * Securely clear memory using volatile pointers (prevents compiler optimization)
 * @param pages Memory to clear
 * @param num_pages Number of pages to clear
 */
void page_secure_clear(void* pages, size_t num_pages);

/**
 * Calculate number of pages needed for given size
 * @param size Size in bytes
 * @return Number of 4KB pages required
 */
size_t page_count_for_size(size_t size);

/**
 * Check if address is page-aligned
 * @param addr Address to check
 * @return 1 if aligned, 0 if not
 */
int page_is_aligned(void* addr);

/**
 * Check if address is cache-line aligned
 * @param addr Address to check
 * @return 1 if aligned, 0 if not
 */
int page_is_cache_aligned(void* addr);

/**
 * Get allocation statistics
 * @param stats Pointer to statistics structure to fill
 */
void page_get_stats(page_alloc_stats_t* stats);

/**
 * Reset allocation statistics
 */
void page_reset_stats(void);

/**
 * Convert error code to human-readable string
 * @param error Error code from page allocation functions
 * @return Error description string
 */
const char* page_error_string(page_alloc_result_t error);

/**
 * Print detailed memory information for debugging
 * @param pages Memory region to analyze
 * @param num_pages Number of pages
 */
void page_print_info(void* pages, size_t num_pages);

#endif // PAGE_ALLOCATOR_H