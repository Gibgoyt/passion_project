/**
 * Simple Authentication Test Client
 *
 * Test client that demonstrates the authentication endpoints by performing
 * a complete authentication flow:
 * 1. User registration
 * 2. User login (get JWT tokens)
 * 3. Token validation
 * 4. JWKS endpoint test
 * 5. User logout
 *
 * Uses basic HTTP requests to communicate with the auth server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

// Configuration
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 2053
#define BUFFER_SIZE 8192

// Colors for output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// Test user credentials
static char test_email[256] = "";
static char test_password[] = "TestPassword123";
static char access_token[2048] = "";
static char refresh_token[2048] = "";
static char user_id[64] = "";

/**
 * Connect to the authentication server
 */
int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_HOST);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

/**
 * Send HTTP request and receive response
 */
int send_http_request(const char *method, const char *path,
                     const char *headers, const char *body,
                     char *response, size_t response_size) {
    int sock = connect_to_server();
    if (sock < 0) {
        printf(RED "❌ Failed to connect to server\n" RESET);
        return -1;
    }

    // Build HTTP request
    char request[BUFFER_SIZE];
    int content_length = body ? strlen(body) : 0;

    int request_len = snprintf(request, sizeof(request),
        "%s %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "%s"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        method, path, SERVER_HOST, SERVER_PORT, content_length,
        headers ? headers : "",
        body ? body : "");

    // Send request
    if (send(sock, request, request_len, 0) < 0) {
        close(sock);
        return -1;
    }

    // Receive response
    ssize_t received = recv(sock, response, response_size - 1, 0);
    if (received < 0) {
        close(sock);
        return -1;
    }

    response[received] = '\0';
    close(sock);
    return 0;
}

/**
 * Extract JSON value from response
 */
int extract_json_string(const char *json, const char *key, char *value, size_t value_size) {
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

    char *start = strstr(json, search_key);
    if (!start) {
        return -1;
    }

    start += strlen(search_key);
    char *end = strchr(start, '"');
    if (!end) {
        return -1;
    }

    size_t length = end - start;
    if (length >= value_size) {
        length = value_size - 1;
    }

    strncpy(value, start, length);
    value[length] = '\0';
    return 0;
}

/**
 * Test user registration
 */
int test_registration() {
    printf(BLUE "🧪 Testing user registration...\n" RESET);

    // Generate unique email with timestamp
    time_t now = time(NULL);
    snprintf(test_email, sizeof(test_email), "test%ld@example.com", now);

    char body[512];
    snprintf(body, sizeof(body),
        "{\"email\":\"%s\",\"password\":\"%s\"}",
        test_email, test_password);

    char response[BUFFER_SIZE];
    if (send_http_request("POST", "/auth/register", NULL, body, response, sizeof(response)) != 0) {
        printf(RED "❌ Registration request failed\n" RESET);
        return -1;
    }

    printf("📤 Registration request sent\n");
    printf("📧 Email: %s\n", test_email);

    // Check response
    if (strstr(response, "HTTP/1.1 201") && strstr(response, "\"success\":true")) {
        printf(GREEN "✅ Registration successful\n" RESET);

        // Extract user ID
        char *body_start = strstr(response, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            if (extract_json_string(body_start, "userId", user_id, sizeof(user_id)) == 0) {
                printf("👤 User ID: %s\n", user_id);
            }
        }
        return 0;
    } else {
        printf(RED "❌ Registration failed\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Test user login
 */
int test_login() {
    printf(BLUE "\n🧪 Testing user login...\n" RESET);

    char body[512];
    snprintf(body, sizeof(body),
        "{\"email\":\"%s\",\"password\":\"%s\"}",
        test_email, test_password);

    char response[BUFFER_SIZE];
    if (send_http_request("POST", "/auth/login", NULL, body, response, sizeof(response)) != 0) {
        printf(RED "❌ Login request failed\n" RESET);
        return -1;
    }

    printf("📤 Login request sent\n");

    // Check response
    if (strstr(response, "HTTP/1.1 200") && strstr(response, "\"success\":true")) {
        printf(GREEN "✅ Login successful\n" RESET);

        // Extract tokens
        char *body_start = strstr(response, "\r\n\r\n");
        if (body_start) {
            body_start += 4;

            if (extract_json_string(body_start, "accessToken", access_token, sizeof(access_token)) == 0) {
                printf("🔑 Access token: %.50s...\n", access_token);
            }

            if (extract_json_string(body_start, "refreshToken", refresh_token, sizeof(refresh_token)) == 0) {
                printf("🔄 Refresh token: %.50s...\n", refresh_token);
            }
        }
        return 0;
    } else {
        printf(RED "❌ Login failed\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Test token validation
 */
int test_validation() {
    printf(BLUE "\n🧪 Testing token validation...\n" RESET);

    if (strlen(access_token) == 0) {
        printf(RED "❌ No access token available\n" RESET);
        return -1;
    }

    char headers[2048];
    snprintf(headers, sizeof(headers), "Authorization: Bearer %s\r\n", access_token);

    char response[BUFFER_SIZE];
    if (send_http_request("POST", "/auth/validate", headers, "", response, sizeof(response)) != 0) {
        printf(RED "❌ Validation request failed\n" RESET);
        return -1;
    }

    printf("📤 Validation request sent\n");

    // Check response
    if (strstr(response, "HTTP/1.1 200") && strstr(response, "\"valid\":true")) {
        printf(GREEN "✅ Token validation successful\n" RESET);

        // Extract user info
        char *body_start = strstr(response, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            char email[256];
            if (extract_json_string(body_start, "email", email, sizeof(email)) == 0) {
                printf("📧 Validated email: %s\n", email);
            }
        }
        return 0;
    } else {
        printf(RED "❌ Token validation failed\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Test JWKS endpoint
 */
int test_jwks() {
    printf(BLUE "\n🧪 Testing JWKS endpoint...\n" RESET);

    char response[BUFFER_SIZE];
    if (send_http_request("GET", "/.well-known/jwks.json", NULL, NULL, response, sizeof(response)) != 0) {
        printf(RED "❌ JWKS request failed\n" RESET);
        return -1;
    }

    printf("📤 JWKS request sent\n");

    // Check response
    if (strstr(response, "HTTP/1.1 200") && strstr(response, "\"keys\"")) {
        printf(GREEN "✅ JWKS endpoint working\n" RESET);

        // Show JWKS preview
        char *body_start = strstr(response, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            printf("🔑 JWKS response: %.100s...\n", body_start);
        }
        return 0;
    } else {
        printf(RED "❌ JWKS endpoint failed\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Test user logout
 */
int test_logout() {
    printf(BLUE "\n🧪 Testing user logout...\n" RESET);

    if (strlen(access_token) == 0) {
        printf(RED "❌ No access token available\n" RESET);
        return -1;
    }

    char headers[2048];
    snprintf(headers, sizeof(headers), "Authorization: Bearer %s\r\n", access_token);

    char response[BUFFER_SIZE];
    if (send_http_request("POST", "/auth/logout", headers, "", response, sizeof(response)) != 0) {
        printf(RED "❌ Logout request failed\n" RESET);
        return -1;
    }

    printf("📤 Logout request sent\n");

    // Check response
    if (strstr(response, "HTTP/1.1 200") && strstr(response, "\"success\":true")) {
        printf(GREEN "✅ Logout successful\n" RESET);
        return 0;
    } else {
        printf(RED "❌ Logout failed\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Test token validation after logout (should fail)
 */
int test_validation_after_logout() {
    printf(BLUE "\n🧪 Testing token validation after logout...\n" RESET);

    char headers[2048];
    snprintf(headers, sizeof(headers), "Authorization: Bearer %s\r\n", access_token);

    char response[BUFFER_SIZE];
    if (send_http_request("POST", "/auth/validate", headers, "", response, sizeof(response)) != 0) {
        printf(RED "❌ Validation request failed\n" RESET);
        return -1;
    }

    printf("📤 Post-logout validation request sent\n");

    // Check response (should be unauthorized)
    if (strstr(response, "HTTP/1.1 401") || strstr(response, "\"valid\":false")) {
        printf(GREEN "✅ Token correctly invalidated after logout\n" RESET);
        return 0;
    } else {
        printf(RED "❌ Token still valid after logout (security issue!)\n" RESET);
        printf("Response: %s\n", response);
        return -1;
    }
}

/**
 * Print test summary
 */
void print_test_summary(int total_tests, int passed_tests) {
    printf("\n" YELLOW "═══════════════════════════════════════\n" RESET);
    printf(YELLOW "           TEST SUMMARY\n" RESET);
    printf(YELLOW "═══════════════════════════════════════\n" RESET);
    printf("Total tests: %d\n", total_tests);
    printf(GREEN "Passed: %d\n" RESET, passed_tests);
    printf(RED "Failed: %d\n" RESET, total_tests - passed_tests);

    if (passed_tests == total_tests) {
        printf(GREEN "\n🎉 All tests passed! Authentication system is working correctly.\n" RESET);
    } else {
        printf(RED "\n❌ Some tests failed. Please check the authentication system.\n" RESET);
    }
    printf(YELLOW "═══════════════════════════════════════\n\n" RESET);
}

/**
 * Main test runner
 */
int main(int argc, char *argv[]) {
    printf(BLUE "🔐 Simple Authentication Client Test Suite\n");
    printf("==========================================\n\n" RESET);

    printf("🔗 Connecting to server: %s:%d\n", SERVER_HOST, SERVER_PORT);

    // Test server connection first
    int sock = connect_to_server();
    if (sock < 0) {
        printf(RED "❌ Cannot connect to authentication server.\n" RESET);
        printf("   Make sure the server is running on %s:%d\n", SERVER_HOST, SERVER_PORT);
        printf("   Start server with: ./server\n\n");
        return 1;
    }
    close(sock);
    printf(GREEN "✅ Server connection successful\n\n" RESET);

    // Run test suite
    int total_tests = 6;
    int passed_tests = 0;

    // Test 1: Registration
    if (test_registration() == 0) passed_tests++;

    // Test 2: Login
    if (test_login() == 0) passed_tests++;

    // Test 3: Token validation
    if (test_validation() == 0) passed_tests++;

    // Test 4: JWKS endpoint
    if (test_jwks() == 0) passed_tests++;

    // Test 5: Logout
    if (test_logout() == 0) passed_tests++;

    // Test 6: Validation after logout
    if (test_validation_after_logout() == 0) passed_tests++;

    // Print summary
    print_test_summary(total_tests, passed_tests);

    return passed_tests == total_tests ? 0 : 1;
}