#include "base64url.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * Convert base64url string to base64 string
 */
char* base64url_to_base64(const char* base64url, size_t* out_len) {
    if (!base64url || !out_len) {
        return NULL;
    }

    size_t input_len = strlen(base64url);
    if (input_len == 0) {
        *out_len = 0;
        char* result = malloc(1);
        if (result) result[0] = '\0';
        return result;
    }

    // Calculate padded length (must be multiple of 4)
    size_t padded_len = input_len;
    while (padded_len % 4 != 0) {
        padded_len++;
    }

    // Allocate memory for result
    char* base64 = malloc(padded_len + 1);
    if (!base64) {
        return NULL;
    }

    // Convert characters: - to +, _ to /
    for (size_t i = 0; i < input_len; i++) {
        char c = base64url[i];
        if (c == '-') {
            base64[i] = '+';
        } else if (c == '_') {
            base64[i] = '/';
        } else {
            base64[i] = c;
        }
    }

    // Add padding
    for (size_t i = input_len; i < padded_len; i++) {
        base64[i] = '=';
    }

    base64[padded_len] = '\0';
    *out_len = padded_len;

    return base64;
}

/**
 * Convert base64 string to base64url string
 */
char* base64_to_base64url(const char* base64, size_t* out_len) {
    if (!base64 || !out_len) {
        return NULL;
    }

    size_t input_len = strlen(base64);
    if (input_len == 0) {
        *out_len = 0;
        char* result = malloc(1);
        if (result) result[0] = '\0';
        return result;
    }

    // Count padding characters
    size_t padding = 0;
    if (input_len > 0 && base64[input_len - 1] == '=') padding++;
    if (input_len > 1 && base64[input_len - 2] == '=') padding++;

    // Calculate output length (without padding)
    size_t output_len = input_len - padding;

    // Allocate memory for result
    char* base64url = malloc(output_len + 1);
    if (!base64url) {
        return NULL;
    }

    // Convert characters: + to -, / to _, remove padding
    for (size_t i = 0; i < output_len; i++) {
        char c = base64[i];
        if (c == '+') {
            base64url[i] = '-';
        } else if (c == '/') {
            base64url[i] = '_';
        } else {
            base64url[i] = c;
        }
    }

    base64url[output_len] = '\0';
    *out_len = output_len;

    return base64url;
}

/**
 * Calculate the required base64 length for a given base64url string
 */
size_t calculate_base64_length(size_t base64url_len) {
    // Round up to nearest multiple of 4
    return ((base64url_len + 3) / 4) * 4;
}

/**
 * Calculate the required base64url length for a given base64 string
 */
size_t calculate_base64url_length(size_t base64_len) {
    // Remove padding to get actual data length
    // This is an approximation - actual calculation would need to examine the string
    return base64_len; // Maximum possible length
}

/**
 * Validate base64url character set
 */
int validate_base64url(const char* base64url, size_t len) {
    if (!base64url) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        char c = base64url[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-' || c == '_')) {
            return 0;
        }
    }

    return 1;
}

/**
 * Validate base64 character set
 */
int validate_base64(const char* base64, size_t len) {
    if (!base64) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        char c = base64[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '+' || c == '/' || c == '=')) {
            return 0;
        }
    }

    return 1;
}

/**
 * Print character analysis for debugging
 */
void print_character_analysis(const char* data, size_t len, const char* name) {
    if (!data || !name) {
        return;
    }

    printf("\n🔍 CHARACTER ANALYSIS: %s\n", name);
    printf("Length: %zu\n", len);
    printf("Length mod 4: %zu\n", len % 4);

    // Character frequency analysis
    int uppercase = 0, lowercase = 0, digits = 0;
    int base64_special = 0, base64url_special = 0, padding = 0, invalid = 0;

    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        if (c >= 'A' && c <= 'Z') uppercase++;
        else if (c >= 'a' && c <= 'z') lowercase++;
        else if (c >= '0' && c <= '9') digits++;
        else if (c == '+' || c == '/') base64_special++;
        else if (c == '-' || c == '_') base64url_special++;
        else if (c == '=') padding++;
        else invalid++;
    }

    printf("Character distribution:\n");
    printf("  Uppercase: %d\n", uppercase);
    printf("  Lowercase: %d\n", lowercase);
    printf("  Digits: %d\n", digits);
    printf("  Base64 special (+/): %d\n", base64_special);
    printf("  Base64url special (-_): %d\n", base64url_special);
    printf("  Padding (=): %d\n", padding);
    printf("  Invalid: %d\n", invalid);

    // Show invalid characters
    if (invalid > 0) {
        printf("Invalid characters found at positions:\n");
        for (size_t i = 0; i < len && i < 100; i++) { // Limit to first 100 chars
            char c = data[i];
            if (!((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '+' || c == '/' || c == '=' ||
                  c == '-' || c == '_')) {
                printf("  Position %zu: '%c' (ASCII %d)\n", i, c, (int)c);
            }
        }
    }

    // Show first and last few characters
    printf("First 20 characters: ");
    for (size_t i = 0; i < len && i < 20; i++) {
        printf("%c", data[i]);
    }
    printf("\n");

    if (len > 20) {
        printf("Last 20 characters: ");
        size_t start = len > 20 ? len - 20 : 0;
        for (size_t i = start; i < len; i++) {
            printf("%c", data[i]);
        }
        printf("\n");
    }
}

/**
 * Print hex dump for debugging
 */
void print_hex_dump(const unsigned char* data, size_t len, const char* name, size_t max_len) {
    if (!data || !name) {
        return;
    }

    size_t display_len = (max_len > 0 && max_len < len) ? max_len : len;

    printf("\n🔍 HEX DUMP: %s (showing %zu/%zu bytes)\n", name, display_len, len);

    for (size_t i = 0; i < display_len; i += 16) {
        printf("%08zx  ", i);

        // Hex bytes
        for (size_t j = 0; j < 16; j++) {
            if (i + j < display_len) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
            if (j == 7) printf(" ");
        }

        printf(" |");

        // ASCII representation
        for (size_t j = 0; j < 16 && i + j < display_len; j++) {
            unsigned char c = data[i + j];
            printf("%c", isprint(c) ? c : '.');
        }

        printf("|\n");
    }

    if (max_len > 0 && len > max_len) {
        printf("... (%zu more bytes)\n", len - max_len);
    }
}