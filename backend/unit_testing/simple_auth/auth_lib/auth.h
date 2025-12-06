#ifndef AUTH_H
#define AUTH_H

#include "rsa_keys.h"
#include "jwt_rs256.h"
#include "jwks.h"
#include "userid.h"
#include "email.h"
#include "password.h"
#include <stddef.h>
#include <mdbx.h>

/**
 * Main Authentication API
 *
 * High-level authentication system that provides complete user management,
 * JWT token handling, and session management using libmdbx for persistence.
 *
 * This module ties together all the individual components:
 * - User registration and management
 * - Password hashing and verification
 * - JWT token creation and validation
 * - Session management and blacklisting
 * - JWKS endpoint support
 *
 * Database schema uses 4 separate libmdbx databases:
 * 1. Users: userId -> user_data
 * 2. Email index: email -> userId (for fast login)
 * 3. Refresh tokens: tokenId -> token_data
 * 4. Blacklist: jwtId -> expiration (for revoked tokens)
 */

// Configuration
#define AUTH_MAX_CONCURRENT_SESSIONS 10   // Max sessions per user
#define AUTH_DEFAULT_ISSUER "simple-auth"
#define AUTH_DEFAULT_AUDIENCE "simple-auth-clients"

/**
 * Authentication system configuration
 */
typedef struct {
    char issuer[256];                     // JWT issuer
    char audience[256];                   // JWT audience
    int access_token_lifetime;            // Access token lifetime (seconds)
    int refresh_token_lifetime;           // Refresh token lifetime (seconds)
    int session_lifetime;                 // Session lifetime (seconds) - independent of token lifetimes
    char database_path[512];              // Database directory path
    int enable_refresh_token_rotation;    // 1 = rotate tokens (secure), 0 = reuse tokens
} auth_config_t;

/**
 * User data structure (stored in database)
 */
typedef struct {
    char user_id[FIREBASE_USERID_LENGTH + 1];  // Firebase-style user ID
    char email[MAX_EMAIL_LENGTH + 1];           // Email address (normalized)
    char password_hash[PASSWORD_STORAGE_MAX_LENGTH]; // PBKDF2 hash
    time_t created_at;                          // Registration timestamp
    time_t last_login;                          // Last login timestamp
    int email_verified;                         // Email verification status
    int is_active;                              // Account status
} user_data_t;

/**
 * Session data structure
 */
typedef struct {
    char session_id[65];                  // Unique session identifier (64 chars + null terminator)
    char user_id[FIREBASE_USERID_LENGTH + 1]; // User ID
    char refresh_token_id[JWT_ID_LENGTH + 1];  // Refresh token ID
    char access_token_id[JWT_ID_LENGTH + 1];   // Current access token ID
    time_t created_at;                    // Session creation time
    time_t last_used;                     // Last activity time
    time_t expires_at;                    // Session expiration
    char user_agent[256];                 // User agent string
    char ip_address[64];                  // Client IP address

    // OAuth 2.1 PKCE fields (for OAuth flows)
    char client_id[64];                   // OAuth client identifier
    char code_challenge[64];              // SHA256 base64url encoded challenge
    char code_verifier[128];              // Original verifier (server-stored for validation)
    char redirect_uri[512];               // Validated redirect URI
    char state[64];                       // CSRF protection state parameter
    char authorization_code[64];          // Generated authorization code
    int oauth_flow_active;                // 0=regular session, 1=OAuth flow in progress
    time_t code_expires_at;               // Authorization code expiration
    int code_used;                        // 1=code already exchanged, 0=unused
} session_data_t;

/**
 * OAuth client data structure
 */
typedef struct {
    char client_id[64];                   // OAuth client identifier
    char client_name[128];                // Human-readable client name
    char redirect_uri[512];               // Pre-registered redirect URI
    int client_type;                      // 0=confidential, 1=public (PKCE required)
    time_t created_at;                    // Client registration time
    int is_active;                        // 1=active, 0=disabled
} client_data_t;

/**
 * Authentication result structure
 */
typedef struct {
    int success;                          // 1 if successful, 0 if failed
    char error_message[256];              // Error description
    user_data_t user;                     // User data (if successful)
    session_data_t session;               // Session data (if successful)
    char access_token[JWT_MAX_LENGTH];    // JWT access token
    char refresh_token[JWT_MAX_LENGTH];   // JWT refresh token
} auth_result_t;

/**
 * Main authentication context
 */
typedef struct {
    auth_config_t config;                 // Configuration
    rsa_keypair_t keypair;               // RSA keys for JWT signing
    jwks_t jwks;                         // JWKS for public key distribution

    // Database environments
    MDBX_env *users_env;                 // Users database
    MDBX_env *email_index_env;           // Email -> userId index
    MDBX_env *sessions_env;              // Active sessions
    MDBX_env *blacklist_env;             // JWT blacklist
    MDBX_env *clients_env;               // OAuth clients

    int is_initialized;                   // 1 if initialized, 0 if not
} auth_context_t;

/**
 * Initialize authentication system
 *
 * Sets up databases, loads or generates RSA keys, and prepares
 * the authentication context for use.
 *
 * @param ctx Authentication context to initialize
 * @param config Configuration parameters (NULL for defaults)
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   auth_context_t auth_ctx;
 *   auth_config_t config = {
 *       .issuer = "my-app",
 *       .audience = "my-app-clients",
 *       .access_token_lifetime = 3600,
 *       .refresh_token_lifetime = 2592000,
 *       .database_path = "./data"
 *   };
 *   if (auth_initialize(&auth_ctx, &config) == 0) {
 *       printf("Authentication system ready\n");
 *   }
 */
int auth_initialize(auth_context_t *ctx, const auth_config_t *config);

/**
 * Cleanup authentication system
 *
 * Closes databases and frees resources
 *
 * @param ctx Authentication context to cleanup
 */
void auth_cleanup(auth_context_t *ctx);

/**
 * Register new user
 *
 * Creates a new user account with email and password validation.
 * Generates Firebase-style user ID and stores hashed password.
 *
 * @param ctx Authentication context
 * @param email User email address
 * @param password Plain text password
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   auth_result_t result;
 *   if (auth_register(&auth_ctx, "user@example.com", "SecurePass123", &result) == 0) {
 *       if (result.success) {
 *           printf("Registered user: %s\n", result.user.user_id);
 *       } else {
 *           printf("Registration failed: %s\n", result.error_message);
 *       }
 *   }
 */
int auth_register(auth_context_t *ctx,
                 const char *email,
                 const char *password,
                 auth_result_t *result);

/**
 * Authenticate user login
 *
 * Validates email/password and creates JWT tokens and session.
 * Supports multiple concurrent sessions per user.
 *
 * @param ctx Authentication context
 * @param email User email address
 * @param password Plain text password
 * @param user_agent Client user agent string (optional)
 * @param ip_address Client IP address (optional)
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 *
 * Example usage:
 *   auth_result_t result;
 *   if (auth_login(&auth_ctx, "user@example.com", "password",
 *                  "MyApp/1.0", "192.168.1.100", &result) == 0) {
 *       if (result.success) {
 *           printf("Access token: %s\n", result.access_token);
 *           printf("Refresh token: %s\n", result.refresh_token);
 *       }
 *   }
 */
int auth_login(auth_context_t *ctx,
              const char *email,
              const char *password,
              const char *user_agent,
              const char *ip_address,
              auth_result_t *result);

/**
 * Refresh access token
 *
 * Uses refresh token to generate a new access token.
 * Updates session activity timestamp.
 *
 * @param ctx Authentication context
 * @param refresh_token Current refresh token
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 */
int auth_refresh(auth_context_t *ctx,
                const char *refresh_token,
                auth_result_t *result);

/**
 * Recover session from expired refresh token
 *
 * Attempts to issue new tokens when refresh token is expired but
 * session is still valid. This provides session-based recovery
 * within the fixed 200-second session lifetime.
 *
 * @param ctx Authentication context
 * @param expired_refresh_token Expired refresh token
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 */
int auth_recover_session(auth_context_t *ctx,
                        const char *expired_refresh_token,
                        auth_result_t *result);

/**
 * Validate access token
 *
 * Verifies JWT signature, expiration, and checks blacklist.
 * Returns user information if token is valid.
 *
 * @param ctx Authentication context
 * @param access_token JWT access token
 * @param result Result structure to populate
 * @return 0 on success, -1 on error
 */
int auth_validate_token(auth_context_t *ctx,
                       const char *access_token,
                       auth_result_t *result);

/**
 * Logout and revoke tokens
 *
 * Adds access/refresh tokens to blacklist and removes session.
 * Can logout single session or all sessions for user.
 *
 * @param ctx Authentication context
 * @param access_token Current access token
 * @param logout_all 1 to logout all sessions, 0 for current only
 * @return 0 on success, -1 on error
 */
int auth_logout(auth_context_t *ctx,
               const char *access_token,
               int logout_all);

/**
 * Get JWKS for public key distribution
 *
 * Generates JWKS JSON response for /.well-known/jwks.json endpoint
 *
 * @param ctx Authentication context
 * @param jwks_json Buffer for JWKS JSON
 * @param json_len Size of JSON buffer
 * @return 0 on success, -1 on error
 */
int auth_get_jwks(auth_context_t *ctx, char *jwks_json, size_t json_len);

/**
 * Get user by ID
 *
 * Retrieves user data from database
 *
 * @param ctx Authentication context
 * @param user_id User ID to lookup
 * @param user_out User data structure to populate
 * @return 0 on success, -1 on error
 */
int auth_get_user(auth_context_t *ctx,
                 const char *user_id,
                 user_data_t *user_out);

/**
 * Update user email verification status
 *
 * Marks user email as verified or unverified
 *
 * @param ctx Authentication context
 * @param user_id User ID
 * @param verified 1 for verified, 0 for unverified
 * @return 0 on success, -1 on error
 */
int auth_set_email_verified(auth_context_t *ctx,
                           const char *user_id,
                           int verified);

/**
 * Change user password
 *
 * Updates user password with new hash, revokes all existing sessions
 *
 * @param ctx Authentication context
 * @param user_id User ID
 * @param current_password Current password (for verification)
 * @param new_password New password
 * @return 0 on success, -1 on error
 */
int auth_change_password(auth_context_t *ctx,
                        const char *user_id,
                        const char *current_password,
                        const char *new_password);

/**
 * Delete user account
 *
 * Removes user and all associated sessions, tokens
 *
 * @param ctx Authentication context
 * @param user_id User ID to delete
 * @return 0 on success, -1 on error
 */
int auth_delete_user(auth_context_t *ctx, const char *user_id);

/**
 * Get user sessions
 *
 * Lists all active sessions for a user
 *
 * @param ctx Authentication context
 * @param user_id User ID
 * @param sessions Array to store session data
 * @param max_sessions Maximum sessions to return
 * @return Number of sessions found, -1 on error
 */
int auth_get_user_sessions(auth_context_t *ctx,
                          const char *user_id,
                          session_data_t *sessions,
                          int max_sessions);

/**
 * Cleanup expired tokens and sessions
 *
 * Removes expired entries from blacklist and sessions
 * Should be called periodically for maintenance
 *
 * @param ctx Authentication context
 * @return Number of items cleaned up, -1 on error
 */
int auth_cleanup_expired(auth_context_t *ctx);

/**
 * Initialize default configuration
 *
 * Sets up default values for auth configuration
 *
 * @param config Configuration structure to initialize
 */
void auth_init_config(auth_config_t *config);

/**
 * Database utility functions
 */

/**
 * Open database environment
 *
 * @param path Database file path
 * @param env_out Database environment pointer
 * @return 0 on success, -1 on error
 */
int auth_open_database(const char *path, MDBX_env **env_out);

/**
 * Store user data in database
 */
int auth_store_user(MDBX_env *env, const user_data_t *user);

/**
 * Load user data from database
 */
int auth_load_user(MDBX_env *env, const char *user_id, user_data_t *user_out);

/**
 * Store email index mapping
 */
int auth_store_email_index(MDBX_env *env, const char *email, const char *user_id);

/**
 * Look up user ID by email
 */
int auth_lookup_user_by_email(MDBX_env *env, const char *email, char *user_id_out);

/**
 * Store session data
 */
int auth_store_session(MDBX_env *env, const session_data_t *session);

/**
 * Load session data
 */
int auth_load_session(MDBX_env *env, const char *session_id, session_data_t *session_out);

/**
 * Delete session
 */
int auth_delete_session(MDBX_env *env, const char *session_id);

/**
 * Generate unique session ID
 *
 * Creates a cryptographically secure session ID (64 characters)
 *
 * @param session_id_out Buffer for session ID (must be >= 65 bytes)
 * @return 0 on success, -1 on error
 */
int auth_generate_session_id(char *session_id_out);

/**
 * Add JWT to blacklist
 */
int auth_blacklist_jwt(MDBX_env *env, const char *jwt_id, time_t expiration);

/**
 * Check if JWT is blacklisted
 */
int auth_is_jwt_blacklisted(MDBX_env *env, const char *jwt_id);

#endif // AUTH_H