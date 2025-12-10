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

    printf("🍎 MAC M1: Initializing bitmap-based page allocator for %zu pages (%zu bytes)\n",
           num_pages, total_size);

    // Allocate main memory region with mmap for better control on Mac
    void *addr = mmap(NULL, total_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1, 0);

    if (addr == MAP_FAILED) {
        printf("❌ MAC M1: Failed to allocate %zu pages: %s\n",
               num_pages, strerror(errno));
        return -1;
    }

    // Calculate bitmap size (1 bit per page, rounded up to bytes)
    size_t bitmap_size = (num_pages + 7) / 8;

    // Allocate bitmap for tracking page allocation status
    uint8_t *bitmap = calloc(bitmap_size, 1);
    if (!bitmap) {
        printf("❌ MAC M1: Failed to allocate bitmap (%zu bytes): %s\n",
               bitmap_size, strerror(errno));
        munmap(addr, total_size);
        return -1;
    }

    // Initialize region structure
    memset(region, 0, sizeof(mac_m1_page_region_t));
    region->base_addr = addr;
    region->total_size = total_size;
    region->total_pages = num_pages;
    region->allocated_pages = 0;
    region->page_bitmap = bitmap;
    region->bitmap_size = bitmap_size;
    region->search_hint = 0;
    region->is_locked = 0;
    region->alloc_time = get_timestamp_us();
    region->alloc_list = NULL;
    region->num_allocations = 0;

    printf("✅ MAC M1: Successfully allocated %zu pages at %p\n",
           num_pages, addr);
    printf("🍎 MAC M1: Bitmap size: %zu bytes, Page size: %d bytes, Total size: %zu bytes\n",
           bitmap_size, MAC_M1_PAGE_SIZE, total_size);

    return 0;
}

// Helper function to find contiguous free pages in bitmap
size_t mac_m1_find_free_pages(mac_m1_page_region_t *region, size_t num_pages, size_t start_hint) {
    if (!region || !region->page_bitmap || num_pages == 0) {
        return SIZE_MAX;
    }

    size_t total_pages = region->total_pages;
    size_t consecutive_free = 0;
    size_t start_pos = SIZE_MAX;

    // Search from hint position first
    for (size_t i = start_hint; i < total_pages; i++) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;

        // Check if page is free (bit = 0)
        if (!(region->page_bitmap[byte_idx] & (1 << bit_idx))) {
            if (consecutive_free == 0) {
                start_pos = i;  // Mark potential start
            }
            consecutive_free++;

            if (consecutive_free == num_pages) {
                return start_pos;  // Found enough contiguous pages
            }
        } else {
            consecutive_free = 0;  // Reset counter
        }
    }

    // If not found from hint, search from beginning
    if (start_hint > 0) {
        consecutive_free = 0;
        for (size_t i = 0; i < start_hint; i++) {
            size_t byte_idx = i / 8;
            size_t bit_idx = i % 8;

            if (!(region->page_bitmap[byte_idx] & (1 << bit_idx))) {
                if (consecutive_free == 0) {
                    start_pos = i;
                }
                consecutive_free++;

                if (consecutive_free == num_pages) {
                    return start_pos;
                }
            } else {
                consecutive_free = 0;
            }
        }
    }

    return SIZE_MAX;  // No suitable block found
}

void *mac_m1_page_alloc(mac_m1_page_region_t *region, size_t num_pages) {
    if (!region || num_pages == 0) {
        return NULL;
    }

    // Check if we have enough total pages available
    if (region->allocated_pages + num_pages > region->total_pages) {
        printf("❌ MAC M1: Not enough pages available (requested: %zu, available: %zu)\n",
               num_pages, region->total_pages - region->allocated_pages);
        return NULL;
    }

    // Find free pages using bitmap search
    size_t start_page = mac_m1_find_free_pages(region, num_pages, region->search_hint);
    if (start_page == SIZE_MAX) {
        printf("❌ MAC M1: Cannot find %zu contiguous free pages (fragmentation)\n", num_pages);
        printf("🍎 MAC M1: Total pages: %zu, Allocated: %zu, Available: %zu\n",
               region->total_pages, region->allocated_pages, region->total_pages - region->allocated_pages);
        return NULL;
    }

    // Mark pages as allocated in bitmap
    for (size_t i = 0; i < num_pages; i++) {
        size_t page_idx = start_page + i;
        size_t byte_idx = page_idx / 8;
        size_t bit_idx = page_idx % 8;
        region->page_bitmap[byte_idx] |= (1 << bit_idx);
    }

    // Calculate pointer to allocated memory
    size_t offset = start_page * MAC_M1_PAGE_SIZE;
    void *allocated_ptr = (char*)region->base_addr + offset;

    // Update counters and search hint
    region->allocated_pages += num_pages;
    region->search_hint = (start_page + num_pages) % region->total_pages;

    // Add allocation record for tracking
    allocation_record_t *record = malloc(sizeof(allocation_record_t));
    if (record) {
        record->start_page = start_page;
        record->num_pages = num_pages;
        record->alloc_time = get_timestamp_us();
        record->ptr = allocated_ptr;
        record->next = region->alloc_list;
        region->alloc_list = record;
        region->num_allocations++;
    }

    printf("✅ MAC M1: Allocated %zu pages at page %zu (ptr: %p)\n",
           num_pages, start_page, allocated_ptr);

    return allocated_ptr;
}

int mac_m1_page_free(mac_m1_page_region_t *region, void *ptr, size_t num_pages) {
    if (!region || !ptr) {
        return -1;
    }

    // Validate pointer is within region bounds
    if (ptr < region->base_addr || ptr >= (char*)region->base_addr + region->total_size) {
        printf("❌ MAC M1: Invalid pointer %p (outside region %p-%p)\n",
               ptr, region->base_addr, (char*)region->base_addr + region->total_size);
        return -1;
    }

    // Calculate which pages are being freed
    size_t offset = (char*)ptr - (char*)region->base_addr;
    if (offset % MAC_M1_PAGE_SIZE != 0) {
        printf("❌ MAC M1: Pointer %p not page-aligned (offset %zu)\n", ptr, offset);
        return -1;
    }

    size_t start_page = offset / MAC_M1_PAGE_SIZE;

    // Find and validate allocation record
    allocation_record_t **current = &region->alloc_list;
    allocation_record_t *found = NULL;

    while (*current) {
        if ((*current)->ptr == ptr && (*current)->start_page == start_page) {
            // Validate num_pages matches allocation record
            if ((*current)->num_pages != num_pages) {
                printf("❌ MAC M1: Page count mismatch (expected: %zu, provided: %zu)\n",
                       (*current)->num_pages, num_pages);
                return -1;
            }
            found = *current;
            *current = (*current)->next;  // Remove from list
            break;
        }
        current = &(*current)->next;
    }

    if (!found) {
        printf("❌ MAC M1: No allocation record found for ptr %p\n", ptr);
        return -1;
    }

    printf("🍎 MAC M1: Freeing %zu pages at page %zu (ptr: %p)\n", num_pages, start_page, ptr);

    // Securely clear the memory
    mac_m1_secure_clear(ptr, num_pages * MAC_M1_PAGE_SIZE);

    // Mark pages as free in bitmap
    for (size_t i = 0; i < num_pages; i++) {
        size_t page_idx = start_page + i;
        size_t byte_idx = page_idx / 8;
        size_t bit_idx = page_idx % 8;

        // Verify page was allocated before freeing
        if (!(region->page_bitmap[byte_idx] & (1 << bit_idx))) {
            printf("⚠️  MAC M1: Warning - page %zu was already free\n", page_idx);
        }

        // Clear the bit (mark as free)
        region->page_bitmap[byte_idx] &= ~(1 << bit_idx);
    }

    // Update counters
    region->allocated_pages -= num_pages;
    region->num_allocations--;

    // Update search hint to freed region for faster next allocation
    region->search_hint = start_page;

    // Free allocation record
    free(found);

    printf("✅ MAC M1: Updated allocated pages: %zu/%zu (freed at page %zu)\n",
           region->allocated_pages, region->total_pages, start_page);

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

    printf("🧹 MAC M1: Cleaning up bitmap-based page allocator...\n");

    // Check for memory leaks
    if (region->num_allocations > 0) {
        printf("⚠️  MAC M1: Warning - %zu allocations still active during cleanup\n",
               region->num_allocations);

        // Debug: list leaked allocations
        allocation_record_t *current = region->alloc_list;
        size_t leak_count = 0;
        while (current && leak_count < 10) {  // Limit output
            printf("🐛 MAC M1: Leaked allocation: %zu pages at page %zu (ptr: %p)\n",
                   current->num_pages, current->start_page, current->ptr);
            current = current->next;
            leak_count++;
        }
        if (region->num_allocations > leak_count) {
            printf("🐛 MAC M1: ... and %zu more leaked allocations\n",
                   region->num_allocations - leak_count);
        }
    }

    // Free all allocation records
    allocation_record_t *current = region->alloc_list;
    while (current) {
        allocation_record_t *next = current->next;
        free(current);
        current = next;
    }

    // Securely clear all memory before freeing
    if (region->base_addr) {
        mac_m1_secure_clear(region->base_addr, region->total_size);
    }

    // Unlock memory if it was locked
    if (region->is_locked && region->base_addr) {
        mac_m1_page_unlock(region->base_addr, region->total_size);
    }

    // Free bitmap
    if (region->page_bitmap) {
        free(region->page_bitmap);
    }

    // Unmap the main memory region
    if (region->base_addr) {
        if (munmap(region->base_addr, region->total_size) != 0) {
            printf("⚠️  MAC M1: Failed to unmap memory: %s\n", strerror(errno));
        } else {
            printf("✅ MAC M1: Successfully freed %zu pages and %zu bytes bitmap\n",
                   region->total_pages, region->bitmap_size);
        }
    }

    // Clear the region structure
    memset(region, 0, sizeof(mac_m1_page_region_t));
}

void mac_m1_debug_allocator(mac_m1_page_region_t *region) {
    if (!region) {
        printf("❌ MAC M1 Debug: Invalid region\n");
        return;
    }

    printf("\n🔍 MAC M1 Debug [Bitmap Allocator State]:\n");
    printf("=====================================\n");
    printf("📊 Memory Layout:\n");
    printf("  Base address: %p\n", region->base_addr);
    printf("  Total size: %zu bytes (%zu MB)\n", region->total_size, region->total_size / (1024*1024));
    printf("  Page size: %d bytes\n", MAC_M1_PAGE_SIZE);
    printf("  Total pages: %zu\n", region->total_pages);
    printf("  Allocated pages: %zu\n", region->allocated_pages);
    printf("  Free pages: %zu\n", region->total_pages - region->allocated_pages);
    printf("  Fragmentation: %.1f%%\n",
           region->total_pages > 0 ?
           (double)(region->allocated_pages * 100) / region->total_pages : 0.0);

    printf("\n📊 Bitmap Info:\n");
    printf("  Bitmap size: %zu bytes\n", region->bitmap_size);
    printf("  Search hint: page %zu\n", region->search_hint);
    printf("  Active allocations: %zu\n", region->num_allocations);

    // Show allocation records
    if (region->num_allocations > 0) {
        printf("\n📋 Active Allocations:\n");
        allocation_record_t *current = region->alloc_list;
        size_t count = 0;
        while (current && count < 10) {  // Limit output
            printf("  %zu: %zu pages at page %zu (ptr: %p, age: %llu μs)\n",
                   count + 1, current->num_pages, current->start_page, current->ptr,
                   get_timestamp_us() - current->alloc_time);
            current = current->next;
            count++;
        }
        if (region->num_allocations > count) {
            printf("  ... and %zu more allocations\n", region->num_allocations - count);
        }
    }

    // Show bitmap visualization (for small allocators)
    if (region->total_pages <= 64) {
        printf("\n🗺️  Bitmap Visualization (A=allocated, .=free):\n");
        printf("  ");
        for (size_t i = 0; i < region->total_pages; i++) {
            size_t byte_idx = i / 8;
            size_t bit_idx = i % 8;

            if (region->page_bitmap[byte_idx] & (1 << bit_idx)) {
                printf("A");
            } else {
                printf(".");
            }

            if ((i + 1) % 16 == 0) {
                printf("\n  ");
            }
        }
        printf("\n");
    }

    printf("=====================================\n\n");
}

int mac_m1_validate_allocator(mac_m1_page_region_t *region) {
    if (!region) {
        printf("❌ Validation: Invalid region\n");
        return 0;
    }

    int valid = 1;

    // Check basic structure integrity
    if (!region->base_addr || !region->page_bitmap) {
        printf("❌ Validation: Missing base_addr or bitmap\n");
        valid = 0;
    }

    if (region->total_pages == 0 || region->bitmap_size == 0) {
        printf("❌ Validation: Invalid size parameters\n");
        valid = 0;
    }

    if (region->allocated_pages > region->total_pages) {
        printf("❌ Validation: Allocated pages (%zu) > total pages (%zu)\n",
               region->allocated_pages, region->total_pages);
        valid = 0;
    }

    // Count pages from bitmap and compare with allocated_pages
    size_t bitmap_allocated = 0;
    for (size_t i = 0; i < region->total_pages; i++) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;
        if (region->page_bitmap[byte_idx] & (1 << bit_idx)) {
            bitmap_allocated++;
        }
    }

    if (bitmap_allocated != region->allocated_pages) {
        printf("❌ Validation: Bitmap count (%zu) != allocated_pages (%zu)\n",
               bitmap_allocated, region->allocated_pages);
        valid = 0;
    }

    // Count allocation records and validate against num_allocations
    size_t record_count = 0;
    allocation_record_t *current = region->alloc_list;
    while (current) {
        record_count++;

        // Validate each record
        if (current->start_page >= region->total_pages) {
            printf("❌ Validation: Invalid start_page %zu in allocation record\n",
                   current->start_page);
            valid = 0;
        }

        if (current->start_page + current->num_pages > region->total_pages) {
            printf("❌ Validation: Allocation exceeds region (start: %zu, pages: %zu)\n",
                   current->start_page, current->num_pages);
            valid = 0;
        }

        current = current->next;
    }

    if (record_count != region->num_allocations) {
        printf("❌ Validation: Record count (%zu) != num_allocations (%zu)\n",
               record_count, region->num_allocations);
        valid = 0;
    }

    if (valid) {
        printf("✅ Validation: Allocator integrity check passed\n");
    }

    return valid;
}