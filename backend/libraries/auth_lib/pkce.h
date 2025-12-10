#ifndef PKCE_H
#define PKCE_H

#include <stddef.h>

/**
 * PKCE (Proof Key for Code Exchange) Implementation
 *
 * RFC 7636: Proof Key for Code Exchange by OAuth Public Clients
 *
 * This module implements PKCE validation as required by OAuth 2.1.
 * PKCE prevents authorization code interception attacks by requiring
 * clients to prove they initiated the authorization request.
 *
 * The flow works as follows:
 * 1. Client generates code_verifier (cryptographically random string)
 * 2. Client creates code_challenge = SHA256(code_verifier)
 * 3. Client sends code_challenge to authorization server
 * 4. Server stores code_challenge and returns authorization code
 * 5. Client sends authorization code + original code_verifier
 * 6. Server validates SHA256(code_verifier) == stored_code_challenge
 *
 * Only S256 (SHA256) method is supported as per OAuth 2.1 requirements.
 */

// PKCE Constants (RFC 7636)
#define PKCE_CODE_VERIFIER_MIN_LENGTH 43       // Minimum code_verifier length
#define PKCE_CODE_VERIFIER_MAX_LENGTH 128      // Maximum code_verifier length
#define PKCE_CODE_CHALLENGE_LENGTH 43          // SHA256 base64url length
#define PKCE_METHOD_S256 "S256"                // Only supported method

// Character set for code_verifier (RFC 7636 Section 4.1)
#define PKCE_VERIFIER_CHARSET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~"

/**
 * Generate PKCE code verifier
 *
 * Creates a cryptographically secure random string suitable for use
 * as a PKCE code verifier. Uses only characters from the unreserved
 * character set defined in RFC 3986.
 *
 * @param verifier_out Buffer for code verifier (must be >= 129 bytes)
 * @param length Desired length (must be 43-128)
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   char verifier[129];
 *   if (pkce_generate_code_verifier(verifier, 64) == 0) {
 *       printf("Code verifier: %s\n", verifier);
 *   }
 */
int pkce_generate_code_verifier(char *verifier_out, size_t length);

/**
 * Generate PKCE code challenge from verifier
 *
 * Creates a code challenge by computing SHA256(code_verifier) and
 * encoding the result as base64url without padding.
 *
 * @param verifier Input code verifier string
 * @param challenge_out Buffer for code challenge (must be >= 44 bytes)
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   char verifier[129];
 *   char challenge[44];
 *   pkce_generate_code_verifier(verifier, 64);
 *   if (pkce_generate_code_challenge(verifier, challenge) == 0) {
 *       printf("Code challenge: %s\n", challenge);
 *   }
 */
int pkce_generate_code_challenge(const char *verifier, char *challenge_out);

/**
 * Validate PKCE code verifier against challenge
 *
 * Verifies that the provided code_verifier, when hashed with SHA256
 * and base64url encoded, matches the stored code_challenge.
 *
 * @param verifier Code verifier provided by client
 * @param stored_challenge Code challenge stored during authorization
 * @return 1 if valid, 0 if invalid
 *
 * Example usage:
 *   char verifier[] = "abc123def456...";
 *   char challenge[] = "xyz789uvw012...";
 *   if (pkce_validate_code_verifier(verifier, challenge)) {
 *       printf("PKCE validation successful\n");
 *   } else {
 *       printf("PKCE validation failed\n");
 *   }
 */
int pkce_validate_code_verifier(const char *verifier, const char *stored_challenge);

/**
 * Validate code verifier format
 *
 * Checks that code_verifier meets RFC 7636 requirements:
 * - Length between 43-128 characters
 * - Contains only unreserved characters: [A-Za-z0-9-._~]
 *
 * @param verifier Code verifier to validate
 * @return 1 if valid format, 0 if invalid
 *
 * Example usage:
 *   if (pkce_validate_verifier_format("abc123-._~")) {
 *       printf("Valid code verifier format\n");
 *   }
 */
int pkce_validate_verifier_format(const char *verifier);

/**
 * Validate code challenge format
 *
 * Checks that code_challenge meets requirements:
 * - Exactly 43 characters (SHA256 base64url)
 * - Contains only base64url characters: [A-Za-z0-9-_]
 * - No padding (base64url without padding)
 *
 * @param challenge Code challenge to validate
 * @return 1 if valid format, 0 if invalid
 *
 * Example usage:
 *   if (pkce_validate_challenge_format("abc123def456xyz...")) {
 *       printf("Valid code challenge format\n");
 *   }
 */
int pkce_validate_challenge_format(const char *challenge);

/**
 * Validate PKCE method
 *
 * Ensures that only "S256" method is used as required by OAuth 2.1.
 * The "plain" method is explicitly rejected for security.
 *
 * @param method Code challenge method
 * @return 1 if valid ("S256"), 0 if invalid
 *
 * Example usage:
 *   if (pkce_validate_method("S256")) {
 *       printf("Supported PKCE method\n");
 *   }
 */
int pkce_validate_method(const char *method);

/**
 * Generate cryptographically secure random bytes
 *
 * Internal utility function for generating random data.
 * Uses platform-appropriate CSPRNG.
 *
 * @param buffer Output buffer for random bytes
 * @param length Number of bytes to generate
 * @return 0 on success, -1 on error
 */
int pkce_generate_random_bytes(unsigned char *buffer, size_t length);

/**
 * Timing-safe string comparison
 *
 * Compares two strings in constant time to prevent timing attacks
 * during PKCE validation. Critical for security.
 *
 * @param a First string
 * @param b Second string
 * @return 1 if strings are equal, 0 if different
 */
int pkce_constant_time_compare(const char *a, const char *b);

#endif // PKCE_H