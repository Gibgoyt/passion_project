#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <cjson/cJSON.h>
#include <stddef.h>

/**
 * JSON Request/Response Utilities for Authentication Server
 *
 * Replaces the terrible hand-rolled JSON parsing with proper cJSON library
 */

// Maximum sizes
#define JSON_MAX_STRING_LENGTH 512
#define JSON_RESPONSE_MAX_SIZE 2048

/**
 * Parse registration request JSON
 * Expected format: {"email": "...", "password": "..."}
 */
typedef struct {
    char email[JSON_MAX_STRING_LENGTH];
    char password[JSON_MAX_STRING_LENGTH];
    int valid;
} register_request_t;

/**
 * Parse login request JSON
 * Expected format: {"email": "...", "password": "..."}
 */
typedef struct {
    char email[JSON_MAX_STRING_LENGTH];
    char password[JSON_MAX_STRING_LENGTH];
    int valid;
} login_request_t;

/**
 * Parse refresh request JSON
 * Expected format: {"refreshToken": "..."}
 */
typedef struct {
    char refresh_token[2048];  // Match JWT token size from login_response_t
    int valid;
} refresh_request_t;

/**
 * Parse recover session request JSON
 * Expected format: {"expiredRefreshToken": "..."}
 */
typedef struct {
    char expired_refresh_token[2048];
    int valid;
} recover_session_request_t;

/**
 * Generate registration response JSON
 */
typedef struct {
    int success;
    char user_id[64];
    char message[256];
    char error[256];
} register_response_t;

/**
 * Generate login response JSON
 */
typedef struct {
    int success;
    char access_token[2048];
    char refresh_token[2048];
    char user_id[64];
    char error[256];
} login_response_t;

/**
 * Generate validation response JSON
 */
typedef struct {
    int valid;
    char user_id[64];
    char email[JSON_MAX_STRING_LENGTH];
    int email_verified;
    char error[256];
} validate_response_t;

// Parser functions
int parse_register_request(const char *json_str, register_request_t *request);
int parse_login_request(const char *json_str, login_request_t *request);
int parse_refresh_request(const char *json_str, refresh_request_t *request);
int parse_recover_session_request(const char *json_str, recover_session_request_t *request);

// Response generators
int generate_register_response(const register_response_t *response, char *json_out, size_t json_len);
int generate_login_response(const login_response_t *response, char *json_out, size_t json_len);
int generate_validate_response(const validate_response_t *response, char *json_out, size_t json_len);
int generate_error_response(const char *error_message, char *json_out, size_t json_len);
int generate_success_response(const char *message, char *json_out, size_t json_len);

#endif // JSON_UTILS_H