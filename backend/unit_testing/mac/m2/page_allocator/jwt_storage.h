#ifndef JWT_STORAGE_H
#define JWT_STORAGE_H

#include "page_allocator.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/**
 * JWT Storage System - Mac M1 2022 Version
 *
 * Secure memory management for JWT tokens optimized for Apple Silicon M1/M2.
 * Uses 16KB pages and Apple-specific security features.
 */

// JWT buffer sizes - optimized for Mac M1 16KB pages
#define MAC_JWT_MAX_LENGTH 16384
#define MAC_JWT_BUFFER_SIZE (MAC_JWT_MAX_LENGTH + 16)  // Extra for null terminator and padding

// Crypto buffer magic number for corruption detection
#define MAC_CRYPTO_BUFFER_MAGIC 0xCAFEBABE42ULL

// Crypto buffer structure for Mac M1
typedef struct {
    uint64_t magic;                // Corruption detection magic number
    size_t size;                   // Allocated size in bytes
    size_t data_length;            // Actual data length
    uint64_t creation_timestamp;   // Creation time (microseconds)
    uint64_t last_access_timestamp; // Last access time (microseconds)
    uint64_t checksum;             // Simple checksum for integrity
    uint8_t is_locked;             // Memory lock status
    uint8_t reserved[7];           // Padding for alignment

    // Data follows immediately after this header
    // char data[]; -- Variable length data
} mac_crypto_buffer_t;

/**
 * Allocate secure buffer for cryptographic operations on Mac M1
 * @param size Buffer size in bytes (will be rounded up to page boundary)
 * @return Pointer to crypto buffer or NULL on failure
 */
mac_crypto_buffer_t* mac_crypto_buffer_alloc(size_t size);

/**
 * Free cryptographic buffer with secure clearing
 * @param buffer Buffer to free (may be NULL)
 */
void mac_crypto_buffer_free(mac_crypto_buffer_t* buffer);

/**
 * Get data pointer from crypto buffer
 * @param buffer Crypto buffer
 * @return Pointer to data section or NULL if invalid
 */
void* mac_crypto_buffer_get_data(mac_crypto_buffer_t* buffer);

/**
 * Get actual data length from crypto buffer
 * @param buffer Crypto buffer
 * @return Data length in bytes or 0 if invalid
 */
size_t mac_crypto_buffer_get_length(mac_crypto_buffer_t* buffer);

/**
 * Set data length in crypto buffer
 * @param buffer Crypto buffer
 * @param length New data length
 * @return 0 on success, -1 on failure
 */
int mac_crypto_buffer_set_length(mac_crypto_buffer_t* buffer, size_t length);

/**
 * Validate crypto buffer integrity
 * @param buffer Buffer to validate
 * @return 1 if valid, 0 if invalid
 */
int mac_crypto_buffer_validate(mac_crypto_buffer_t* buffer);

/**
 * Lock crypto buffer in memory (prevent swapping)
 * @param buffer Buffer to lock
 * @return 0 on success, -1 on failure
 */
int mac_crypto_buffer_lock(mac_crypto_buffer_t* buffer);

/**
 * Unlock crypto buffer from memory
 * @param buffer Buffer to unlock
 * @return 0 on success, -1 on failure
 */
int mac_crypto_buffer_unlock(mac_crypto_buffer_t* buffer);

/**
 * Update access timestamp for buffer
 * @param buffer Buffer to update
 */
void mac_crypto_buffer_touch(mac_crypto_buffer_t* buffer);

/**
 * Print buffer debug information
 * @param buffer Buffer to debug
 * @param label Optional label for the buffer
 */
void mac_crypto_buffer_debug(mac_crypto_buffer_t* buffer, const char* label);

// Global page region for JWT storage
extern mac_m1_page_region_t g_mac_jwt_region;

/**
 * Initialize JWT storage system for Mac M1
 * @return 0 on success, -1 on failure
 */
int mac_jwt_storage_init(void);

/**
 * Cleanup JWT storage system
 */
void mac_jwt_storage_cleanup(void);

/**
 * Get JWT storage statistics
 * @param used_bytes Pointer to store used bytes count
 * @param total_bytes Pointer to store total bytes count
 * @return 0 on success, -1 on failure
 */
int mac_jwt_storage_stats(size_t* used_bytes, size_t* total_bytes);

#endif // JWT_STORAGE_H