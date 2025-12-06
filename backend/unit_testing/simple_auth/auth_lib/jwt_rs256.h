#ifndef JWT_RS256_H
#define JWT_RS256_H

#include "rsa_keys.h"
#include <stddef.h>
#include <time.h>

/**
 * JWT RS256 Implementation
 *
 * JSON Web Token implementation using RSA SHA-256 signatures (RS256).
 * Supports standard JWT claims and custom claims for authentication.
 *
 * JWT Structure: header.payload.signature (all base64url encoded)
 * Algorithm: RS256 (RSA signature with SHA-256)
 *
 * Standard Claims Supported:
 * - iss (Issuer)
 * - sub (Subject - User ID)
 * - aud (Audience)
 * - exp (Expiration Time)
 * - iat (Issued At)
 * - jti (JWT ID for blacklisting)
 *
 * Custom Claims:
 * - email (User email address)
 * - email_verified (Email verification status)
 */

// JWT configuration
#define JWT_MAX_LENGTH 2048        // Maximum JWT token length
#define JWT_CLAIM_MAX_LENGTH 256   // Maximum individual claim length
#define JWT_ID_LENGTH 32          // JWT ID length (for blacklisting)

// Default token lifetimes (in seconds)
// #define JWT_ACCESS_TOKEN_LIFETIME (60 * 60)      // 1 hour
// #define JWT_REFRESH_TOKEN_LIFETIME (30 * 24 * 60 * 60) // 30 days

// Testing purposes (token, refresh, session lifecycle testing)
#define JWT_ACCESS_TOKEN_LIFETIME 10             // 10 seconds
#define JWT_REFRESH_TOKEN_LIFETIME 100           // 100 seconds

/**
 * JWT claims structure
 *
 * Contains all the claims that will be included in the JWT payload
 */
typedef struct {
    // Standard claims
    char issuer[JWT_CLAIM_MAX_LENGTH + 1];     // iss - token issuer
    char subject[JWT_CLAIM_MAX_LENGTH + 1];    // sub - user ID
    char audience[JWT_CLAIM_MAX_LENGTH + 1];   // aud - intended audience
    time_t issued_at;                          // iat - issued at timestamp
    time_t expires_at;                         // exp - expiration timestamp
    char jwt_id[JWT_ID_LENGTH + 1];           // jti - unique token ID

    // Custom claims for authentication
    char email[JWT_CLAIM_MAX_LENGTH + 1];      // User email
    int email_verified;                        // Email verification status (0/1)
    char session_id[64 + 1];                   // Session ID (64 chars + null terminator)

    // Token type
    char token_type[32];                       // "access" or "refresh"
} jwt_claims_t;

/**
 * JWT validation result
 *
 * Contains the result of JWT validation and extracted claims
 */
typedef struct {
    int is_valid;                              // 1 if valid, 0 if invalid
    int is_expired;                            // 1 if expired, 0 if not
    jwt_claims_t claims;                       // Extracted claims
    char error_message[256];                   // Error description if invalid
} jwt_validation_result_t;

/**
 * Initialize JWT claims structure
 *
 * Sets default values and clears all fields
 *
 * @param claims Claims structure to initialize
 */
void jwt_init_claims(jwt_claims_t *claims);

/**
 * Create a JWT access token
 *
 * Generates a signed JWT with the provided claims using RS256 algorithm.
 * Access tokens have short lifetime and contain user authentication data.
 *
 * @param claims Claims to include in the token
 * @param keypair RSA keypair for signing
 * @param token_out Buffer for generated token (must be >= JWT_MAX_LENGTH)
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   jwt_claims_t claims;
 *   jwt_init_claims(&claims);
 *   strcpy(claims.subject, user_id);
 *   strcpy(claims.email, user_email);
 *   char token[JWT_MAX_LENGTH];
 *   if (jwt_create_access_token(&claims, &keypair, token) == 0) {
 *       printf("Access token: %s\n", token);
 *   }
 */
int jwt_create_access_token(jwt_claims_t *claims,
                           const rsa_keypair_t *keypair,
                           char *token_out);

/**
 * Create a JWT refresh token
 *
 * Generates a signed JWT for token refresh purposes.
 * Refresh tokens have longer lifetime and minimal claims.
 *
 * @param claims Claims to include in the token
 * @param keypair RSA keypair for signing
 * @param token_out Buffer for generated token (must be >= JWT_MAX_LENGTH)
 * @return 0 on success, -1 on error
 */
int jwt_create_refresh_token(jwt_claims_t *claims,
                            const rsa_keypair_t *keypair,
                            char *token_out);

/**
 * Validate and parse JWT token
 *
 * Verifies the JWT signature and extracts claims.
 * Checks expiration, signature validity, and claim format.
 *
 * @param token JWT token string
 * @param keypair RSA keypair for verification
 * @param result Validation result structure to populate
 * @return 0 on success (check result->is_valid), -1 on parse error
 *
 * Example usage:
 *   jwt_validation_result_t result;
 *   if (jwt_validate_token(token, &keypair, &result) == 0) {
 *       if (result.is_valid && !result.is_expired) {
 *           printf("Valid token for user: %s\n", result.claims.subject);
 *       }
 *   }
 */
int jwt_validate_token(const char *token,
                      const rsa_keypair_t *keypair,
                      jwt_validation_result_t *result);

/**
 * Extract header from JWT without validation
 *
 * Decodes the JWT header to extract algorithm and key ID information.
 * Useful for routing to correct validation key.
 *
 * @param token JWT token string
 * @param alg_out Buffer for algorithm (e.g., "RS256")
 * @param alg_len Size of algorithm buffer
 * @param kid_out Buffer for key ID (if present)
 * @param kid_len Size of key ID buffer
 * @return 0 on success, -1 on error
 */
int jwt_extract_header(const char *token,
                      char *alg_out, size_t alg_len,
                      char *kid_out, size_t kid_len);

/**
 * Generate unique JWT ID
 *
 * Creates a unique identifier for the JWT token.
 * Used for blacklisting and tracking purposes.
 *
 * @param jti_out Buffer for JWT ID (must be >= JWT_ID_LENGTH + 1)
 * @return 0 on success, -1 on error
 */
int jwt_generate_id(char *jti_out);

/**
 * Set standard claims for user authentication
 *
 * Convenience function to set common claims for user tokens
 *
 * @param claims Claims structure to populate
 * @param user_id User identifier (subject)
 * @param email User email address
 * @param email_verified Email verification status
 * @param issuer Token issuer
 * @param audience Token audience
 * @param lifetime_seconds Token lifetime in seconds
 * @return 0 on success, -1 on error
 */
int jwt_set_user_claims(jwt_claims_t *claims,
                       const char *user_id,
                       const char *email,
                       int email_verified,
                       const char *issuer,
                       const char *audience,
                       int lifetime_seconds);

/**
 * Check if token is expired
 *
 * Compares token expiration time with current time
 *
 * @param exp_timestamp Expiration timestamp from token
 * @return 1 if expired, 0 if still valid
 */
int jwt_is_expired(time_t exp_timestamp);

/**
 * Parse JWT components
 *
 * Splits JWT into header, payload, and signature components
 *
 * @param token Complete JWT string
 * @param header_out Buffer for base64url header
 * @param payload_out Buffer for base64url payload
 * @param signature_out Buffer for base64url signature
 * @param component_len Size of each component buffer
 * @return 0 on success, -1 on error
 */
int jwt_parse_components(const char *token,
                        char *header_out,
                        char *payload_out,
                        char *signature_out,
                        size_t component_len);

/**
 * Create JWT signature
 *
 * Signs the header.payload string using RS256
 *
 * @param signing_input Base64url encoded "header.payload"
 * @param keypair RSA keypair for signing
 * @param signature_out Buffer for base64url signature
 * @param sig_len Size of signature buffer
 * @return 0 on success, -1 on error
 */
int jwt_create_signature(const char *signing_input,
                        const rsa_keypair_t *keypair,
                        char *signature_out,
                        size_t sig_len);

/**
 * Verify JWT signature
 *
 * Verifies the signature against header.payload using public key
 *
 * @param signing_input Base64url encoded "header.payload"
 * @param signature Base64url encoded signature
 * @param keypair RSA keypair for verification
 * @return 1 if valid, 0 if invalid, -1 on error
 */
int jwt_verify_signature(const char *signing_input,
                        const char *signature,
                        const rsa_keypair_t *keypair);

/**
 * Format claims as JSON payload
 *
 * Converts claims structure to JSON string for JWT payload
 *
 * @param claims Claims to format
 * @param json_out Buffer for JSON string
 * @param json_len Size of JSON buffer
 * @return 0 on success, -1 on error
 */
int jwt_format_claims_json(const jwt_claims_t *claims,
                          char *json_out,
                          size_t json_len);

/**
 * Parse claims from JSON payload
 *
 * Extracts claims from JWT payload JSON string
 *
 * @param json JSON payload string
 * @param claims Claims structure to populate
 * @return 0 on success, -1 on error
 */
int jwt_parse_claims_json(const char *json, jwt_claims_t *claims);

#endif // JWT_RS256_H