#include "oauth.h"
#include "pkce.h"
#include "base64url.h"
#include "json_utils.h"
#include "email.h"
#include "password.h"
#include "jwt_rs256.h"
#include "../memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/rand.h>

/**
 * OAuth 2.1 with PKCE Implementation
 *
 * This module implements OAuth 2.1 authorization code flow with PKCE
 * as specified in RFC 7636. All validation is performed server-side
 * for maximum security.
 */

int oauth_authorize_init(auth_context_t *ctx,
                        const oauth_init_request_t *request,
                        const char *user_agent,
                        const char *ip_address,
                        oauth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !request || !result) {
        return -1;
    }

    memset(result, 0, sizeof(oauth_result_t));

    // Validate input parameters
    if (strlen(request->client_id) == 0 ||
        strlen(request->redirect_uri) == 0 ||
        strlen(request->code_challenge) == 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Missing required parameters");
        return 0;
    }

    // Validate PKCE method
    if (!pkce_validate_method(request->code_challenge_method)) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Unsupported code_challenge_method. Only S256 is supported");
        return 0;
    }

    // Validate code challenge format
    if (!pkce_validate_challenge_format(request->code_challenge)) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Invalid code_challenge format");
        return 0;
    }

    // Load and validate client
    client_data_t client;
    if (oauth_get_client(ctx, request->client_id, &client) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_client");
        strcpy(result->error_description, "Unknown client_id");
        return 0;
    }

    if (!client.is_active) {
        result->success = 0;
        strcpy(result->error_code, "invalid_client");
        strcpy(result->error_description, "Client is inactive");
        return 0;
    }

    // Validate redirect URI
    if (!oauth_validate_redirect_uri(request->redirect_uri, client.redirect_uri)) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Invalid redirect_uri");
        return 0;
    }

    // Create OAuth session
    session_data_t session;
    memset(&session, 0, sizeof(session_data_t));

    // Generate session ID
    if (auth_generate_session_id(session.session_id) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate session ID");
        return 0;
    }

    // Set session metadata
    time_t now = time(NULL);
    session.created_at = now;
    session.last_used = now;
    session.expires_at = now + OAUTH_DEFAULT_CODE_LIFETIME; // 10 minutes
    session.oauth_flow_active = 1;

    // Store PKCE parameters
    printf("🔍 PKCE STORE DEBUG: Storing PKCE parameters\n");
    strcpy(session.client_id, request->client_id);
    strcpy(session.code_challenge, request->code_challenge);
    strcpy(session.redirect_uri, request->redirect_uri);
    strcpy(session.state, request->state);
    printf("📝 PKCE STORE DEBUG: client_id='%s'\n", session.client_id);
    printf("📝 PKCE STORE DEBUG: code_challenge='%s' (length: %zu)\n", session.code_challenge, strlen(session.code_challenge));
    printf("📝 PKCE STORE DEBUG: redirect_uri='%s'\n", session.redirect_uri);
    printf("📝 PKCE STORE DEBUG: state='%s'\n", session.state);
    session.code_expires_at = now + OAUTH_DEFAULT_CODE_LIFETIME;
    session.code_used = 0;

    // Store user agent and IP if provided
    if (user_agent) {
        strncpy(session.user_agent, user_agent, sizeof(session.user_agent) - 1);
    }
    if (ip_address) {
        strncpy(session.ip_address, ip_address, sizeof(session.ip_address) - 1);
    }

    // Save session to database
    if (auth_store_session(ctx->sessions_env, &session) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to store OAuth session");
        return 0;
    }

    // Return success response
    result->success = 1;
    strcpy(result->data.init.session_id, session.session_id);
    snprintf(result->data.init.authorization_url, sizeof(result->data.init.authorization_url),
             "/oauth/authorize?session_id=%s", session.session_id);
    result->data.init.expires_in = OAUTH_DEFAULT_CODE_LIFETIME;

    return 0;
}

int oauth_authorize_complete(auth_context_t *ctx,
                            const oauth_complete_request_t *request,
                            oauth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !request || !result) {
        return -1;
    }

    memset(result, 0, sizeof(oauth_result_t));

    // Validate input
    if (strlen(request->session_id) == 0 ||
        strlen(request->email) == 0 ||
        strlen(request->password) == 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Missing required parameters");
        return 0;
    }

    if (!request->consent_granted) {
        result->success = 0;
        strcpy(result->error_code, "access_denied");
        strcpy(result->error_description, "User denied authorization");
        return 0;
    }

    // Load OAuth session
    session_data_t session;
    if (auth_load_session(ctx->sessions_env, request->session_id, &session) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Invalid session_id");
        return 0;
    }

    // Validate session state
    if (!session.oauth_flow_active) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Session is not an active OAuth flow");
        return 0;
    }

    time_t now = time(NULL);
    if (now > session.code_expires_at) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "OAuth session expired");
        return 0;
    }

    // Authenticate user
    char normalized_email[MAX_EMAIL_LENGTH + 1];
    if (normalize_email(request->email, normalized_email) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Invalid email format");
        return 0;
    }

    // Look up user by email
    char user_id[FIREBASE_USERID_LENGTH + 1];
    if (auth_lookup_user_by_email(ctx->email_index_env, normalized_email, user_id) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Invalid email or password");
        return 0;
    }

    // Load user data
    user_data_t user;
    if (auth_load_user(ctx->users_env, user_id, &user) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Invalid email or password");
        return 0;
    }

    // Verify password
    if (verify_password(request->password, user.password_hash) != 1) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Invalid email or password");
        return 0;
    }

    if (!user.is_active) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "User account is inactive");
        return 0;
    }

    // Generate authorization code
    if (oauth_generate_authorization_code(session.authorization_code) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate authorization code");
        return 0;
    }

    // Update session with user ID and authorization code
    strcpy(session.user_id, user_id);
    session.last_used = now;

    // Update user's last login time
    user.last_login = now;
    auth_store_user(ctx->users_env, &user);

    // Save updated session
    if (auth_store_session(ctx->sessions_env, &session) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to update OAuth session");
        return 0;
    }

    // Return success response
    result->success = 1;
    strcpy(result->data.complete.authorization_code, session.authorization_code);
    strcpy(result->data.complete.redirect_uri, session.redirect_uri);
    strcpy(result->data.complete.state, session.state);

    return 0;
}

int oauth_token_exchange(auth_context_t *ctx,
                        const oauth_token_request_t *request,
                        oauth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !request || !result) {
        return -1;
    }

    memset(result, 0, sizeof(oauth_result_t));

    // Validate grant type
    if (strcmp(request->grant_type, "authorization_code") != 0) {
        result->success = 0;
        strcpy(result->error_code, "unsupported_grant_type");
        strcpy(result->error_description, "Only authorization_code grant type is supported");
        return 0;
    }

    // Validate input parameters
    if (strlen(request->code) == 0 ||
        strlen(request->code_verifier) == 0 ||
        strlen(request->client_id) == 0 ||
        strlen(request->redirect_uri) == 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Missing required parameters");
        return 0;
    }

    // Validate code verifier format
    if (!pkce_validate_verifier_format(request->code_verifier)) {
        result->success = 0;
        strcpy(result->error_code, "invalid_request");
        strcpy(result->error_description, "Invalid code_verifier format");
        return 0;
    }

    // Find session by authorization code
    session_data_t session;
    int session_found = 0;

    // We need to search through sessions to find the one with matching authorization code
    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;
    MDBX_cursor *cursor = NULL;

    int rc = mdbx_txn_begin(ctx->sessions_env, NULL, MDBX_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Database error");
        return 0;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc == MDBX_SUCCESS) {
        rc = mdbx_cursor_open(txn, dbi, &cursor);
        if (rc == MDBX_SUCCESS) {
            MDBX_val key, data;
            while ((rc = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) == MDBX_SUCCESS) {
                if (data.iov_len == sizeof(session_data_t)) {
                    session_data_t *temp_session = (session_data_t*)data.iov_base;
                    if (strcmp(temp_session->authorization_code, request->code) == 0) {
                        session = *temp_session;
                        session_found = 1;
                        break;
                    }
                }
            }
            mdbx_cursor_close(cursor);
        }
    }
    mdbx_txn_abort(txn);

    if (!session_found) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Invalid authorization code");
        return 0;
    }

    // Validate session state
    if (!session.oauth_flow_active || strlen(session.authorization_code) == 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Authorization code not found");
        return 0;
    }

    if (session.code_used) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Authorization code already used");
        return 0;
    }

    time_t now = time(NULL);
    if (now > session.code_expires_at) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Authorization code expired");
        return 0;
    }

    // Validate client_id and redirect_uri match
    if (strcmp(session.client_id, request->client_id) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Client ID mismatch");
        return 0;
    }

    if (strcmp(session.redirect_uri, request->redirect_uri) != 0) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "Redirect URI mismatch");
        return 0;
    }

    // Validate PKCE code verifier
    printf("🔍 PKCE VERIFY DEBUG: Starting PKCE validation\n");
    printf("📝 PKCE VERIFY DEBUG: code_verifier='%s' (length: %zu)\n", request->code_verifier, strlen(request->code_verifier));
    printf("📝 PKCE VERIFY DEBUG: stored_challenge='%s' (length: %zu)\n", session.code_challenge, strlen(session.code_challenge));

    int pkce_valid = pkce_validate_code_verifier(request->code_verifier, session.code_challenge);
    printf("📊 PKCE VERIFY DEBUG: Validation result: %s\n", pkce_valid ? "PASS" : "FAIL");

    if (!pkce_valid) {
        result->success = 0;
        strcpy(result->error_code, "invalid_grant");
        strcpy(result->error_description, "PKCE verification failed");
        return 0;
    }

    printf("✅ PKCE VERIFY DEBUG: PKCE validation successful\n");

    // Load user data
    user_data_t user;
    if (auth_load_user(ctx->users_env, session.user_id, &user) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "User not found");
        return 0;
    }

    // Generate JWT tokens
    jwt_claims_t claims;
    jwt_init_claims(&claims);
    strcpy(claims.issuer, ctx->config.issuer);
    strcpy(claims.audience, ctx->config.audience);
    strcpy(claims.subject, user.user_id);
    claims.issued_at = now;

    // Generate access token
    claims.expires_at = now + ctx->config.access_token_lifetime;
    if (jwt_generate_id(claims.jwt_id) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate token ID");
        return 0;
    }

    // Add custom claims
    strcpy(claims.email, user.email);
    claims.email_verified = user.email_verified;
    strcpy(claims.session_id, session.session_id);

    if (jwt_create_access_token(&claims, &ctx->keypair, result->data.token.access_token) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate access token");
        return 0;
    }

    strcpy(session.access_token_id, claims.jwt_id);

    // Generate refresh token
    claims.expires_at = now + ctx->config.refresh_token_lifetime;
    if (jwt_generate_id(claims.jwt_id) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate refresh token ID");
        return 0;
    }

    if (jwt_create_refresh_token(&claims, &ctx->keypair, result->data.token.refresh_token) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to generate refresh token");
        return 0;
    }

    strcpy(session.refresh_token_id, claims.jwt_id);

    // Convert OAuth session to regular authenticated session
    session.oauth_flow_active = 0;
    session.code_used = 1;
    session.expires_at = now + ctx->config.session_lifetime;
    session.last_used = now;

    // Save updated session
    if (auth_store_session(ctx->sessions_env, &session) != 0) {
        result->success = 0;
        strcpy(result->error_code, "server_error");
        strcpy(result->error_description, "Failed to update session");
        return 0;
    }

    // Set response data
    result->success = 1;
    strcpy(result->data.token.token_type, "Bearer");
    result->data.token.expires_in = ctx->config.access_token_lifetime;

    return 0;
}

int oauth_generate_authorization_code(char *code_out) {
    printf("🔍 AUTH CODE DEBUG: Starting authorization code generation\n");

    if (!code_out) {
        printf("❌ AUTH CODE DEBUG: code_out is NULL\n");
        return -1;
    }

    // Use crypto buffer for secure random generation (follows existing patterns)
    printf("🔍 AUTH CODE DEBUG: Allocating crypto buffer (30 bytes)\n");
    crypto_buffer_t* buffer = platform_crypto_buffer_alloc(30);
    if (!buffer) {
        printf("❌ AUTH CODE DEBUG: Failed to allocate crypto buffer\n");
        return -1;
    }
    printf("✅ AUTH CODE DEBUG: Crypto buffer allocated successfully\n");

    // Generate secure random bytes using page-allocated buffer
    unsigned char *random_bytes = (unsigned char*)platform_crypto_buffer_get_data(buffer);
    if (!random_bytes) {
        printf("❌ AUTH CODE DEBUG: Failed to get data from crypto buffer\n");
        platform_crypto_buffer_free(buffer);
        return -1;
    }
    printf("✅ AUTH CODE DEBUG: Got crypto buffer data pointer\n");

    printf("🔍 AUTH CODE DEBUG: Generating random bytes\n");
    if (RAND_bytes(random_bytes, 30) != 1) {
        printf("❌ AUTH CODE DEBUG: RAND_bytes failed\n");
        platform_crypto_buffer_free(buffer);
        return -1;
    }
    printf("✅ AUTH CODE DEBUG: Random bytes generated successfully\n");

    // Encode to base64url with flexible length validation (like working functions)
    printf("🔍 AUTH CODE DEBUG: Encoding to base64url (buffer size: %d)\n", OAUTH_AUTHORIZATION_CODE_LENGTH + 1);
    int encoded_len = base64url_encode(random_bytes, 30,
                                      code_out, OAUTH_AUTHORIZATION_CODE_LENGTH + 1);
    printf("📝 AUTH CODE DEBUG: base64url_encode returned length: %d\n", encoded_len);

    // Secure cleanup using page allocator
    platform_crypto_buffer_free(buffer);
    printf("✅ AUTH CODE DEBUG: Crypto buffer freed\n");

    // Flexible length check (learn from working session ID and JWT ID patterns)
    printf("🔍 AUTH CODE DEBUG: Checking length bounds (min: %d, max: %d, actual: %d)\n",
           OAUTH_AUTHORIZATION_CODE_MIN_LENGTH, OAUTH_AUTHORIZATION_CODE_MAX_LENGTH, encoded_len);

    if (encoded_len < OAUTH_AUTHORIZATION_CODE_MIN_LENGTH ||
        encoded_len > OAUTH_AUTHORIZATION_CODE_MAX_LENGTH) {
        printf("❌ AUTH CODE DEBUG: Length check failed - encoded_len=%d not in range [%d,%d]\n",
               encoded_len, OAUTH_AUTHORIZATION_CODE_MIN_LENGTH, OAUTH_AUTHORIZATION_CODE_MAX_LENGTH);
        return -1;
    }

    // Ensure null termination
    code_out[encoded_len] = '\0';
    printf("✅ AUTH CODE DEBUG: Authorization code generated successfully: %.10s...\n", code_out);

    return 0;
}

int oauth_validate_redirect_uri(const char *provided_uri, const char *registered_uri) {
    if (!provided_uri || !registered_uri) {
        return 0;
    }

    // For security, require exact match of redirect URI
    return strcmp(provided_uri, registered_uri) == 0;
}

int oauth_register_client(auth_context_t *ctx, const client_data_t *client_data) {
    if (!ctx || !ctx->is_initialized || !client_data) {
        return -1;
    }

    return oauth_store_client(ctx->clients_env, client_data);
}

int oauth_get_client(auth_context_t *ctx, const char *client_id, client_data_t *client_out) {
    if (!ctx || !ctx->is_initialized || !client_id || !client_out) {
        return -1;
    }

    return oauth_load_client(ctx->clients_env, client_id, client_out);
}

int oauth_delete_client(auth_context_t *ctx, const char *client_id) {
    if (!ctx || !ctx->is_initialized || !client_id) {
        return -1;
    }

    return oauth_delete_client_from_db(ctx->clients_env, client_id);
}

// Database utility functions

int oauth_store_client(MDBX_env *env, const client_data_t *client) {
    if (!env || !client) {
        return -1;
    }

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        return -1;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    MDBX_val key = {(void*)client->client_id, strlen(client->client_id)};
    MDBX_val data = {(void*)client, sizeof(client_data_t)};

    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int oauth_load_client(MDBX_env *env, const char *client_id, client_data_t *client_out) {
    if (!env || !client_id || !client_out) {
        return -1;
    }

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, MDBX_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        return -1;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    MDBX_val key = {(void*)client_id, strlen(client_id)};
    MDBX_val data;

    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc == MDBX_SUCCESS && data.iov_len == sizeof(client_data_t)) {
        memcpy(client_out, data.iov_base, sizeof(client_data_t));
    }

    mdbx_txn_abort(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int oauth_delete_client_from_db(MDBX_env *env, const char *client_id) {
    if (!env || !client_id) {
        return -1;
    }

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        return -1;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    MDBX_val key = {(void*)client_id, strlen(client_id)};

    rc = mdbx_del(txn, dbi, &key, NULL);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}