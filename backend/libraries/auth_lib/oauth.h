#ifndef OAUTH_H
#define OAUTH_H

#include "auth.h"
#include <stddef.h>
#include <time.h>

/**
 * OAuth 2.1 with PKCE Support
 *
 * This module implements OAuth 2.1 authorization code flow with PKCE
 * (Proof Key for Code Exchange) as specified in RFC 7636 and RFC 8252.
 *
 * The implementation follows a pure REST API approach where:
 * - Backend validates PKCE parameters and issues tokens
 * - Frontend handles redirects and user experience
 * - No server-side redirects are performed
 *
 * Flow Overview:
 * 1. Client initiates OAuth flow with /oauth/authorize/init
 * 2. User authenticates via /oauth/authorize/complete
 * 3. Client exchanges authorization code via /oauth/token
 *
 * All PKCE validation is handled server-side for security.
 */

// Configuration constants
#define OAUTH_CLIENT_ID_MAX_LENGTH 63
#define OAUTH_CLIENT_NAME_MAX_LENGTH 127
#define OAUTH_REDIRECT_URI_MAX_LENGTH 511
#define OAUTH_CODE_CHALLENGE_LENGTH 43          // Base64url(SHA256) = 43 chars
#define OAUTH_CODE_VERIFIER_MAX_LENGTH 127      // RFC 7636: 43-128 chars
#define OAUTH_STATE_MAX_LENGTH 63
#define OAUTH_AUTHORIZATION_CODE_LENGTH 43      // Base64url random = ~43 chars (may vary slightly)
#define OAUTH_AUTHORIZATION_CODE_MIN_LENGTH 40  // Minimum acceptable length
#define OAUTH_AUTHORIZATION_CODE_MAX_LENGTH 44  // Maximum acceptable length
#define OAUTH_DEFAULT_CODE_LIFETIME 600         // 10 minutes

/**
 * OAuth initialization request structure
 */
typedef struct {
    char client_id[OAUTH_CLIENT_ID_MAX_LENGTH + 1];
    char redirect_uri[OAUTH_REDIRECT_URI_MAX_LENGTH + 1];
    char code_challenge[OAUTH_CODE_CHALLENGE_LENGTH + 1];
    char code_challenge_method[8];               // "S256" only supported
    char state[OAUTH_STATE_MAX_LENGTH + 1];
} oauth_init_request_t;

/**
 * OAuth initialization response structure
 */
typedef struct {
    char session_id[65];                        // OAuth session identifier
    char authorization_url[512];                // URL for authorization page
    int expires_in;                             // Session expiration time
} oauth_init_response_t;

/**
 * OAuth authorization completion request structure
 */
typedef struct {
    char session_id[65];                        // OAuth session ID
    char email[256];                            // User email
    char password[256];                         // User password
    int consent_granted;                        // 1=consent given, 0=denied
} oauth_complete_request_t;

/**
 * OAuth authorization completion response structure
 */
typedef struct {
    char authorization_code[OAUTH_AUTHORIZATION_CODE_LENGTH + 1];
    char redirect_uri[OAUTH_REDIRECT_URI_MAX_LENGTH + 1];
    char state[OAUTH_STATE_MAX_LENGTH + 1];
} oauth_complete_response_t;

/**
 * OAuth token exchange request structure
 */
typedef struct {
    char grant_type[32];                        // "authorization_code"
    char code[OAUTH_AUTHORIZATION_CODE_LENGTH + 1];
    char code_verifier[OAUTH_CODE_VERIFIER_MAX_LENGTH + 1];
    char client_id[OAUTH_CLIENT_ID_MAX_LENGTH + 1];
    char redirect_uri[OAUTH_REDIRECT_URI_MAX_LENGTH + 1];
} oauth_token_request_t;

/**
 * OAuth token response structure
 */
typedef struct {
    char access_token[JWT_MAX_LENGTH];
    char refresh_token[JWT_MAX_LENGTH];
    char token_type[16];                        // "Bearer"
    int expires_in;                             // Access token lifetime
} oauth_token_response_t;

/**
 * OAuth result structure
 */
typedef struct {
    int success;                                // 1 if successful, 0 if failed
    char error_code[32];                        // OAuth error code (invalid_request, etc.)
    char error_description[256];                // Human-readable error description
    union {
        oauth_init_response_t init;
        oauth_complete_response_t complete;
        oauth_token_response_t token;
    } data;
} oauth_result_t;

/**
 * Initialize OAuth flow
 *
 * Creates a new OAuth session with PKCE parameters. The client provides
 * the code_challenge, and the server stores it for later validation.
 *
 * @param ctx Authentication context
 * @param request OAuth initialization request
 * @param user_agent Client user agent string (optional)
 * @param ip_address Client IP address (optional)
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   oauth_init_request_t request = {
 *       .client_id = "astro-test-app",
 *       .redirect_uri = "http://localhost:3000/callback",
 *       .code_challenge = "abc123def456...",
 *       .code_challenge_method = "S256",
 *       .state = "csrf_token_xyz"
 *   };
 *   oauth_result_t result;
 *   if (oauth_authorize_init(&auth_ctx, &request, "MyApp/1.0",
 *                            "192.168.1.100", &result) == 0) {
 *       if (result.success) {
 *           printf("Session ID: %s\n", result.data.init.session_id);
 *           printf("Authorization URL: %s\n", result.data.init.authorization_url);
 *       }
 *   }
 */
int oauth_authorize_init(auth_context_t *ctx,
                        const oauth_init_request_t *request,
                        const char *user_agent,
                        const char *ip_address,
                        oauth_result_t *result);

/**
 * Complete OAuth authorization
 *
 * Authenticates user and generates authorization code. This is where
 * the user provides their credentials and grants consent.
 *
 * @param ctx Authentication context
 * @param request OAuth completion request
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   oauth_complete_request_t request = {
 *       .session_id = "sess_abc123",
 *       .email = "user@example.com",
 *       .password = "userpassword",
 *       .consent_granted = 1
 *   };
 *   oauth_result_t result;
 *   if (oauth_authorize_complete(&auth_ctx, &request, &result) == 0) {
 *       if (result.success) {
 *           printf("Authorization code: %s\n", result.data.complete.authorization_code);
 *       }
 *   }
 */
int oauth_authorize_complete(auth_context_t *ctx,
                            const oauth_complete_request_t *request,
                            oauth_result_t *result);

/**
 * Exchange authorization code for tokens
 *
 * Validates PKCE code_verifier against stored code_challenge and
 * issues JWT access/refresh tokens. This completes the OAuth flow.
 *
 * @param ctx Authentication context
 * @param request Token exchange request
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   oauth_token_request_t request = {
 *       .grant_type = "authorization_code",
 *       .code = "auth_code_456",
 *       .code_verifier = "original_verifier_from_client",
 *       .client_id = "astro-test-app",
 *       .redirect_uri = "http://localhost:3000/callback"
 *   };
 *   oauth_result_t result;
 *   if (oauth_token_exchange(&auth_ctx, &request, &result) == 0) {
 *       if (result.success) {
 *           printf("Access token: %s\n", result.data.token.access_token);
 *           printf("Refresh token: %s\n", result.data.token.refresh_token);
 *       }
 *   }
 */
int oauth_token_exchange(auth_context_t *ctx,
                        const oauth_token_request_t *request,
                        oauth_result_t *result);

/**
 * Register OAuth client
 *
 * Adds a new OAuth client to the database. Required before clients
 * can initiate OAuth flows.
 *
 * @param ctx Authentication context
 * @param client_data Client registration data
 * @return 0 on success, -1 on error
 */
int oauth_register_client(auth_context_t *ctx, const client_data_t *client_data);

/**
 * Get OAuth client by ID
 *
 * Retrieves client configuration from database
 *
 * @param ctx Authentication context
 * @param client_id Client identifier
 * @param client_out Client data structure to populate
 * @return 0 on success, -1 on error
 */
int oauth_get_client(auth_context_t *ctx, const char *client_id, client_data_t *client_out);

/**
 * Delete OAuth client
 *
 * Removes client from database and invalidates all associated sessions
 *
 * @param ctx Authentication context
 * @param client_id Client identifier to delete
 * @return 0 on success, -1 on error
 */
int oauth_delete_client(auth_context_t *ctx, const char *client_id);

/**
 * Generate authorization code
 *
 * Creates a cryptographically secure authorization code
 *
 * @param code_out Buffer for authorization code (must be >= 44 bytes)
 * @return 0 on success, -1 on error
 */
int oauth_generate_authorization_code(char *code_out);

/**
 * Validate redirect URI
 *
 * Checks if provided redirect URI matches the registered URI for client
 *
 * @param provided_uri URI provided in request
 * @param registered_uri URI registered for client
 * @return 1 if valid, 0 if invalid
 */
int oauth_validate_redirect_uri(const char *provided_uri, const char *registered_uri);

/**
 * Database utility functions for OAuth clients
 */

/**
 * Store OAuth client in database
 */
int oauth_store_client(MDBX_env *env, const client_data_t *client);

/**
 * Load OAuth client from database
 */
int oauth_load_client(MDBX_env *env, const char *client_id, client_data_t *client_out);

/**
 * Delete OAuth client from database
 */
int oauth_delete_client_from_db(MDBX_env *env, const char *client_id);

#endif // OAUTH_H