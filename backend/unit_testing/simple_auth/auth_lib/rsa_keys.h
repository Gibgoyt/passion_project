#ifndef RSA_KEYS_H
#define RSA_KEYS_H

#include <stddef.h>

// OpenSSL includes for RSA and EVP_PKEY types
#include <openssl/rsa.h>
#include <openssl/evp.h>

/**
 * RSA Key Management for JWT RS256 Signatures
 *
 * Handles RSA key pair generation, loading, saving, and management
 * for JWT signing and verification. Uses OpenSSL for cryptographic
 * operations.
 *
 * Key Features:
 * - 2048-bit RSA key pairs (secure for JWT use)
 * - PEM format storage (standard)
 * - Key validation and verification
 * - Public key extraction from private key
 * - Support for key rotation (multiple active keys)
 */

// RSA key parameters
#define RSA_KEY_BITS 2048      // Standard 2048-bit keys
#define RSA_PUBLIC_EXPONENT 65537  // Standard public exponent (0x10001)

// File paths for key storage
#define PRIVATE_KEY_PATH "keys/private_key.pem"
#define PUBLIC_KEY_PATH "keys/public_key.pem"

// Key identifier for JWKS
#define KEY_ID_MAX_LENGTH 32

/**
 * RSA key pair structure
 *
 * Holds both private and public keys with metadata
 */
typedef struct {
    EVP_PKEY *private_key;     // Private key (for signing)
    EVP_PKEY *public_key;      // Public key (for verification)
    char key_id[KEY_ID_MAX_LENGTH + 1];  // Key identifier for JWKS
    int is_loaded;             // 1 if keys are loaded, 0 otherwise
} rsa_keypair_t;

/**
 * Generate a new RSA key pair
 *
 * Creates a new 2048-bit RSA key pair suitable for JWT RS256 signatures.
 * Keys are generated in memory and must be saved using rsa_save_keys().
 *
 * @param keypair Keypair structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   rsa_keypair_t keys;
 *   if (rsa_generate_keypair(&keys) == 0) {
 *       printf("Generated RSA key pair\n");
 *   }
 */
int rsa_generate_keypair(rsa_keypair_t *keypair);

/**
 * Load RSA keys from PEM files
 *
 * Loads private and public keys from the standard file paths.
 * If only private key exists, extracts public key from it.
 *
 * @param keypair Keypair structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   rsa_keypair_t keys;
 *   if (rsa_load_keys(&keys) == 0) {
 *       printf("Loaded existing keys\n");
 *   }
 */
int rsa_load_keys(rsa_keypair_t *keypair);

/**
 * Save RSA keys to PEM files
 *
 * Saves the private and public keys to standard file paths with
 * appropriate file permissions (600 for private key, 644 for public).
 *
 * @param keypair Keypair containing keys to save
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   if (rsa_save_keys(&keys) == 0) {
 *       printf("Keys saved successfully\n");
 *   }
 */
int rsa_save_keys(const rsa_keypair_t *keypair);

/**
 * Get or generate RSA keys
 *
 * Convenience function that tries to load existing keys first,
 * and generates new ones if loading fails. Automatically saves
 * newly generated keys.
 *
 * @param keypair Keypair structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   rsa_keypair_t keys;
 *   if (rsa_get_or_generate_keys(&keys) == 0) {
 *       printf("Keys ready for use\n");
 *   }
 */
int rsa_get_or_generate_keys(rsa_keypair_t *keypair);

/**
 * Validate RSA keypair
 *
 * Verifies that the keypair is valid and that private/public keys match.
 * Performs a sign/verify test to ensure correctness.
 *
 * @param keypair Keypair to validate
 * @return 1 if valid, 0 if invalid, -1 on error
 *
 * Example usage:
 *   if (rsa_validate_keypair(&keys) == 1) {
 *       printf("Keypair is valid\n");
 *   }
 */
int rsa_validate_keypair(const rsa_keypair_t *keypair);

/**
 * Extract public key from private key
 *
 * Derives the public key from an existing private key.
 * Useful when only the private key is available.
 *
 * @param private_key Private key
 * @param public_key_out Pointer to store extracted public key
 * @return 0 on success, -1 on error
 */
int rsa_extract_public_key(EVP_PKEY *private_key, EVP_PKEY **public_key_out);

/**
 * Get RSA key size in bits
 *
 * Returns the key size for the given RSA key.
 *
 * @param key RSA key (private or public)
 * @return Key size in bits, -1 on error
 */
int rsa_get_key_bits(EVP_PKEY *key);

/**
 * Generate key identifier for JWKS
 *
 * Creates a unique identifier for the key based on its public key hash.
 * Used in JWT headers and JWKS responses.
 *
 * @param keypair Keypair to generate ID for
 * @return 0 on success, -1 on error
 */
int rsa_generate_key_id(rsa_keypair_t *keypair);

/**
 * Export public key to PEM format (string)
 *
 * Converts the public key to PEM format string for sharing or transmission.
 * Used by JWKS endpoint generation.
 *
 * @param public_key Public key to export
 * @param pem_out Buffer for PEM string
 * @param pem_len Size of output buffer
 * @return Length of PEM string on success, -1 on error
 */
int rsa_export_public_key_pem(EVP_PKEY *public_key, char *pem_out, size_t pem_len);

/**
 * Get RSA public key components (n, e)
 *
 * Extracts the modulus (n) and exponent (e) from the public key
 * for use in JWKS format. Returns base64url encoded values.
 *
 * @param public_key Public key to extract from
 * @param n_out Buffer for base64url encoded modulus
 * @param n_len Size of modulus buffer
 * @param e_out Buffer for base64url encoded exponent
 * @param e_len Size of exponent buffer
 * @return 0 on success, -1 on error
 */
int rsa_get_public_key_components(EVP_PKEY *public_key,
                                 char *n_out, size_t n_len,
                                 char *e_out, size_t e_len);

/**
 * Free RSA keypair resources
 *
 * Cleans up memory allocated for the keypair.
 * Should be called when done using the keypair.
 *
 * @param keypair Keypair to free
 */
void rsa_free_keypair(rsa_keypair_t *keypair);

/**
 * Initialize RSA keypair structure
 *
 * Initializes a keypair structure to safe default values.
 * Should be called before using a keypair.
 *
 * @param keypair Keypair to initialize
 */
void rsa_init_keypair(rsa_keypair_t *keypair);

/**
 * Check if keys directory exists and create if needed
 *
 * Ensures the keys directory exists with appropriate permissions.
 *
 * @return 0 on success, -1 on error
 */
int rsa_ensure_keys_directory(void);

#endif // RSA_KEYS_H