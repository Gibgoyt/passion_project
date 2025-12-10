#include "json_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_register_request(const char *json_str, register_request_t *request) {
    if (!json_str || !request) {
        return -1;
    }

    memset(request, 0, sizeof(register_request_t));

    cJSON *json = cJSON_Parse(json_str);
    if (!json) {
        return -1;
    }

    // Parse email
    cJSON *email_item = cJSON_GetObjectItem(json, "email");
    if (!email_item || !cJSON_IsString(email_item)) {
        cJSON_Delete(json);
        return -1;
    }

    // Parse password
    cJSON *password_item = cJSON_GetObjectItem(json, "password");
    if (!password_item || !cJSON_IsString(password_item)) {
        cJSON_Delete(json);
        return -1;
    }

    // Copy values with length check
    const char *email_str = cJSON_GetStringValue(email_item);
    const char *password_str = cJSON_GetStringValue(password_item);

    if (!email_str || !password_str ||
        strlen(email_str) >= JSON_MAX_STRING_LENGTH ||
        strlen(password_str) >= JSON_MAX_STRING_LENGTH) {
        cJSON_Delete(json);
        return -1;
    }

    strcpy(request->email, email_str);
    strcpy(request->password, password_str);
    request->valid = 1;

    cJSON_Delete(json);
    return 0;
}

int parse_login_request(const char *json_str, login_request_t *request) {
    // Same logic as register_request since they have identical structure
    return parse_register_request(json_str, (register_request_t*)request);
}

int generate_register_response(const register_response_t *response, char *json_out, size_t json_len) {
    if (!response || !json_out || json_len == 0) {
        return -1;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    if (response->success) {
        cJSON_AddBoolToObject(json, "success", 1);  // Use explicit 1 for true
        cJSON_AddStringToObject(json, "userId", response->user_id);
        cJSON_AddStringToObject(json, "message", response->message);
    } else {
        cJSON_AddBoolToObject(json, "success", 0);  // Use explicit 0 for false
        cJSON_AddStringToObject(json, "error", response->error);
    }

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return -1;
    }

    if (strlen(json_string) >= json_len) {
        free(json_string);
        cJSON_Delete(json);
        return -1;
    }

    strcpy(json_out, json_string);
    free(json_string);
    cJSON_Delete(json);
    return 0;
}

int generate_login_response(const login_response_t *response, char *json_out, size_t json_len) {
    if (!response || !json_out || json_len == 0) {
        return -1;
    }

    printf("🔍 JSON DEBUG: response->success = %d\n", response->success);
    printf("🔍 JSON DEBUG: response->error = '%s'\n", response->error);

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    if (response->success) {
        printf("🔍 JSON DEBUG: Adding SUCCESS fields\n");
        cJSON_AddBoolToObject(json, "success", 1);  // Use explicit 1 for true
        cJSON_AddStringToObject(json, "accessToken", response->access_token);
        cJSON_AddStringToObject(json, "refreshToken", response->refresh_token);
        cJSON_AddStringToObject(json, "userId", response->user_id);
    } else {
        printf("🔍 JSON DEBUG: Adding ERROR fields\n");
        cJSON_AddBoolToObject(json, "success", 0);  // Use explicit 0 for false
        if (response->error && response->error[0] != '\0') {
            cJSON_AddStringToObject(json, "error", response->error);
        } else {
            cJSON_AddStringToObject(json, "error", "Unknown error");
        }
    }

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return -1;
    }

    if (strlen(json_string) >= json_len) {
        free(json_string);
        cJSON_Delete(json);
        return -1;
    }

    strcpy(json_out, json_string);
    free(json_string);
    cJSON_Delete(json);
    return 0;
}

int generate_validate_response(const validate_response_t *response, char *json_out, size_t json_len) {
    if (!response || !json_out || json_len == 0) {
        return -1;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    if (response->valid) {
        cJSON_AddBoolToObject(json, "valid", 1);  // Use explicit 1 for true
        cJSON_AddStringToObject(json, "userId", response->user_id);
        cJSON_AddStringToObject(json, "email", response->email);
        cJSON_AddBoolToObject(json, "emailVerified", response->email_verified ? 1 : 0);  // Fix this too
    } else {
        cJSON_AddBoolToObject(json, "valid", 0);  // Use explicit 0 for false
        cJSON_AddStringToObject(json, "error", response->error);
    }

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return -1;
    }

    if (strlen(json_string) >= json_len) {
        free(json_string);
        cJSON_Delete(json);
        return -1;
    }

    strcpy(json_out, json_string);
    free(json_string);
    cJSON_Delete(json);
    return 0;
}

int generate_error_response(const char *error_message, char *json_out, size_t json_len) {
    if (!error_message || !json_out || json_len == 0) {
        return -1;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    cJSON_AddStringToObject(json, "error", error_message);

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return -1;
    }

    if (strlen(json_string) >= json_len) {
        free(json_string);
        cJSON_Delete(json);
        return -1;
    }

    strcpy(json_out, json_string);
    free(json_string);
    cJSON_Delete(json);
    return 0;
}

int generate_success_response(const char *message, char *json_out, size_t json_len) {
    if (!message || !json_out || json_len == 0) {
        return -1;
    }

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    cJSON_AddBoolToObject(json, "success", cJSON_True);
    cJSON_AddStringToObject(json, "message", message);

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return -1;
    }

    if (strlen(json_string) >= json_len) {
        free(json_string);
        cJSON_Delete(json);
        return -1;
    }

    strcpy(json_out, json_string);
    free(json_string);
    cJSON_Delete(json);
    return 0;
}

int parse_refresh_request(const char *json_str, refresh_request_t *request) {
    if (!json_str || !request) {
        return -1;
    }

    memset(request, 0, sizeof(refresh_request_t));

    cJSON *json = cJSON_Parse(json_str);
    if (!json) {
        return -1;
    }

    // Parse refreshToken
    cJSON *token_item = cJSON_GetObjectItem(json, "refreshToken");
    if (!token_item || !cJSON_IsString(token_item)) {
        cJSON_Delete(json);
        return -1;
    }

    const char *token_str = cJSON_GetStringValue(token_item);
    if (!token_str || strlen(token_str) >= sizeof(request->refresh_token)) {
        cJSON_Delete(json);
        return -1;
    }

    strcpy(request->refresh_token, token_str);
    request->valid = 1;

    cJSON_Delete(json);
    return 0;
}

int parse_recover_session_request(const char *json_str, recover_session_request_t *request) {
    if (!json_str || !request) {
        return -1;
    }

    memset(request, 0, sizeof(recover_session_request_t));

    cJSON *json = cJSON_Parse(json_str);
    if (!json) {
        return -1;
    }

    // Parse expiredRefreshToken
    cJSON *token_item = cJSON_GetObjectItem(json, "expiredRefreshToken");
    if (!token_item || !cJSON_IsString(token_item)) {
        cJSON_Delete(json);
        return -1;
    }

    const char *token_str = cJSON_GetStringValue(token_item);
    if (!token_str || strlen(token_str) >= sizeof(request->expired_refresh_token)) {
        cJSON_Delete(json);
        return -1;
    }

    strcpy(request->expired_refresh_token, token_str);
    request->valid = 1;

    cJSON_Delete(json);
    return 0;
}