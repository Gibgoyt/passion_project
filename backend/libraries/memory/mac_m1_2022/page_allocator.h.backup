#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

/**
 * Memory Page Allocator - Mac M1 2022 Version
 *
 * This implementation is specifically designed for Apple Silicon M1/M2 Macs
 * which use 16KB pages instead of the standard 4KB pages.
 *
 * Key differences from Oracle A1.Flex version:
 * - Uses 16KB page size (16384 bytes)
 * - Optimized for Apple Silicon memory architecture
 * - Uses Apple-specific memory alignment and protection
 */

// Mac M1/M2 specific page size (16KB)
#define MAC_M1_PAGE_SIZE 16384
#define MAC_M1_PAGE_ALIGNMENT 16384

// Memory protection flags for Mac
#define MAC_M1_PROT_READ   0x1
#define MAC_M1_PROT_WRITE  0x2
#define MAC_M1_PROT_EXEC   0x4

// Page allocator structure for Mac M1
typedef struct {
    void *base_addr;        // Base address of allocated region
    size_t total_size;      // Total allocated size
    size_t used_pages;      // Number of pages currently used
    size_t total_pages;     // Total number of pages allocated
    int is_locked;          // Memory lock status
    uint64_t alloc_time;    // Allocation timestamp
} mac_m1_page_region_t;

/**
 * Initialize page allocator for Mac M1
 * @param region Pointer to region structure to initialize
 * @param num_pages Number of 16KB pages to allocate
 * @return 0 on success, -1 on failure
 */
int mac_m1_page_init(mac_m1_page_region_t *region, size_t num_pages);

/**
 * Allocate pages from the region
 * @param region Page region
 * @param num_pages Number of pages to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void *mac_m1_page_alloc(mac_m1_page_region_t *region, size_t num_pages);

/**
 * Free pages back to the region
 * @param region Page region
 * @param ptr Pointer to memory to free
 * @param num_pages Number of pages to free
 * @return 0 on success, -1 on failure
 */
int mac_m1_page_free(mac_m1_page_region_t *region, void *ptr, size_t num_pages);

/**
 * Lock pages in memory (prevent swapping)
 * @param ptr Pointer to memory
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int mac_m1_page_lock(void *ptr, size_t size);

/**
 * Unlock pages from memory
 * @param ptr Pointer to memory
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int mac_m1_page_unlock(void *ptr, size_t size);

/**
 * Securely clear memory
 * @param ptr Pointer to memory
 * @param size Size in bytes
 */
void mac_m1_secure_clear(void *ptr, size_t size);

/**
 * Cleanup and free all allocated pages
 * @param region Page region to cleanup
 */
void mac_m1_page_cleanup(mac_m1_page_region_t *region);

/**
 * Get page size for Mac M1
 * @return Page size in bytes (16384)
 */
static inline size_t mac_m1_get_page_size(void) {
    return MAC_M1_PAGE_SIZE;
}

/**
 * Round size up to page boundary
 * @param size Size to round up
 * @return Size rounded up to 16KB boundary
 */
static inline size_t mac_m1_round_to_page(size_t size) {
    return (size + MAC_M1_PAGE_SIZE - 1) & ~(MAC_M1_PAGE_SIZE - 1);
}

/**
 * Calculate number of pages needed for given size
 * @param size Size in bytes
 * @return Number of 16KB pages needed
 */
static inline size_t mac_m1_pages_for_size(size_t size) {
    return (size + MAC_M1_PAGE_SIZE - 1) / MAC_M1_PAGE_SIZE;
}

#endif // PAGE_ALLOCATOR_H