#include "auth.h"
#include "base64url.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <openssl/rand.h>

void auth_init_config(auth_config_t *config) {
    if (!config) return;

    strcpy(config->issuer, AUTH_DEFAULT_ISSUER);
    strcpy(config->audience, AUTH_DEFAULT_AUDIENCE);
    config->access_token_lifetime = JWT_ACCESS_TOKEN_LIFETIME;
    config->refresh_token_lifetime = JWT_REFRESH_TOKEN_LIFETIME;
    config->session_lifetime = 200; // Session lifetime (200 seconds) - independent of token lifetimes
    strcpy(config->database_path, "data");
    config->enable_refresh_token_rotation = 1; // Default to secure behavior
}

int auth_open_database(const char *path, MDBX_env **env_out) {
    if (!path || !env_out) {
        return -1;
    }

    // Create directories recursively
    char tmp_path[512];
    strncpy(tmp_path, path, sizeof(tmp_path) - 1);
    tmp_path[sizeof(tmp_path) - 1] = '\0';
    
    for (char *p = tmp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            struct stat st = {0};
            if (stat(tmp_path, &st) == -1) {
                if (mkdir(tmp_path, 0755) != 0 && errno != EEXIST) {
                    return -1;
                }
            }
            *p = '/';
        }
    }
    
    // Create the final directory
    struct stat st = {0};
    if (stat(tmp_path, &st) == -1) {
        if (mkdir(tmp_path, 0755) != 0 && errno != EEXIST) {
            return -1;
        }
    }

    int rc = mdbx_env_create(env_out);
    if (rc != MDBX_SUCCESS) {
        return -1;
    }

    // Set database size limit (64MB)
    rc = mdbx_env_set_mapsize(*env_out, 64 * 1024 * 1024);
    if (rc != MDBX_SUCCESS) {
        mdbx_env_close(*env_out);
        return -1;
    }

    // Open the database
    rc = mdbx_env_open(*env_out, path, MDBX_CREATE, 0664);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX Error: Failed to open %s\n", path);
        printf("   Error code: %d (%s)\n", rc, mdbx_strerror(rc));
        printf("   Database path: %s\n", path);
        printf("   Attempted flags: MDBX_CREATE\n");
        mdbx_env_close(*env_out);
        return -1;
    }

    return 0;
}

int auth_initialize(auth_context_t *ctx, const auth_config_t *config) {
    if (!ctx) {
        return -1;
    }

    memset(ctx, 0, sizeof(auth_context_t));

    // Set configuration
    if (config) {
        ctx->config = *config;
    } else {
        auth_init_config(&ctx->config);
    }

    // Initialize RSA keypair
    rsa_init_keypair(&ctx->keypair);
    if (rsa_get_or_generate_keys(&ctx->keypair) != 0) {
        return -1;
    }

    // Initialize JWKS
    printf("🔧 Initializing JWKS...\n");
    jwks_init(&ctx->jwks);
    if (jwks_add_key(&ctx->jwks, &ctx->keypair) != 0) {
        printf("❌ Failed to add key to JWKS\n");
        rsa_free_keypair(&ctx->keypair);
        return -1;
    }
    printf("✅ JWKS initialized successfully\n");

    // Open databases
    printf("🔧 Opening databases...\n");
    char db_path[600];

    // Users database
    printf("🔧 Opening users database...\n");
    snprintf(db_path, sizeof(db_path), "%s/users", ctx->config.database_path);
    if (auth_open_database(db_path, &ctx->users_env) != 0) {
        printf("❌ Failed to open users database at: %s\n", db_path);
        auth_cleanup(ctx);
        return -1;
    }
    printf("✅ Users database opened\n");

    // Email index database
    printf("🔧 Opening email index database...\n");
    snprintf(db_path, sizeof(db_path), "%s/email_index", ctx->config.database_path);
    if (auth_open_database(db_path, &ctx->email_index_env) != 0) {
        printf("❌ Failed to open email index database at: %s\n", db_path);
        auth_cleanup(ctx);
        return -1;
    }
    printf("✅ Email index database opened\n");

    // Sessions database
    printf("🔧 Opening sessions database...\n");
    snprintf(db_path, sizeof(db_path), "%s/sessions", ctx->config.database_path);
    if (auth_open_database(db_path, &ctx->sessions_env) != 0) {
        printf("❌ Failed to open sessions database at: %s\n", db_path);
        auth_cleanup(ctx);
        return -1;
    }
    printf("✅ Sessions database opened\n");

    // Blacklist database
    printf("🔧 Opening blacklist database...\n");
    snprintf(db_path, sizeof(db_path), "%s/blacklist", ctx->config.database_path);
    if (auth_open_database(db_path, &ctx->blacklist_env) != 0) {
        printf("❌ Failed to open blacklist database at: %s\n", db_path);
        auth_cleanup(ctx);
        return -1;
    }
    printf("✅ Blacklist database opened\n");

    // OAuth clients database
    printf("🔧 Opening OAuth clients database...\n");
    snprintf(db_path, sizeof(db_path), "%s/clients", ctx->config.database_path);
    if (auth_open_database(db_path, &ctx->clients_env) != 0) {
        printf("❌ Failed to open OAuth clients database at: %s\n", db_path);
        auth_cleanup(ctx);
        return -1;
    }
    printf("✅ OAuth clients database opened\n");

    printf("✅ All databases opened successfully\n");
    ctx->is_initialized = 1;
    return 0;
}

void auth_cleanup(auth_context_t *ctx) {
    if (!ctx) return;

    // Close databases
    if (ctx->users_env) {
        mdbx_env_close(ctx->users_env);
        ctx->users_env = NULL;
    }

    if (ctx->email_index_env) {
        mdbx_env_close(ctx->email_index_env);
        ctx->email_index_env = NULL;
    }

    if (ctx->sessions_env) {
        mdbx_env_close(ctx->sessions_env);
        ctx->sessions_env = NULL;
    }

    if (ctx->blacklist_env) {
        mdbx_env_close(ctx->blacklist_env);
        ctx->blacklist_env = NULL;
    }

    if (ctx->clients_env) {
        mdbx_env_close(ctx->clients_env);
        ctx->clients_env = NULL;
    }

    // Free RSA keypair
    rsa_free_keypair(&ctx->keypair);

    ctx->is_initialized = 0;
}

int auth_store_user(MDBX_env *env, const user_data_t *user) {
    if (!env || !user) {
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

    MDBX_val key = {user->user_id, strlen(user->user_id)};
    MDBX_val data = {(void*)user, sizeof(user_data_t)};

    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int auth_load_user(MDBX_env *env, const char *user_id, user_data_t *user_out) {
    if (!env || !user_id || !user_out) {
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

    MDBX_val key = {(void*)user_id, strlen(user_id)};
    MDBX_val data;

    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    if (data.iov_len == sizeof(user_data_t)) {
        memcpy(user_out, data.iov_base, sizeof(user_data_t));
        mdbx_txn_abort(txn);
        return 0;
    }

    mdbx_txn_abort(txn);
    return -1;
}

int auth_store_email_index(MDBX_env *env, const char *email, const char *user_id) {
    if (!env || !email || !user_id) {
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

    MDBX_val key = {(void*)email, strlen(email)};
    MDBX_val data = {(void*)user_id, strlen(user_id)};

    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int auth_lookup_user_by_email(MDBX_env *env, const char *email, char *user_id_out) {
    if (!env || !email || !user_id_out) {
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

    MDBX_val key = {(void*)email, strlen(email)};
    MDBX_val data;

    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    if (data.iov_len < FIREBASE_USERID_LENGTH + 1) {
        strncpy(user_id_out, (char*)data.iov_base, data.iov_len);
        user_id_out[data.iov_len] = '\0';
        mdbx_txn_abort(txn);
        return 0;
    }

    mdbx_txn_abort(txn);
    return -1;
}

int auth_blacklist_jwt(MDBX_env *env, const char *jwt_id, time_t expiration) {
    if (!env || !jwt_id) {
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

    MDBX_val key = {(void*)jwt_id, strlen(jwt_id)};
    MDBX_val data = {&expiration, sizeof(time_t)};

    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int auth_is_jwt_blacklisted(MDBX_env *env, const char *jwt_id) {
    if (!env || !jwt_id) {
        return 0; // Assume not blacklisted on error
    }

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, MDBX_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        return 0;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return 0;
    }

    MDBX_val key = {(void*)jwt_id, strlen(jwt_id)};
    MDBX_val data;

    rc = mdbx_get(txn, dbi, &key, &data);
    mdbx_txn_abort(txn);

    return rc == MDBX_SUCCESS ? 1 : 0; // Found = blacklisted
}

int auth_register(auth_context_t *ctx,
                 const char *email,
                 const char *password,
                 auth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !email || !password || !result) {
        return -1;
    }

    memset(result, 0, sizeof(auth_result_t));

    // Validate email format
    char normalized_email[MAX_EMAIL_LENGTH + 1];
    if (normalize_email(email, normalized_email) != 0) {
        strcpy(result->error_message, "Invalid email format");
        return 0;
    }

    // Validate password strength
    if (!is_password_strong(password)) {
        strcpy(result->error_message, "Password does not meet requirements");
        return 0;
    }

    // Check if email already exists
    char existing_user_id[FIREBASE_USERID_LENGTH + 1];
    if (auth_lookup_user_by_email(ctx->email_index_env, normalized_email, existing_user_id) == 0) {
        strcpy(result->error_message, "Email already registered");
        return 0;
    }

    // Generate user ID
    user_data_t user;
    memset(&user, 0, sizeof(user));

    if (generate_firebase_userid(user.user_id) != 0) {
        strcpy(result->error_message, "Failed to generate user ID");
        return 0;
    }

    // Hash password
    if (hash_password(password, user.password_hash) != 0) {
        strcpy(result->error_message, "Failed to hash password");
        return 0;
    }

    // Set user data
    strcpy(user.email, normalized_email);
    user.created_at = time(NULL);
    user.last_login = 0;
    user.email_verified = 0;
    user.is_active = 1;

    // Store user in database
    if (auth_store_user(ctx->users_env, &user) != 0) {
        strcpy(result->error_message, "Failed to store user data");
        return 0;
    }

    // Store email index
    if (auth_store_email_index(ctx->email_index_env, normalized_email, user.user_id) != 0) {
        strcpy(result->error_message, "Failed to store email index");
        return 0;
    }

    // Success
    result->success = 1;
    result->user = user;
    strcpy(result->error_message, "Registration successful");

    return 0;
}

int auth_login(auth_context_t *ctx,
              const char *email,
              const char *password,
              const char *user_agent,
              const char *ip_address,
              auth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !email || !password || !result) {
        return -1;
    }

    memset(result, 0, sizeof(auth_result_t));

    // Normalize email
    char normalized_email[MAX_EMAIL_LENGTH + 1];
    if (normalize_email(email, normalized_email) != 0) {
        strcpy(result->error_message, "Invalid email format");
        return 0;
    }

    // Look up user by email
    char user_id[FIREBASE_USERID_LENGTH + 1];
    if (auth_lookup_user_by_email(ctx->email_index_env, normalized_email, user_id) != 0) {
        strcpy(result->error_message, "Invalid email or password");
        return 0;
    }

    // Load user data
    user_data_t user;
    if (auth_load_user(ctx->users_env, user_id, &user) != 0) {
        strcpy(result->error_message, "Invalid email or password");
        return 0;
    }

    // Check if user is active
    if (!user.is_active) {
        strcpy(result->error_message, "Account is disabled");
        return 0;
    }

    // Verify password
    if (verify_password(password, user.password_hash) != 1) {
        strcpy(result->error_message, "Invalid email or password");
        return 0;
    }

    // Generate session ID
    session_data_t session;
    if (auth_generate_session_id(session.session_id) != 0) {
        strcpy(result->error_message, "Failed to generate session ID");
        return 0;
    }

    printf("🔍 SESSION CREATION DEBUG:\n");
    printf("📝 Generated session ID: '%s'\n", session.session_id);
    printf("📝 Session ID length: %zu\n", strlen(session.session_id));

    // Create JWT claims
    jwt_claims_t claims;
    if (jwt_set_user_claims(&claims, user.user_id, user.email, user.email_verified,
                           ctx->config.issuer, ctx->config.audience,
                           ctx->config.access_token_lifetime) != 0) {
        strcpy(result->error_message, "Failed to create access token");
        return 0;
    }

    // Add session ID to claims
    strcpy(claims.session_id, session.session_id);

    // Generate access token
    if (jwt_create_access_token(&claims, &ctx->keypair, result->access_token) != 0) {
        strcpy(result->error_message, "Failed to create access token");
        return 0;
    }

    // Create refresh token claims
    jwt_claims_t refresh_claims = claims;
    refresh_claims.expires_at = refresh_claims.issued_at + ctx->config.refresh_token_lifetime;

    // Generate NEW JWT ID for refresh token (critical security fix)
    if (jwt_generate_id(refresh_claims.jwt_id) != 0) {
        strcpy(result->error_message, "Failed to generate refresh token ID");
        return 0;
    }

    if (jwt_create_refresh_token(&refresh_claims, &ctx->keypair, result->refresh_token) != 0) {
        strcpy(result->error_message, "Failed to create refresh token");
        return 0;
    }

    // Create session record
    strcpy(session.user_id, user.user_id);
    strcpy(session.refresh_token_id, refresh_claims.jwt_id);
    strcpy(session.access_token_id, claims.jwt_id);
    session.created_at = time(NULL);
    session.last_used = time(NULL);
    session.expires_at = session.created_at + ctx->config.session_lifetime; // Fixed 200s deadline from login

    if (user_agent) {
        strncpy(session.user_agent, user_agent, sizeof(session.user_agent) - 1);
        session.user_agent[sizeof(session.user_agent) - 1] = '\0';
    } else {
        session.user_agent[0] = '\0';
    }

    if (ip_address) {
        strncpy(session.ip_address, ip_address, sizeof(session.ip_address) - 1);
        session.ip_address[sizeof(session.ip_address) - 1] = '\0';
    } else {
        session.ip_address[0] = '\0';
    }

    // Store session in database
    printf("📝 Session user ID: '%s'\n", session.user_id);
    printf("📝 Session refresh token ID: '%s'\n", session.refresh_token_id);
    printf("📝 Session access token ID: '%s'\n", session.access_token_id);
    printf("🔍 Storing session in database...\n");

    int store_result = auth_store_session(ctx->sessions_env, &session);
    printf("📊 Session storage result: %s\n", store_result == 0 ? "SUCCESS" : "FAILED");

    if (store_result != 0) {
        strcpy(result->error_message, "Failed to create session");
        return 0;
    }

    // Update last login time
    user.last_login = time(NULL);
    auth_store_user(ctx->users_env, &user);

    // Success
    result->success = 1;
    result->user = user;
    result->session = session;
    strcpy(result->error_message, "Login successful");

    return 0;
}

int auth_refresh(auth_context_t *ctx,
                const char *refresh_token,
                auth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !refresh_token || !result) {
        return -1;
    }

    memset(result, 0, sizeof(auth_result_t));

    // Validate refresh token cryptographically (allow expired tokens for potential recovery)
    jwt_validation_result_t jwt_result;
    if (jwt_validate_token(refresh_token, &ctx->keypair, &jwt_result) != 0) {
        strcpy(result->error_message, "Invalid refresh token");
        return 0;
    }

    // Check token validity - if invalid due to expiration, we'll handle recovery below
    if (!jwt_result.is_valid && !jwt_result.is_expired) {
        strcpy(result->error_message, "Invalid refresh token");
        return 0;
    }

    // Check if token is expired - reject and direct to recovery endpoint
    if (jwt_result.is_expired) {
        printf("[AUTH] Refresh token expired - directing user to recovery endpoint\n");
        strcpy(result->error_message, "Refresh token expired - use POST /auth/recover-session to attempt session recovery");
        return 0;
    }

    // Check if token is blacklisted
    if (auth_is_jwt_blacklisted(ctx->blacklist_env, jwt_result.claims.jwt_id)) {
        strcpy(result->error_message, "Refresh token has been revoked");
        return 0;
    }

    // Load session using session ID from token
    session_data_t session;

    printf("🔍 SESSION LOOKUP DEBUG:\n");
    printf("📝 JWT claims session ID: '%s'\n", jwt_result.claims.session_id);
    printf("📝 Session ID length: %zu\n", strlen(jwt_result.claims.session_id));

    if (strlen(jwt_result.claims.session_id) == 0) {
        printf("❌ No session ID found in JWT claims\n");
        strcpy(result->error_message, "No session ID in refresh token");
        return 0;
    }

    printf("🔍 Attempting to load session from database...\n");
    int session_result = auth_load_session(ctx->sessions_env, jwt_result.claims.session_id, &session);
    printf("📊 Session load result: %d\n", session_result);

    if (session_result != 0) {
        printf("❌ Session NOT found in database\n");
        strcpy(result->error_message, "Session not found or expired");
        return 0;
    }

    printf("✅ Session found!\n");
    printf("📝 Loaded session ID: '%s'\n", session.session_id);
    printf("📝 Loaded refresh token ID: '%s'\n", session.refresh_token_id);
    printf("📝 Loaded access token ID: '%s'\n", session.access_token_id);

    // Check session expiration
    if (time(NULL) >= session.expires_at) {
        // Clean up expired session
        auth_delete_session(ctx->sessions_env, session.session_id);
        strcpy(result->error_message, "Session has expired");
        return 0;
    }

    // Verify that the refresh token ID matches the session
    if (strcmp(session.refresh_token_id, jwt_result.claims.jwt_id) != 0) {
        strcpy(result->error_message, "Token does not match session");
        return 0;
    }

    // Load user data
    user_data_t user;
    if (auth_load_user(ctx->users_env, jwt_result.claims.subject, &user) != 0) {
        strcpy(result->error_message, "User not found");
        return 0;
    }

    // Check if user is still active
    if (!user.is_active) {
        strcpy(result->error_message, "Account is disabled");
        return 0;
    }

    // Generate new access token with same session ID
    jwt_claims_t new_claims;
    if (jwt_set_user_claims(&new_claims, user.user_id, user.email, user.email_verified,
                           ctx->config.issuer, ctx->config.audience,
                           ctx->config.access_token_lifetime) != 0) {
        strcpy(result->error_message, "Failed to create new access token");
        return 0;
    }

    // Maintain same session ID
    strcpy(new_claims.session_id, session.session_id);

    if (jwt_create_access_token(&new_claims, &ctx->keypair, result->access_token) != 0) {
        strcpy(result->error_message, "Failed to create new access token");
        return 0;
    }

    // Handle refresh token rotation (configurable for security vs compatibility)
    if (ctx->config.enable_refresh_token_rotation) {
        // Generate NEW refresh token (OAuth 2.0 Security Best Practice)
        jwt_claims_t new_refresh_claims;
        if (jwt_set_user_claims(&new_refresh_claims, user.user_id, user.email, user.email_verified,
                               ctx->config.issuer, ctx->config.audience,
                               ctx->config.refresh_token_lifetime) != 0) {
            strcpy(result->error_message, "Failed to create new refresh token");
            return 0;
        }

        // Maintain same session ID in new refresh token
        strcpy(new_refresh_claims.session_id, session.session_id);

        if (jwt_create_refresh_token(&new_refresh_claims, &ctx->keypair, result->refresh_token) != 0) {
            strcpy(result->error_message, "Failed to create new refresh token");
            return 0;
        }

        // Blacklist the old refresh token immediately (prevents reuse attacks)
        auth_blacklist_jwt(ctx->blacklist_env, jwt_result.claims.jwt_id,
                          jwt_result.claims.expires_at);

        // SESSION PERSISTENCE: Update existing session with new token IDs (three-tier hierarchy)
        // Session ID remains stable, only token references and timestamps change

        printf("[AUTH] Session persistence: Keeping session_id=%s, updating token references\n", session.session_id);

        // Capture old token IDs for logging before updating
        char old_refresh_token_id[JWT_ID_LENGTH + 1];
        char old_access_token_id[JWT_ID_LENGTH + 1];
        strcpy(old_refresh_token_id, session.refresh_token_id);
        strcpy(old_access_token_id, session.access_token_id);

        // Update EXISTING session with new token IDs (keep same session_id)
        strcpy(session.refresh_token_id, new_refresh_claims.jwt_id);
        strcpy(session.access_token_id, new_claims.jwt_id);
        // created_at: UNCHANGED (preserve original login time)
        session.last_used = time(NULL);  // Update activity timestamp

        // DO NOT extend session lifetime - session has FIXED 200-second deadline from login
        // expires_at: UNCHANGED (session dies exactly 200 seconds after login)

        // user_agent, ip_address: UNCHANGED (preserve device metadata)

        printf("[AUTH] Session deadline unchanged: expires_at remains %ld (200s from login)\n",
               session.expires_at);
        printf("[AUTH] Token rotation: refresh_token_id %s -> %s\n",
               old_refresh_token_id, new_refresh_claims.jwt_id);
        printf("[AUTH] Token rotation: access_token_id %s -> %s\n",
               old_access_token_id, new_claims.jwt_id);

        // Store updated session (same session_id, updated token pointers)
        if (auth_store_session(ctx->sessions_env, &session) != 0) {
            strcpy(result->error_message, "Failed to update session");
            return 0;
        }

        printf("[AUTH] Session persistence complete: session_id=%s maintained across token rotation\n", session.session_id);

    } else {
        // Return same refresh token (less secure but compatible with some clients)
        // NO session extension - session has FIXED 200-second deadline from login
        printf("[AUTH] No token rotation: session deadline unchanged, session_id=%s\n", session.session_id);
        session.last_used = time(NULL);

        // DO NOT extend session lifetime - session has FIXED 200-second deadline from login
        // expires_at: UNCHANGED (session dies exactly 200 seconds after login)

        strcpy(session.access_token_id, new_claims.jwt_id);

        printf("[AUTH] Session deadline unchanged without rotation: expires_at remains %ld (200s from login)\n",
               session.expires_at);

        if (auth_store_session(ctx->sessions_env, &session) != 0) {
            strcpy(result->error_message, "Failed to update session");
            return 0;
        }

        strcpy(result->refresh_token, refresh_token);
    }

    // Success
    result->success = 1;
    result->user = user;
    result->session = session;
    strcpy(result->error_message, "Token refreshed successfully");

    return 0;
}

int auth_recover_session(auth_context_t *ctx,
                        const char *expired_refresh_token,
                        auth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !expired_refresh_token || !result) {
        return -1;
    }

    memset(result, 0, sizeof(auth_result_t));

    // Validate expired refresh token cryptographically (signature must still be valid)
    jwt_validation_result_t jwt_result;
    if (jwt_validate_token(expired_refresh_token, &ctx->keypair, &jwt_result) != 0) {
        strcpy(result->error_message, "Invalid refresh token");
        return 0;
    }

    // Check token validity - if invalid for reasons other than expiration, fail
    if (!jwt_result.is_valid && !jwt_result.is_expired) {
        strcpy(result->error_message, "Invalid refresh token");
        return 0;
    }

    // Token must be expired for recovery to be appropriate
    if (!jwt_result.is_expired) {
        strcpy(result->error_message, "Token not expired - use /auth/refresh endpoint instead");
        return 0;
    }

    printf("[AUTH] Session recovery: expired refresh token provided, checking session validity...\n");

    // Load session to check if it's still valid despite expired refresh token
    session_data_t session;
    if (strlen(jwt_result.claims.session_id) == 0) {
        strcpy(result->error_message, "No session ID in expired refresh token");
        return 0;
    }

    if (auth_load_session(ctx->sessions_env, jwt_result.claims.session_id, &session) != 0) {
        strcpy(result->error_message, "Session not found - login required at POST /auth/login");
        return 0;
    }

    // Check if session is still valid (session lifetime independent of refresh token lifetime)
    if (time(NULL) >= session.expires_at) {
        printf("[AUTH] Session also expired (expires_at=%ld, now=%ld) - login required\n",
               session.expires_at, time(NULL));
        strcpy(result->error_message, "Session has expired (200 seconds since login) - login required at POST /auth/login");
        return 0;
    }

    // SESSION RECOVERY: Session still valid, issue new refresh token
    printf("[AUTH] Session recovery: session_id=%s still valid (expires_at=%ld), issuing new refresh token\n",
           session.session_id, session.expires_at);

    // Load user for token generation
    user_data_t user;
    if (auth_load_user(ctx->users_env, session.user_id, &user) != 0 || !user.is_active) {
        strcpy(result->error_message, "User account not found or disabled");
        return 0;
    }

    // Blacklist the old expired refresh token (security best practice)
    auth_blacklist_jwt(ctx->blacklist_env, jwt_result.claims.jwt_id, jwt_result.claims.expires_at);

    // Generate recovery refresh token
    jwt_claims_t recovery_refresh_claims;
    if (jwt_set_user_claims(&recovery_refresh_claims, user.user_id, user.email, user.email_verified,
                           ctx->config.issuer, ctx->config.audience,
                           ctx->config.refresh_token_lifetime) != 0) {
        strcpy(result->error_message, "Failed to create recovery refresh token");
        return 0;
    }

    // Link recovery token to existing session
    strcpy(recovery_refresh_claims.session_id, session.session_id);

    if (jwt_create_refresh_token(&recovery_refresh_claims, &ctx->keypair, result->refresh_token) != 0) {
        strcpy(result->error_message, "Failed to create recovery refresh token");
        return 0;
    }

    // Generate new access token
    jwt_claims_t recovery_access_claims;
    if (jwt_set_user_claims(&recovery_access_claims, user.user_id, user.email, user.email_verified,
                           ctx->config.issuer, ctx->config.audience,
                           ctx->config.access_token_lifetime) != 0) {
        strcpy(result->error_message, "Failed to create recovery access token");
        return 0;
    }

    strcpy(recovery_access_claims.session_id, session.session_id);

    if (jwt_create_access_token(&recovery_access_claims, &ctx->keypair, result->access_token) != 0) {
        strcpy(result->error_message, "Failed to create recovery access token");
        return 0;
    }

    // Update session with recovery token IDs but DO NOT extend session lifetime
    // Recovery should not extend session - session expires at original time
    strcpy(session.refresh_token_id, recovery_refresh_claims.jwt_id);
    strcpy(session.access_token_id, recovery_access_claims.jwt_id);
    session.last_used = time(NULL);
    // expires_at: UNCHANGED (recovery does not extend session lifetime)

    if (auth_store_session(ctx->sessions_env, &session) != 0) {
        strcpy(result->error_message, "Failed to update session during recovery");
        return 0;
    }

    // Return successful recovery result
    result->success = 1;
    result->user = user;
    result->session = session;
    strcpy(result->error_message, "Session recovered successfully - please review your account activity");

    printf("[AUTH] Session recovery complete: new tokens issued, session expires_at UNCHANGED at %ld\n", session.expires_at);
    return 0;
}

int auth_validate_token(auth_context_t *ctx,
                       const char *access_token,
                       auth_result_t *result) {
    if (!ctx || !ctx->is_initialized || !access_token || !result) {
        return -1;
    }

    memset(result, 0, sizeof(auth_result_t));

    // Validate JWT
    jwt_validation_result_t jwt_result;
    if (jwt_validate_token(access_token, &ctx->keypair, &jwt_result) != 0) {
        strcpy(result->error_message, "Failed to validate token");
        return 0;
    }

    if (!jwt_result.is_valid) {
        strcpy(result->error_message, jwt_result.error_message);
        return 0;
    }

    if (jwt_result.is_expired) {
        strcpy(result->error_message, "Token has expired");
        return 0;
    }

    // Check blacklist
    if (auth_is_jwt_blacklisted(ctx->blacklist_env, jwt_result.claims.jwt_id)) {
        strcpy(result->error_message, "Token has been revoked");
        return 0;
    }

    // Load user data
    user_data_t user;
    if (auth_load_user(ctx->users_env, jwt_result.claims.subject, &user) != 0) {
        strcpy(result->error_message, "User not found");
        return 0;
    }

    if (!user.is_active) {
        strcpy(result->error_message, "Account is disabled");
        return 0;
    }

    // Success
    result->success = 1;
    result->user = user;
    strcpy(result->error_message, "Token is valid");

    return 0;
}

int auth_logout(auth_context_t *ctx,
               const char *access_token,
               int logout_all) {
    if (!ctx || !ctx->is_initialized || !access_token) {
        return -1;
    }

    // Validate token first
    jwt_validation_result_t jwt_result;
    if (jwt_validate_token(access_token, &ctx->keypair, &jwt_result) != 0 ||
        !jwt_result.is_valid) {
        return -1;
    }

    // Add current token to blacklist
    auth_blacklist_jwt(ctx->blacklist_env, jwt_result.claims.jwt_id, jwt_result.claims.expires_at);

    // Delete session if session ID is present in token
    if (strlen(jwt_result.claims.session_id) > 0) {
        session_data_t session;
        if (auth_load_session(ctx->sessions_env, jwt_result.claims.session_id, &session) == 0) {
            // Verify this token matches the session's current access token
            if (strcmp(session.access_token_id, jwt_result.claims.jwt_id) == 0) {
                // Also blacklist the refresh token associated with this session
                auth_blacklist_jwt(ctx->blacklist_env, session.refresh_token_id,
                                  time(NULL) + ctx->config.refresh_token_lifetime);

                // Delete the session
                auth_delete_session(ctx->sessions_env, session.session_id);
            }
        }
    }

    // TODO: Implement logout_all functionality (find and delete all sessions for user)
    // if (logout_all) {
    //     // Would need to iterate through all sessions and find ones for this user
    //     // Then delete them and blacklist their tokens
    // }

    return 0;
}

int auth_get_jwks(auth_context_t *ctx, char *jwks_json, size_t json_len) {
    if (!ctx || !ctx->is_initialized || !jwks_json || json_len == 0) {
        return -1;
    }

    return jwks_generate_json(&ctx->jwks, jwks_json, json_len);
}

int auth_get_user(auth_context_t *ctx,
                 const char *user_id,
                 user_data_t *user_out) {
    if (!ctx || !ctx->is_initialized || !user_id || !user_out) {
        return -1;
    }

    return auth_load_user(ctx->users_env, user_id, user_out);
}

int auth_store_session(MDBX_env *env, const session_data_t *session) {
    if (!env || !session) {
        printf("❌ MDBX STORE DEBUG: Invalid parameters (env=%p, session=%p)\n", env, session);
        return -1;
    }

    printf("🔍 MDBX STORE DEBUG: Storing session '%s' (%zu bytes)\n",
           session->session_id, sizeof(session_data_t));

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX STORE DEBUG: Failed to begin transaction, rc=%d\n", rc);
        return -1;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX STORE DEBUG: Failed to open DBI, rc=%d\n", rc);
        mdbx_txn_abort(txn);
        return -1;
    }

    MDBX_val key = {(void*)session->session_id, strlen(session->session_id)};
    MDBX_val data = {(void*)session, sizeof(session_data_t)};

    printf("📝 MDBX STORE DEBUG: Key='%s', key_size=%zu, data_size=%zu\n",
           session->session_id, key.iov_len, data.iov_len);

    rc = mdbx_put(txn, dbi, &key, &data, 0);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX STORE DEBUG: Failed to put data, rc=%d\n", rc);
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    if (rc == MDBX_SUCCESS) {
        printf("✅ MDBX STORE DEBUG: Session stored successfully\n");
        return 0;
    } else {
        printf("❌ MDBX STORE DEBUG: Failed to commit transaction, rc=%d\n", rc);
        return -1;
    }
}

int auth_load_session(MDBX_env *env, const char *session_id, session_data_t *session_out) {
    if (!env || !session_id || !session_out) {
        printf("❌ MDBX LOAD DEBUG: Invalid parameters (env=%p, session_id=%p, session_out=%p)\n", env, session_id, session_out);
        return -1;
    }

    printf("🔍 MDBX LOAD DEBUG: Looking for session '%s'\n", session_id);

    MDBX_txn *txn = NULL;
    MDBX_dbi dbi = 0;

    int rc = mdbx_txn_begin(env, NULL, MDBX_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX LOAD DEBUG: Failed to begin transaction, rc=%d\n", rc);
        return -1;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        printf("❌ MDBX LOAD DEBUG: Failed to open DBI, rc=%d\n", rc);
        mdbx_txn_abort(txn);
        return -1;
    }

    MDBX_val key = {(void*)session_id, strlen(session_id)};
    MDBX_val data;

    printf("📝 MDBX LOAD DEBUG: Key='%s', key_size=%zu\n", session_id, key.iov_len);

    rc = mdbx_get(txn, dbi, &key, &data);
    printf("📊 MDBX LOAD DEBUG: Get result: %s (rc=%d)\n", rc == MDBX_SUCCESS ? "FOUND" : "NOT FOUND", rc);

    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    if (data.iov_len == sizeof(session_data_t)) {
        memcpy(session_out, data.iov_base, sizeof(session_data_t));
        mdbx_txn_abort(txn);
        return 0;
    }

    mdbx_txn_abort(txn);
    return -1;
}

int auth_delete_session(MDBX_env *env, const char *session_id) {
    if (!env || !session_id) {
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

    MDBX_val key = {(void*)session_id, strlen(session_id)};

    rc = mdbx_del(txn, dbi, &key, NULL);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }

    rc = mdbx_txn_commit(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int auth_generate_session_id(char *session_id_out) {
    if (!session_id_out) {
        return -1;
    }

    // Generate random bytes for session ID (48 bytes = 64 chars in base64url)
    unsigned char random_bytes[48]; // 384 bits
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        return -1;
    }

    // Encode to base64url (will be exactly 64 characters)
    int encoded_len = base64url_encode(random_bytes, sizeof(random_bytes),
                                      session_id_out, 65); // 64 + 1 for null

    if (encoded_len != 64) {
        return -1;
    }

    return 0;
}

int auth_cleanup_expired(auth_context_t *ctx) {
    if (!ctx || !ctx->is_initialized) {
        return -1;
    }

    // TODO: Implement cleanup of expired blacklist entries
    // This would iterate through the blacklist database and remove
    // entries where the expiration time has passed

    return 0;
}

#ifdef AUTH_TEST_MAIN
/**
 * Test program for authentication API
 * Compile with: gcc -DAUTH_TEST_MAIN auth.c -o test_auth
 */
#include <stdio.h>

int main() {
    printf("Testing Authentication API\n");
    printf("==========================\n\n");

    auth_context_t auth_ctx;
    auth_config_t config;
    auth_init_config(&config);
    strcpy(config.database_path, "test_data");

    // Initialize authentication system
    printf("Initializing authentication system...\n");
    if (auth_initialize(&auth_ctx, &config) != 0) {
        printf("✗ Failed to initialize authentication system\n");
        return 1;
    }
    printf("✓ Authentication system initialized\n\n");

    // Test user registration
    printf("Testing user registration...\n");
    auth_result_t reg_result;
    if (auth_register(&auth_ctx, "test@example.com", "SecurePass123", &reg_result) == 0) {
        if (reg_result.success) {
            printf("✓ User registered successfully\n");
            printf("  User ID: %s\n", reg_result.user.user_id);
            printf("  Email: %s\n", reg_result.user.email);
        } else {
            printf("✗ Registration failed: %s\n", reg_result.error_message);
        }
    } else {
        printf("✗ Registration error\n");
    }
    printf("\n");

    // Test user login
    printf("Testing user login...\n");
    auth_result_t login_result;
    if (auth_register(&auth_ctx, "test@example.com", "SecurePass123", &login_result) == 0) {
        if (login_result.success) {
            printf("✓ User logged in successfully\n");
            printf("  Access token length: %zu\n", strlen(login_result.access_token));
            printf("  Refresh token length: %zu\n", strlen(login_result.refresh_token));

            // Test token validation
            printf("\nTesting token validation...\n");
            auth_result_t val_result;
            if (auth_validate_token(&auth_ctx, login_result.access_token, &val_result) == 0) {
                if (val_result.success) {
                    printf("✓ Token is valid\n");
                    printf("  Validated user: %s\n", val_result.user.email);
                } else {
                    printf("✗ Token validation failed: %s\n", val_result.error_message);
                }
            } else {
                printf("✗ Token validation error\n");
            }

            // Test logout
            printf("\nTesting logout...\n");
            if (auth_logout(&auth_ctx, login_result.access_token, 0) == 0) {
                printf("✓ User logged out successfully\n");

                // Verify token is now invalid
                auth_result_t val2_result;
                if (auth_validate_token(&auth_ctx, login_result.access_token, &val2_result) == 0) {
                    if (!val2_result.success) {
                        printf("✓ Token correctly invalidated after logout\n");
                    } else {
                        printf("✗ Token still valid after logout\n");
                    }
                }
            } else {
                printf("✗ Logout failed\n");
            }

        } else {
            printf("✗ Login failed: %s\n", login_result.error_message);
        }
    } else {
        printf("✗ Login error\n");
    }

    // Test JWKS
    printf("\nTesting JWKS generation...\n");
    char jwks_json[4096];
    if (auth_get_jwks(&auth_ctx, jwks_json, sizeof(jwks_json)) == 0) {
        printf("✓ JWKS generated successfully\n");
        printf("  JWKS: %s\n", jwks_json);
    } else {
        printf("✗ JWKS generation failed\n");
    }

    auth_cleanup(&auth_ctx);
    printf("\n✓ All authentication tests completed\n");
    return 0;
}
#endif