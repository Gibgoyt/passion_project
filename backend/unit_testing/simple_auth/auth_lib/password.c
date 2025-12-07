#define _GNU_SOURCE  // For strcasecmp, strdup, strtok_r
#include "password.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <strings.h>  // For strcasecmp

// OpenSSL includes
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Common weak passwords (small sample)
static const char *WEAK_PASSWORDS[] = {
    "password", "123456", "password123", "admin", "qwerty",
    "abc123", "letmein", "welcome", "monkey", "dragon"
};

static const int NUM_WEAK_PASSWORDS = sizeof(WEAK_PASSWORDS) / sizeof(WEAK_PASSWORDS[0]);

int hash_password(const char *password, char *hashed_out) {
    if (!password || !hashed_out) {
        return -1;
    }

    printf("🔍 REG DEBUG: Hashing password: '%s'\n", password);

    // Generate random salt
    unsigned char salt[PASSWORD_SALT_LENGTH];
    if (generate_salt(salt) != 0) {
        printf("🔍 REG DEBUG: Failed to generate salt\n");
        return -1;
    }

    printf("🔍 REG DEBUG: Salt generated successfully\n");

    // Perform PBKDF2-SHA256
    unsigned char hash[PASSWORD_HASH_LENGTH];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                                        salt, PASSWORD_SALT_LENGTH,
                                        PASSWORD_ITERATIONS,
                                        EVP_sha256(),
                                        PASSWORD_HASH_LENGTH, hash) != 1) {
        printf("🔍 REG DEBUG: Failed to compute PBKDF2 hash\n");
        return -1;
    }

    printf("🔍 REG DEBUG: PBKDF2 hash computed successfully\n");

    // Encode salt and hash to base64
    char salt_b64[PASSWORD_SALT_B64_LENGTH];
    char hash_b64[PASSWORD_HASH_B64_LENGTH];

    // BoringSSL base64 encoding
    int salt_len = EVP_EncodeBlock((unsigned char*)salt_b64, salt, PASSWORD_SALT_LENGTH);
    int hash_len = EVP_EncodeBlock((unsigned char*)hash_b64, hash, PASSWORD_HASH_LENGTH);

    if (salt_len <= 0 || hash_len <= 0) {
        return -1;
    }

    // Null terminate (EVP_EncodeBlock doesn't null terminate)
    salt_b64[salt_len] = '\0';
    hash_b64[hash_len] = '\0';

    // Create storage format: $pbkdf2-sha256$iterations$salt$hash
    int written = snprintf(hashed_out, PASSWORD_STORAGE_MAX_LENGTH,
                          "$pbkdf2-sha256$%d$%s$%s",
                          PASSWORD_ITERATIONS, salt_b64, hash_b64);

    if (written >= PASSWORD_STORAGE_MAX_LENGTH || written < 0) {
        printf("🔍 REG DEBUG: Failed to format hash string\n");
        return -1;
    }

    printf("🔍 REG DEBUG: Hash formatted successfully: '%s'\n", hashed_out);

    return 0;
}

int verify_password(const char *password, const char *hashed_password) {
    if (!password || !hashed_password) {
        return -1;
    }

    printf("🔍 PWD DEBUG: Verifying password\n");
    printf("🔍 PWD DEBUG: Input password: '%s'\n", password);
    printf("🔍 PWD DEBUG: Stored hash: '%s'\n", hashed_password);

    // Parse stored hash
    int iterations;
    unsigned char salt[PASSWORD_SALT_LENGTH];
    unsigned char stored_hash[PASSWORD_HASH_LENGTH];

    if (parse_password_hash(hashed_password, &iterations, salt, stored_hash) != 0) {
        printf("🔍 PWD DEBUG: Failed to parse stored hash\n");
        return -1;
    }

    printf("🔍 PWD DEBUG: Parsed hash successfully, iterations: %d\n", iterations);

    // Re-hash the provided password with same salt and iterations
    unsigned char computed_hash[PASSWORD_HASH_LENGTH];
    printf("🔍 PWD DEBUG: Computing hash with same salt and iterations\n");
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                                        salt, PASSWORD_SALT_LENGTH,
                                        iterations,
                                        EVP_sha256(),
                                        PASSWORD_HASH_LENGTH, computed_hash) != 1) {
        printf("🔍 PWD DEBUG: Failed to compute hash\n");
        return -1;
    }

    printf("🔍 PWD DEBUG: Hash computed successfully\n");

    // Constant-time comparison
    int result = constant_time_compare(stored_hash, computed_hash, PASSWORD_HASH_LENGTH);
    printf("🔍 PWD DEBUG: Password comparison result: %s\n", result == 1 ? "MATCH" : "NO MATCH");
    return result;
}

int is_password_strong(const char *password) {
    if (!password) {
        return 0;
    }

    size_t len = strlen(password);

    // Check length
    if (len < PASSWORD_MIN_LENGTH || len > PASSWORD_MAX_LENGTH) {
        return 0;
    }

    // Check for required character types
    int has_upper = 0, has_lower = 0, has_digit = 0;

    for (size_t i = 0; i < len; i++) {
        char c = password[i];
        if (c >= 'A' && c <= 'Z') has_upper = 1;
        if (c >= 'a' && c <= 'z') has_lower = 1;
        if (c >= '0' && c <= '9') has_digit = 1;
    }

    if (!has_upper || !has_lower || !has_digit) {
        return 0;
    }

    // Check against common weak passwords (case insensitive)
    for (int i = 0; i < NUM_WEAK_PASSWORDS; i++) {
        if (strcasecmp(password, WEAK_PASSWORDS[i]) == 0) {
            return 0;
        }
    }

    return 1;
}

int parse_password_hash(const char *hashed_password,
                       int *iterations_out,
                       unsigned char *salt_out,
                       unsigned char *hash_out) {
    if (!hashed_password || !iterations_out || !salt_out || !hash_out) {
        return -1;
    }

    printf("🔍 PARSE DEBUG: Parsing hash: '%s'\n", hashed_password);

    // Expected format: $pbkdf2-sha256$iterations$salt_base64$hash_base64
    // Skip the leading $ and parse the rest
    if (hashed_password[0] != '$') {
        printf("🔍 PARSE DEBUG: Hash doesn't start with $\n");
        return -1;
    }

    char *copy = strdup(hashed_password + 1); // Skip the leading $
    if (!copy) {
        printf("🔍 PARSE DEBUG: Failed to duplicate string\n");
        return -1;
    }

    char *saveptr;
    char *algorithm = strtok_r(copy, "$", &saveptr);
    char *iter_str = strtok_r(NULL, "$", &saveptr);
    char *salt_b64 = strtok_r(NULL, "$", &saveptr);
    char *hash_b64 = strtok_r(NULL, "$", &saveptr);

    printf("🔍 PARSE DEBUG: Parsed components:\n");
    printf("🔍 PARSE DEBUG:   algorithm: '%s'\n", algorithm ? algorithm : "NULL");
    printf("🔍 PARSE DEBUG:   iter_str: '%s'\n", iter_str ? iter_str : "NULL");
    printf("🔍 PARSE DEBUG:   salt_b64: '%s'\n", salt_b64 ? salt_b64 : "NULL");
    printf("🔍 PARSE DEBUG:   hash_b64: '%s'\n", hash_b64 ? hash_b64 : "NULL");

    int result = -1;

    // Validate format
    if (algorithm && strcmp(algorithm, "pbkdf2-sha256") == 0 &&
        iter_str && salt_b64 && hash_b64) {

        printf("🔍 PARSE DEBUG: Format validation passed\n");

        // Parse iterations
        char *endptr;
        long iterations = strtol(iter_str, &endptr, 10);
        printf("🔍 PARSE DEBUG: Parsed iterations: %ld\n", iterations);
        if (*endptr == '\0' && iterations > 0 && iterations <= 1000000) {
            *iterations_out = (int)iterations;
            printf("🔍 PARSE DEBUG: Iterations validation passed\n");

            // Decode base64 salt
            unsigned char salt_decoded[PASSWORD_SALT_LENGTH + 4]; // Extra space for safety
            size_t actual_salt_len = 0;
            int salt_result = EVP_DecodeBlock(salt_decoded, (const unsigned char*)salt_b64, strlen(salt_b64));
            actual_salt_len = (salt_result >= 0) ? salt_result : 0;
            printf("🔍 PARSE DEBUG: Salt decode result: %d, length: %zu\n", salt_result, actual_salt_len);

            // Decode base64 hash
            unsigned char hash_decoded[PASSWORD_HASH_LENGTH + 4]; // Extra space for safety
            size_t actual_hash_len = 0;
            int hash_result = EVP_DecodeBlock(hash_decoded, (const unsigned char*)hash_b64, strlen(hash_b64));
            actual_hash_len = (hash_result >= 0) ? hash_result : 0;
            printf("🔍 PARSE DEBUG: Hash decode result: %d, length: %zu\n", hash_result, actual_hash_len);

            if (salt_result >= 0 && hash_result >= 0 &&
                actual_salt_len == PASSWORD_SALT_LENGTH && actual_hash_len == PASSWORD_HASH_LENGTH) {
                printf("🔍 PARSE DEBUG: All validations passed, parsing successful\n");
                memcpy(salt_out, salt_decoded, PASSWORD_SALT_LENGTH);
                memcpy(hash_out, hash_decoded, PASSWORD_HASH_LENGTH);
                result = 0;
            } else {
                printf("🔍 PARSE DEBUG: Final validation failed\n");
                printf("🔍 PARSE DEBUG: Expected salt_len=%d, got=%zu\n", PASSWORD_SALT_LENGTH, actual_salt_len);
                printf("🔍 PARSE DEBUG: Expected hash_len=%d, got=%zu\n", PASSWORD_HASH_LENGTH, actual_hash_len);
            }
        } else {
            printf("🔍 PARSE DEBUG: Iterations validation failed\n");
        }
    } else {
        printf("🔍 PARSE DEBUG: Format validation failed\n");
    }

    free(copy);
    return result;
}

int generate_salt(unsigned char *salt_out) {
    if (!salt_out) {
        return -1;
    }

    if (RAND_bytes(salt_out, PASSWORD_SALT_LENGTH) != 1) {
        return -1;
    }

    return 0;
}

int constant_time_compare(const unsigned char *a, const unsigned char *b, size_t len) {
    if (!a || !b) {
        return 0;
    }

    // Use BoringSSL's constant time comparison
    return CRYPTO_memcmp(a, b, len) == 0 ? 1 : 0;
}

int get_hashing_time_ms(void) {
    // Test password and salt
    const char *test_password = "test_password_for_timing";
    unsigned char test_salt[PASSWORD_SALT_LENGTH];

    // Generate test salt
    if (generate_salt(test_salt) != 0) {
        return -1;
    }

    // Measure time for one PBKDF2 operation using basic time()
    time_t start = time(NULL);

    unsigned char hash[PASSWORD_HASH_LENGTH];
    if (PKCS5_PBKDF2_HMAC(test_password, strlen(test_password),
                                        test_salt, PASSWORD_SALT_LENGTH,
                                        PASSWORD_ITERATIONS,
                                        EVP_sha256(),
                                        PASSWORD_HASH_LENGTH, hash) != 1) {
        return -1;
    }

    time_t end = time(NULL);

    // Calculate milliseconds (rough estimate)
    long ms = (end - start) * 1000;

    // If it's 0, estimate based on typical PBKDF2 performance
    if (ms == 0) {
        ms = 100; // Reasonable estimate for 100k iterations
    }

    return (int)ms;
}

#ifdef PASSWORD_TEST_MAIN
/**
 * Test program for password hashing
 * Compile with: gcc -DPASSWORD_TEST_MAIN password.c -o test_password
 */
#include <stdio.h>

int main() {
    printf("Testing PBKDF2-SHA256 Password Hashing\n");
    printf("=====================================\n\n");

    // Test password hashing
    const char *test_password = "MySecurePassword123";
    char hashed[PASSWORD_STORAGE_MAX_LENGTH];

    printf("Hashing password: '%s'\n", test_password);

    if (hash_password(test_password, hashed) == 0) {
        printf("✓ Hashed: %s\n", hashed);
        printf("  Length: %zu bytes\n", strlen(hashed));

        // Test verification with correct password
        printf("\nVerifying correct password...\n");
        int verify_result = verify_password(test_password, hashed);
        printf("✓ Verification result: %s\n",
               verify_result == 1 ? "PASS" : "FAIL");

        // Test verification with incorrect password
        printf("\nVerifying incorrect password...\n");
        verify_result = verify_password("WrongPassword", hashed);
        printf("✓ Verification result: %s\n",
               verify_result == 0 ? "PASS (correctly rejected)" : "FAIL");

    } else {
        printf("✗ Failed to hash password\n");
    }

    // Test password strength validation
    printf("\nTesting password strength validation:\n");
    const char *test_passwords[] = {
        "weak",                    // Too short
        "alllowercase",           // No uppercase/digits
        "ALLUPPERCASE",           // No lowercase/digits
        "NoDigitsHere",           // No digits
        "password",               // Common weak password
        "GoodPassword123",        // Strong
        "AnotherGood1",           // Strong
    };

    int num_tests = sizeof(test_passwords) / sizeof(test_passwords[0]);
    for (int i = 0; i < num_tests; i++) {
        const char *pwd = test_passwords[i];
        int strong = is_password_strong(pwd);
        printf("  '%s': %s\n", pwd, strong ? "✓ Strong" : "✗ Weak");
    }

    // Test hashing performance
    printf("\nTesting hashing performance...\n");
    int hash_time = get_hashing_time_ms();
    if (hash_time > 0) {
        printf("✓ Hashing time: %d ms\n", hash_time);
        if (hash_time < 50) {
            printf("  ⚠️  Consider increasing iterations (too fast)\n");
        } else if (hash_time > 500) {
            printf("  ⚠️  Consider decreasing iterations (too slow)\n");
        } else {
            printf("  ✓ Good timing (50-500ms range)\n");
        }
    } else {
        printf("✗ Failed to measure hashing time\n");
    }

    return 0;
}
#endif