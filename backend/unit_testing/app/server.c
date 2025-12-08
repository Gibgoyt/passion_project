#include <libusockets.h>
#include <mdbx.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/rand.h>

/* Configuration */
const int ENABLE_SSL = 1;
const int PORT = 2053;
const char *DB_PATH = "data";
const size_t MAX_BODY_SIZE = 1024 * 1024; // 1MB

/* Global MDBX Environment */
MDBX_env *env = NULL;
MDBX_dbi dbi;

/* Base62 Encoding for User IDs */
static const char BASE62_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int base62_encode(const unsigned char *input, size_t input_len, char *output, size_t output_len) {
    if (!input || !output || input_len == 0 || output_len == 0) return -1;
    
    unsigned char num[21]; // Supports up to 21 bytes input (168 bits)
    if (input_len > sizeof(num)) return -1;
    
    memcpy(num, input, input_len);
    int out_pos = 0;

    while (out_pos < (int)(output_len - 1)) {
        int is_zero = 1;
        for (size_t i = 0; i < input_len; i++) {
            if (num[i] != 0) { is_zero = 0; break; }
        }
        if (is_zero && out_pos > 0) break;

        int remainder = 0;
        for (int i = 0; i < (int)input_len; i++) {
            int temp = remainder * 256 + num[i];
            num[i] = temp / 62;
            remainder = temp % 62;
        }
        output[out_pos++] = BASE62_CHARS[remainder];
    }

    // Pad if needed (for consistent 28 chars)
    while (out_pos < 28 && out_pos < (int)(output_len - 1)) {
        output[out_pos++] = BASE62_CHARS[0];
    }

    // Reverse
    for (int i = 0; i < out_pos / 2; i++) {
        char temp = output[i];
        output[i] = output[out_pos - 1 - i];
        output[out_pos - 1 - i] = temp;
    }
    output[out_pos] = '\0';
    return 0;
}

void generate_firebase_userid(char *userid_out) {
    unsigned char random_bytes[21]; // 168 bits
    RAND_bytes(random_bytes, sizeof(random_bytes));
    base62_encode(random_bytes, sizeof(random_bytes), userid_out, 29); // 28 chars + null
}

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
void route_request(struct us_socket_t *s, const char *method, const char *url, const char *body) {
    // Simple routing
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
    struct socket_context *ctx = (struct socket_context *)us_socket_ext(ENABLE_SSL, s);
    ctx->buffer = malloc(4096);
    ctx->capacity = 4096;
    ctx->length = 0;
    ctx->offset = 0;
    return s;
}

struct us_socket_t *on_http_close(struct us_socket_t *s, int code, void *reason) {
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
    // Note: This is a simplified HTTP parser for the sake of "very simple server"
    // A robust one would parse Content-Length properly. 
    // Here we assume one packet or accumulation until \r\n\r\n + body.
    
    char *header_end = strstr(ctx->buffer, "\r\n\r\n");
    if (header_end) {
        // Check content length
        size_t header_len = (header_end - ctx->buffer) + 4;
        size_t body_len = 0;
        
        char *cl = strstr(ctx->buffer, "Content-Length: ");
        if (cl && cl < header_end) {
            body_len = atoi(cl + 16);
        }
        
        if (ctx->length >= header_len + body_len) {
            // We have the full request
            
            // Null terminate body
            char *body = header_end + 4;
            // Remove unused variable warning
            // char saved_char = body[body_len];
            body[body_len] = '\0';
            
            // Parse Method and URL
            char method[16] = {0};
            char url[256] = {0};
            sscanf(ctx->buffer, "%15s %255s", method, url);
            
            route_request(s, method, url, body);
            
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
    
    if (init_db() != 0) {
        fprintf(stderr, "Failed to initialize MDBX\n");
        return 1;
    }

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
        us_loop_run(loop);
    } else {
        fprintf(stderr, "Failed to listen on port %d\n", PORT);
    }

    return 0;
}