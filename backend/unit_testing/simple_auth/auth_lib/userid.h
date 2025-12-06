#ifndef USERID_H
#define USERID_H

#include <stddef.h>

/**
 * Firebase-style User ID Generation
 *
 * Generates 28-character URL-safe user IDs using base62 encoding
 * of cryptographically secure random bytes.
 *
 * Character set: 0-9A-Za-z (no special characters)
 * Length: Always 28 characters
 * Uniqueness: 168 bits of entropy (very high)
 *
 * Example: "xK8fG2mNpQrS7vW9yB4cD6eH8jL"
 */

// Firebase user ID length (constant)
#define FIREBASE_USERID_LENGTH 28

/**
 * Generate a Firebase-style 28-character user ID
 *
 * Uses BoringSSL's RAND_bytes for cryptographically secure random generation.
 * Encodes 21 random bytes (168 bits) into 28 base62 characters.
 *
 * @param userid_out Buffer to store the generated user ID (must be >= 29 bytes)
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   char userid[FIREBASE_USERID_LENGTH + 1];
 *   if (generate_firebase_userid(userid) == 0) {
 *       printf("Generated user ID: %s\n", userid);
 *   }
 */
int generate_firebase_userid(char *userid_out);

/**
 * Validate a Firebase-style user ID format
 *
 * Checks that the ID is exactly 28 characters and contains only
 * valid base62 characters (0-9, A-Z, a-z).
 *
 * @param userid The user ID string to validate
 * @return 1 if valid, 0 if invalid
 *
 * Example usage:
 *   if (is_valid_firebase_userid("xK8fG2mNpQrS7vW9yB4cD6eH8jL")) {
 *       printf("Valid user ID format\n");
 *   }
 */
int is_valid_firebase_userid(const char *userid);

/**
 * Base62 encoding function (internal use)
 *
 * Encodes binary data into base62 representation using
 * the character set: 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
 *
 * @param input Binary input data
 * @param input_len Length of input data in bytes
 * @param output Output buffer for base62 string
 * @param output_len Size of output buffer
 * @return 0 on success, -1 on error
 */
int base62_encode(const unsigned char *input, size_t input_len,
                  char *output, size_t output_len);

#endif // USERID_H