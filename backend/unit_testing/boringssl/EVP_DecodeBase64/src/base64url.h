#ifndef BASE64URL_H
#define BASE64URL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Base64URL Conversion Library
 *
 * This library provides functions to convert between base64url encoding
 * (as used in JWT) and standard base64 encoding (as expected by BoringSSL).
 *
 * Base64URL differences from Base64:
 * - Uses '-' instead of '+'
 * - Uses '_' instead of '/'
 * - Omits padding '=' characters
 */

/**
 * Convert base64url string to base64 string
 *
 * @param base64url Input base64url encoded string (null-terminated)
 * @param out_len   Pointer to store the length of the output string
 * @return          Allocated base64 string (caller must free), or NULL on error
 *
 * The function performs:
 * 1. Character substitution: '-' -> '+', '_' -> '/'
 * 2. Padding addition to make length multiple of 4
 * 3. Memory allocation for the result
 */
char* base64url_to_base64(const char* base64url, size_t* out_len);

/**
 * Convert base64 string to base64url string
 *
 * @param base64   Input base64 encoded string (null-terminated)
 * @param out_len  Pointer to store the length of the output string
 * @return         Allocated base64url string (caller must free), or NULL on error
 *
 * The function performs:
 * 1. Character substitution: '+' -> '-', '/' -> '_'
 * 2. Padding removal
 * 3. Memory allocation for the result
 */
char* base64_to_base64url(const char* base64, size_t* out_len);

/**
 * Calculate the required base64 length for a given base64url string
 *
 * @param base64url_len Length of the base64url string
 * @return              Required length for base64 representation (with padding)
 */
size_t calculate_base64_length(size_t base64url_len);

/**
 * Calculate the required base64url length for a given base64 string
 *
 * @param base64_len Length of the base64 string
 * @return           Required length for base64url representation (without padding)
 */
size_t calculate_base64url_length(size_t base64_len);

/**
 * Validate base64url character set
 *
 * @param base64url Input string to validate
 * @param len       Length of input string
 * @return          1 if valid, 0 if invalid characters found
 */
int validate_base64url(const char* base64url, size_t len);

/**
 * Validate base64 character set
 *
 * @param base64 Input string to validate
 * @param len    Length of input string
 * @return       1 if valid, 0 if invalid characters found
 */
int validate_base64(const char* base64, size_t len);

/**
 * Print character analysis for debugging
 *
 * @param data Input string to analyze
 * @param len  Length of input string
 * @param name Name/description for the analysis
 */
void print_character_analysis(const char* data, size_t len, const char* name);

/**
 * Print hex dump for debugging
 *
 * @param data    Input data to dump
 * @param len     Length of input data
 * @param name    Name/description for the dump
 * @param max_len Maximum bytes to display (0 for all)
 */
void print_hex_dump(const unsigned char* data, size_t len, const char* name, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* BASE64URL_H */