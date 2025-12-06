#ifndef BASE64URL_H
#define BASE64URL_H

#include <stddef.h>

/**
 * Base64URL Encoding/Decoding for JWTs
 *
 * Base64URL is a URL-safe variant of base64 encoding used in JWTs.
 * Key differences from standard base64:
 * - Uses '-' instead of '+'
 * - Uses '_' instead of '/'
 * - No padding '=' characters
 * - URL-safe (no special characters that need encoding)
 *
 * This is required for JWT implementation as per RFC 7515.
 */

/**
 * Calculate the output length for base64url encoding
 *
 * Base64URL encoding produces 4 characters for every 3 bytes of input,
 * but without padding, so the formula is: ceil(input_len * 4 / 3)
 *
 * @param input_len Length of input data in bytes
 * @return Required output buffer size (including space for null terminator)
 */
size_t base64url_encode_len(size_t input_len);

/**
 * Calculate the maximum output length for base64url decoding
 *
 * Base64URL decoding produces 3 bytes for every 4 characters of input.
 * This function calculates the maximum possible output size.
 *
 * @param input_len Length of input string in characters
 * @return Maximum output buffer size needed
 */
size_t base64url_decode_len(size_t input_len);

/**
 * Encode binary data to base64url string
 *
 * Converts binary data to base64url encoding suitable for use in JWTs.
 * Output is null-terminated and contains no padding.
 *
 * @param input Binary data to encode
 * @param input_len Length of input data in bytes
 * @param output Buffer for encoded string (use base64url_encode_len() for size)
 * @param output_len Size of output buffer
 * @return Length of encoded string on success, -1 on error
 *
 * Example usage:
 *   unsigned char data[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f}; // "Hello"
 *   size_t out_len = base64url_encode_len(5);
 *   char *encoded = malloc(out_len);
 *   int result = base64url_encode(data, 5, encoded, out_len);
 *   printf("Encoded: %s\n", encoded); // "SGVsbG8"
 */
int base64url_encode(const unsigned char *input, size_t input_len,
                     char *output, size_t output_len);

/**
 * Decode base64url string to binary data
 *
 * Converts base64url encoded string back to binary data.
 * Input string should not contain padding characters.
 *
 * @param input Base64url encoded string
 * @param input_len Length of input string (or 0 to auto-detect)
 * @param output Buffer for decoded data (use base64url_decode_len() for size)
 * @param output_len Size of output buffer
 * @return Length of decoded data on success, -1 on error
 *
 * Example usage:
 *   const char *encoded = "SGVsbG8";
 *   size_t out_len = base64url_decode_len(strlen(encoded));
 *   unsigned char *decoded = malloc(out_len);
 *   int result = base64url_decode(encoded, 0, decoded, out_len);
 *   printf("Decoded: %.*s\n", result, decoded); // "Hello"
 */
int base64url_decode(const char *input, size_t input_len,
                     unsigned char *output, size_t output_len);

/**
 * Encode JSON string to base64url (for JWT headers/payloads)
 *
 * Convenience function for encoding JSON strings in JWT tokens.
 * This is commonly needed for JWT header and payload encoding.
 *
 * @param json_string JSON string to encode
 * @param output Buffer for encoded string
 * @param output_len Size of output buffer
 * @return Length of encoded string on success, -1 on error
 *
 * Example usage:
 *   const char *header = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
 *   char encoded[256];
 *   int len = base64url_encode_json(header, encoded, sizeof(encoded));
 */
int base64url_encode_json(const char *json_string, char *output, size_t output_len);

/**
 * Decode base64url to JSON string (for JWT verification)
 *
 * Convenience function for decoding JWT components back to JSON.
 * Ensures the result is null-terminated for use as a string.
 *
 * @param base64url_string Base64url encoded string
 * @param output Buffer for decoded JSON string
 * @param output_len Size of output buffer
 * @return Length of decoded string on success, -1 on error
 *
 * Example usage:
 *   const char *encoded = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9";
 *   char json[256];
 *   int len = base64url_decode_json(encoded, json, sizeof(json));
 */
int base64url_decode_json(const char *base64url_string, char *output, size_t output_len);

/**
 * Check if character is valid base64url character
 *
 * Valid characters: A-Z, a-z, 0-9, -, _
 *
 * @param c Character to check
 * @return 1 if valid, 0 if invalid
 */
int is_base64url_char(char c);

/**
 * Remove padding from base64 string (convert to base64url)
 *
 * Removes trailing '=' characters from standard base64 to create
 * base64url format. Modifies the string in-place.
 *
 * @param str String to modify (must be null-terminated)
 * @return Length of string after padding removal
 */
int remove_base64_padding(char *str);

#endif // BASE64URL_H