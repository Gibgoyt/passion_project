#include <libusockets.h>
#include <mdbx.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/rand.h>

// New Libraries Includes
#include "../../libraries/auth_lib/auth.h"
#include "../../libraries/auth_lib/oauth.h"
#include "../../libraries/auth_lib/json_utils.h"
#include "../../libraries/memory/platform_detection.h"
#include "../../libraries/memory/jwt_storage_compat.h"

/* Configuration */
const int ENABLE_SSL = 1;
const int PORT = 2053;
const char *DB_PATH = "data";
const char *AUTH_DB_PATH = "auth_data"; // Separate DB for auth to avoid locking issues
const size_t MAX_BODY_SIZE = 1024 * 1024; // 1MB
#define MAX_REQUEST_SIZE 4096 // Defined for buffer sizes

/* Global Contexts */
MDBX_env *env = NULL;
MDBX_dbi dbi;
static auth_context_t auth_ctx;

/* Base62 Encoding for User IDs */
static const char BASE62_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

// Removed local generate_firebase_userid to use library function

/* Socket Extension Data */
struct socket_context {
    char *buffer;
    size_t length;
    size_t capacity;
    int offset; // For streaming response if needed
};

/* Helper: Get ISO8601 Time */
void get_iso8601_time(char *buffer, size_t size) {
    time_t now;
    time(&now);
    struct tm *t = gmtime(&now);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", t);
}

/* Helper: Send JSON Response */
void send_json_response(struct us_socket_t *s, int status, cJSON *json) {
    char *body = cJSON_PrintUnformatted(json);
    size_t body_len = strlen(body);
    char headers[1024];
    
    int header_len = snprintf(headers, sizeof(headers),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        status == 200 ? "OK" : (status == 201 ? "Created" : (status == 404 ? "Not Found" : (status == 400 ? "Bad Request" : "Internal Server Error"))),
        body_len
    );

    us_socket_write(ENABLE_SSL, s, headers, header_len, 0);
    us_socket_write(ENABLE_SSL, s, body, body_len, 0);
    us_socket_close(ENABLE_SSL, s, 0, NULL);
    
    free(body);
}

/* Helper: Send Raw JSON Response (from string buffer) */
void send_raw_json_response(struct us_socket_t *s, int status, const char *json_body) {
    size_t body_len = strlen(json_body);
    char headers[1024];
    
    int header_len = snprintf(headers, sizeof(headers),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        status == 200 ? "OK" : (status == 201 ? "Created" : (status == 404 ? "Not Found" : (status == 400 ? "Bad Request" : "Internal Server Error"))),
        body_len
    );

    us_socket_write(ENABLE_SSL, s, headers, header_len, 0);
    us_socket_write(ENABLE_SSL, s, json_body, body_len, 0);
    us_socket_close(ENABLE_SSL, s, 0, NULL);
}

/* Helper: Send OPTIONS Response for CORS */
void send_options_response(struct us_socket_t *s) {
    char headers[1024];
    int header_len = snprintf(headers, sizeof(headers),
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Connection: close\r\n"
        "\r\n"
    );
    us_socket_write(ENABLE_SSL, s, headers, header_len, 0);
    us_socket_close(ENABLE_SSL, s, 0, NULL);
}

/* Helper: Send Error Response */
void send_error(struct us_socket_t *s, int status, const char *message) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "error", message);
    send_json_response(s, status, json);
    cJSON_Delete(json);
}

/* Loop event handlers */
void on_wakeup(struct us_loop_t *loop) {
    (void)loop;
}

void on_pre(struct us_loop_t *loop) {
    (void)loop;
}

void on_post(struct us_loop_t *loop) {
    (void)loop;
}

/* Initialize MDBX */
int init_db() {
    int rc;
    
    // Create/Open Environment
    rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) return -1;
    
    // Set Limits
    rc = mdbx_env_set_geometry(env, -1, -1, 10485760, -1, -1, -1); // 10MB max
    if (rc != MDBX_SUCCESS) return -1;
    
    rc = mdbx_env_open(env, DB_PATH, MDBX_NOSUBDIR | MDBX_LIFORECLAIM, 0664);
    if (rc != MDBX_SUCCESS) return -1;
    
    // Open Transaction & DBI
    MDBX_txn *txn;
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) return -1;
    
    rc = mdbx_dbi_open(txn, NULL, MDBX_CREATE, &dbi);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        return -1;
    }
    
    rc = mdbx_txn_commit(txn);
    return (rc == MDBX_SUCCESS) ? 0 : -1;
}

/* -------------------------------------------------------------------------
 * AUTHENTICATION HANDLERS (Ported from simple_auth)
 * ------------------------------------------------------------------------- */

void handle_auth_register(struct us_socket_t *s, const char *body) {
    register_request_t req;
    if (parse_register_request(body, &req) != 0) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    auth_result_t result;
    if (auth_register(&auth_ctx, req.email, req.password, &result) == 0) {
        register_response_t resp;
        memset(&resp, 0, sizeof(register_response_t));
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.user_id, result.user.user_id);
            strcpy(resp.message, "Registration successful");
        } else {
            strcpy(resp.error, result.error_message);
        }

        char json_buf[MAX_REQUEST_SIZE];
        generate_register_response(&resp, json_buf, sizeof(json_buf));
        send_raw_json_response(s, result.success ? 201 : 400, json_buf);
    } else {
        send_error(s, 500, "Internal server error");
    }
}

void handle_auth_login(struct us_socket_t *s, const char *body) {
    login_request_t req;
    if (parse_login_request(body, &req) != 0) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    auth_result_t result;
    if (auth_login(&auth_ctx, req.email, req.password, "AstroApp/1.0", "127.0.0.1", &result) == 0) {
        login_response_t resp;
        memset(&resp, 0, sizeof(login_response_t));
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.access_token, result.access_token);
            strcpy(resp.refresh_token, result.refresh_token);
            strcpy(resp.user_id, result.user.user_id);
            resp.error[0] = '\0';
        } else {
            strcpy(resp.error, result.error_message);
        }

        char json_buf[MAX_REQUEST_SIZE];
        generate_login_response(&resp, json_buf, sizeof(json_buf));
        send_raw_json_response(s, result.success ? 200 : 401, json_buf);
    } else {
        send_error(s, 500, "Internal server error");
    }
}

void handle_auth_validate(struct us_socket_t *s, const char *auth_header) {
    if (!auth_header || strncmp(auth_header, "Bearer ", 7) != 0) {
        send_error(s, 401, "Missing or invalid Authorization header");
        return;
    }

    const char *token = auth_header + 7;
    auth_result_t result;
    validate_response_t resp;

    if (auth_validate_token(&auth_ctx, token, &result) == 0) {
        resp.valid = result.success;

        if (result.success) {
            strcpy(resp.user_id, result.user.user_id);
            strcpy(resp.email, result.user.email);
            resp.email_verified = result.user.email_verified;
            
            char json_buf[MAX_REQUEST_SIZE];
            generate_validate_response(&resp, json_buf, sizeof(json_buf));
            send_raw_json_response(s, 200, json_buf);
        } else {
            strcpy(resp.error, result.error_message);
            char json_buf[MAX_REQUEST_SIZE];
            generate_validate_response(&resp, json_buf, sizeof(json_buf));
            send_raw_json_response(s, 401, json_buf);
        }
    } else {
        send_error(s, 500, "Internal server error");
    }
}

void handle_auth_refresh(struct us_socket_t *s, const char *body) {
    refresh_request_t req;
    if (parse_refresh_request(body, &req) != 0) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    auth_result_t result;
    if (auth_refresh(&auth_ctx, req.refresh_token, &result) == 0) {
        login_response_t resp;
        memset(&resp, 0, sizeof(login_response_t));
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.access_token, result.access_token);
            strcpy(resp.refresh_token, result.refresh_token);
            strcpy(resp.user_id, result.user.user_id);
            resp.error[0] = '\0';
            
            char json_buf[MAX_REQUEST_SIZE];
            generate_login_response(&resp, json_buf, sizeof(json_buf));
            send_raw_json_response(s, 200, json_buf);
        } else {
            strcpy(resp.error, result.error_message);
            char json_buf[MAX_REQUEST_SIZE];
            generate_login_response(&resp, json_buf, sizeof(json_buf));
            send_raw_json_response(s, 401, json_buf);
        }
    } else {
        send_error(s, 500, "Internal server error");
    }
}

void handle_auth_logout(struct us_socket_t *s, const char *auth_header) {
    if (!auth_header || strncmp(auth_header, "Bearer ", 7) != 0) {
        send_error(s, 401, "Missing or invalid Authorization header");
        return;
    }

    const char *token = auth_header + 7;
    
    if (auth_logout(&auth_ctx, token, 0) == 0) {
        char json_buf[MAX_REQUEST_SIZE];
        generate_success_response("Logout successful", json_buf, sizeof(json_buf));
        send_raw_json_response(s, 200, json_buf);
    } else {
        send_error(s, 500, "Logout failed");
    }
}

void handle_oauth_authorize_init(struct us_socket_t *s, const char *body) {
    oauth_init_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    cJSON *client_id = cJSON_GetObjectItemCaseSensitive(json, "client_id");
    cJSON *redirect_uri = cJSON_GetObjectItemCaseSensitive(json, "redirect_uri");
    cJSON *code_challenge = cJSON_GetObjectItemCaseSensitive(json, "code_challenge");
    cJSON *code_challenge_method = cJSON_GetObjectItemCaseSensitive(json, "code_challenge_method");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(json, "state");

    if (!cJSON_IsString(client_id) || !cJSON_IsString(redirect_uri) ||
        !cJSON_IsString(code_challenge) || !cJSON_IsString(code_challenge_method)) {
        cJSON_Delete(json);
        send_error(s, 400, "Missing required parameters");
        return;
    }

    strncpy(oauth_req.client_id, client_id->valuestring, sizeof(oauth_req.client_id) - 1);
    strncpy(oauth_req.redirect_uri, redirect_uri->valuestring, sizeof(oauth_req.redirect_uri) - 1);
    strncpy(oauth_req.code_challenge, code_challenge->valuestring, sizeof(oauth_req.code_challenge) - 1);
    strncpy(oauth_req.code_challenge_method, code_challenge_method->valuestring, sizeof(oauth_req.code_challenge_method) - 1);
    if (cJSON_IsString(state)) strncpy(oauth_req.state, state->valuestring, sizeof(oauth_req.state) - 1);

    cJSON_Delete(json);

    oauth_result_t result;
    if (oauth_authorize_init(&auth_ctx, &oauth_req, "AstroApp/1.0", "127.0.0.1", &result) == 0) {
        if (result.success) {
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "session_id", result.data.init.session_id);
            cJSON_AddStringToObject(resp_json, "authorization_url", result.data.init.authorization_url);
            cJSON_AddNumberToObject(resp_json, "expires_in", result.data.init.expires_in);
            send_json_response(s, 200, resp_json);
            cJSON_Delete(resp_json);
        } else {
            send_error(s, 400, result.error_description);
        }
    } else {
        send_error(s, 500, "OAuth initialization failed");
    }
}

void handle_oauth_authorize_complete(struct us_socket_t *s, const char *body) {
    oauth_complete_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    cJSON *session_id = cJSON_GetObjectItemCaseSensitive(json, "session_id");
    cJSON *email = cJSON_GetObjectItemCaseSensitive(json, "email");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(json, "password");
    cJSON *consent_granted = cJSON_GetObjectItemCaseSensitive(json, "consent_granted");

    if (!cJSON_IsString(session_id) || !cJSON_IsString(email) ||
        !cJSON_IsString(password) || !cJSON_IsBool(consent_granted)) {
        cJSON_Delete(json);
        send_error(s, 400, "Missing required parameters");
        return;
    }

    strncpy(oauth_req.session_id, session_id->valuestring, sizeof(oauth_req.session_id) - 1);
    strncpy(oauth_req.email, email->valuestring, sizeof(oauth_req.email) - 1);
    strncpy(oauth_req.password, password->valuestring, sizeof(oauth_req.password) - 1);
    oauth_req.consent_granted = cJSON_IsTrue(consent_granted) ? 1 : 0;
    cJSON_Delete(json);

    oauth_result_t result;
    if (oauth_authorize_complete(&auth_ctx, &oauth_req, &result) == 0) {
        if (result.success) {
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "authorization_code", result.data.complete.authorization_code);
            cJSON_AddStringToObject(resp_json, "redirect_uri", result.data.complete.redirect_uri);
            cJSON_AddStringToObject(resp_json, "state", result.data.complete.state);
            send_json_response(s, 200, resp_json);
            cJSON_Delete(resp_json);
        } else {
            send_error(s, 400, result.error_description);
        }
    } else {
        send_error(s, 500, "OAuth completion failed");
    }
}

void handle_oauth_token(struct us_socket_t *s, const char *body) {
    oauth_token_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_error(s, 400, "Invalid JSON format");
        return;
    }

    cJSON *grant_type = cJSON_GetObjectItemCaseSensitive(json, "grant_type");
    cJSON *code = cJSON_GetObjectItemCaseSensitive(json, "code");
    cJSON *code_verifier = cJSON_GetObjectItemCaseSensitive(json, "code_verifier");
    cJSON *client_id = cJSON_GetObjectItemCaseSensitive(json, "client_id");
    cJSON *redirect_uri = cJSON_GetObjectItemCaseSensitive(json, "redirect_uri");

    if (!cJSON_IsString(grant_type) || !cJSON_IsString(code) ||
        !cJSON_IsString(code_verifier) || !cJSON_IsString(client_id) ||
        !cJSON_IsString(redirect_uri)) {
        cJSON_Delete(json);
        send_error(s, 400, "Missing required parameters");
        return;
    }

    strncpy(oauth_req.grant_type, grant_type->valuestring, sizeof(oauth_req.grant_type) - 1);
    strncpy(oauth_req.code, code->valuestring, sizeof(oauth_req.code) - 1);
    strncpy(oauth_req.code_verifier, code_verifier->valuestring, sizeof(oauth_req.code_verifier) - 1);
    strncpy(oauth_req.client_id, client_id->valuestring, sizeof(oauth_req.client_id) - 1);
    strncpy(oauth_req.redirect_uri, redirect_uri->valuestring, sizeof(oauth_req.redirect_uri) - 1);
    cJSON_Delete(json);

    oauth_result_t result;
    if (oauth_token_exchange(&auth_ctx, &oauth_req, &result) == 0) {
        if (result.success) {
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "access_token", result.data.token.access_token);
            cJSON_AddStringToObject(resp_json, "refresh_token", result.data.token.refresh_token);
            cJSON_AddStringToObject(resp_json, "token_type", result.data.token.token_type);
            cJSON_AddNumberToObject(resp_json, "expires_in", result.data.token.expires_in);
            send_json_response(s, 200, resp_json);
            cJSON_Delete(resp_json);
        } else {
            send_error(s, 400, result.error_description);
        }
    } else {
        send_error(s, 500, "OAuth token exchange failed");
    }
}

void handle_jwks(struct us_socket_t *s) {
    char jwks_json[4096];
    if (auth_get_jwks(&auth_ctx, jwks_json, sizeof(jwks_json)) == 0) {
        send_raw_json_response(s, 200, jwks_json);
    } else {
        send_error(s, 500, "Failed to generate JWKS");
    }
}


/* -------------------------------------------------------------------------
 * USER HANDLERS (Existing)
 * ------------------------------------------------------------------------- */
/* Handler: GET /api/v1/users */
void handle_get_users(struct us_socket_t *s) {
    MDBX_txn *txn;
    int rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        send_error(s, 500, "Database error");
        return;
    }

    MDBX_cursor *cursor;
    rc = mdbx_cursor_open(txn, dbi, &cursor);
    if (rc != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        send_error(s, 500, "Cursor error");
        return;
    }

    cJSON *users_array = cJSON_CreateArray();
    MDBX_val key, data;
    
    rc = mdbx_cursor_get(cursor, &key, &data, MDBX_FIRST);
    while (rc == MDBX_SUCCESS) {
        // Data is stored as JSON string
        cJSON *user_json = cJSON_Parse((const char *)data.iov_base);
        if (user_json) {
            // Add ID to the object if it's not there (it's the key)
            char *key_str = strndup((char*)key.iov_base, key.iov_len);
            cJSON_AddStringToObject(user_json, "id", key_str);
            free(key_str);
            cJSON_AddItemToArray(users_array, user_json);
        }
        rc = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT);
    }

    mdbx_cursor_close(cursor);
    mdbx_txn_abort(txn);

    send_json_response(s, 200, users_array);
    cJSON_Delete(users_array);
}

/* Handler: GET /api/v1/user/<userId> */
void handle_get_user(struct us_socket_t *s, const char *user_id) {
    MDBX_txn *txn;
    int rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        send_error(s, 500, "Database error");
        return;
    }

    MDBX_val key = {(void*)user_id, strlen(user_id)};
    MDBX_val data;
    
    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc == MDBX_SUCCESS) {
        cJSON *user_json = cJSON_Parse((const char *)data.iov_base);
        if (user_json) {
            cJSON_AddStringToObject(user_json, "id", user_id);
            mdbx_txn_abort(txn);
            send_json_response(s, 200, user_json);
            cJSON_Delete(user_json);
            return;
        }
    }

    mdbx_txn_abort(txn);
    send_error(s, 404, "User not found");
}

/* Handler: POST /api/v1/users */
void handle_create_user(struct us_socket_t *s, const char *body) {
    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_error(s, 400, "Invalid JSON");
        return;
    }

    cJSON *name = cJSON_GetObjectItem(json, "name");
    cJSON *surname = cJSON_GetObjectItem(json, "surname");

    if (!cJSON_IsString(name) || !cJSON_IsString(surname) || 
        strlen(name->valuestring) > 64 || strlen(surname->valuestring) > 64) {
        cJSON_Delete(json);
        send_error(s, 400, "Invalid name or surname");
        return;
    }

    // Generate ID (Firebase-style)
    char user_id[64];
    generate_firebase_userid(user_id);

    // Add timestamps
    char now_str[32];
    get_iso8601_time(now_str, sizeof(now_str));
    
    cJSON_AddStringToObject(json, "createdAt", now_str);
    cJSON_AddStringToObject(json, "updatedAt", now_str);

    // Store in DB
    char *stored_json_str = cJSON_PrintUnformatted(json);
    
    MDBX_txn *txn;
    if (mdbx_txn_begin(env, NULL, 0, &txn) != MDBX_SUCCESS) {
        free(stored_json_str);
        cJSON_Delete(json);
        send_error(s, 500, "Database error");
        return;
    }

    MDBX_val key = {user_id, strlen(user_id)};
    MDBX_val data = {stored_json_str, strlen(stored_json_str) + 1}; // Include null terminator

    if (mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE) == MDBX_SUCCESS) {
        mdbx_txn_commit(txn);
        cJSON_AddStringToObject(json, "id", user_id);
        send_json_response(s, 201, json);
    } else {
        mdbx_txn_abort(txn);
        send_error(s, 500, "Failed to store user");
    }

    free(stored_json_str);
    cJSON_Delete(json);
}

/* Handler: PUT /api/v1/user/<userId> */
void handle_update_user(struct us_socket_t *s, const char *user_id, const char *body) {
    cJSON *json = cJSON_Parse(body);
    if (!json) {
        send_error(s, 400, "Invalid JSON");
        return;
    }

    MDBX_txn *txn;
    if (mdbx_txn_begin(env, NULL, 0, &txn) != MDBX_SUCCESS) {
        cJSON_Delete(json);
        send_error(s, 500, "Database error");
        return;
    }

    MDBX_val key = {(void*)user_id, strlen(user_id)};
    MDBX_val data;
    
    // Check existence
    if (mdbx_get(txn, dbi, &key, &data) != MDBX_SUCCESS) {
        mdbx_txn_abort(txn);
        cJSON_Delete(json);
        send_error(s, 404, "User not found");
        return;
    }

    // Parse existing to preserve createdAt
    cJSON *existing = cJSON_Parse((char*)data.iov_base);
    // Remove Unused variable warning
    // cJSON *created_at = cJSON_GetObjectItem(existing, "createdAt");
    
    // Update fields
    cJSON *name = cJSON_GetObjectItem(json, "name");
    if (cJSON_IsString(name)) {
        cJSON_ReplaceItemInObject(existing, "name", cJSON_Duplicate(name, 1));
    }
    cJSON *surname = cJSON_GetObjectItem(json, "surname");
    if (cJSON_IsString(surname)) {
        cJSON_ReplaceItemInObject(existing, "surname", cJSON_Duplicate(surname, 1));
    }

    // Update timestamp
    char now_str[32];
    get_iso8601_time(now_str, sizeof(now_str));
    cJSON_ReplaceItemInObject(existing, "updatedAt", cJSON_CreateString(now_str));

    // Save back
    char *new_stored_str = cJSON_PrintUnformatted(existing);
    data.iov_base = new_stored_str;
    data.iov_len = strlen(new_stored_str) + 1;

    if (mdbx_put(txn, dbi, &key, &data, 0) == MDBX_SUCCESS) {
        mdbx_txn_commit(txn);
        cJSON_AddStringToObject(existing, "id", user_id);
        send_json_response(s, 200, existing);
    } else {
        mdbx_txn_abort(txn);
        send_error(s, 500, "Failed to update user");
    }

    free(new_stored_str);
    cJSON_Delete(existing);
    cJSON_Delete(json);
}

/* Handler: DELETE /api/v1/user/<userId> */
void handle_delete_user(struct us_socket_t *s, const char *user_id) {
    printf("DELETE Request for User ID: '%s'\n", user_id); // Debug log

    MDBX_txn *txn;
    if (mdbx_txn_begin(env, NULL, 0, &txn) != MDBX_SUCCESS) {
        send_error(s, 500, "Database error");
        return;
    }

    MDBX_val key = {(void*)user_id, strlen(user_id)};
    
    int rc = mdbx_del(txn, dbi, &key, NULL);
    if (rc == MDBX_SUCCESS) {
        mdbx_txn_commit(txn);
        cJSON *json = cJSON_CreateObject();
        cJSON_AddBoolToObject(json, "success", 1);
        send_json_response(s, 200, json);
        cJSON_Delete(json);
    } else {
        printf("DELETE Failed: rc=%d (%s)\n", rc, mdbx_strerror(rc)); // Debug log
        mdbx_txn_abort(txn);
        send_error(s, 404, "User not found");
    }
}

/* Router */
void route_request(struct us_socket_t *s, const char *method, const char *url, const char *body, const char *auth_header) {
    // Handle Preflight OPTIONS request
    if (strcmp(method, "OPTIONS") == 0) {
        send_options_response(s);
        return;
    }

    // AUTH V1 Endpoints
    if (strncmp(url, "/auth/v1/register", 17) == 0 && strcmp(method, "POST") == 0) {
        handle_auth_register(s, body);
        return;
    }
    if (strncmp(url, "/auth/v1/login", 14) == 0 && strcmp(method, "POST") == 0) {
        handle_auth_login(s, body);
        return;
    }
    if (strncmp(url, "/auth/v1/validate", 17) == 0 && (strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0)) {
        handle_auth_validate(s, auth_header);
        return;
    }
    if (strncmp(url, "/auth/v1/refresh", 16) == 0 && strcmp(method, "POST") == 0) {
        handle_auth_refresh(s, body);
        return;
    }
    if (strncmp(url, "/auth/v1/logout", 15) == 0 && strcmp(method, "POST") == 0) {
        handle_auth_logout(s, auth_header);
        return;
    }

    // OAUTH V1 Endpoints
    if (strncmp(url, "/oauth/v1/authorize/init", 24) == 0 && strcmp(method, "POST") == 0) {
        handle_oauth_authorize_init(s, body);
        return;
    }
    if (strncmp(url, "/oauth/v1/authorize/complete", 28) == 0 && strcmp(method, "POST") == 0) {
        handle_oauth_authorize_complete(s, body);
        return;
    }
    if (strncmp(url, "/oauth/v1/token", 15) == 0 && strcmp(method, "POST") == 0) {
        handle_oauth_token(s, body);
        return;
    }
    if (strncmp(url, "/.well-known/jwks.json", 22) == 0 && strcmp(method, "GET") == 0) {
        handle_jwks(s);
        return;
    }

    // API V1 USERS Endpoints (Legacy/App Specific)
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/api/v1/users") == 0) {
            handle_get_users(s);
        } else if (strncmp(url, "/api/v1/users/", 14) == 0) {
            handle_get_user(s, url + 14);
        } else {
            send_error(s, 404, "Not Found");
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(url, "/api/v1/users") == 0) {
            handle_create_user(s, body);
        } else {
            send_error(s, 404, "Not Found");
        }
    } else if (strcmp(method, "PUT") == 0) {
        if (strncmp(url, "/api/v1/users/", 14) == 0) {
            handle_update_user(s, url + 14, body);
        } else {
            send_error(s, 404, "Not Found");
        }
    } else if (strcmp(method, "DELETE") == 0) {
        if (strncmp(url, "/api/v1/users/", 14) == 0) {
            handle_delete_user(s, url + 14);
        } else {
            send_error(s, 404, "Not Found");
        }
    } else {
        send_error(s, 405, "Method Not Allowed");
    }
}

/* uSockets Event Handlers */
struct us_socket_t *on_http_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    (void)is_client;
    (void)ip;
    (void)ip_length;
    struct socket_context *ctx = (struct socket_context *)us_socket_ext(ENABLE_SSL, s);
    ctx->buffer = malloc(4096);
    ctx->capacity = 4096;
    ctx->length = 0;
    ctx->offset = 0;
    return s;
}

struct us_socket_t *on_http_close(struct us_socket_t *s, int code, void *reason) {
    (void)code;
    (void)reason;
    struct socket_context *ctx = (struct socket_context *)us_socket_ext(ENABLE_SSL, s);
    if (ctx->buffer) free(ctx->buffer);
    return s;
}

struct us_socket_t *on_http_data(struct us_socket_t *s, char *data, int length) {
    struct socket_context *ctx = (struct socket_context *)us_socket_ext(ENABLE_SSL, s);
    
    // Grow buffer if needed
    if (ctx->length + length > ctx->capacity) {
        size_t new_cap = ctx->capacity * 2;
        if (new_cap > MAX_BODY_SIZE) new_cap = MAX_BODY_SIZE;
        if (ctx->length + length > new_cap) {
            send_error(s, 413, "Request too large");
            return s;
        }
        ctx->buffer = realloc(ctx->buffer, new_cap);
        ctx->capacity = new_cap;
    }
    
    memcpy(ctx->buffer + ctx->length, data, length);
    ctx->length += length;
    
    // Simple check for full request (header end)
    char *header_end = strstr(ctx->buffer, "\r\n\r\n");
    if (header_end) {
        // Debug: Print request headers
        printf("Received Request:\n%.*s\n", (int)(header_end - ctx->buffer), ctx->buffer);

        // Check content length (case-insensitive attempt)
        size_t header_len = (header_end - ctx->buffer) + 4;
        size_t body_len = 0;
        
        char *cl = strstr(ctx->buffer, "Content-Length: ");
        if (!cl) cl = strstr(ctx->buffer, "content-length: ");
        if (!cl) cl = strstr(ctx->buffer, "Content-length: ");
        
        if (cl && cl < header_end) {
            body_len = atoi(cl + 16);
        }
        
        printf("Body Length: %zu\n", body_len);

        if (ctx->length >= header_len + body_len) {
            // We have the full request
            
            // Null terminate body
            char *body = header_end + 4;
            // Remove unused variable warning
            // char saved_char = body[body_len];
            body[body_len] = '\0';

            // Extract Headers (Naive)
            char *auth_header = NULL;
            char *auth_ptr = strstr(ctx->buffer, "Authorization: ");
            if (!auth_ptr) auth_ptr = strstr(ctx->buffer, "authorization: ");
            
            if (auth_ptr && auth_ptr < header_end) {
                // Determine length until next CRLF
                char *end_of_line = strstr(auth_ptr, "\r\n");
                if (end_of_line && end_of_line < header_end) {
                    size_t len = end_of_line - auth_ptr;
                    // Temporary stack buffer for header value if needed, 
                    // but we can just pass the pointer if we assume it's null-terminated by standard or we copy it.
                    // Let's allocate a temp string to be safe and clean.
                    auth_header = strndup(auth_ptr + 15, len - 15); // Skip "Authorization: "
                    // Actually, let's keep the whole line or just the value? The handlers expect "Bearer ..." usually.
                    // Let's pass the value.
                }
            }
            
            // Parse Method and URL
            char method[16] = {0};
            char url[256] = {0};
            sscanf(ctx->buffer, "%15s %255s", method, url);
            
            route_request(s, method, url, body, auth_header);
            
            if (auth_header) free(auth_header);
            
            // Reset for keep-alive or close (we close for simplicity)
            // ctx->length = 0; // If keep-alive
        }
    }
    
    return s;
}

struct us_socket_t *on_http_writable(struct us_socket_t *s) {
    return s;
}

struct us_socket_t *on_http_timeout(struct us_socket_t *s) {
    return us_socket_close(ENABLE_SSL, s, 0, NULL);
}

struct us_socket_t *on_http_end(struct us_socket_t *s) {
    return us_socket_close(ENABLE_SSL, s, 0, NULL);
}

/* Main */
int main() {
    srand(time(NULL));
    
    // Initialize Memory System
    if (memory_system_init() != 0) {
        fprintf(stderr, "Failed to initialize memory system\n");
        return 1;
    }

    if (init_db() != 0) {
        fprintf(stderr, "Failed to initialize MDBX\n");
        return 1;
    }

    // Initialize Authentication System
    auth_config_t config;
    auth_init_config(&config);
    strncpy(config.database_path, AUTH_DB_PATH, sizeof(config.database_path) - 1);
    config.database_path[sizeof(config.database_path) - 1] = '\0';
    
    if (auth_initialize(&auth_ctx, &config) != 0) {
        fprintf(stderr, "Failed to initialize auth system\n");
        return 1;
    }
    printf("Auth System Initialized. DB: %s, KeyID: %s\n", config.database_path, auth_ctx.keypair.key_id);


    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);
    
    struct us_socket_context_options_t options = {};
    options.key_file_name = "../../../unit_testing/http11_server/certs/server.key";
    options.cert_file_name = "../../../unit_testing/http11_server/certs/server.crt";
    options.passphrase = "";

    struct us_socket_context_t *context = us_create_socket_context(ENABLE_SSL, loop, sizeof(struct socket_context), options);
    
    if (!context) {
        fprintf(stderr, "Failed to create SSL context (check cert paths)\n");
        return 1;
    }

    us_socket_context_on_open(ENABLE_SSL, context, on_http_open);
    us_socket_context_on_data(ENABLE_SSL, context, on_http_data);
    us_socket_context_on_writable(ENABLE_SSL, context, on_http_writable);
    us_socket_context_on_close(ENABLE_SSL, context, on_http_close);
    us_socket_context_on_timeout(ENABLE_SSL, context, on_http_timeout);
    us_socket_context_on_end(ENABLE_SSL, context, on_http_end);

    struct us_listen_socket_t *listen_socket = us_socket_context_listen(ENABLE_SSL, context, 0, PORT, 0, sizeof(struct socket_context));

    if (listen_socket) {
        printf("Server listening on https://localhost:%d\n", PORT);
        printf("Endpoints available:\n");
        printf("  - /api/v1/users (CRUD)\n");
        printf("  - /auth/v1/* (Register, Login, etc.)\n");
        printf("  - /oauth/v1/* (Authorize, Token)\n");
        us_loop_run(loop);
    } else {
        fprintf(stderr, "Failed to listen on port %d\n", PORT);
    }

    auth_cleanup(&auth_ctx);
    return 0;
}
