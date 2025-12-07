#include "jwt_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Global JWT storage region
mac_m1_page_region_t g_mac_jwt_region = {0};

// Get current timestamp in microseconds
static uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

// Calculate simple checksum for integrity verification
static uint64_t calculate_checksum(const void* data, size_t size) {
    if (!data || size == 0) return 0;

    const uint8_t* bytes = (const uint8_t*)data;
    uint64_t checksum = 0;

    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + bytes[i];
    }

    return checksum;
}

int mac_jwt_storage_init(void) {
    printf("🍎 Initializing MAC M1 JWT Storage System\n");
    printf("======================================\n");

    // Calculate pages needed for JWT operations
    // We want to allocate enough for multiple JWT tokens
    size_t jwt_buffer_size = sizeof(mac_crypto_buffer_t) + MAC_JWT_BUFFER_SIZE;
    size_t pages_needed = mac_m1_pages_for_size(jwt_buffer_size * 10); // Space for 10 tokens

    printf("🍎 JWT buffer size: %zu bytes\n", jwt_buffer_size);
    printf("🍎 Pages needed: %zu (16KB each)\n", pages_needed);
    printf("🍎 Total allocation: %zu bytes\n", pages_needed * MAC_M1_PAGE_SIZE);

    int result = mac_m1_page_init(&g_mac_jwt_region, pages_needed);

    if (result == 0) {
        printf("✅ MAC M1 JWT storage initialized successfully\n");
    } else {
        printf("❌ Failed to initialize MAC M1 JWT storage\n");
    }

    return result;
}

void mac_jwt_storage_cleanup(void) {
    printf("🧹 Cleaning up MAC M1 JWT storage...\n");
    mac_m1_page_cleanup(&g_mac_jwt_region);
    printf("✅ MAC M1 JWT storage cleanup complete\n");
}

mac_crypto_buffer_t* mac_crypto_buffer_alloc(size_t size) {
    if (size == 0 || size > MAC_JWT_MAX_LENGTH) {
        printf("❌ MAC M1: Invalid buffer size: %zu\n", size);
        return NULL;
    }

    // Calculate total size including header
    size_t total_size = sizeof(mac_crypto_buffer_t) + size;
    size_t pages_needed = mac_m1_pages_for_size(total_size);

    printf("🍎 MAC M1: Allocating crypto buffer (data: %zu, total: %zu, pages: %zu)\n",
           size, total_size, pages_needed);

    // Allocate from our page region
    mac_crypto_buffer_t* buffer = (mac_crypto_buffer_t*)mac_m1_page_alloc(&g_mac_jwt_region, pages_needed);
    if (!buffer) {
        printf("❌ MAC M1: Failed to allocate crypto buffer\n");
        return NULL;
    }

    // Initialize buffer header
    uint64_t now = get_timestamp_us();
    buffer->magic = MAC_CRYPTO_BUFFER_MAGIC;
    buffer->size = pages_needed * MAC_M1_PAGE_SIZE;
    buffer->data_length = 0; // No data yet
    buffer->creation_timestamp = now;
    buffer->last_access_timestamp = now;
    buffer->checksum = 0; // Will be calculated when data is set
    buffer->is_locked = 0;
    memset(buffer->reserved, 0, sizeof(buffer->reserved));

    // Clear the data area
    void* data_ptr = (char*)buffer + sizeof(mac_crypto_buffer_t);
    memset(data_ptr, 0, size);

    printf("✅ MAC M1: Crypto buffer allocated at %p\n", buffer);

    return buffer;
}

void mac_crypto_buffer_free(mac_crypto_buffer_t* buffer) {
    if (!buffer) return;

    printf("🍎 MAC M1: Freeing crypto buffer at %p\n", buffer);

    // Validate before freeing
    if (!mac_crypto_buffer_validate(buffer)) {
        printf("⚠️  MAC M1: Warning - freeing potentially corrupted buffer\n");
    }

    // Unlock if locked
    if (buffer->is_locked) {
        mac_crypto_buffer_unlock(buffer);
    }

    // Calculate pages to free
    size_t pages_to_free = buffer->size / MAC_M1_PAGE_SIZE;

    // Free the pages
    mac_m1_page_free(&g_mac_jwt_region, buffer, pages_to_free);

    printf("✅ MAC M1: Crypto buffer freed\n");
}

void* mac_crypto_buffer_get_data(mac_crypto_buffer_t* buffer) {
    if (!mac_crypto_buffer_validate(buffer)) {
        return NULL;
    }

    mac_crypto_buffer_touch(buffer);
    return (char*)buffer + sizeof(mac_crypto_buffer_t);
}

size_t mac_crypto_buffer_get_length(mac_crypto_buffer_t* buffer) {
    if (!mac_crypto_buffer_validate(buffer)) {
        return 0;
    }

    mac_crypto_buffer_touch(buffer);
    return buffer->data_length;
}

int mac_crypto_buffer_set_length(mac_crypto_buffer_t* buffer, size_t length) {
    if (!mac_crypto_buffer_validate(buffer)) {
        return -1;
    }

    size_t max_data_size = buffer->size - sizeof(mac_crypto_buffer_t);
    if (length > max_data_size) {
        printf("❌ MAC M1: Data length %zu exceeds buffer capacity %zu\n",
               length, max_data_size);
        return -1;
    }

    buffer->data_length = length;
    mac_crypto_buffer_touch(buffer);

    // Update checksum
    if (length > 0) {
        void* data_ptr = mac_crypto_buffer_get_data(buffer);
        buffer->checksum = calculate_checksum(data_ptr, length);
    } else {
        buffer->checksum = 0;
    }

    return 0;
}

int mac_crypto_buffer_validate(mac_crypto_buffer_t* buffer) {
    if (!buffer) {
        printf("🔍 MAC M1: Validation failed - NULL buffer\n");
        return 0;
    }

    if (buffer->magic != MAC_CRYPTO_BUFFER_MAGIC) {
        printf("🔍 MAC M1: Validation failed - invalid magic (0x%016llX)\n",
               buffer->magic);
        return 0;
    }

    if (buffer->size < sizeof(mac_crypto_buffer_t)) {
        printf("🔍 MAC M1: Validation failed - size too small (%zu)\n",
               buffer->size);
        return 0;
    }

    if (buffer->data_length > buffer->size - sizeof(mac_crypto_buffer_t)) {
        printf("🔍 MAC M1: Validation failed - data length exceeds buffer\n");
        return 0;
    }

    return 1;
}

int mac_crypto_buffer_lock(mac_crypto_buffer_t* buffer) {
    if (!mac_crypto_buffer_validate(buffer)) {
        return -1;
    }

    if (buffer->is_locked) {
        return 0; // Already locked
    }

    int result = mac_m1_page_lock(buffer, buffer->size);
    if (result == 0) {
        buffer->is_locked = 1;
        printf("🔒 MAC M1: Buffer locked in memory\n");
    }

    return result;
}

int mac_crypto_buffer_unlock(mac_crypto_buffer_t* buffer) {
    if (!mac_crypto_buffer_validate(buffer)) {
        return -1;
    }

    if (!buffer->is_locked) {
        return 0; // Not locked
    }

    int result = mac_m1_page_unlock(buffer, buffer->size);
    if (result == 0) {
        buffer->is_locked = 0;
        printf("🔓 MAC M1: Buffer unlocked from memory\n");
    }

    return result;
}

void mac_crypto_buffer_touch(mac_crypto_buffer_t* buffer) {
    if (buffer) {
        buffer->last_access_timestamp = get_timestamp_us();
    }
}

void mac_crypto_buffer_debug(mac_crypto_buffer_t* buffer, const char* label) {
    if (!buffer) {
        printf("🔍 MAC M1 Debug [%s]: NULL buffer\n", label ? label : "Unknown");
        return;
    }

    printf("🔍 MAC M1 Debug [%s]:\n", label ? label : "Unknown");
    printf("  Magic: 0x%016llX %s\n", buffer->magic,
           (buffer->magic == MAC_CRYPTO_BUFFER_MAGIC) ? "✅" : "❌");
    printf("  Size: %zu bytes\n", buffer->size);
    printf("  Data length: %zu bytes\n", buffer->data_length);
    printf("  Checksum: 0x%016llX\n", buffer->checksum);
    printf("  Locked: %s\n", buffer->is_locked ? "Yes" : "No");

    uint64_t now = get_timestamp_us();
    printf("  Created: %llu μs ago\n", now - buffer->creation_timestamp);
    printf("  Last access: %llu μs ago\n", now - buffer->last_access_timestamp);

    // Validate data checksum if we have data
    if (buffer->data_length > 0) {
        void* data_ptr = mac_crypto_buffer_get_data(buffer);
        if (data_ptr) {
            uint64_t actual_checksum = calculate_checksum(data_ptr, buffer->data_length);
            printf("  Checksum validation: %s\n",
                   (actual_checksum == buffer->checksum) ? "✅ Valid" : "❌ Invalid");
        }
    }
}

int mac_jwt_storage_stats(size_t* used_bytes, size_t* total_bytes) {
    if (!used_bytes || !total_bytes) {
        return -1;
    }

    *used_bytes = g_mac_jwt_region.used_pages * MAC_M1_PAGE_SIZE;
    *total_bytes = g_mac_jwt_region.total_pages * MAC_M1_PAGE_SIZE;

    return 0;
}