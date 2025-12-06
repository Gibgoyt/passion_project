#define _DEFAULT_SOURCE
#include "jwt_storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

// Simple hash function for checksum calculation
static uint64_t simple_hash(const unsigned char* data, size_t length) {
    uint64_t hash = 0x14650FB0739D0383ULL; // Prime seed
    for (size_t i = 0; i < length; i++) {
        hash ^= data[i];
        hash *= 0x100000001B3ULL; // FNV prime
    }
    return hash;
}

// Get current timestamp in microseconds
static uint64_t get_timestamp_us(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

jwt_storage_t* jwt_storage_create(size_t max_token_size, uint32_t validation_flags) {
    // Use default size if not specified
    if (max_token_size == 0) {
        max_token_size = JWT_MAX_LENGTH;
    }

    // Validate parameters
    if (max_token_size > JWT_MAX_LENGTH * 2) {
        return NULL;  // Unreasonably large
    }

    // Calculate pages needed
    size_t pages_for_metadata = 1;  // First page for jwt_storage_t
    size_t pages_for_token = page_count_for_size(max_token_size);
    size_t total_pages = pages_for_metadata + pages_for_token;

    // Allocate pages with cache alignment for performance
    void* raw_memory = page_allocate_aligned(total_pages);
    if (!raw_memory) {
        return NULL;
    }

    // Initialize the storage structure in the first page
    jwt_storage_t* storage = (jwt_storage_t*)raw_memory;
    memset(storage, 0, sizeof(jwt_storage_t));

    storage->magic = JWT_STORAGE_MAGIC;
    storage->pages = raw_memory;
    storage->num_pages = total_pages;
    storage->capacity = pages_for_token * ARM64_PAGE_SIZE;
    storage->token_length = 0;
    storage->validation_flags = validation_flags;
    storage->checksum = 0;
    storage->creation_timestamp = get_timestamp_us();
    storage->last_access_timestamp = storage->creation_timestamp;
    storage->is_locked = 0;
    storage->is_readonly = 0;

    // Lock pages in memory for security
    if (page_lock_memory(raw_memory, total_pages) == PAGE_ALLOC_SUCCESS) {
        storage->is_locked = 1;
    }

    return storage;
}

jwt_storage_t* jwt_storage_create_default(void) {
    return jwt_storage_create(JWT_MAX_LENGTH, JWT_VALIDATE_DEFAULT);
}

void jwt_storage_destroy(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return;
    }

    // Clear sensitive data first
    jwt_storage_clear(storage);

    // Unlock memory if it was locked
    if (storage->is_locked) {
        page_unlock_memory(storage->pages, storage->num_pages);
    }

    // Clear the entire allocation including metadata
    page_secure_clear(storage->pages, storage->num_pages);

    // Free pages back to kernel
    page_free(storage->pages, storage->num_pages);
}

unsigned char* jwt_storage_get_data_ptr(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return NULL;
    }

    // Token data starts after the first page (which contains metadata)
    return (unsigned char*)storage->pages + ARM64_PAGE_SIZE;
}

void jwt_storage_update_checksum(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC || storage->token_length == 0) {
        storage->checksum = 0;
        return;
    }

    unsigned char* data_ptr = jwt_storage_get_data_ptr(storage);
    storage->checksum = simple_hash(data_ptr, storage->token_length);
}

int jwt_storage_verify_checksum(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return 0;
    }

    if (storage->token_length == 0) {
        return storage->checksum == 0;
    }

    unsigned char* data_ptr = jwt_storage_get_data_ptr(storage);
    uint64_t computed = simple_hash(data_ptr, storage->token_length);
    return computed == storage->checksum;
}

jwt_storage_result_t jwt_storage_validate_token(const unsigned char* token,
                                              size_t length,
                                              uint32_t validation_flags) {
    if (!token && length > 0) {
        return JWT_STORAGE_ERROR_INVALID_PARAM;
    }

    if (length == 0) {
        return JWT_STORAGE_SUCCESS;  // Empty token is valid
    }

    // Check length limits
    if (validation_flags & JWT_VALIDATE_LENGTH_LIMITS) {
        if (length > JWT_MAX_LENGTH) {
            return JWT_STORAGE_ERROR_TOKEN_TOO_LARGE;
        }
    }

    // Check for embedded null bytes
    if (validation_flags & JWT_VALIDATE_NO_EMBEDDED_NULLS) {
        for (size_t i = 0; i < length; i++) {
            if (token[i] == '\0') {
                return JWT_STORAGE_ERROR_EMBEDDED_NULL_FOUND;
            }
        }
    }

    // Check ASCII-only constraint
    if (validation_flags & JWT_VALIDATE_ASCII_ONLY) {
        for (size_t i = 0; i < length; i++) {
            if (!isascii(token[i]) || iscntrl(token[i])) {
                // Allow specific characters that are valid in JWT
                if (token[i] != '\n' && token[i] != '\r' && token[i] != '\t') {
                    return JWT_STORAGE_ERROR_NON_ASCII_CHARACTER;
                }
            }
        }
    }

    return JWT_STORAGE_SUCCESS;
}

jwt_storage_result_t jwt_storage_store_token(jwt_storage_t* storage,
                                           const unsigned char* token,
                                           size_t length) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    if (storage->is_readonly) {
        return JWT_STORAGE_ERROR_INVALID_PARAM;
    }

    if (!jwt_storage_verify_checksum(storage)) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    // Validate token
    jwt_storage_result_t validation = jwt_storage_validate_token(token, length,
                                                               storage->validation_flags);
    if (validation != JWT_STORAGE_SUCCESS) {
        return validation;
    }

    // Check capacity
    if (length > storage->capacity) {
        return JWT_STORAGE_ERROR_TOKEN_TOO_LARGE;
    }

    // Clear existing token first
    jwt_storage_clear(storage);

    if (length > 0 && token) {
        unsigned char* data_ptr = jwt_storage_get_data_ptr(storage);
        memcpy(data_ptr, token, length);
        storage->token_length = length;

        // Add null termination if validation requires it
        if (storage->validation_flags & JWT_VALIDATE_NULL_TERMINATION) {
            if (length < storage->capacity) {
                data_ptr[length] = '\0';
            }
        }
    }

    // Update metadata
    storage->last_access_timestamp = get_timestamp_us();
    jwt_storage_update_checksum(storage);

    return JWT_STORAGE_SUCCESS;
}

jwt_storage_result_t jwt_storage_store_string(jwt_storage_t* storage,
                                            const char* token_str) {
    if (!token_str) {
        return jwt_storage_store_token(storage, NULL, 0);
    }

    size_t length = strlen(token_str);
    return jwt_storage_store_token(storage, (const unsigned char*)token_str, length);
}

const unsigned char* jwt_storage_get_token(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return NULL;
    }

    if (!jwt_storage_verify_checksum(storage)) {
        return NULL;
    }

    if (storage->token_length == 0) {
        return NULL;
    }

    storage->last_access_timestamp = get_timestamp_us();
    return jwt_storage_get_data_ptr(storage);
}

const char* jwt_storage_get_string(jwt_storage_t* storage) {
    const unsigned char* token = jwt_storage_get_token(storage);
    if (!token) {
        return NULL;
    }

    // Ensure null termination for string access
    if (storage->validation_flags & JWT_VALIDATE_NULL_TERMINATION) {
        unsigned char* data_ptr = jwt_storage_get_data_ptr(storage);
        if (storage->token_length < storage->capacity) {
            data_ptr[storage->token_length] = '\0';
        }
    }

    return (const char*)token;
}

size_t jwt_storage_get_length(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return 0;
    }

    if (!jwt_storage_verify_checksum(storage)) {
        return 0;
    }

    return storage->token_length;
}

int jwt_storage_is_valid(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return 0;
    }

    return jwt_storage_verify_checksum(storage);
}

int jwt_storage_has_token(jwt_storage_t* storage) {
    return jwt_storage_is_valid(storage) && storage->token_length > 0;
}

jwt_storage_result_t jwt_storage_clear(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    if (storage->is_readonly) {
        return JWT_STORAGE_ERROR_INVALID_PARAM;
    }

    if (storage->token_length > 0) {
        unsigned char* data_ptr = jwt_storage_get_data_ptr(storage);
        // Secure clear using volatile pointers
        volatile unsigned char* volatile_ptr = data_ptr;
        for (size_t i = 0; i < storage->capacity; i++) {
            volatile_ptr[i] = 0;
        }
    }

    storage->token_length = 0;
    storage->checksum = 0;
    storage->last_access_timestamp = get_timestamp_us();

    return JWT_STORAGE_SUCCESS;
}

jwt_storage_result_t jwt_storage_make_readonly(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    storage->is_readonly = 1;

    // Make the pages read-only at the OS level
    page_alloc_result_t result = page_make_readonly(storage->pages, storage->num_pages);
    if (result != PAGE_ALLOC_SUCCESS) {
        return JWT_STORAGE_ERROR_INVALID_PARAM;
    }

    return JWT_STORAGE_SUCCESS;
}

jwt_storage_result_t jwt_storage_extract_bearer_token(jwt_storage_t* storage,
                                                    const char* auth_header) {
    if (!storage || !auth_header) {
        return JWT_STORAGE_ERROR_INVALID_PARAM;
    }

    if (storage->magic != JWT_STORAGE_MAGIC) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    // Look for "Bearer " prefix (case-sensitive)
    const char bearer_prefix[] = "Bearer ";
    const size_t prefix_len = sizeof(bearer_prefix) - 1;

    if (strncmp(auth_header, bearer_prefix, prefix_len) != 0) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN_FORMAT;
    }

    // Extract token part after "Bearer "
    const char* token_start = auth_header + prefix_len;
    size_t token_length = strlen(token_start);

    // Remove trailing whitespace
    while (token_length > 0 && isspace(token_start[token_length - 1])) {
        token_length--;
    }

    if (token_length == 0) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN_FORMAT;
    }

    return jwt_storage_store_token(storage, (const unsigned char*)token_start, token_length);
}

size_t jwt_storage_get_capacity(jwt_storage_t* storage) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return 0;
    }
    return storage->capacity;
}

jwt_storage_result_t jwt_storage_get_info(jwt_storage_t* storage,
                                        size_t* pages_used,
                                        size_t* bytes_used,
                                        int* is_locked) {
    if (!storage || storage->magic != JWT_STORAGE_MAGIC) {
        return JWT_STORAGE_ERROR_CORRUPTION_DETECTED;
    }

    if (pages_used) *pages_used = storage->num_pages;
    if (bytes_used) *bytes_used = storage->num_pages * ARM64_PAGE_SIZE;
    if (is_locked) *is_locked = storage->is_locked;

    return JWT_STORAGE_SUCCESS;
}

const char* jwt_storage_error_string(jwt_storage_result_t error) {
    switch (error) {
        case JWT_STORAGE_SUCCESS:
            return "Success";
        case JWT_STORAGE_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case JWT_STORAGE_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case JWT_STORAGE_ERROR_CORRUPTION_DETECTED:
            return "Memory corruption detected";
        case JWT_STORAGE_ERROR_TOKEN_TOO_LARGE:
            return "Token exceeds maximum size";
        case JWT_STORAGE_ERROR_INVALID_TOKEN_FORMAT:
            return "Invalid token format";
        case JWT_STORAGE_ERROR_NULL_TERMINATION_MISSING:
            return "Null termination required but missing";
        case JWT_STORAGE_ERROR_EMBEDDED_NULL_FOUND:
            return "Embedded null byte found in token";
        case JWT_STORAGE_ERROR_NON_ASCII_CHARACTER:
            return "Non-ASCII character found in token";
        case JWT_STORAGE_ERROR_MEMORY_LOCK_FAILED:
            return "Memory lock failed";
        case JWT_STORAGE_ERROR_PAGE_ALLOCATION_FAILED:
            return "Page allocation failed";
        default:
            return "Unknown error";
    }
}

void jwt_storage_print_info(jwt_storage_t* storage) {
    if (!storage) {
        printf("=== JWT Storage: NULL POINTER ===\n");
        return;
    }

    printf("=== JWT Storage Info ===\n");

    if (storage->magic != JWT_STORAGE_MAGIC) {
        printf("⚠️  CORRUPTED: Invalid magic number (0x%016lX)\n", storage->magic);
        return;
    }

    printf("Magic: 0x%016lX %s\n", storage->magic,
           storage->magic == JWT_STORAGE_MAGIC ? "✓" : "✗");
    printf("Pages: %zu (%zu bytes)\n", storage->num_pages,
           storage->num_pages * ARM64_PAGE_SIZE);
    printf("Capacity: %zu bytes\n", storage->capacity);
    printf("Token length: %zu bytes\n", storage->token_length);
    printf("Memory locked: %s\n", storage->is_locked ? "Yes" : "No");
    printf("Read-only: %s\n", storage->is_readonly ? "Yes" : "No");
    printf("Checksum: 0x%016lX %s\n", storage->checksum,
           jwt_storage_verify_checksum(storage) ? "✓" : "✗");

    if (storage->token_length > 0) {
        const char* token_str = jwt_storage_get_string(storage);
        if (token_str) {
            printf("Token preview: %.50s%s\n", token_str,
                   storage->token_length > 50 ? "..." : "");
        }
    }

    // Show validation flags
    printf("Validation flags: 0x%02X\n", storage->validation_flags);
    if (storage->validation_flags & JWT_VALIDATE_NULL_TERMINATION)
        printf("  - Null termination required\n");
    if (storage->validation_flags & JWT_VALIDATE_NO_EMBEDDED_NULLS)
        printf("  - No embedded nulls\n");
    if (storage->validation_flags & JWT_VALIDATE_ASCII_ONLY)
        printf("  - ASCII only\n");
    if (storage->validation_flags & JWT_VALIDATE_LENGTH_LIMITS)
        printf("  - Length limits enforced\n");

    // Show timing information
    printf("Created: %lu μs ago\n", get_timestamp_us() - storage->creation_timestamp);
    printf("Last access: %lu μs ago\n", get_timestamp_us() - storage->last_access_timestamp);

    // Show page information
    page_print_info(storage->pages, storage->num_pages);

    printf("=======================\n");
}

// ========================================================================
// Cryptographic Buffer Implementation
// ========================================================================

crypto_buffer_t* crypto_buffer_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Calculate pages needed for the buffer data
    size_t data_pages = page_count_for_size(size);

    // Allocate one page for metadata + calculated pages for data
    size_t total_pages = 1 + data_pages;

    // Use cache-aligned allocation for performance
    void* raw_memory = page_allocate_aligned(total_pages);
    if (!raw_memory) {
        return NULL;
    }

    // Initialize crypto buffer structure in first page
    crypto_buffer_t* buffer = (crypto_buffer_t*)raw_memory;
    buffer->data = (unsigned char*)raw_memory + ARM64_PAGE_SIZE;  // Data starts after metadata page
    buffer->size = size;
    buffer->num_pages = total_pages;
    buffer->magic = CRYPTO_BUFFER_MAGIC;
    buffer->is_locked = 0;

    // Lock pages in memory for security (prevent swapping)
    if (page_lock_memory(raw_memory, total_pages) == PAGE_ALLOC_SUCCESS) {
        buffer->is_locked = 1;
    }

    return buffer;
}

void crypto_buffer_free(crypto_buffer_t* buffer) {
    if (!buffer || !crypto_buffer_is_valid(buffer)) {
        return;
    }

    // Secure clear all data pages
    if (buffer->data && buffer->num_pages > 1) {
        page_secure_clear(buffer->data, buffer->num_pages - 1);
    }

    // Clear magic to prevent reuse
    buffer->magic = 0;

    // Free all pages including metadata
    page_free(buffer, buffer->num_pages);
}

int crypto_buffer_is_valid(crypto_buffer_t* buffer) {
    if (!buffer) {
        return 0;
    }

    // Check magic number
    if (buffer->magic != CRYPTO_BUFFER_MAGIC) {
        return 0;
    }

    // Check basic consistency
    if (buffer->size == 0 || buffer->num_pages == 0 || !buffer->data) {
        return 0;
    }

    return 1;
}

void* crypto_buffer_get_data(crypto_buffer_t* buffer) {
    if (!crypto_buffer_is_valid(buffer)) {
        return NULL;
    }
    return buffer->data;
}

size_t crypto_buffer_get_size(crypto_buffer_t* buffer) {
    if (!crypto_buffer_is_valid(buffer)) {
        return 0;
    }
    return buffer->size;
}