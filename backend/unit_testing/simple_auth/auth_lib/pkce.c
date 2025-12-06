#include "pkce.h"
#include "base64url.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/**
 * PKCE (Proof Key for Code Exchange) Implementation
 *
 * This module implements RFC 7636 PKCE validation for OAuth 2.1 compliance.
 * Uses BoringSSL for cryptographic operations (SHA256, random generation).
 */

int pkce_generate_code_verifier(char *verifier_out, size_t length) {
    if (!verifier_out || length < PKCE_CODE_VERIFIER_MIN_LENGTH ||
        length > PKCE_CODE_VERIFIER_MAX_LENGTH) {
        return -1;
    }

    // Generate random bytes for the verifier
    size_t random_bytes_needed = (length * 3) / 4; // Base64url expansion factor
    unsigned char *random_bytes = malloc(random_bytes_needed);
    if (!random_bytes) {
        return -1;
    }

    // Use BoringSSL RAND_bytes for cryptographically secure random generation
    if (RAND_bytes(random_bytes, random_bytes_needed) != 1) {
        free(random_bytes);
        return -1;
    }

    // Encode to base64url and truncate to desired length
    char temp_verifier[256];
    int encoded_len = base64url_encode(random_bytes, random_bytes_needed,
                                      temp_verifier, sizeof(temp_verifier));
    free(random_bytes);

    if (encoded_len < 0) {
        return -1;
    }

    // Truncate to exact length and ensure null termination
    if ((size_t)encoded_len > length) {
        temp_verifier[length] = '\0';
    }

    // Replace any characters not in the allowed set with safe alternatives
    for (size_t i = 0; i < length && temp_verifier[i]; i++) {
        char c = temp_verifier[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.' ||
              c == '_' || c == '~')) {
            // Replace invalid chars with valid ones
            temp_verifier[i] = PKCE_VERIFIER_CHARSET[i % (sizeof(PKCE_VERIFIER_CHARSET) - 1)];
        }
    }

    strncpy(verifier_out, temp_verifier, length);
    verifier_out[length] = '\0';
    return 0;
}

int pkce_generate_code_challenge(const char *verifier, char *challenge_out) {
    printf("🔍 CHALLENGE GEN DEBUG: Generating code challenge\n");

    if (!verifier || !challenge_out) {
        printf("❌ CHALLENGE GEN DEBUG: NULL parameter - verifier=%p, challenge_out=%p\n",
               (void*)verifier, (void*)challenge_out);
        return -1;
    }

    printf("📝 CHALLENGE GEN DEBUG: Input verifier='%s' (len: %zu)\n", verifier, strlen(verifier));

    // Validate verifier format
    printf("🔍 CHALLENGE GEN DEBUG: Validating verifier format\n");
    if (!pkce_validate_verifier_format(verifier)) {
        printf("❌ CHALLENGE GEN DEBUG: Verifier format validation failed\n");
        return -1;
    }
    printf("✅ CHALLENGE GEN DEBUG: Verifier format is valid\n");

    // Compute SHA256 hash of the verifier
    printf("🔍 CHALLENGE GEN DEBUG: Computing SHA256 hash\n");
    unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        printf("❌ CHALLENGE GEN DEBUG: Failed to create MD context\n");
        return -1;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(mdctx, verifier, strlen(verifier)) != 1 ||
        EVP_DigestFinal_ex(mdctx, sha256_hash, NULL) != 1) {
        printf("❌ CHALLENGE GEN DEBUG: SHA256 computation failed\n");
        EVP_MD_CTX_free(mdctx);
        return -1;
    }
    printf("✅ CHALLENGE GEN DEBUG: SHA256 hash computed successfully\n");

    EVP_MD_CTX_free(mdctx);

    // Encode hash as base64url
    printf("🔍 CHALLENGE GEN DEBUG: Encoding SHA256 hash to base64url\n");
    size_t required_size = base64url_encode_len(SHA256_DIGEST_LENGTH);
    printf("📝 CHALLENGE GEN DEBUG: Hash size: %d bytes, required buffer size: %zu bytes\n",
           SHA256_DIGEST_LENGTH, required_size);

    int encoded_len = base64url_encode(sha256_hash, SHA256_DIGEST_LENGTH,
                                      challenge_out, required_size);

    printf("📝 CHALLENGE GEN DEBUG: base64url_encode returned length: %d (expected: %d)\n",
           encoded_len, PKCE_CODE_CHALLENGE_LENGTH);

    if (encoded_len != PKCE_CODE_CHALLENGE_LENGTH) {
        printf("❌ CHALLENGE GEN DEBUG: Encoded length mismatch - got %d, expected %d\n",
               encoded_len, PKCE_CODE_CHALLENGE_LENGTH);
        return -1;
    }

    printf("✅ CHALLENGE GEN DEBUG: Challenge generated successfully: '%.10s...'\n", challenge_out);
    return 0;
}

int pkce_validate_code_verifier(const char *verifier, const char *stored_challenge) {
    printf("🔍 PKCE VALIDATE DEBUG: Starting code verifier validation\n");

    if (!verifier || !stored_challenge) {
        printf("❌ PKCE VALIDATE DEBUG: NULL parameter - verifier=%p, stored_challenge=%p\n",
               (void*)verifier, (void*)stored_challenge);
        return 0;
    }

    printf("📝 PKCE VALIDATE DEBUG: verifier='%s' (len: %zu)\n", verifier, strlen(verifier));
    printf("📝 PKCE VALIDATE DEBUG: stored_challenge='%s' (len: %zu)\n", stored_challenge, strlen(stored_challenge));

    // Validate input formats
    printf("🔍 PKCE VALIDATE DEBUG: Validating verifier format\n");
    int verifier_valid = pkce_validate_verifier_format(verifier);
    printf("📊 PKCE VALIDATE DEBUG: Verifier format valid: %s\n", verifier_valid ? "YES" : "NO");

    printf("🔍 PKCE VALIDATE DEBUG: Validating challenge format\n");
    int challenge_valid = pkce_validate_challenge_format(stored_challenge);
    printf("📊 PKCE VALIDATE DEBUG: Challenge format valid: %s\n", challenge_valid ? "YES" : "NO");

    if (!verifier_valid || !challenge_valid) {
        printf("❌ PKCE VALIDATE DEBUG: Format validation failed\n");
        return 0;
    }

    // Generate challenge from provided verifier
    printf("🔍 PKCE VALIDATE DEBUG: Computing challenge from verifier\n");
    char computed_challenge[base64url_encode_len(SHA256_DIGEST_LENGTH)];
    if (pkce_generate_code_challenge(verifier, computed_challenge) != 0) {
        printf("❌ PKCE VALIDATE DEBUG: Challenge generation failed\n");
        return 0;
    }

    printf("📝 PKCE VALIDATE DEBUG: computed_challenge='%s' (len: %zu)\n", computed_challenge, strlen(computed_challenge));
    printf("📝 PKCE VALIDATE DEBUG: stored_challenge='%s' (len: %zu)\n", stored_challenge, strlen(stored_challenge));

    // Use constant-time comparison to prevent timing attacks
    printf("🔍 PKCE VALIDATE DEBUG: Comparing computed vs stored challenge\n");
    int comparison_result = pkce_constant_time_compare(computed_challenge, stored_challenge);
    printf("📊 PKCE VALIDATE DEBUG: Challenge comparison: %s\n", comparison_result ? "MATCH" : "MISMATCH");

    return comparison_result;
}

int pkce_validate_verifier_format(const char *verifier) {
    if (!verifier) {
        return 0;
    }

    size_t len = strlen(verifier);
    if (len < PKCE_CODE_VERIFIER_MIN_LENGTH || len > PKCE_CODE_VERIFIER_MAX_LENGTH) {
        return 0;
    }

    // Check that all characters are in the allowed set: [A-Za-z0-9-._~]
    for (size_t i = 0; i < len; i++) {
        char c = verifier[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.' ||
              c == '_' || c == '~')) {
            return 0;
        }
    }

    return 1;
}

int pkce_validate_challenge_format(const char *challenge) {
    printf("🔍 CHALLENGE FORMAT DEBUG: Validating challenge format\n");

    if (!challenge) {
        printf("❌ CHALLENGE FORMAT DEBUG: Challenge is NULL\n");
        return 0;
    }

    size_t len = strlen(challenge);
    printf("📝 CHALLENGE FORMAT DEBUG: Challenge length: %zu (expected: %d)\n", len, PKCE_CODE_CHALLENGE_LENGTH);

    if (len != PKCE_CODE_CHALLENGE_LENGTH) {
        printf("❌ CHALLENGE FORMAT DEBUG: Length mismatch - got %zu, expected %d\n", len, PKCE_CODE_CHALLENGE_LENGTH);
        return 0;
    }

    // Check that all characters are valid base64url: [A-Za-z0-9-_]
    printf("🔍 CHALLENGE FORMAT DEBUG: Checking character validity\n");
    for (size_t i = 0; i < len; i++) {
        char c = challenge[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            printf("❌ CHALLENGE FORMAT DEBUG: Invalid character '%c' at position %zu\n", c, i);
            return 0;
        }
    }

    printf("✅ CHALLENGE FORMAT DEBUG: Challenge format is valid\n");

    return 1;
}

int pkce_validate_method(const char *method) {
    if (!method) {
        return 0;
    }

    // OAuth 2.1 only allows S256 method
    return strcmp(method, PKCE_METHOD_S256) == 0;
}

int pkce_generate_random_bytes(unsigned char *buffer, size_t length) {
    if (!buffer || length == 0) {
        return -1;
    }

    // Use BoringSSL RAND_bytes for cryptographically secure random generation
    if (RAND_bytes(buffer, length) != 1) {
        return -1;
    }

    return 0;
}

int pkce_constant_time_compare(const char *a, const char *b) {
    if (!a || !b) {
        return 0;
    }

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    // If lengths differ, still do comparison to prevent timing attacks
    size_t min_len = (len_a < len_b) ? len_a : len_b;
    size_t max_len = (len_a > len_b) ? len_a : len_b;

    int result = 1;

    // Compare up to minimum length
    for (size_t i = 0; i < min_len; i++) {
        if (a[i] != b[i]) {
            result = 0;
        }
    }

    // Continue comparison with padding to prevent length-based timing attacks
    for (size_t i = min_len; i < max_len; i++) {
        char pad_a = (i < len_a) ? a[i] : '\0';
        char pad_b = (i < len_b) ? b[i] : '\0';
        if (pad_a != pad_b) {
            result = 0;
        }
    }

    // If lengths differ, result should be 0
    if (len_a != len_b) {
        result = 0;
    }

    return result;
}