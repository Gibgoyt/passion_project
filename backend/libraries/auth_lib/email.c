#include "email.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int is_valid_email(const char *email) {
    if (!email) {
        return 0;
    }

    size_t len = strlen(email);

    // Check total length
    if (len == 0 || len > MAX_EMAIL_LENGTH) {
        return 0;
    }

    // Must contain exactly one '@'
    const char *at_pos = strchr(email, '@');
    if (!at_pos || strchr(at_pos + 1, '@')) {
        return 0; // No @ or multiple @
    }

    // Extract local and domain parts
    char local[MAX_LOCAL_LENGTH + 1];
    char domain[MAX_DOMAIN_LENGTH + 1];

    if (extract_email_parts(email, local, domain) != 0) {
        return 0;
    }

    // Validate local part length
    if (strlen(local) == 0 || strlen(local) > MAX_LOCAL_LENGTH) {
        return 0;
    }

    // Validate domain part length
    if (strlen(domain) == 0 || strlen(domain) > MAX_DOMAIN_LENGTH) {
        return 0;
    }

    // Validate local part characters
    for (size_t i = 0; local[i]; i++) {
        if (!is_valid_local_char(local[i])) {
            return 0;
        }
    }

    // Validate domain
    return is_valid_domain(domain);
}

int normalize_email(const char *email, char *normalized_out) {
    if (!email || !normalized_out) {
        return -1;
    }

    // First, trim whitespace and copy to output buffer
    const char *start = email;
    const char *end = email + strlen(email) - 1;

    // Skip leading whitespace
    while (*start && isspace(*start)) {
        start++;
    }

    // Skip trailing whitespace
    while (end > start && isspace(*end)) {
        end--;
    }

    // Calculate trimmed length
    size_t trimmed_len = end - start + 1;

    if (trimmed_len > MAX_EMAIL_LENGTH) {
        return -1;
    }

    // Copy and convert to lowercase
    size_t i;
    for (i = 0; i < trimmed_len; i++) {
        normalized_out[i] = tolower(start[i]);
    }
    normalized_out[i] = '\0';

    // Validate the normalized email
    if (!is_valid_email(normalized_out)) {
        return -1;
    }

    return 0;
}

int is_valid_local_char(char c) {
    // Alphanumeric
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
        return 1;
    }

    // Special characters allowed in local part
    if (c == '.' || c == '_' || c == '%' || c == '+' || c == '-') {
        return 1;
    }

    return 0;
}

int is_valid_domain_char(char c) {
    // Alphanumeric
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
        return 1;
    }

    // Special characters allowed in domain part
    if (c == '.' || c == '-') {
        return 1;
    }

    return 0;
}

int is_valid_domain(const char *domain) {
    if (!domain || strlen(domain) == 0) {
        return 0;
    }

    size_t len = strlen(domain);

    // Domain cannot start or end with dot or hyphen
    if (domain[0] == '.' || domain[0] == '-' ||
        domain[len - 1] == '.' || domain[len - 1] == '-') {
        return 0;
    }

    // Check for at least one dot (for TLD separation)
    if (!strchr(domain, '.')) {
        return 0;
    }

    // Validate characters and check for consecutive dots
    char prev_char = '\0';
    int dots_found = 0;
    for (size_t i = 0; i < len; i++) {
        char c = domain[i];

        // Check valid character
        if (!is_valid_domain_char(c)) {
            return 0;
        }

        // Check for consecutive dots
        if (c == '.' && prev_char == '.') {
            return 0;
        }

        if (c == '.') {
            dots_found++;
        }

        prev_char = c;
    }

    // Must have at least one dot
    if (dots_found == 0) {
        return 0;
    }

    // Check TLD (last part after final dot)
    const char *last_dot = strrchr(domain, '.');
    if (last_dot) {
        const char *tld = last_dot + 1;
        size_t tld_len = strlen(tld);

        // TLD must be at least 2 characters
        if (tld_len < 2) {
            return 0;
        }

        // TLD must be all alphabetic
        for (size_t i = 0; tld[i]; i++) {
            if (!((tld[i] >= 'a' && tld[i] <= 'z') ||
                  (tld[i] >= 'A' && tld[i] <= 'Z'))) {
                return 0;
            }
        }
    }

    return 1;
}

int extract_email_parts(const char *email, char *local_out, char *domain_out) {
    if (!email || !local_out || !domain_out) {
        return -1;
    }

    const char *at_pos = strchr(email, '@');
    if (!at_pos) {
        return -1;
    }

    // Calculate lengths
    size_t local_len = at_pos - email;
    size_t domain_len = strlen(at_pos + 1);

    // Check length limits
    if (local_len > MAX_LOCAL_LENGTH || domain_len > MAX_DOMAIN_LENGTH) {
        return -1;
    }

    // Copy local part
    strncpy(local_out, email, local_len);
    local_out[local_len] = '\0';

    // Copy domain part
    strcpy(domain_out, at_pos + 1);

    return 0;
}

#ifdef EMAIL_TEST_MAIN
/**
 * Test program for email validation
 * Compile with: gcc -DEMAIL_TEST_MAIN email.c -o test_email
 */
#include <stdio.h>

int main() {
    printf("Testing Email Validation\n");
    printf("=======================\n\n");

    // Test cases
    const char *test_emails[] = {
        // Valid emails
        "user@example.com",
        "test.email+tag@domain.org",
        "user_name@test-domain.co.uk",
        "simple@domain.io",
        "email123@test.museum",

        // Invalid emails
        "invalid",              // No @
        "user@@domain.com",     // Double @
        "@domain.com",          // No local part
        "user@",                // No domain
        "user@domain",          // No TLD
        "user@domain.c",        // TLD too short
        "user@domain..com",     // Consecutive dots
        "user@.domain.com",     // Domain starts with dot
        "user@domain.com.",     // Domain ends with dot
        "user@domain-.com",     // Domain has hyphen at start
        "user@domain.-com",     // Invalid hyphen position
        "user with spaces@domain.com", // Spaces in local
        "user@dom ain.com",     // Spaces in domain
        "",                     // Empty string
        "a@b.c",               // Very short but valid
    };

    int num_tests = sizeof(test_emails) / sizeof(test_emails[0]);

    for (int i = 0; i < num_tests; i++) {
        const char *email = test_emails[i];
        int valid = is_valid_email(email);

        printf("Email: '%s'\n", email);
        printf("  Valid: %s\n", valid ? "✓ Yes" : "✗ No");

        if (valid) {
            char normalized[MAX_EMAIL_LENGTH + 1];
            if (normalize_email(email, normalized) == 0) {
                printf("  Normalized: '%s'\n", normalized);
            }

            // Extract parts
            char local[MAX_LOCAL_LENGTH + 1];
            char domain[MAX_DOMAIN_LENGTH + 1];
            if (extract_email_parts(email, local, domain) == 0) {
                printf("  Local: '%s', Domain: '%s'\n", local, domain);
            }
        }
        printf("\n");
    }

    return 0;
}
#endif