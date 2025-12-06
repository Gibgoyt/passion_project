#include "jwks.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void jwks_init(jwks_t *jwks) {
    if (!jwks) return;

    memset(jwks, 0, sizeof(jwks_t));
    jwks->key_count = 0;

    // Initialize all keys as inactive
    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        jwks->keys[i].is_active = 0;
    }
}

int jwks_create_jwk(const rsa_keypair_t *keypair, jwk_t *jwk_out) {
    if (!keypair || !jwk_out || !keypair->public_key) {
        return -1;
    }

    memset(jwk_out, 0, sizeof(jwk_t));

    // Set standard JWK fields
    strcpy(jwk_out->kty, "RSA");
    strcpy(jwk_out->use, "sig");
    strcpy(jwk_out->alg, "RS256");
    strncpy(jwk_out->kid, keypair->key_id, sizeof(jwk_out->kid) - 1);

    // Extract RSA public key components (n, e)
    if (rsa_get_public_key_components(keypair->public_key,
                                     jwk_out->n, sizeof(jwk_out->n),
                                     jwk_out->e, sizeof(jwk_out->e)) != 0) {
        return -1;
    }

    jwk_out->is_active = 1;
    return 0;
}

int jwks_validate_jwk(const jwk_t *jwk) {
    if (!jwk) return 0;

    // Check required fields
    if (strcmp(jwk->kty, "RSA") != 0) return 0;
    if (strcmp(jwk->use, "sig") != 0) return 0;
    if (strcmp(jwk->alg, "RS256") != 0) return 0;
    if (strlen(jwk->kid) == 0) return 0;
    if (strlen(jwk->n) == 0) return 0;
    if (strlen(jwk->e) == 0) return 0;

    return 1;
}

int jwks_add_key(jwks_t *jwks, const rsa_keypair_t *keypair) {
    if (!jwks || !keypair) {
        return -1;
    }

    // Check if we have space for more keys
    if (jwks->key_count >= JWKS_MAX_KEYS) {
        return -1;
    }

    // Find first inactive slot
    int slot = -1;
    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        if (!jwks->keys[i].is_active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return -1; // No available slots
    }

    // Create JWK from keypair
    jwk_t jwk;
    if (jwks_create_jwk(keypair, &jwk) != 0) {
        return -1;
    }

    // Add to JWKS
    jwks->keys[slot] = jwk;
    jwks->key_count++;

    return 0;
}

jwk_t *jwks_find_key(const jwks_t *jwks, const char *kid) {
    if (!jwks || !kid) {
        return NULL;
    }

    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        if (jwks->keys[i].is_active &&
            strcmp(jwks->keys[i].kid, kid) == 0) {
            return &jwks->keys[i];
        }
    }

    return NULL;
}

int jwks_remove_key(jwks_t *jwks, const char *kid) {
    if (!jwks || !kid) {
        return -1;
    }

    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        if (jwks->keys[i].is_active &&
            strcmp(jwks->keys[i].kid, kid) == 0) {
            jwks->keys[i].is_active = 0;
            jwks->key_count--;
            return 0;
        }
    }

    return -1; // Key not found
}

int jwks_has_active_keys(const jwks_t *jwks) {
    return jwks && jwks->key_count > 0;
}

jwk_t *jwks_get_primary_key(const jwks_t *jwks) {
    if (!jwks) return NULL;

    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        if (jwks->keys[i].is_active) {
            return &jwks->keys[i];
        }
    }

    return NULL;
}

int jwks_format_jwk_json(const jwk_t *jwk, char *json_out, size_t json_len) {
    if (!jwk || !json_out || !jwks_validate_jwk(jwk)) {
        return -1;
    }

    int written = snprintf(json_out, json_len,
        "{"
        "\"kty\":\"%s\","
        "\"kid\":\"%s\","
        "\"use\":\"%s\","
        "\"alg\":\"%s\","
        "\"n\":\"%s\","
        "\"e\":\"%s\""
        "}",
        jwk->kty, jwk->kid, jwk->use, jwk->alg, jwk->n, jwk->e);

    if (written >= (int)json_len || written < 0) {
        return -1;
    }

    return 0;
}

int jwks_generate_json(const jwks_t *jwks, char *json_out, size_t json_len) {
    if (!jwks || !json_out || json_len == 0) {
        return -1;
    }

    // Start JWKS object
    int pos = snprintf(json_out, json_len, "{\"keys\":[");
    if (pos >= (int)json_len || pos < 0) {
        return -1;
    }

    // Add each active key
    int first_key = 1;
    for (int i = 0; i < JWKS_MAX_KEYS; i++) {
        if (!jwks->keys[i].is_active) continue;

        // Add comma separator for subsequent keys
        if (!first_key) {
            if (pos + 1 >= (int)json_len) return -1;
            json_out[pos++] = ',';
        }

        // Format this JWK
        char jwk_json[1024];
        if (jwks_format_jwk_json(&jwks->keys[i], jwk_json, sizeof(jwk_json)) != 0) {
            return -1;
        }

        // Append to output
        int jwk_len = strlen(jwk_json);
        if (pos + jwk_len >= (int)json_len) {
            return -1;
        }

        strcpy(json_out + pos, jwk_json);
        pos += jwk_len;
        first_key = 0;
    }

    // Close JWKS object
    if (pos + 2 >= (int)json_len) return -1;
    strcpy(json_out + pos, "]}");

    return 0;
}

int jwks_generate_http_response(const jwks_t *jwks,
                               char *response_out,
                               size_t response_len) {
    if (!jwks || !response_out || response_len == 0) {
        return -1;
    }

    // Generate JWKS JSON first
    char jwks_json[JWKS_MAX_SIZE];
    if (jwks_generate_json(jwks, jwks_json, sizeof(jwks_json)) != 0) {
        return -1;
    }

    // Calculate content length
    int content_length = strlen(jwks_json);

    // Format HTTP response
    int written = snprintf(response_out, response_len,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Cache-Control: public, max-age=3600\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        content_length, jwks_json);

    if (written >= (int)response_len || written < 0) {
        return -1;
    }

    return 0;
}

int jwks_load_from_keypairs(jwks_t *jwks,
                           const rsa_keypair_t *keypairs,
                           int num_keys) {
    if (!jwks || !keypairs || num_keys <= 0) {
        return -1;
    }

    jwks_init(jwks);

    for (int i = 0; i < num_keys; i++) {
        if (jwks_add_key(jwks, &keypairs[i]) != 0) {
            return -1; // Failed to add key
        }
    }

    return 0;
}

#ifdef JWKS_TEST_MAIN
/**
 * Test program for JWKS implementation
 * Compile with: gcc -DJWKS_TEST_MAIN jwks.c -o test_jwks
 */
#include <stdio.h>

int main() {
    printf("Testing JWKS Implementation\n");
    printf("==========================\n\n");

    // Generate test RSA keypair
    rsa_keypair_t keypair;
    rsa_init_keypair(&keypair);

    if (rsa_generate_keypair(&keypair) != 0) {
        printf("✗ Failed to generate RSA keypair\n");
        return 1;
    }

    printf("✓ Generated RSA keypair (Key ID: %s)\n", keypair.key_id);

    // Test JWK creation
    printf("\nTesting JWK creation:\n");
    jwk_t jwk;
    if (jwks_create_jwk(&keypair, &jwk) != 0) {
        printf("✗ Failed to create JWK\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    printf("✓ Created JWK\n");
    printf("  Key Type: %s\n", jwk.kty);
    printf("  Key ID: %s\n", jwk.kid);
    printf("  Use: %s\n", jwk.use);
    printf("  Algorithm: %s\n", jwk.alg);
    printf("  Modulus (n): %.40s...\n", jwk.n);
    printf("  Exponent (e): %s\n", jwk.e);

    // Test JWK validation
    printf("\nTesting JWK validation:\n");
    int valid = jwks_validate_jwk(&jwk);
    printf("  JWK validation: %s\n", valid ? "✓ Valid" : "✗ Invalid");

    // Test JWKS operations
    printf("\nTesting JWKS operations:\n");
    jwks_t jwks;
    jwks_init(&jwks);

    if (jwks_add_key(&jwks, &keypair) != 0) {
        printf("✗ Failed to add key to JWKS\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    printf("✓ Added key to JWKS\n");
    printf("  Key count: %d\n", jwks.key_count);
    printf("  Has active keys: %s\n", jwks_has_active_keys(&jwks) ? "Yes" : "No");

    // Test key lookup
    jwk_t *found_key = jwks_find_key(&jwks, keypair.key_id);
    printf("  Key lookup: %s\n", found_key ? "✓ Found" : "✗ Not found");

    // Test primary key
    jwk_t *primary = jwks_get_primary_key(&jwks);
    printf("  Primary key: %s\n", primary ? "✓ Found" : "✗ None");

    // Test single JWK JSON formatting
    printf("\nTesting JWK JSON formatting:\n");
    char jwk_json[1024];
    if (jwks_format_jwk_json(&jwk, jwk_json, sizeof(jwk_json)) == 0) {
        printf("✓ Formatted JWK as JSON\n");
        printf("  JSON: %s\n", jwk_json);
    } else {
        printf("✗ Failed to format JWK as JSON\n");
    }

    // Test JWKS JSON generation
    printf("\nTesting JWKS JSON generation:\n");
    char jwks_json[JWKS_MAX_SIZE];
    if (jwks_generate_json(&jwks, jwks_json, sizeof(jwks_json)) == 0) {
        printf("✓ Generated JWKS JSON\n");
        printf("  JWKS: %s\n", jwks_json);
    } else {
        printf("✗ Failed to generate JWKS JSON\n");
    }

    // Test HTTP response generation
    printf("\nTesting HTTP response generation:\n");
    char http_response[JWKS_MAX_SIZE * 2];
    if (jwks_generate_http_response(&jwks, http_response, sizeof(http_response)) == 0) {
        printf("✓ Generated HTTP response\n");
        printf("  Response length: %zu bytes\n", strlen(http_response));
        printf("  Response preview:\n");

        // Print first few lines of HTTP response
        char *line_end = strchr(http_response, '\r');
        if (line_end) {
            printf("    %.*s\n", (int)(line_end - http_response), http_response);
        }
    } else {
        printf("✗ Failed to generate HTTP response\n");
    }

    // Test key removal
    printf("\nTesting key removal:\n");
    if (jwks_remove_key(&jwks, keypair.key_id) == 0) {
        printf("✓ Removed key from JWKS\n");
        printf("  Key count after removal: %d\n", jwks.key_count);
        printf("  Has active keys: %s\n", jwks_has_active_keys(&jwks) ? "Yes" : "No");
    } else {
        printf("✗ Failed to remove key from JWKS\n");
    }

    rsa_free_keypair(&keypair);
    printf("\n✓ All JWKS tests completed\n");
    return 0;
}
#endif