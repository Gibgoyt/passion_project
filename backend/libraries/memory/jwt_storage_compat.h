#ifndef JWT_STORAGE_COMPAT_H
#define JWT_STORAGE_COMPAT_H

/**
 * JWT Storage Compatibility Layer
 *
 * This header provides backward compatibility for the old jwt_storage API
 * by mapping it to the new platform-specific crypto buffer system.
 */

#include "platform_detection.h"

// Compatibility type definitions
typedef crypto_buffer_t jwt_storage_t;

// Result codes for compatibility
typedef enum {
    JWT_STORAGE_SUCCESS = 0,
    JWT_STORAGE_ERROR_INVALID_FORMAT = -1,
    JWT_STORAGE_ERROR_BUFFER_TOO_SMALL = -2,
    JWT_STORAGE_ERROR_INVALID_TOKEN = -3,
    JWT_STORAGE_ERROR_NULL_POINTER = -4
} jwt_storage_result_t;

/**
 * Create default JWT storage buffer
 * @return Pointer to JWT storage or NULL on failure
 */
static inline jwt_storage_t* jwt_storage_create_default(void) {
    return platform_crypto_buffer_alloc(2048); // Default JWT buffer size
}

/**
 * Destroy JWT storage buffer
 * @param storage JWT storage to destroy
 */
static inline void jwt_storage_destroy(jwt_storage_t* storage) {
    if (storage) {
        platform_crypto_buffer_free(storage);
    }
}

/**
 * Check if JWT storage is valid
 * @param storage JWT storage to validate
 * @return 1 if valid, 0 if invalid
 */
static inline int jwt_storage_is_valid(const jwt_storage_t* storage) {
    return platform_crypto_buffer_validate((crypto_buffer_t*)storage);
}

/**
 * Check if JWT storage has a token
 * @param storage JWT storage to check
 * @return 1 if has token, 0 if empty
 */
int jwt_storage_has_token(const jwt_storage_t* storage);

/**
 * Get JWT token string from storage
 * @param storage JWT storage
 * @return Pointer to token string or NULL
 */
const char* jwt_storage_get_string(const jwt_storage_t* storage);

/**
 * Get JWT token length from storage
 * @param storage JWT storage
 * @return Token length in bytes
 */
size_t jwt_storage_get_length(const jwt_storage_t* storage);

/**
 * Extract Bearer token from Authorization header
 * @param storage JWT storage to store token
 * @param auth_header Authorization header string
 * @return JWT_STORAGE_SUCCESS on success, error code on failure
 */
jwt_storage_result_t jwt_storage_extract_bearer_token(jwt_storage_t* storage, const char* auth_header);

/**
 * Get error string for result code
 * @param result Result code
 * @return Human-readable error string
 */
const char* jwt_storage_error_string(jwt_storage_result_t result);

/**
 * Print JWT storage debug information
 * @param storage JWT storage to debug
 */
void jwt_storage_print_info(const jwt_storage_t* storage);

/**
 * Store JWT token in storage
 * @param storage JWT storage
 * @param token JWT token string
 * @return JWT_STORAGE_SUCCESS on success, error code on failure
 */
jwt_storage_result_t jwt_storage_store_token(jwt_storage_t* storage, const char* token);

#endif // JWT_STORAGE_COMPAT_H