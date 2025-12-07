#include "page_allocator.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Apple-specific includes
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

// Get current time in microseconds
static uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

int mac_m1_page_init(mac_m1_page_region_t *region, size_t num_pages) {
    if (!region || num_pages == 0) {
        return -1;
    }

    size_t total_size = num_pages * MAC_M1_PAGE_SIZE;

    printf("🍎 MAC M1: Initializing page allocator for %zu pages (%zu bytes)\n",
           num_pages, total_size);

    // Allocate memory with mmap for better control on Mac
    void *addr = mmap(NULL, total_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

    if (addr == MAP_FAILED) {
        printf("❌ MAC M1: Failed to allocate %zu pages: %s\n",
               num_pages, strerror(errno));
        return -1;
    }

    // Initialize region structure
    region->base_addr = addr;
    region->total_size = total_size;
    region->used_pages = 0;
    region->total_pages = num_pages;
    region->is_locked = 0;
    region->alloc_time = get_timestamp_us();

    printf("✅ MAC M1: Successfully allocated %zu pages at %p\n",
           num_pages, addr);
    printf("🍎 MAC M1: Page size: %d bytes, Total size: %zu bytes\n",
           MAC_M1_PAGE_SIZE, total_size);

    return 0;
}

void *mac_m1_page_alloc(mac_m1_page_region_t *region, size_t num_pages) {
    if (!region || num_pages == 0) {
        return NULL;
    }

    if (region->used_pages + num_pages > region->total_pages) {
        printf("❌ MAC M1: Not enough pages available (requested: %zu, available: %zu)\n",
               num_pages, region->total_pages - region->used_pages);
        return NULL;
    }

    // Calculate offset for new allocation
    size_t offset = region->used_pages * MAC_M1_PAGE_SIZE;
    void *allocated_ptr = (char*)region->base_addr + offset;

    // Update used pages
    region->used_pages += num_pages;

    printf("✅ MAC M1: Allocated %zu pages at offset %zu (ptr: %p)\n",
           num_pages, offset, allocated_ptr);

    return allocated_ptr;
}

int mac_m1_page_free(mac_m1_page_region_t *region, void *ptr, size_t num_pages) {
    if (!region || !ptr) {
        return -1;
    }

    // Calculate which pages are being freed
    size_t offset = (char*)ptr - (char*)region->base_addr;
    size_t page_offset = offset / MAC_M1_PAGE_SIZE;

    printf("🍎 MAC M1: Freeing %zu pages at offset %zu (ptr: %p)\n", num_pages, page_offset, ptr);

    // Securely clear the memory
    mac_m1_secure_clear(ptr, num_pages * MAC_M1_PAGE_SIZE);

    // Update used pages counter (simplified - assumes LIFO order for this demo)
    if (region->used_pages >= num_pages) {
        region->used_pages -= num_pages;
        printf("✅ MAC M1: Updated used pages: %zu/%zu\n", region->used_pages, region->total_pages);
    } else {
        printf("⚠️  MAC M1: Warning - freeing more pages than allocated\n");
        region->used_pages = 0;
    }

    return 0;
}

int mac_m1_page_lock(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return -1;
    }

    // Round size to page boundary
    size_t rounded_size = mac_m1_round_to_page(size);

    if (mlock(ptr, rounded_size) != 0) {
        printf("⚠️  MAC M1: Failed to lock %zu bytes: %s\n",
               rounded_size, strerror(errno));
        return -1;
    }

    printf("🔒 MAC M1: Successfully locked %zu bytes in memory\n", rounded_size);
    return 0;
}

int mac_m1_page_unlock(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return -1;
    }

    // Round size to page boundary
    size_t rounded_size = mac_m1_round_to_page(size);

    if (munlock(ptr, rounded_size) != 0) {
        printf("⚠️  MAC M1: Failed to unlock %zu bytes: %s\n",
               rounded_size, strerror(errno));
        return -1;
    }

    printf("🔓 MAC M1: Successfully unlocked %zu bytes from memory\n", rounded_size);
    return 0;
}

void mac_m1_secure_clear(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }

    // Use memset_s if available (C11), otherwise use volatile memset for secure clearing
    #if defined(__STDC_LIB_EXT1__)
        memset_s(ptr, size, 0, size);
    #else
        // Use volatile pointer to prevent compiler optimization
        volatile unsigned char *p = ptr;
        while (size--) {
            *p++ = 0;
        }
    #endif

    // Memory barrier to prevent compiler optimization
    __asm__ __volatile__("" : : "r"(ptr) : "memory");
}

void mac_m1_page_cleanup(mac_m1_page_region_t *region) {
    if (!region || !region->base_addr) {
        return;
    }

    printf("🧹 MAC M1: Cleaning up page allocator...\n");

    // Securely clear all memory before freeing
    mac_m1_secure_clear(region->base_addr, region->total_size);

    // Unlock memory if it was locked
    if (region->is_locked) {
        mac_m1_page_unlock(region->base_addr, region->total_size);
    }

    // Unmap the memory
    if (munmap(region->base_addr, region->total_size) != 0) {
        printf("⚠️  MAC M1: Failed to unmap memory: %s\n", strerror(errno));
    } else {
        printf("✅ MAC M1: Successfully freed %zu pages\n", region->total_pages);
    }

    // Clear the region structure
    memset(region, 0, sizeof(mac_m1_page_region_t));
}