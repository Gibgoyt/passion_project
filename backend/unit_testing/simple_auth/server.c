/**
 * Simple Authentication HTTP Server
 *
 * HTTP server using libusockets that provides authentication endpoints:
 * - POST /auth/register - User registration
 * - POST /auth/login/ropc - User login (Resource Owner Password Credentials)
 * - POST /auth/refresh - Token refresh
 * - POST /auth/logout - User logout
 * - POST /auth/validate - Token validation
 * - POST /oauth/authorize/init - Initialize OAuth 2.1 PKCE flow
 * - POST /oauth/authorize/complete - Complete OAuth authorization
 * - POST /oauth/token - Exchange authorization code for tokens
 * - GET /.well-known/jwks.json - Public keys for JWT verification
 *
 * Uses SSL/TLS for secure communication and follows RESTful API patterns.
 */

#define _GNU_SOURCE

#include "auth_lib/auth.h"
#include "auth_lib/oauth.h"
#include "auth_lib/json_utils.h"
#include "memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// libusockets includes
#include <libusockets.h>

// Configuration
#define SERVER_PORT 2053
#define MAX_REQUEST_SIZE 4096
#define MAX_RESPONSE_SIZE 8192
#define SSL 1  // MUST use SSL with /etc/ssl/splitdo_api/ certificates

// Global authentication context
static auth_context_t auth_ctx;
static int server_running = 1;

// Request structure for parsing HTTP requests with page-allocated JWT storage
typedef struct {
    char method[16];
    char path[256];
    char body[MAX_REQUEST_SIZE];
    size_t body_length;
    char content_type[128];
    crypto_buffer_t* jwt_auth;  // Platform-specific JWT storage (replaces authorization[512])
} http_request_t;

// Response structure for building HTTP responses
typedef struct {
    int status_code;
    char content_type[128];
    char body[MAX_RESPONSE_SIZE];
    size_t body_length;
} http_response_t;

// Socket state structure for buffering partial HTTP requests
typedef struct {
    char buffer[MAX_REQUEST_SIZE];
    size_t buffer_used;
    int request_complete;
} socket_state_t;

/**
 * Initialize HTTP request structure with JWT storage
 */
void init_http_request(http_request_t* request) {
    if (!request) return;

    memset(request, 0, sizeof(http_request_t));

    // Create platform-specific crypto buffer for JWT storage
    request->jwt_auth = platform_crypto_buffer_alloc(2048); // Max JWT size
    if (!request->jwt_auth) {
        printf("⚠️ Failed to create JWT storage for HTTP request\n");
    }
}

/**
 * Cleanup HTTP request structure and JWT storage
 */
void cleanup_http_request(http_request_t* request) {
    if (!request) return;

    if (request->jwt_auth) {
        platform_crypto_buffer_free(request->jwt_auth);
        request->jwt_auth = NULL;
    }
}

/**
 * Signal handler for graceful shutdown
 */
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("\n🛑 Received shutdown signal, stopping server...\n");
        server_running = 0;
    }
}

/**
 * Loop event handlers (required by libusockets)
 */
void on_wakeup(struct us_loop_t *loop) {
    (void)loop; // Suppress unused parameter warning
}

void on_pre(struct us_loop_t *loop) {
    (void)loop; // Suppress unused parameter warning
}

void on_post(struct us_loop_t *loop) {
    (void)loop; // Suppress unused parameter warning
}

/**
 * Parse HTTP request from raw data with page-allocated JWT storage
 */
int parse_http_request(const char *data, size_t length, http_request_t *request) {
    if (!data || !request || length == 0) {
        return -1;
    }

    // Initialize request with JWT storage
    init_http_request(request);

    // Find the end of headers (double CRLF)
    const char *header_end = strstr(data, "\r\n\r\n");
    if (!header_end) {
        return -1; // Incomplete request
    }

    size_t header_length = header_end - data;
    const char *body_start = header_end + 4;
    size_t body_length = length - (body_start - data);

    // Parse the request line (first line)
    char *line_end = strstr(data, "\r\n");
    if (!line_end || (line_end - data) > 255) {
        return -1;
    }

    char request_line[256];
    strncpy(request_line, data, line_end - data);
    request_line[line_end - data] = '\0';

    // Parse method and path
    char *saveptr;
    char *token = strtok_r(request_line, " ", &saveptr);
    if (token) {
        strncpy(request->method, token, sizeof(request->method) - 1);
        request->method[sizeof(request->method) - 1] = '\0';
        token = strtok_r(NULL, " ", &saveptr);
        if (token) {
            strncpy(request->path, token, sizeof(request->path) - 1);
            request->path[sizeof(request->path) - 1] = '\0';
        }
    }

    // Parse headers
    char headers[header_length + 1];
    strncpy(headers, data, header_length);
    headers[header_length] = '\0';

    // Look for Content-Type header
    char *content_type = strstr(headers, "Content-Type: ");
    if (content_type) {
        content_type += 14; // Skip "Content-Type: "
        char *end = strstr(content_type, "\r\n");
        if (end) {
            size_t ct_length = end - content_type;
            if (ct_length < sizeof(request->content_type)) {
                strncpy(request->content_type, content_type, ct_length);
                request->content_type[ct_length] = '\0';
            }
        }
    }

    // Debug: Print raw request data
    printf("🔍 RAW REQUEST DEBUG:\n");
    printf("Request length: %zu bytes\n", length);
    printf("Raw data (first 500 chars): %.500s\n", data);
    printf("Header length: %zu bytes\n", header_length);
    printf("Headers section:\n%.*s\n", (int)header_length, headers);
    printf("=== END HEADERS ===\n");

    // Look for Authorization header and store in page-allocated JWT storage
    printf("🔍 JWT STORAGE AUTH DEBUG:\n");
    printf("Searching for 'Authorization: ' in headers...\n");
    char *auth = strstr(headers, "Authorization: ");
    printf("Authorization search result: %s\n", auth ? "FOUND" : "NOT FOUND");

    if (auth && request->jwt_auth) {
        printf("Found Authorization at position: %ld\n", auth - headers);
        auth += 15; // Skip "Authorization: "
        printf("Auth value pointer after skip: '%.50s...'\n", auth);

        char *end = strstr(auth, "\r\n");
        if (!end) {
            // Authorization might be the last header, check for end of headers sequence
            printf("🔍 Authorization is likely the last header, looking for end of headers...\n");
            char *header_end_seq = strstr(auth, "\r\n\r\n");
            if (header_end_seq) {
                end = header_end_seq;
                printf("Found end of headers at position: %ld\n", header_end_seq - auth);
            } else {
                // Fallback: use end of the auth string
                end = auth + strlen(auth);
                printf("Using end of string as fallback\n");
            }
        }

        if (end > auth) {
            size_t auth_length = end - auth;
            printf("Auth value length: %zu\n", auth_length);
            printf("Auth value preview: '%.50s%s'\n", auth,
                   auth_length > 50 ? "..." : "");

            // Create a null-terminated string for the authorization header
            char* auth_header_str = malloc(auth_length + 1);
            if (auth_header_str) {
                strncpy(auth_header_str, auth, auth_length);
                auth_header_str[auth_length] = '\0';

                // Extract Bearer token using JWT storage
                jwt_storage_result_t result = jwt_storage_extract_bearer_token(
                    request->jwt_auth, auth_header_str);

                free(auth_header_str);

                if (result == JWT_STORAGE_SUCCESS) {
                    size_t token_length = jwt_storage_get_length(request->jwt_auth);
                    printf("✅ JWT token stored successfully: %zu bytes\n", token_length);
                    printf("Token preview: %.50s%s\n",
                           jwt_storage_get_string(request->jwt_auth),
                           token_length > 50 ? "..." : "");
                } else {
                    printf("❌ JWT storage failed: %s\n",
                           jwt_storage_error_string(result));
                }
            } else {
                printf("❌ Memory allocation failed for authorization header\n");
            }
        } else {
            printf("⚠️ Invalid authorization header position\n");
        }
    } else if (!request->jwt_auth) {
        printf("❌ JWT storage not initialized\n");
    } else {
        printf("⚠️ Authorization header NOT FOUND in headers\n");
        // Debug: Let's see if it's there with different case/formatting
        printf("🔍 Checking for alternative formats...\n");
        char *auth_alt1 = strstr(headers, "authorization: ");
        printf("Lowercase 'authorization: ': %s\n", auth_alt1 ? "FOUND" : "NOT FOUND");
        char *auth_alt2 = strstr(headers, "Authorization:");
        printf("No space 'Authorization:': %s\n", auth_alt2 ? "FOUND" : "NOT FOUND");
    }
    printf("🔍 === JWT STORAGE AUTH DEBUG END ===\n");

    // Copy body
    if (body_length > 0 && body_length < sizeof(request->body)) {
        memcpy(request->body, body_start, body_length);
        request->body_length = body_length;
    }

    return 0;
}

/**
 * Build HTTP response string
 */
int build_http_response(const http_response_t *response, char *output, size_t output_size) {
    if (!response || !output) {
        return -1;
    }

    const char *status_text = "OK";
    switch (response->status_code) {
        case 200: status_text = "OK"; break;
        case 201: status_text = "Created"; break;
        case 400: status_text = "Bad Request"; break;
        case 401: status_text = "Unauthorized"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 405: status_text = "Method Not Allowed"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }

    int written = snprintf(output, output_size,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: http://localhost:3000\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        response->status_code, status_text,
        strlen(response->content_type) > 0 ? response->content_type : "application/json",
        response->body_length,
        response->body);

    if (written >= (int)output_size || written < 0) {
        return -1;
    }

    return written;
}

/**
 * Handle user registration endpoint
 */
void handle_register(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse JSON request using cJSON (finally!)
    register_request_t req;
    if (parse_register_request(request->body, &req) != 0) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Perform registration
    auth_result_t result;
    if (auth_register(&auth_ctx, req.email, req.password, &result) == 0) {
        register_response_t resp;
        memset(&resp, 0, sizeof(register_response_t));  // Initialize all fields to zero
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.user_id, result.user.user_id);
            strcpy(resp.message, "Registration successful");
            response->status_code = 201;
        } else {
            strcpy(resp.error, result.error_message);
            response->status_code = 400;
        }

        strcpy(response->content_type, "application/json");
        generate_register_response(&resp, response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle user login endpoint
 */
void handle_login(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse JSON request using cJSON (no more terrible string parsing!)
    login_request_t req;
    if (parse_login_request(request->body, &req) != 0) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Perform login
    auth_result_t result;
    if (auth_login(&auth_ctx, req.email, req.password, "SimpleAuth-Server/1.0", "127.0.0.1", &result) == 0) {
        login_response_t resp;
        memset(&resp, 0, sizeof(login_response_t));  // Initialize all fields to zero
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.access_token, result.access_token);
            strcpy(resp.refresh_token, result.refresh_token);
            strcpy(resp.user_id, result.user.user_id);
            resp.error[0] = '\0';  // Explicitly clear error field on success
            response->status_code = 200;
        } else {
            strcpy(resp.error, result.error_message);
            response->status_code = 401;
        }

        strcpy(response->content_type, "application/json");
        generate_login_response(&resp, response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle token validation endpoint
 */
void handle_validate(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "GET") != 0 && strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Debug: Print validation details using JWT storage
    printf("🔍 JWT STORAGE VALIDATE DEBUG:\n");
    printf("Method: '%s'\n", request->method);
    printf("Path: '%s'\n", request->path);

    // Check if JWT storage is available and has a token
    if (!request->jwt_auth) {
        printf("❌ VALIDATION FAILED: JWT storage not initialized\n");
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error: JWT storage not initialized",
                               response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    if (!jwt_storage_is_valid(request->jwt_auth)) {
        printf("❌ VALIDATION FAILED: JWT storage is corrupted\n");
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error: JWT storage corrupted",
                               response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    if (!jwt_storage_has_token(request->jwt_auth)) {
        printf("❌ VALIDATION FAILED: No JWT token found in storage\n");
        printf("🔍 JWT storage info:\n");
        jwt_storage_print_info(request->jwt_auth);

        response->status_code = 401;
        strcpy(response->content_type, "application/json");
        generate_error_response("Missing or invalid Authorization header",
                               response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Get the JWT token from storage
    const char *token = jwt_storage_get_string(request->jwt_auth);
    if (!token) {
        printf("❌ VALIDATION FAILED: Could not retrieve JWT token from storage\n");
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error: Token retrieval failed",
                               response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    size_t token_length = jwt_storage_get_length(request->jwt_auth);
    printf("✅ JWT token retrieved from storage: %zu bytes\n", token_length);
    printf("Token preview: %.50s%s\n", token, token_length > 50 ? "..." : "");

    // Add detailed token integrity checking
    printf("🔍 TOKEN INTEGRITY DEBUG:\n");
    printf("📝 Full token (first 200 chars): %.200s%s\n", token, strlen(token) > 200 ? "..." : "");
    printf("📝 Token ends with: ...%.20s\n", token + strlen(token) - 20);
    printf("📝 Token null-terminated: %s\n", token[strlen(token)] == '\0' ? "YES" : "NO");

    // Check for basic JWT format (header.payload.signature)
    int dot_count = 0;
    for (size_t i = 0; i < strlen(token); i++) {
        if (token[i] == '.') dot_count++;
    }
    printf("📝 JWT format check: %d dots found (expected 2)\n", dot_count);

    if (dot_count == 2) {
        printf("✅ JWT format appears valid\n");
    } else {
        printf("❌ JWT format invalid - wrong number of dots\n");
    }

    // Validate token
    printf("🔍 Calling auth_validate_token...\n");
    auth_result_t result;
    validate_response_t resp;

    if (auth_validate_token(&auth_ctx, token, &result) == 0) {
        resp.valid = result.success;

        if (result.success) {
            strcpy(resp.user_id, result.user.user_id);
            strcpy(resp.email, result.user.email);
            resp.email_verified = result.user.email_verified;
            response->status_code = 200;
        } else {
            strcpy(resp.error, result.error_message);
            response->status_code = 401;
        }
    } else {
        resp.valid = 0;
        strcpy(resp.error, "Internal server error");
        response->status_code = 500;
    }

    strcpy(response->content_type, "application/json");
    generate_validate_response(&resp, response->body, sizeof(response->body));
    response->body_length = strlen(response->body);
}

/**
 * Handle token refresh endpoint
 */
void handle_refresh(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse JSON request body
    refresh_request_t req;
    if (parse_refresh_request(request->body, &req) != 0) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format - expected {\"refreshToken\": \"...\"}", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Perform token refresh
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
            response->status_code = 200;
        } else {
            strcpy(resp.error, result.error_message);
            response->status_code = 401;
        }

        strcpy(response->content_type, "application/json");
        generate_login_response(&resp, response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle session recovery endpoint
 */
void handle_recover_session(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse JSON request body
    recover_session_request_t req;
    if (parse_recover_session_request(request->body, &req) != 0) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format - expected {\"expiredRefreshToken\": \"...\"}", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Perform session recovery
    auth_result_t result;
    if (auth_recover_session(&auth_ctx, req.expired_refresh_token, &result) == 0) {
        login_response_t resp;
        memset(&resp, 0, sizeof(login_response_t));
        resp.success = result.success;

        if (result.success) {
            strcpy(resp.access_token, result.access_token);
            strcpy(resp.refresh_token, result.refresh_token);
            strcpy(resp.user_id, result.user.user_id);
            resp.error[0] = '\0';
            response->status_code = 200;
        } else {
            strcpy(resp.error, result.error_message);
            response->status_code = 401;
        }

        strcpy(response->content_type, "application/json");
        generate_login_response(&resp, response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Internal server error", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle JWKS endpoint for public key distribution
 */
void handle_jwks(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "GET") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        strcpy(response->body, "{\"error\":\"Method not allowed\"}");
        response->body_length = strlen(response->body);
        return;
    }

    char jwks_json[4096];
    if (auth_get_jwks(&auth_ctx, jwks_json, sizeof(jwks_json)) == 0) {
        response->status_code = 200;
        strcpy(response->content_type, "application/json");
        strcpy(response->body, jwks_json);
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        strcpy(response->body, "{\"error\":\"Failed to generate JWKS\"}");
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle logout endpoint
 */
void handle_logout(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Extract Bearer token from JWT storage
    if (!request->jwt_auth || !jwt_storage_has_token(request->jwt_auth)) {
        response->status_code = 401;
        strcpy(response->content_type, "application/json");
        generate_error_response("Missing or invalid Authorization header", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    const char *token = jwt_storage_get_string(request->jwt_auth);

    // Perform logout
    if (auth_logout(&auth_ctx, token, 0) == 0) {
        response->status_code = 200;
        strcpy(response->content_type, "application/json");
        generate_success_response("Logout successful", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("Logout failed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle OAuth authorization initialization endpoint
 */
void handle_oauth_authorize_init(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse OAuth initialization request
    oauth_init_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    // Simple JSON parsing for OAuth parameters
    cJSON *json = cJSON_Parse(request->body);
    if (!json) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
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
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Missing required parameters", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    strncpy(oauth_req.client_id, client_id->valuestring, sizeof(oauth_req.client_id) - 1);
    strncpy(oauth_req.redirect_uri, redirect_uri->valuestring, sizeof(oauth_req.redirect_uri) - 1);
    strncpy(oauth_req.code_challenge, code_challenge->valuestring, sizeof(oauth_req.code_challenge) - 1);
    strncpy(oauth_req.code_challenge_method, code_challenge_method->valuestring, sizeof(oauth_req.code_challenge_method) - 1);

    if (cJSON_IsString(state)) {
        strncpy(oauth_req.state, state->valuestring, sizeof(oauth_req.state) - 1);
    }

    cJSON_Delete(json);

    // Process OAuth initialization
    oauth_result_t result;
    if (oauth_authorize_init(&auth_ctx, &oauth_req, "SimpleAuth-Server/1.0", "127.0.0.1", &result) == 0) {
        response->status_code = result.success ? 200 : 400;
        strcpy(response->content_type, "application/json");

        if (result.success) {
            // Generate success response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "session_id", result.data.init.session_id);
            cJSON_AddStringToObject(resp_json, "authorization_url", result.data.init.authorization_url);
            cJSON_AddNumberToObject(resp_json, "expires_in", result.data.init.expires_in);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        } else {
            // Generate error response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "error", result.error_code);
            cJSON_AddStringToObject(resp_json, "error_description", result.error_description);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        }
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("OAuth initialization failed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle OAuth authorization completion endpoint
 */
void handle_oauth_authorize_complete(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse OAuth completion request
    oauth_complete_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    cJSON *json = cJSON_Parse(request->body);
    if (!json) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    cJSON *session_id = cJSON_GetObjectItemCaseSensitive(json, "session_id");
    cJSON *email = cJSON_GetObjectItemCaseSensitive(json, "email");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(json, "password");
    cJSON *consent_granted = cJSON_GetObjectItemCaseSensitive(json, "consent_granted");

    if (!cJSON_IsString(session_id) || !cJSON_IsString(email) ||
        !cJSON_IsString(password) || !cJSON_IsBool(consent_granted)) {
        cJSON_Delete(json);
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Missing required parameters", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    strncpy(oauth_req.session_id, session_id->valuestring, sizeof(oauth_req.session_id) - 1);
    strncpy(oauth_req.email, email->valuestring, sizeof(oauth_req.email) - 1);
    strncpy(oauth_req.password, password->valuestring, sizeof(oauth_req.password) - 1);
    oauth_req.consent_granted = cJSON_IsTrue(consent_granted) ? 1 : 0;

    cJSON_Delete(json);

    // Process OAuth completion
    oauth_result_t result;
    if (oauth_authorize_complete(&auth_ctx, &oauth_req, &result) == 0) {
        response->status_code = result.success ? 200 : 400;
        strcpy(response->content_type, "application/json");

        if (result.success) {
            // Generate success response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "authorization_code", result.data.complete.authorization_code);
            cJSON_AddStringToObject(resp_json, "redirect_uri", result.data.complete.redirect_uri);
            cJSON_AddStringToObject(resp_json, "state", result.data.complete.state);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        } else {
            // Generate error response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "error", result.error_code);
            cJSON_AddStringToObject(resp_json, "error_description", result.error_description);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        }
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("OAuth completion failed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Handle OAuth token exchange endpoint
 */
void handle_oauth_token(const http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        response->status_code = 405;
        strcpy(response->content_type, "application/json");
        generate_error_response("Method not allowed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    // Parse OAuth token request
    oauth_token_request_t oauth_req;
    memset(&oauth_req, 0, sizeof(oauth_req));

    cJSON *json = cJSON_Parse(request->body);
    if (!json) {
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Invalid JSON format", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
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
        response->status_code = 400;
        strcpy(response->content_type, "application/json");
        generate_error_response("Missing required parameters", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
        return;
    }

    strncpy(oauth_req.grant_type, grant_type->valuestring, sizeof(oauth_req.grant_type) - 1);
    strncpy(oauth_req.code, code->valuestring, sizeof(oauth_req.code) - 1);
    strncpy(oauth_req.code_verifier, code_verifier->valuestring, sizeof(oauth_req.code_verifier) - 1);
    strncpy(oauth_req.client_id, client_id->valuestring, sizeof(oauth_req.client_id) - 1);
    strncpy(oauth_req.redirect_uri, redirect_uri->valuestring, sizeof(oauth_req.redirect_uri) - 1);

    cJSON_Delete(json);

    // Process OAuth token exchange
    oauth_result_t result;
    if (oauth_token_exchange(&auth_ctx, &oauth_req, &result) == 0) {
        response->status_code = result.success ? 200 : 400;
        strcpy(response->content_type, "application/json");

        if (result.success) {
            // Generate success response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "access_token", result.data.token.access_token);
            cJSON_AddStringToObject(resp_json, "refresh_token", result.data.token.refresh_token);
            cJSON_AddStringToObject(resp_json, "token_type", result.data.token.token_type);
            cJSON_AddNumberToObject(resp_json, "expires_in", result.data.token.expires_in);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        } else {
            // Generate error response
            cJSON *resp_json = cJSON_CreateObject();
            cJSON_AddStringToObject(resp_json, "error", result.error_code);
            cJSON_AddStringToObject(resp_json, "error_description", result.error_description);

            char *json_string = cJSON_Print(resp_json);
            strncpy(response->body, json_string, sizeof(response->body) - 1);
            response->body_length = strlen(response->body);

            cJSON_Delete(resp_json);
            free(json_string);
        }
    } else {
        response->status_code = 500;
        strcpy(response->content_type, "application/json");
        generate_error_response("OAuth token exchange failed", response->body, sizeof(response->body));
        response->body_length = strlen(response->body);
    }
}

/**
 * Main request router
 */
void handle_request(const http_request_t *request, http_response_t *response) {
    // Initialize response
    memset(response, 0, sizeof(http_response_t));
    strcpy(response->content_type, "application/json");

    printf("📥 %s %s\n", request->method, request->path);

    // Handle CORS preflight requests FIRST (before path-specific routing)
    if (strcmp(request->method, "OPTIONS") == 0) {
        response->status_code = 200;
        strcpy(response->body, "");
        response->body_length = 0;
        return;
    }

    // Route requests
    if (strncmp(request->path, "/auth/register", 14) == 0) {
        handle_register(request, response);
    } else if (strncmp(request->path, "/auth/login/ropc", 16) == 0) {
        handle_login(request, response);
    } else if (strncmp(request->path, "/auth/validate", 14) == 0) {
        handle_validate(request, response);
    } else if (strncmp(request->path, "/auth/recover-session", 21) == 0) {
        handle_recover_session(request, response);
    } else if (strncmp(request->path, "/auth/refresh", 13) == 0) {
        handle_refresh(request, response);
    } else if (strncmp(request->path, "/auth/logout", 12) == 0) {
        handle_logout(request, response);
    } else if (strncmp(request->path, "/oauth/authorize/init", 21) == 0) {
        handle_oauth_authorize_init(request, response);
    } else if (strncmp(request->path, "/oauth/authorize/complete", 25) == 0) {
        handle_oauth_authorize_complete(request, response);
    } else if (strncmp(request->path, "/oauth/token", 12) == 0) {
        handle_oauth_token(request, response);
    } else if (strncmp(request->path, "/.well-known/jwks.json", 23) == 0) {
        handle_jwks(request, response);
    } else {
        // 404 Not Found
        response->status_code = 404;
        strcpy(response->body, "{\"error\":\"Endpoint not found\"}");
        response->body_length = strlen(response->body);
    }
}

/**
 * Check if a request buffer contains a complete HTTP request
 * Returns 1 if complete, 0 if incomplete, -1 if malformed
 */
int is_request_complete(const char *data, size_t length) {
    // Find end of headers
    const char *header_end = strstr(data, "\r\n\r\n");
    if (!header_end) {
        return 0;  // Headers incomplete
    }

    // Calculate header length
    size_t header_length = header_end - data;

    // Parse Content-Length header if present
    size_t expected_body_length = 0;
    char *cl_start = strstr(data, "Content-Length: ");
    if (cl_start && cl_start < header_end) {
        cl_start += 16;  // Skip "Content-Length: "
        char *cl_end = strstr(cl_start, "\r\n");
        if (cl_end) {
            char cl_str[32];
            size_t cl_len = cl_end - cl_start;
            if (cl_len < sizeof(cl_str)) {
                strncpy(cl_str, cl_start, cl_len);
                cl_str[cl_len] = '\0';
                expected_body_length = atoi(cl_str);
            }
        }
    }

    // Calculate actual body length received
    size_t body_start = header_length + 4;  // +4 for "\r\n\r\n"
    size_t actual_body_length = (length > body_start) ? (length - body_start) : 0;

    // Check if body is complete
    if (expected_body_length > 0) {
        return actual_body_length >= expected_body_length ? 1 : 0;
    }

    // No body expected, request is complete
    return 1;
}

/**
 * HTTP socket data handler
 */
struct us_socket_t *on_http_data(struct us_socket_t *s, char *data, int length) {
    if (length <= 0) {
        return s;
    }

    // Get socket state from extension area
    socket_state_t *state = (socket_state_t *)us_socket_ext(SSL, s);

    // Check if we can fit the new data in the buffer
    if (state->buffer_used + length > MAX_REQUEST_SIZE) {
        printf("⚠️ Request buffer overflow: %zu + %d > %d\n",
               state->buffer_used, length, MAX_REQUEST_SIZE);
        // Send 413 Payload Too Large
        const char *too_large =
            "HTTP/1.1 413 Payload Too Large\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 33\r\n"
            "Access-Control-Allow-Origin: http://localhost:3000\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"error\":\"Request too large\"}";
        us_socket_write(SSL, s, too_large, strlen(too_large), 0);
        return us_socket_close(SSL, s, 0, NULL);
    }

    // Append new data to buffer
    memcpy(state->buffer + state->buffer_used, data, length);
    state->buffer_used += length;

    printf("🔍 BUFFER DEBUG: Added %d bytes, total: %zu\n", length, state->buffer_used);

    // Check if request is complete
    int complete_status = is_request_complete(state->buffer, state->buffer_used);
    if (complete_status == 0) {
        printf("📥 Request incomplete, waiting for more data...\n");
        return s;  // Wait for more data
    } else if (complete_status == -1) {
        printf("❌ Malformed request detected\n");
        // Send 400 Bad Request
        const char *bad_request =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 27\r\n"
            "Access-Control-Allow-Origin: http://localhost:3000\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"error\":\"Bad Request\"}";
        us_socket_write(SSL, s, bad_request, strlen(bad_request), 0);
        return us_socket_close(SSL, s, 0, NULL);
    }

    printf("📥 Request complete! Processing %zu bytes\n", state->buffer_used);

    // Parse the complete HTTP request
    http_request_t request;
    if (parse_http_request(state->buffer, state->buffer_used, &request) != 0) {
        printf("❌ Failed to parse complete request\n");
        // Send 400 Bad Request
        const char *bad_request =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 27\r\n"
            "Access-Control-Allow-Origin: http://localhost:3000\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"error\":\"Bad Request\"}";
        us_socket_write(SSL, s, bad_request, strlen(bad_request), 0);
        return us_socket_close(SSL, s, 0, NULL);
    }

    // Handle the request
    http_response_t response;
    handle_request(&request, &response);

    // Build response string
    char response_str[MAX_RESPONSE_SIZE + 512];
    if (build_http_response(&response, response_str, sizeof(response_str)) > 0) {
        us_socket_write(SSL, s, response_str, strlen(response_str), 0);
    }

    // Cleanup JWT storage to prevent memory leaks
    cleanup_http_request(&request);

    // Check if client wants to keep connection alive
    const char *connection_header = strstr(state->buffer, "Connection: keep-alive");
    if (connection_header && connection_header < strstr(state->buffer, "\r\n\r\n")) {
        printf("🔄 Keeping connection alive for next request\n");
        // Reset buffer for next request on same connection
        state->buffer_used = 0;
        state->request_complete = 0;
        return s;  // Keep connection open
    } else {
        printf("🔚 Closing connection as requested\n");
        return us_socket_close(SSL, s, 0, NULL);
    }
}

/**
 * HTTP socket close handler
 */
struct us_socket_t *on_http_close(struct us_socket_t *s, int code, void *reason) {
    (void)reason;
    printf("Client disconnected with code: %d\n", code);
    return s;
}

/**
 * HTTP socket open handler
 */
struct us_socket_t *on_http_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    (void)is_client;
    printf("Client connected from IP: %.*s\n", ip_length, ip);

    // Initialize socket state in extension area
    socket_state_t *state = (socket_state_t *)us_socket_ext(SSL, s);
    memset(state, 0, sizeof(socket_state_t));
    printf("🔄 Socket state initialized for new connection\n");

    // Set timeout to 30 seconds
    us_socket_timeout(SSL, s, 30);
    return s;
}

/**
 * HTTP socket timeout handler
 */
struct us_socket_t *on_http_timeout(struct us_socket_t *s) {
    printf("Client connection timed out\n");
    return us_socket_close(SSL, s, 0, NULL);
}

/**
 * HTTP socket end handler
 */
struct us_socket_t *on_http_end(struct us_socket_t *s) {
    printf("Client connection ended gracefully\n");
    return us_socket_close(SSL, s, 0, NULL);
}

/**
 * Listen socket handler
 */
struct us_listen_socket_t *on_http_listen(struct us_listen_socket_t *ls, int is_ssl) {
    if (ls) {
        printf("🚀 Authentication server listening on port %d (SSL: %s)\n",
               SERVER_PORT, is_ssl ? "Yes" : "No");
    } else {
        printf("❌ Failed to listen on port %d\n", SERVER_PORT);
    }
    return ls;
}

/**
 * Key generation mode
 */
int generate_keys_mode() {
    printf("🔐 Generating RSA key pair...\n");

    auth_context_t temp_ctx;
    auth_config_t config;
    auth_init_config(&config);

    if (auth_initialize(&temp_ctx, &config) == 0) {
        printf("✅ RSA key pair generated successfully\n");
        printf("   Private key: %s\n", PRIVATE_KEY_PATH);
        printf("   Public key: %s\n", PUBLIC_KEY_PATH);
        printf("   Key ID: %s\n", temp_ctx.keypair.key_id);
        auth_cleanup(&temp_ctx);
        return 0;
    } else {
        printf("❌ Failed to generate RSA key pair\n");
        return 1;
    }
}

/**
 * Main server entry point
 */
int main(int argc, char *argv[]) {
    // Check for key generation mode
    if (argc > 1 && strcmp(argv[1], "--generate-keys") == 0) {
        return generate_keys_mode();
    }

    printf("🔐 Simple Authentication Server\n");
    printf("===============================\n\n");

    // Validate memory system at startup
    printf("🔍 Validating memory system...\n");
    if (memory_system_validate() != 0) {
        printf("❌ Memory system validation failed\n");
        printf("   This system may not be compatible with the page allocator\n");
        return 1;
    }
    printf("✅ Memory system validation passed\n\n");

    // Initialize platform-specific memory system
    printf("🔧 Initializing memory system...\n");
    if (memory_system_init() != 0) {
        printf("❌ Failed to initialize memory system\n");
        return 1;
    }
    printf("✅ Memory system initialized\n\n");

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize authentication system
    printf("🔧 Initializing authentication system...\n");
    auth_config_t config;
    auth_init_config(&config);

    if (auth_initialize(&auth_ctx, &config) != 0) {
        printf("❌ Failed to initialize authentication system\n");
        return 1;
    }

    printf("✅ Authentication system ready\n");
    printf("   Issuer: %s\n", config.issuer);
    printf("   Database: %s\n", config.database_path);
    printf("   Key ID: %s\n\n", auth_ctx.keypair.key_id);

    // Create event loop with proper handlers
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    // SSL context options - MUST use SSL certificates
    struct us_socket_context_options_t options = {};
    options.key_file_name = "/etc/ssl/splitdo_api/private/key.pem";
    options.cert_file_name = "/etc/ssl/splitdo_api/cert.pem";
    options.passphrase = "";

    // Create SSL socket context with socket state extension
    struct us_socket_context_t *context = us_create_socket_context(SSL, loop, sizeof(socket_state_t), options);

    if (!context) {
        printf("❌ Failed to create SSL socket context\n");
        printf("   Make sure SSL certificates exist at /etc/ssl/splitdo_api/\n");
        auth_cleanup(&auth_ctx);
        return 1;
    }

    // Set up event handlers
    us_socket_context_on_open(SSL, context, on_http_open);
    us_socket_context_on_data(SSL, context, on_http_data);
    us_socket_context_on_close(SSL, context, on_http_close);
    us_socket_context_on_timeout(SSL, context, on_http_timeout);
    us_socket_context_on_end(SSL, context, on_http_end);

    // Listen on port
    struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, context, 0, SERVER_PORT, 0, sizeof(socket_state_t));

    if (!listen_socket) {
        printf("❌ Failed to listen on port %d\n", SERVER_PORT);
        auth_cleanup(&auth_ctx);
        return 1;
    }

    on_http_listen(listen_socket, 0);

    printf("\n📡 Available endpoints:\n");
    printf("   POST https://localhost:%d/auth/register\n", SERVER_PORT);
    printf("   POST https://localhost:%d/auth/login/ropc\n", SERVER_PORT);
    printf("   POST https://localhost:%d/auth/validate\n", SERVER_PORT);
    printf("   POST https://localhost:%d/auth/logout\n", SERVER_PORT);
    printf("   POST https://localhost:%d/oauth/authorize/init\n", SERVER_PORT);
    printf("   POST https://localhost:%d/oauth/authorize/complete\n", SERVER_PORT);
    printf("   POST https://localhost:%d/oauth/token\n", SERVER_PORT);
    printf("   GET  https://localhost:%d/.well-known/jwks.json\n\n", SERVER_PORT);

    printf("🔄 Server running... (Press Ctrl+C to stop)\n\n");

    // Main event loop with periodic memory monitoring
    time_t last_stats_time = time(NULL);
    while (server_running) {
        us_loop_run(loop);

        // Print memory statistics every 60 seconds
        time_t current_time = time(NULL);
        if (current_time - last_stats_time >= 60) {
            page_alloc_stats_t stats;
            page_get_stats(&stats);

            printf("\n📊 Memory Statistics (after %ld seconds):\n", current_time - last_stats_time);
            printf("   Total allocated: %zu pages (%zu bytes)\n",
                   stats.total_pages_allocated, stats.total_bytes_allocated);
            printf("   Currently locked: %zu pages (%zu bytes)\n",
                   stats.pages_currently_locked, stats.bytes_currently_locked);
            printf("   Allocations: %zu, Deallocations: %zu\n",
                   stats.allocation_count, stats.deallocation_count);

            if (stats.allocation_count > stats.deallocation_count) {
                printf("   ⚠️ Potential leak: %zu unfreed allocations\n",
                       stats.allocation_count - stats.deallocation_count);
            } else {
                printf("   ✅ No memory leaks detected\n");
            }
            printf("\n");

            last_stats_time = current_time;
        }
    }

    // Cleanup
    printf("\n🧹 Cleaning up...\n");
    auth_cleanup(&auth_ctx);
    us_loop_free(loop);

    printf("✅ Server stopped gracefully\n");
    return 0;
}