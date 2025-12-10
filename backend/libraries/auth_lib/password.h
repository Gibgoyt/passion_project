#ifndef PASSWORD_H
#define PASSWORD_H

#include <stddef.h>

/**
 * PBKDF2-SHA256 Password Hashing
 *
 * Secure password hashing using PBKDF2-SHA256 with configurable iterations.
 * Uses BoringSSL's PKCS5_PBKDF2_HMAC for cryptographically secure hashing.
 *
 * Features:
 * - PBKDF2-SHA256 algorithm (NIST approved)
 * - 16-byte random salt per password
 * - 100,000 iterations (tuned for ~100ms on modern hardware)
 * - 32-byte output hash (SHA-256 size)
 * - Constant-time verification (timing attack protection)
 *
 * Storage format: $pbkdf2-sha256$iterations$salt_base64$hash_base64
 */

// Password hashing parameters
#define PASSWORD_SALT_LENGTH 16       // 128 bits of salt
#define PASSWORD_HASH_LENGTH 32       // 256 bits (SHA-256)
#define PASSWORD_ITERATIONS 100000    // NIST recommended minimum

// Storage format lengths
#define PASSWORD_SALT_B64_LENGTH 25   // Base64 encoded salt + null
#define PASSWORD_HASH_B64_LENGTH 45   // Base64 encoded hash + null
#define PASSWORD_STORAGE_MAX_LENGTH 128 // Complete storage string

// Password policy
#define PASSWORD_MIN_LENGTH 8
#define PASSWORD_MAX_LENGTH 128

/**
 * Hash a password using PBKDF2-SHA256
 *
 * Generates a cryptographically secure hash with random salt.
 * Uses BoringSSL's PKCS5_PBKDF2_HMAC for the actual hashing.
 *
 * @param password Plain text password to hash
 * @param hashed_out Buffer for hashed password string (must be >= PASSWORD_STORAGE_MAX_LENGTH)
 * @return 0 on success, -1 on error
 *
 * Output format: $pbkdf2-sha256$100000$salt_base64$hash_base64
 *
 * Example usage:
 *   char hashed[PASSWORD_STORAGE_MAX_LENGTH];
 *   if (hash_password("mypassword", hashed) == 0) {
 *       printf("Hashed: %s\n", hashed);
 *   }
 */
int hash_password(const char *password, char *hashed_out);

/**
 * Verify a password against its hash
 *
 * Uses constant-time comparison to prevent timing attacks.
 * Extracts salt and iterations from stored hash and re-hashes
 * the provided password for comparison.
 *
 * @param password Plain text password to verify
 * @param hashed_password Stored hash string from hash_password()
 * @return 1 if password matches, 0 if not, -1 on error
 *
 * Example usage:
 *   if (verify_password("mypassword", stored_hash) == 1) {
 *       printf("Password correct\n");
 *   }
 */
int verify_password(const char *password, const char *hashed_password);

/**
 * Validate password strength
 *
 * Checks password against basic security requirements:
 * - Length between PASSWORD_MIN_LENGTH and PASSWORD_MAX_LENGTH
 * - Contains at least one uppercase letter
 * - Contains at least one lowercase letter
 * - Contains at least one digit
 * - No common weak passwords
 *
 * @param password Password to validate
 * @return 1 if strong enough, 0 if weak
 *
 * Example usage:
 *   if (is_password_strong("MySecurePass123")) {
 *       printf("Password meets requirements\n");
 *   }
 */
int is_password_strong(const char *password);

/**
 * Parse stored password hash
 *
 * Extracts components from stored hash string for verification.
 *
 * @param hashed_password Complete hash string
 * @param iterations_out Extracted iteration count
 * @param salt_out Buffer for extracted salt (must be >= PASSWORD_SALT_LENGTH)
 * @param hash_out Buffer for extracted hash (must be >= PASSWORD_HASH_LENGTH)
 * @return 0 on success, -1 on parse error
 */
int parse_password_hash(const char *hashed_password,
                       int *iterations_out,
                       unsigned char *salt_out,
                       unsigned char *hash_out);

/**
 * Generate cryptographically secure random salt
 *
 * Uses BoringSSL's RAND_bytes for secure random generation.
 *
 * @param salt_out Buffer for salt (must be >= PASSWORD_SALT_LENGTH)
 * @return 0 on success, -1 on error
 */
int generate_salt(unsigned char *salt_out);

/**
 * Constant-time memory comparison
 *
 * Compares two memory regions in constant time to prevent
 * timing attacks during password verification.
 *
 * @param a First memory region
 * @param b Second memory region
 * @param len Length to compare
 * @return 1 if equal, 0 if different
 */
int constant_time_compare(const unsigned char *a, const unsigned char *b, size_t len);

/**
 * Get estimated hashing time
 *
 * Measures the time taken for one PBKDF2 operation with current
 * iteration count. Useful for tuning performance.
 *
 * @return Time in milliseconds, -1 on error
 */
int get_hashing_time_ms(void);

#endif // PASSWORD_H