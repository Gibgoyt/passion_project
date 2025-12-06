#include "userid.h"
#include <string.h>
#include <stdio.h>

// BoringSSL includes with proper prefix
#include <openssl/rand.h>

/**
 * Base62 character set for Firebase-style IDs
 * Uses 0-9, A-Z, a-z (total 62 characters)
 * This is URL-safe and doesn't require escaping
 */
static const char BASE62_CHARS[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int generate_firebase_userid(char *userid_out) {
    if (!userid_out) {
        return -1;
    }

    // Generate 21 random bytes (168 bits of entropy)
    // This encodes to exactly 28 base62 characters
    unsigned char random_bytes[21];

    if (USOCKETS_BSSL_RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        return -1; // Random generation failed
    }

    // Encode to base62
    if (base62_encode(random_bytes, sizeof(random_bytes),
                      userid_out, FIREBASE_USERID_LENGTH + 1) != 0) {
        return -1;
    }

    return 0;
}

int is_valid_firebase_userid(const char *userid) {
    if (!userid) {
        return 0;
    }

    // Check length
    size_t len = strlen(userid);
    if (len != FIREBASE_USERID_LENGTH) {
        return 0;
    }

    // Check characters (must be base62)
    for (size_t i = 0; i < len; i++) {
        char c = userid[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z'))) {
            return 0;
        }
    }

    return 1;
}

int base62_encode(const unsigned char *input, size_t input_len,
                  char *output, size_t output_len) {
    if (!input || !output || input_len == 0 || output_len == 0) {
        return -1;
    }

    // For Firebase user IDs: 21 bytes -> 28 characters
    // We need to implement big integer division by 62

    // Convert input bytes to a big integer (as array of bytes)
    // We'll use a simple implementation for the specific case of 21 bytes
    unsigned char num[21];
    memcpy(num, input, input_len);

    // Output position (fill from right to left)
    int out_pos = 0;

    // Convert to base62 by repeatedly dividing by 62
    while (out_pos < (int)(output_len - 1)) {
        // Check if number is zero
        int is_zero = 1;
        for (size_t i = 0; i < input_len; i++) {
            if (num[i] != 0) {
                is_zero = 0;
                break;
            }
        }

        if (is_zero && out_pos > 0) {
            break; // Done converting
        }

        // Divide by 62 and get remainder
        int remainder = 0;
        for (int i = 0; i < (int)input_len; i++) {
            int temp = remainder * 256 + num[i];
            num[i] = temp / 62;
            remainder = temp % 62;
        }

        // Add character for this remainder
        output[out_pos++] = BASE62_CHARS[remainder];

        if (out_pos >= (int)output_len) {
            return -1; // Output buffer too small
        }
    }

    // Pad with leading zeros if needed for Firebase format
    while (out_pos < FIREBASE_USERID_LENGTH && out_pos < (int)(output_len - 1)) {
        output[out_pos++] = BASE62_CHARS[0]; // '0'
    }

    // Reverse the string (we built it backwards)
    for (int i = 0; i < out_pos / 2; i++) {
        char temp = output[i];
        output[i] = output[out_pos - 1 - i];
        output[out_pos - 1 - i] = temp;
    }

    // Null terminate
    output[out_pos] = '\0';

    return 0;
}

#ifdef USERID_TEST_MAIN
/**
 * Test program for userid generation
 * Compile with: gcc -DUSERID_TEST_MAIN userid.c -o test_userid
 */
#include <stdio.h>
#include <time.h>

int main() {
    printf("Testing Firebase-style User ID Generation\n");
    printf("=========================================\n\n");

    // Test multiple ID generation
    for (int i = 0; i < 10; i++) {
        char userid[FIREBASE_USERID_LENGTH + 1];

        if (generate_firebase_userid(userid) == 0) {
            printf("Generated ID %d: %s (length: %zu)\n",
                   i + 1, userid, strlen(userid));

            // Validate the generated ID
            if (is_valid_firebase_userid(userid)) {
                printf("  ✓ Valid format\n");
            } else {
                printf("  ✗ Invalid format!\n");
            }
        } else {
            printf("  ✗ Failed to generate ID %d\n", i + 1);
        }
        printf("\n");
    }

    // Test validation function
    printf("Testing validation function:\n");
    printf("Valid ID: %s\n",
           is_valid_firebase_userid("xK8fG2mNpQrS7vW9yB4cD6eH8jL") ? "✓" : "✗");
    printf("Too short: %s\n",
           is_valid_firebase_userid("short") ? "✓" : "✗");
    printf("Too long: %s\n",
           is_valid_firebase_userid("xK8fG2mNpQrS7vW9yB4cD6eH8jLTooLong") ? "✓" : "✗");
    printf("Invalid chars: %s\n",
           is_valid_firebase_userid("xK8fG2mNpQrS7vW9yB4cD6eH8j@") ? "✓" : "✗");

    return 0;
}
#endif