#ifndef EMAIL_H
#define EMAIL_H

#include <stddef.h>

/**
 * Email Validation and Normalization
 *
 * Simple email validation without external regex dependencies.
 * Follows RFC 5322 simplified rules for practical use cases.
 *
 * Validation rules:
 * - Must contain exactly one '@' symbol
 * - Local part (before @): 1-64 characters, alphanumeric + ._%+-
 * - Domain part (after @): 1-253 characters, alphanumeric + .-
 * - Domain must contain at least one dot
 * - TLD must be 2+ characters
 * - Total length must be <= 254 characters
 */

// Maximum email length according to RFC 5322
#define MAX_EMAIL_LENGTH 254

// Maximum local part length (before @)
#define MAX_LOCAL_LENGTH 64

// Maximum domain part length (after @)
#define MAX_DOMAIN_LENGTH 253

/**
 * Validate email address format
 *
 * Performs comprehensive validation of email format according to
 * simplified RFC 5322 rules suitable for most applications.
 *
 * @param email The email address to validate
 * @return 1 if valid, 0 if invalid
 *
 * Example usage:
 *   if (is_valid_email("user@example.com")) {
 *       printf("Valid email\n");
 *   }
 */
int is_valid_email(const char *email);

/**
 * Normalize email address for storage
 *
 * Performs normalization operations:
 * - Converts to lowercase
 * - Trims leading/trailing whitespace
 * - Validates format during normalization
 *
 * @param email Input email address
 * @param normalized_out Buffer for normalized email (must be >= MAX_EMAIL_LENGTH + 1)
 * @return 0 on success, -1 on error (invalid format or buffer too small)
 *
 * Example usage:
 *   char normalized[MAX_EMAIL_LENGTH + 1];
 *   if (normalize_email("User@EXAMPLE.COM", normalized) == 0) {
 *       printf("Normalized: %s\n", normalized); // "user@example.com"
 *   }
 */
int normalize_email(const char *email, char *normalized_out);

/**
 * Check if character is valid for email local part
 *
 * Valid characters for local part (before @):
 * - Alphanumeric: a-z, A-Z, 0-9
 * - Special: . _ % + -
 *
 * @param c Character to check
 * @return 1 if valid, 0 if invalid
 */
int is_valid_local_char(char c);

/**
 * Check if character is valid for email domain part
 *
 * Valid characters for domain part (after @):
 * - Alphanumeric: a-z, A-Z, 0-9
 * - Special: . -
 *
 * @param c Character to check
 * @return 1 if valid, 0 if invalid
 */
int is_valid_domain_char(char c);

/**
 * Validate domain part of email
 *
 * Checks that domain:
 * - Contains only valid characters
 * - Has at least one dot
 * - TLD is at least 2 characters
 * - No consecutive dots
 * - Doesn't start or end with dot/hyphen
 *
 * @param domain The domain part to validate
 * @return 1 if valid, 0 if invalid
 */
int is_valid_domain(const char *domain);

/**
 * Extract local and domain parts from email
 *
 * Splits email at '@' symbol and extracts parts for separate validation.
 *
 * @param email Complete email address
 * @param local_out Buffer for local part (must be >= MAX_LOCAL_LENGTH + 1)
 * @param domain_out Buffer for domain part (must be >= MAX_DOMAIN_LENGTH + 1)
 * @return 0 on success, -1 on error
 */
int extract_email_parts(const char *email, char *local_out, char *domain_out);

#endif // EMAIL_H