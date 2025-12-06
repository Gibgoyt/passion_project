#include "base64url.h"
#include <string.h>
#include <stdio.h>

// BoringSSL includes with proper prefix
#include <openssl/evp.h>

// Base64URL character set
static const char BASE64URL_CHARS[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Lookup table for base64url decoding
static const unsigned char BASE64URL_DECODE_TABLE[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 255,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255, 255,
    255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 63,
    255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
    41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

size_t base64url_encode_len(size_t input_len) {
    // Base64 without padding: ceil(input_len * 4 / 3)
    return ((input_len + 2) / 3) * 4 + 1; // +1 for null terminator
}

size_t base64url_decode_len(size_t input_len) {
    // Calculate required buffer size with proper padding alignment
    // This ensures adequate space for BoringSSL's EVP_DecodeBase64
    return ((input_len + 3) / 4) * 3;
}

int base64url_encode(const unsigned char *input, size_t input_len,
                     char *output, size_t output_len) {
    if (!input || !output || input_len == 0) {
        return -1;
    }

    size_t required_len = base64url_encode_len(input_len);
    if (output_len < required_len) {
        return -1;
    }

    // First encode using standard base64 with BoringSSL
    unsigned char temp_output[required_len];
    int b64_len = USOCKETS_BSSL_EVP_EncodeBlock(temp_output, input, input_len);
    if (b64_len <= 0) {
        return -1;
    }

    // Convert standard base64 to base64url
    int out_pos = 0;
    for (int i = 0; i < b64_len && temp_output[i] != '=' && out_pos < (int)(output_len - 1); i++) {
        char c = temp_output[i];
        if (c == '+') {
            output[out_pos++] = '-';
        } else if (c == '/') {
            output[out_pos++] = '_';
        } else if (c != '=') {
            output[out_pos++] = c;
        }
        // Skip padding characters ('=')
    }

    output[out_pos] = '\0';
    return out_pos;
}

int base64url_decode(const char *input, size_t input_len,
                     unsigned char *output, size_t output_len) {
    if (!input || !output) {
        return -1;
    }

    if (input_len == 0) {
        input_len = strlen(input);
    }

    if (input_len == 0) {
        return 0;
    }

    // Check output buffer size
    size_t required_output_len = base64url_decode_len(input_len);
    if (output_len < required_output_len) {
        return -1;
    }

    // Convert base64url to standard base64 for BoringSSL
    // Calculate padding needed
    size_t padded_len = input_len;
    while (padded_len % 4 != 0) {
        padded_len++;
    }

    char standard_b64[padded_len + 1];
    size_t i;

    // Convert characters and validate
    for (i = 0; i < input_len; i++) {
        char c = input[i];
        if (c == '-') {
            standard_b64[i] = '+';
        } else if (c == '_') {
            standard_b64[i] = '/';
        } else if (is_base64url_char(c)) {
            standard_b64[i] = c;
        } else {
            return -1; // Invalid character
        }
    }

    // Add padding
    for (; i < padded_len; i++) {
        standard_b64[i] = '=';
    }
    standard_b64[padded_len] = '\0';

    // Decode using BoringSSL
    size_t actual_output_len = 0;
    size_t actual_string_len = strlen(standard_b64);
    int result = USOCKETS_BSSL_EVP_DecodeBase64(output, &actual_output_len,
                                               output_len, (unsigned char*)standard_b64, actual_string_len);

    if (result == 1) {
        return (int)actual_output_len;
    } else {
        return -1;
    }
}

int base64url_encode_json(const char *json_string, char *output, size_t output_len) {
    if (!json_string) {
        return -1;
    }

    return base64url_encode((unsigned char*)json_string, strlen(json_string),
                           output, output_len);
}

int base64url_decode_json(const char *base64url_string, char *output, size_t output_len) {
    if (!base64url_string || !output || output_len == 0) {
        return -1;
    }

    // Decode to binary first
    int decoded_len = base64url_decode(base64url_string, 0,
                                      (unsigned char*)output, output_len - 1);
    if (decoded_len < 0) {
        return -1;
    }

    // Null terminate for string use
    output[decoded_len] = '\0';
    return decoded_len;
}

int is_base64url_char(char c) {
    // A-Z, a-z, 0-9, -, _
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_');
}

int remove_base64_padding(char *str) {
    if (!str) {
        return -1;
    }

    int len = strlen(str);

    // Remove trailing '=' characters
    while (len > 0 && str[len - 1] == '=') {
        str[len - 1] = '\0';
        len--;
    }

    return len;
}

#ifdef BASE64URL_TEST_MAIN
/**
 * Test program for base64url encoding/decoding
 * Compile with: gcc -DBASE64URL_TEST_MAIN base64url.c -o test_base64url
 */
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Testing Base64URL Encoding/Decoding\n");
    printf("==================================\n\n");

    // Test cases for base64url encoding
    struct {
        const char *input;
        const char *expected;
    } test_cases[] = {
        {"", ""},
        {"f", "Zg"},
        {"fo", "Zm8"},
        {"foo", "Zm9v"},
        {"foob", "Zm9vYg"},
        {"fooba", "Zm9vYmE"},
        {"foobar", "Zm9vYmFy"},
        {"Hello, World!", "SGVsbG8sIFdvcmxkIQ"},
        {"{\"alg\":\"RS256\",\"typ\":\"JWT\"}", "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9"},
    };

    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    // Test encoding
    printf("Testing encoding:\n");
    for (int i = 0; i < num_tests; i++) {
        const char *input = test_cases[i].input;
        const char *expected = test_cases[i].expected;

        size_t out_len = base64url_encode_len(strlen(input));
        char *encoded = malloc(out_len);

        int result = base64url_encode((unsigned char*)input, strlen(input), encoded, out_len);

        printf("Input: '%s'\n", input);
        printf("Expected: '%s'\n", expected);
        printf("Actual: '%s'\n", encoded);
        printf("Result: %s\n", (result >= 0 && strcmp(encoded, expected) == 0) ? "✓ PASS" : "✗ FAIL");
        printf("\n");

        // Test decoding
        if (strlen(expected) > 0) {
            size_t decode_len = base64url_decode_len(strlen(expected));
            unsigned char *decoded = malloc(decode_len + 1);

            int decode_result = base64url_decode(expected, 0, decoded, decode_len);
            if (decode_result >= 0) {
                decoded[decode_result] = '\0';
                printf("Decode back: '%s'\n", decoded);
                printf("Round-trip: %s\n",
                       (decode_result == (int)strlen(input) &&
                        memcmp(decoded, input, decode_result) == 0) ? "✓ PASS" : "✗ FAIL");
            } else {
                printf("Decode failed: ✗ FAIL\n");
            }
            free(decoded);
        }

        free(encoded);
        printf("\n");
    }

    // Test JSON convenience functions
    printf("Testing JSON convenience functions:\n");
    const char *jwt_header = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
    char encoded_header[256];
    char decoded_header[256];

    int encode_len = base64url_encode_json(jwt_header, encoded_header, sizeof(encoded_header));
    printf("JSON Header: %s\n", jwt_header);
    printf("Encoded: %s\n", encoded_header);

    if (encode_len > 0) {
        int decode_len = base64url_decode_json(encoded_header, decoded_header, sizeof(decoded_header));
        printf("Decoded: %s\n", decoded_header);
        printf("JSON Round-trip: %s\n",
               strcmp(jwt_header, decoded_header) == 0 ? "✓ PASS" : "✗ FAIL");
    }

    return 0;
}
#endif