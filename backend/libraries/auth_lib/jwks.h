#ifndef JWKS_H
#define JWKS_H

#include "rsa_keys.h"
#include <stddef.h>

/**
 * JSON Web Key Set (JWKS) Implementation
 *
 * Generates JWKS (JSON Web Key Set) format responses for the
 * /.well-known/jwks.json endpoint. This allows other services
 * to validate JWTs independently using public keys.
 *
 * JWKS Format (RFC 7517):
 * {
 *   "keys": [
 *     {
 *       "kty": "RSA",
 *       "kid": "key-id",
 *       "use": "sig",
 *       "alg": "RS256",
 *       "n": "base64url-encoded-modulus",
 *       "e": "base64url-encoded-exponent"
 *     }
 *   ]
 * }
 */

// JWKS configuration
#define JWKS_MAX_KEYS 10           // Maximum keys in JWKS
#define JWKS_MAX_SIZE 4096         // Maximum JWKS JSON size
#define JWK_COMPONENT_MAX_SIZE 512 // Maximum size for n/e components

/**
 * Individual JWK (JSON Web Key) structure
 */
typedef struct {
    char kty[8];                               // Key type (always "RSA")
    char kid[33];                              // Key ID
    char use[8];                               // Key use (always "sig" for signature)
    char alg[8];                               // Algorithm (always "RS256")
    char n[JWK_COMPONENT_MAX_SIZE];           // Modulus (base64url encoded)
    char e[JWK_COMPONENT_MAX_SIZE];           // Exponent (base64url encoded)
    int is_active;                             // 1 if key is active, 0 if not
} jwk_t;

/**
 * JWKS (JSON Web Key Set) structure
 */
typedef struct {
    jwk_t keys[JWKS_MAX_KEYS];                // Array of JWK keys
    int key_count;                             // Number of active keys
} jwks_t;

/**
 * Initialize JWKS structure
 *
 * Sets default values and clears all keys
 *
 * @param jwks JWKS structure to initialize
 */
void jwks_init(jwks_t *jwks);

/**
 * Add RSA keypair to JWKS
 *
 * Converts an RSA keypair to JWK format and adds it to the JWKS.
 * Extracts the public key components (n, e) for inclusion.
 *
 * @param jwks JWKS structure to add key to
 * @param keypair RSA keypair to add
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   jwks_t jwks;
 *   jwks_init(&jwks);
 *   if (jwks_add_key(&jwks, &keypair) == 0) {
 *       printf("Added key to JWKS\n");
 *   }
 */
int jwks_add_key(jwks_t *jwks, const rsa_keypair_t *keypair);

/**
 * Generate JWKS JSON response
 *
 * Creates the complete JWKS JSON response suitable for
 * the /.well-known/jwks.json endpoint.
 *
 * @param jwks JWKS structure containing keys
 * @param json_out Buffer for JSON output
 * @param json_len Size of output buffer
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   char jwks_json[JWKS_MAX_SIZE];
 *   if (jwks_generate_json(&jwks, jwks_json, sizeof(jwks_json)) == 0) {
 *       printf("JWKS: %s\n", jwks_json);
 *   }
 */
int jwks_generate_json(const jwks_t *jwks, char *json_out, size_t json_len);

/**
 * Create JWK from RSA keypair
 *
 * Converts a single RSA keypair to JWK format
 *
 * @param keypair RSA keypair to convert
 * @param jwk_out JWK structure to populate
 * @return 0 on success, -1 on error
 */
int jwks_create_jwk(const rsa_keypair_t *keypair, jwk_t *jwk_out);

/**
 * Find key by ID in JWKS
 *
 * Searches for a specific key ID in the JWKS
 *
 * @param jwks JWKS structure to search
 * @param kid Key ID to find
 * @return Pointer to JWK if found, NULL if not found
 */
jwk_t *jwks_find_key(const jwks_t *jwks, const char *kid);

/**
 * Remove key from JWKS
 *
 * Removes a key with the specified ID from the JWKS.
 * Used during key rotation.
 *
 * @param jwks JWKS structure to modify
 * @param kid Key ID to remove
 * @return 0 on success, -1 if key not found
 */
int jwks_remove_key(jwks_t *jwks, const char *kid);

/**
 * Validate JWK structure
 *
 * Checks that a JWK has all required fields properly set
 *
 * @param jwk JWK to validate
 * @return 1 if valid, 0 if invalid
 */
int jwks_validate_jwk(const jwk_t *jwk);

/**
 * Generate HTTP response for JWKS endpoint
 *
 * Creates complete HTTP response including headers for
 * the JWKS endpoint
 *
 * @param jwks JWKS structure containing keys
 * @param response_out Buffer for HTTP response
 * @param response_len Size of response buffer
 * @return 0 on success, -1 on error
 *
 * Example output:
 *   HTTP/1.1 200 OK
 *   Content-Type: application/json
 *   Cache-Control: public, max-age=3600
 *   Content-Length: 234
 *
 *   {"keys":[...]}
 */
int jwks_generate_http_response(const jwks_t *jwks,
                               char *response_out,
                               size_t response_len);

/**
 * Load JWKS from multiple keypairs
 *
 * Convenience function to create JWKS from an array of keypairs.
 * Useful for key rotation scenarios.
 *
 * @param jwks JWKS structure to populate
 * @param keypairs Array of RSA keypairs
 * @param num_keys Number of keypairs in array
 * @return 0 on success, -1 on error
 */
int jwks_load_from_keypairs(jwks_t *jwks,
                           const rsa_keypair_t *keypairs,
                           int num_keys);

/**
 * Check if JWKS has any active keys
 *
 * @param jwks JWKS structure to check
 * @return 1 if has active keys, 0 if empty
 */
int jwks_has_active_keys(const jwks_t *jwks);

/**
 * Get primary (first) active key
 *
 * Returns the first active key in the JWKS, which is typically
 * used for signing new tokens.
 *
 * @param jwks JWKS structure to search
 * @return Pointer to primary JWK, NULL if no active keys
 */
jwk_t *jwks_get_primary_key(const jwks_t *jwks);

/**
 * Format single JWK as JSON
 *
 * Converts a single JWK to JSON format for inclusion in JWKS
 *
 * @param jwk JWK to format
 * @param json_out Buffer for JSON output
 * @param json_len Size of output buffer
 * @return 0 on success, -1 on error
 */
int jwks_format_jwk_json(const jwk_t *jwk, char *json_out, size_t json_len);

#endif // JWKS_H