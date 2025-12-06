#ifndef JWT_STORAGE_H
#define JWT_STORAGE_H

#include "page_allocator.h"
#include <stddef.h>
#include <stdint.h>

/**
 * JWT Token Storage System with Direct Page Allocation
 *
 * Provides secure, high-performance storage for JWT tokens using:
 * - Direct page allocation for complete memory control
 * - Memory locking to prevent swapping sensitive data
 * - Corruption detection with magic number validation
 * - Secure clearing to prevent data leakage
 * - Oracle A1.Flex ARM64 optimization
 *
 * Security Features:
 * - Page-level isolation from other memory
 * - Volatile pointer clearing prevents compiler optimization
 * - Input validation for null bytes and invalid ASCII
 * - Magic number corruption detection
 * - Memory locking prevents swap file exposure
 */

// Include JWT max length from auth library
#ifndef JWT_MAX_LENGTH
#define JWT_MAX_LENGTH 2048
#endif

// JWT storage magic number for corruption detection
#define JWT_STORAGE_MAGIC 0xDEADBEEFCAFEBABEULL

// JWT storage validation flags
#define JWT_VALIDATE_NULL_TERMINATION   (1 << 0)
#define JWT_VALIDATE_NO_EMBEDDED_NULLS  (1 << 1)
#define JWT_VALIDATE_ASCII_ONLY         (1 << 2)
#define JWT_VALIDATE_LENGTH_LIMITS      (1 << 3)
#define JWT_VALIDATE_ALL               (0x0F)

// Default validation mode (all checks enabled)
#define JWT_VALIDATE_DEFAULT JWT_VALIDATE_ALL

// Compile-time validations for JWT storage
_Static_assert(JWT_MAX_LENGTH > 0, "JWT_MAX_LENGTH must be positive");
_Static_assert(JWT_MAX_LENGTH <= (16 * ARM64_PAGE_SIZE),
               "JWT_MAX_LENGTH must fit in reasonable number of pages");
_Static_assert((JWT_STORAGE_MAGIC & 0xFFFFFFFF) != 0,
               "JWT storage magic must not have zero low bits");

// JWT storage error codes
typedef enum {
    JWT_STORAGE_SUCCESS = 0,
    JWT_STORAGE_ERROR_INVALID_PARAM = -1,
    JWT_STORAGE_ERROR_OUT_OF_MEMORY = -2,
    JWT_STORAGE_ERROR_CORRUPTION_DETECTED = -3,
    JWT_STORAGE_ERROR_TOKEN_TOO_LARGE = -4,
    JWT_STORAGE_ERROR_INVALID_TOKEN_FORMAT = -5,
    JWT_STORAGE_ERROR_NULL_TERMINATION_MISSING = -6,
    JWT_STORAGE_ERROR_EMBEDDED_NULL_FOUND = -7,
    JWT_STORAGE_ERROR_NON_ASCII_CHARACTER = -8,
    JWT_STORAGE_ERROR_MEMORY_LOCK_FAILED = -9,
    JWT_STORAGE_ERROR_PAGE_ALLOCATION_FAILED = -10
} jwt_storage_result_t;

// JWT storage structure (metadata stored in first page)
typedef struct {
    uint64_t magic;                    // Corruption detection magic
    void* pages;                       // Pointer to allocated pages
    size_t num_pages;                  // Number of pages allocated
    size_t capacity;                   // Total bytes available for token
    size_t token_length;               // Actual token length stored
    uint32_t validation_flags;         // Validation options used
    uint64_t checksum;                 // Simple checksum of token data
    uint64_t creation_timestamp;       // When storage was created
    uint64_t last_access_timestamp;    // Last time token was accessed
    uint8_t is_locked;                 // Memory lock status
    uint8_t is_readonly;               // Read-only mode flag
    uint8_t reserved[46];              // Reserved for future use, total 128 bytes
} jwt_storage_t;

// Compile-time validation of jwt_storage_t size
_Static_assert(sizeof(jwt_storage_t) <= 128,
               "jwt_storage_t must fit in cache line boundary");
_Static_assert(sizeof(jwt_storage_t) <= ARM64_CACHE_LINE_SIZE * 2,
               "jwt_storage_t should fit in reasonable cache boundary");

/**
 * Create new JWT storage with page allocation
 * @param max_token_size Maximum token size to support (0 = use JWT_MAX_LENGTH)
 * @param validation_flags Validation options to enable
 * @return Pointer to JWT storage or NULL on failure
 */
jwt_storage_t* jwt_storage_create(size_t max_token_size, uint32_t validation_flags);

/**
 * Create JWT storage with default settings
 * @return Pointer to JWT storage with JWT_MAX_LENGTH capacity and default validation
 */
jwt_storage_t* jwt_storage_create_default(void);

/**
 * Destroy JWT storage and securely clear all data
 * @param storage Storage to destroy (may be NULL)
 */
void jwt_storage_destroy(jwt_storage_t* storage);

/**
 * Store JWT token in secure storage
 * @param storage JWT storage instance
 * @param token Token data to store
 * @param length Token length in bytes
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_store_token(jwt_storage_t* storage,
                                           const unsigned char* token,
                                           size_t length);

/**
 * Store JWT token from null-terminated string
 * @param storage JWT storage instance
 * @param token_str Null-terminated token string
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_store_string(jwt_storage_t* storage,
                                            const char* token_str);

/**
 * Get pointer to stored token data
 * @param storage JWT storage instance
 * @return Pointer to token data or NULL if empty/corrupted
 */
const unsigned char* jwt_storage_get_token(jwt_storage_t* storage);

/**
 * Get stored token as null-terminated string
 * @param storage JWT storage instance
 * @return Pointer to token string or NULL if empty/corrupted
 */
const char* jwt_storage_get_string(jwt_storage_t* storage);

/**
 * Get length of stored token
 * @param storage JWT storage instance
 * @return Token length or 0 if empty/corrupted
 */
size_t jwt_storage_get_length(jwt_storage_t* storage);

/**
 * Check if storage is valid and uncorrupted
 * @param storage JWT storage instance
 * @return 1 if valid, 0 if corrupted or NULL
 */
int jwt_storage_is_valid(jwt_storage_t* storage);

/**
 * Check if storage contains a token
 * @param storage JWT storage instance
 * @return 1 if has token, 0 if empty or corrupted
 */
int jwt_storage_has_token(jwt_storage_t* storage);

/**
 * Clear stored token securely
 * @param storage JWT storage instance
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_clear(jwt_storage_t* storage);

/**
 * Make storage read-only (prevents further token updates)
 * @param storage JWT storage instance
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_make_readonly(jwt_storage_t* storage);

/**
 * Extract Bearer token from authorization header value
 * @param storage JWT storage to store extracted token
 * @param auth_header Full authorization header value (e.g., "Bearer eyJ...")
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_extract_bearer_token(jwt_storage_t* storage,
                                                    const char* auth_header);

/**
 * Validate token format and content
 * @param token Token data to validate
 * @param length Token length
 * @param validation_flags Validation options
 * @return JWT_STORAGE_SUCCESS or specific error code
 */
jwt_storage_result_t jwt_storage_validate_token(const unsigned char* token,
                                              size_t length,
                                              uint32_t validation_flags);

/**
 * Get storage capacity (maximum token size)
 * @param storage JWT storage instance
 * @return Capacity in bytes or 0 if invalid
 */
size_t jwt_storage_get_capacity(jwt_storage_t* storage);

/**
 * Get storage statistics and information
 * @param storage JWT storage instance
 * @param pages_used Output: number of pages used
 * @param bytes_used Output: number of bytes used
 * @param is_locked Output: memory lock status
 * @return JWT_STORAGE_SUCCESS or error code
 */
jwt_storage_result_t jwt_storage_get_info(jwt_storage_t* storage,
                                        size_t* pages_used,
                                        size_t* bytes_used,
                                        int* is_locked);

/**
 * Convert error code to human-readable string
 * @param error Error code from JWT storage functions
 * @return Error description string
 */
const char* jwt_storage_error_string(jwt_storage_result_t error);

/**
 * Print detailed storage information for debugging
 * @param storage JWT storage instance
 */
void jwt_storage_print_info(jwt_storage_t* storage);

/**
 * Internal function: Get pointer to token data area (after metadata)
 * @param storage JWT storage instance
 * @return Pointer to data area or NULL
 */
unsigned char* jwt_storage_get_data_ptr(jwt_storage_t* storage);

/**
 * Internal function: Update checksum of stored token
 * @param storage JWT storage instance
 */
void jwt_storage_update_checksum(jwt_storage_t* storage);

/**
 * Internal function: Verify checksum of stored token
 * @param storage JWT storage instance
 * @return 1 if checksum valid, 0 if corrupted
 */
int jwt_storage_verify_checksum(jwt_storage_t* storage);

/**
 * Cryptographic Buffer System for Secure Memory Management
 *
 * Provides secure, page-allocated buffers for cryptographic operations:
 * - RSA signature creation and verification
 * - Public/private key material handling
 * - Temporary cryptographic computations
 *
 * Security Features:
 * - Memory locked in RAM (cannot be swapped to disk)
 * - Secure clearing on deallocation
 * - Page-level isolation from heap memory
 * - Cache-aligned allocation for performance
 */

// Cryptographic buffer structure
typedef struct {
    void* data;                    // Pointer to allocated data
    size_t size;                   // Buffer size in bytes
    size_t num_pages;              // Number of pages allocated
    uint64_t magic;                // Corruption detection
    uint8_t is_locked;             // Memory lock status
    uint8_t reserved[7];           // Padding for alignment
} crypto_buffer_t;

// Crypto buffer magic number for validation
#define CRYPTO_BUFFER_MAGIC 0xC8ACED0CAFEBABE42ULL

/**
 * Allocate secure buffer for cryptographic operations
 * @param size Buffer size in bytes (will be rounded up to page boundary)
 * @return Pointer to crypto buffer or NULL on failure
 */
crypto_buffer_t* crypto_buffer_alloc(size_t size);

/**
 * Free cryptographic buffer with secure clearing
 * @param buffer Buffer to free (may be NULL)
 */
void crypto_buffer_free(crypto_buffer_t* buffer);

/**
 * Check if crypto buffer is valid
 * @param buffer Buffer to validate
 * @return 1 if valid, 0 if corrupted or NULL
 */
int crypto_buffer_is_valid(crypto_buffer_t* buffer);

/**
 * Get data pointer from crypto buffer
 * @param buffer Crypto buffer instance
 * @return Pointer to data or NULL if invalid
 */
void* crypto_buffer_get_data(crypto_buffer_t* buffer);

/**
 * Get buffer size
 * @param buffer Crypto buffer instance
 * @return Buffer size in bytes or 0 if invalid
 */
size_t crypto_buffer_get_size(crypto_buffer_t* buffer);

#endif // JWT_STORAGE_H