#include "jwt_storage_compat.h"
#include <string.h>
#include <stdio.h>

int jwt_storage_has_token(const jwt_storage_t* storage) {
    if (!storage || !platform_crypto_buffer_validate((crypto_buffer_t*)storage)) {
        return 0;
    }

    size_t length = mac_crypto_buffer_get_length((mac_crypto_buffer_t*)storage);
    return (length > 0);
}

const char* jwt_storage_get_string(const jwt_storage_t* storage) {
    if (!storage || !platform_crypto_buffer_validate((crypto_buffer_t*)storage)) {
        return NULL;
    }

    return (const char*)mac_crypto_buffer_get_data((mac_crypto_buffer_t*)storage);
}

size_t jwt_storage_get_length(const jwt_storage_t* storage) {
    if (!storage || !platform_crypto_buffer_validate((crypto_buffer_t*)storage)) {
        return 0;
    }

    return mac_crypto_buffer_get_length((mac_crypto_buffer_t*)storage);
}

jwt_storage_result_t jwt_storage_extract_bearer_token(jwt_storage_t* storage, const char* auth_header) {
    if (!storage || !auth_header) {
        return JWT_STORAGE_ERROR_NULL_POINTER;
    }

    if (!platform_crypto_buffer_validate(storage)) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    // Check for "Bearer " prefix
    const char* bearer_prefix = "Bearer ";
    size_t prefix_len = strlen(bearer_prefix);

    if (strncmp(auth_header, bearer_prefix, prefix_len) != 0) {
        return JWT_STORAGE_ERROR_INVALID_FORMAT;
    }

    // Extract token part (skip "Bearer ")
    const char* token_start = auth_header + prefix_len;

    // Skip any whitespace
    while (*token_start == ' ' || *token_start == '\t') {
        token_start++;
    }

    if (*token_start == '\0') {
        return JWT_STORAGE_ERROR_INVALID_FORMAT;
    }

    // Calculate token length (until end of string or whitespace)
    size_t token_len = 0;
    const char* token_end = token_start;
    while (*token_end != '\0' && *token_end != ' ' && *token_end != '\t' && *token_end != '\r' && *token_end != '\n') {
        token_end++;
        token_len++;
    }

    if (token_len == 0) {
        return JWT_STORAGE_ERROR_INVALID_FORMAT;
    }

    if (token_len > 2048) { // Max JWT size
        return JWT_STORAGE_ERROR_BUFFER_TOO_SMALL;
    }

    // Store token in crypto buffer
    void* data = mac_crypto_buffer_get_data((mac_crypto_buffer_t*)storage);
    if (!data) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    // Copy token and null-terminate
    memcpy(data, token_start, token_len);
    ((char*)data)[token_len] = '\0';

    // Set buffer length
    if (mac_crypto_buffer_set_length((mac_crypto_buffer_t*)storage, token_len) != 0) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    return JWT_STORAGE_SUCCESS;
}

const char* jwt_storage_error_string(jwt_storage_result_t result) {
    switch (result) {
        case JWT_STORAGE_SUCCESS:
            return "Success";
        case JWT_STORAGE_ERROR_INVALID_FORMAT:
            return "Invalid Authorization header format";
        case JWT_STORAGE_ERROR_BUFFER_TOO_SMALL:
            return "Token too large for buffer";
        case JWT_STORAGE_ERROR_INVALID_TOKEN:
            return "Invalid token or buffer";
        case JWT_STORAGE_ERROR_NULL_POINTER:
            return "NULL pointer provided";
        default:
            return "Unknown error";
    }
}

void jwt_storage_print_info(const jwt_storage_t* storage) {
    if (!storage) {
        printf("🔍 JWT Storage: NULL\n");
        return;
    }

    printf("🔍 JWT Storage Debug:\n");

    if (!platform_crypto_buffer_validate((crypto_buffer_t*)storage)) {
        printf("  Status: ❌ Invalid buffer\n");
        return;
    }

    size_t length = mac_crypto_buffer_get_length((mac_crypto_buffer_t*)storage);
    const char* data = (const char*)mac_crypto_buffer_get_data((mac_crypto_buffer_t*)storage);

    printf("  Status: ✅ Valid\n");
    printf("  Length: %zu bytes\n", length);

    if (length > 0 && data) {
        printf("  Has Token: Yes\n");
        printf("  Preview: %.50s%s\n", data, (length > 50) ? "..." : "");
    } else {
        printf("  Has Token: No\n");
    }

    // Print additional crypto buffer debug info
    mac_crypto_buffer_debug((mac_crypto_buffer_t*)storage, "JWT Storage");
}

jwt_storage_result_t jwt_storage_store_token(jwt_storage_t* storage, const char* token) {
    if (!storage || !token) {
        return JWT_STORAGE_ERROR_NULL_POINTER;
    }

    if (!platform_crypto_buffer_validate(storage)) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    size_t token_len = strlen(token);
    if (token_len > 2048) {
        return JWT_STORAGE_ERROR_BUFFER_TOO_SMALL;
    }

    void* data = mac_crypto_buffer_get_data((mac_crypto_buffer_t*)storage);
    if (!data) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    strcpy((char*)data, token);

    if (mac_crypto_buffer_set_length((mac_crypto_buffer_t*)storage, token_len) != 0) {
        return JWT_STORAGE_ERROR_INVALID_TOKEN;
    }

    return JWT_STORAGE_SUCCESS;
}