#include "jwt_rs256.h"
#include "base64url.h"
#include "../memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// OpenSSL includes
#include <openssl/evp.h>
#include <openssl/rand.h>

void jwt_init_claims(jwt_claims_t *claims) {
    if (!claims) return;

    memset(claims, 0, sizeof(jwt_claims_t));
    claims->issued_at = time(NULL);
    claims->expires_at = 0;
    claims->email_verified = 0;
}

int jwt_generate_id(char *jti_out) {
    if (!jti_out) {
        return -1;
    }

    // Generate random bytes for JWT ID
    unsigned char random_bytes[24]; // 192 bits
    if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
        return -1;
    }

    // Encode to base64url (will be exactly 32 characters)
    int encoded_len = base64url_encode(random_bytes, sizeof(random_bytes),
                                      jti_out, JWT_ID_LENGTH + 1);

    if (encoded_len != JWT_ID_LENGTH) {
        return -1;
    }

    return 0;
}

int jwt_set_user_claims(jwt_claims_t *claims,
                       const char *user_id,
                       const char *email,
                       int email_verified,
                       const char *issuer,
                       const char *audience,
                       int lifetime_seconds) {
    if (!claims || !user_id || !email || !issuer || !audience) {
        return -1;
    }

    jwt_init_claims(claims);

    // Set standard claims
    strncpy(claims->subject, user_id, JWT_CLAIM_MAX_LENGTH);
    strncpy(claims->issuer, issuer, JWT_CLAIM_MAX_LENGTH);
    strncpy(claims->audience, audience, JWT_CLAIM_MAX_LENGTH);

    time_t now = time(NULL);
    claims->issued_at = now;
    claims->expires_at = now + lifetime_seconds;

    // Set user claims
    strncpy(claims->email, email, JWT_CLAIM_MAX_LENGTH);
    claims->email_verified = email_verified;

    // Generate unique JWT ID
    if (jwt_generate_id(claims->jwt_id) != 0) {
        return -1;
    }

    return 0;
}

int jwt_is_expired(time_t exp_timestamp) {
    return time(NULL) >= exp_timestamp;
}

int jwt_format_claims_json(const jwt_claims_t *claims,
                          char *json_out,
                          size_t json_len) {
    if (!claims || !json_out || json_len == 0) {
        return -1;
    }

    int written = snprintf(json_out, json_len,
        "{"
        "\"iss\":\"%s\","
        "\"sub\":\"%s\","
        "\"aud\":\"%s\","
        "\"iat\":%ld,"
        "\"exp\":%ld,"
        "\"jti\":\"%s\","
        "\"email\":\"%s\","
        "\"email_verified\":%s"
        "%s%s%s"
        "%s%s%s"
        "}",
        claims->issuer,
        claims->subject,
        claims->audience,
        claims->issued_at,
        claims->expires_at,
        claims->jwt_id,
        claims->email,
        claims->email_verified ? "true" : "false",
        strlen(claims->session_id) > 0 ? ",\"sid\":\"" : "",
        claims->session_id,
        strlen(claims->session_id) > 0 ? "\"" : "",
        strlen(claims->token_type) > 0 ? ",\"token_type\":\"" : "",
        claims->token_type,
        strlen(claims->token_type) > 0 ? "\"" : ""
    );

    if (written >= (int)json_len || written < 0) {
        return -1;
    }

    return 0;
}

int jwt_parse_claims_json(const char *json, jwt_claims_t *claims) {
    if (!json || !claims) {
        return -1;
    }

    jwt_init_claims(claims);

    // Simple JSON parsing for known structure
    // This is a basic implementation - for production, consider a proper JSON library

    const char *ptr = json;

    // Parse each field
    char *iss_start = strstr(ptr, "\"iss\":\"");
    if (iss_start) {
        iss_start += 7; // Skip "iss":"
        char *iss_end = strchr(iss_start, '"');
        if (iss_end && (iss_end - iss_start) <= JWT_CLAIM_MAX_LENGTH) {
            strncpy(claims->issuer, iss_start, iss_end - iss_start);
        }
    }

    char *sub_start = strstr(ptr, "\"sub\":\"");
    if (sub_start) {
        sub_start += 7;
        char *sub_end = strchr(sub_start, '"');
        if (sub_end && (sub_end - sub_start) <= JWT_CLAIM_MAX_LENGTH) {
            strncpy(claims->subject, sub_start, sub_end - sub_start);
        }
    }

    char *aud_start = strstr(ptr, "\"aud\":\"");
    if (aud_start) {
        aud_start += 7;
        char *aud_end = strchr(aud_start, '"');
        if (aud_end && (aud_end - aud_start) <= JWT_CLAIM_MAX_LENGTH) {
            strncpy(claims->audience, aud_start, aud_end - aud_start);
        }
    }

    char *iat_start = strstr(ptr, "\"iat\":");
    if (iat_start) {
        claims->issued_at = strtol(iat_start + 6, NULL, 10);
    }

    char *exp_start = strstr(ptr, "\"exp\":");
    if (exp_start) {
        claims->expires_at = strtol(exp_start + 6, NULL, 10);
    }

    char *jti_start = strstr(ptr, "\"jti\":\"");
    if (jti_start) {
        jti_start += 7;
        char *jti_end = strchr(jti_start, '"');
        if (jti_end && (jti_end - jti_start) <= JWT_ID_LENGTH) {
            strncpy(claims->jwt_id, jti_start, jti_end - jti_start);
        }
    }

    char *email_start = strstr(ptr, "\"email\":\"");
    if (email_start) {
        email_start += 9;
        char *email_end = strchr(email_start, '"');
        if (email_end && (email_end - email_start) <= JWT_CLAIM_MAX_LENGTH) {
            strncpy(claims->email, email_start, email_end - email_start);
        }
    }

    char *verified_start = strstr(ptr, "\"email_verified\":");
    if (verified_start) {
        verified_start += 17;
        if (strncmp(verified_start, "true", 4) == 0) {
            claims->email_verified = 1;
        } else {
            claims->email_verified = 0;
        }
    }

    printf("🔍 JWT SID PARSE DEBUG: Looking for 'sid' in JWT payload...\n");
    char *sid_start = strstr(ptr, "\"sid\":\"");
    if (sid_start) {
        printf("✅ Found 'sid' field in JWT\n");
        sid_start += 7; // Skip "sid":"
        char *sid_end = strchr(sid_start, '"');
        if (sid_end && (sid_end - sid_start) <= 64) { // 64 char session ID
            strncpy(claims->session_id, sid_start, sid_end - sid_start);
            printf("📝 Extracted session ID: '%s' (length: %zu)\n", claims->session_id, strlen(claims->session_id));
        } else {
            printf("❌ Invalid session ID format in JWT\n");
        }
    } else {
        printf("❌ No 'sid' field found in JWT payload\n");
    }

    char *type_start = strstr(ptr, "\"token_type\":\"");
    if (type_start) {
        type_start += 14;
        char *type_end = strchr(type_start, '"');
        if (type_end && (type_end - type_start) < sizeof(claims->token_type)) {
            strncpy(claims->token_type, type_start, type_end - type_start);
        }
    }

    return 0;
}

int jwt_parse_components(const char *token,
                        char *header_out,
                        char *payload_out,
                        char *signature_out,
                        size_t component_len) {
    if (!token || !header_out || !payload_out || !signature_out) {
        return -1;
    }

    // Find the dots separating components
    const char *first_dot = strchr(token, '.');
    if (!first_dot) return -1;

    const char *second_dot = strchr(first_dot + 1, '.');
    if (!second_dot) return -1;

    // Extract header
    size_t header_len = first_dot - token;
    if (header_len >= component_len) return -1;
    strncpy(header_out, token, header_len);
    header_out[header_len] = '\0';

    // Extract payload
    size_t payload_len = second_dot - (first_dot + 1);
    if (payload_len >= component_len) return -1;
    strncpy(payload_out, first_dot + 1, payload_len);
    payload_out[payload_len] = '\0';

    // Extract signature
    size_t sig_len = strlen(second_dot + 1);
    if (sig_len >= component_len) return -1;
    strcpy(signature_out, second_dot + 1);

    return 0;
}

int jwt_create_signature(const char *signing_input,
                        const rsa_keypair_t *keypair,
                        char *signature_out,
                        size_t sig_len) {
    if (!signing_input || !keypair || !signature_out || !keypair->private_key) {
        return -1;
    }

    // Create signing context
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return -1;
    }

    // Initialize signing with SHA-256
    if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(),
                                         NULL, keypair->private_key) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    // Update with signing input
    if (EVP_DigestSignUpdate(ctx, signing_input, strlen(signing_input)) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    // Get signature length
    size_t signature_len = 0;
    if (EVP_DigestSignFinal(ctx, NULL, &signature_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    // Allocate secure buffer for binary signature
    crypto_buffer_t *sig_buffer = platform_crypto_buffer_alloc(signature_len);
    if (!sig_buffer) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    unsigned char *signature = (unsigned char*)platform_crypto_buffer_get_data(sig_buffer);

    // Create signature
    if (EVP_DigestSignFinal(ctx, signature, &signature_len) != 1) {
        platform_crypto_buffer_free(sig_buffer);
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    EVP_MD_CTX_free(ctx);

    // Encode signature to base64url
    int encoded_len = base64url_encode(signature, signature_len, signature_out, sig_len);
    platform_crypto_buffer_free(sig_buffer);

    if (encoded_len <= 0) {
        return -1;
    }

    return 0;
}

int jwt_verify_signature(const char *signing_input,
                        const char *signature,
                        const rsa_keypair_t *keypair) {
    if (!signing_input || !signature || !keypair || !keypair->public_key) {
        return -1;
    }

    // Decode signature from base64url using fixed buffer size calculation
    size_t sig_decode_len = base64url_decode_len(strlen(signature));

    // Allocate secure buffer for signature decoding
    crypto_buffer_t *sig_buffer = platform_crypto_buffer_alloc(sig_decode_len);
    if (!sig_buffer) {
        return -1;
    }
    unsigned char *sig_binary = (unsigned char*)platform_crypto_buffer_get_data(sig_buffer);

    int decoded_len = base64url_decode(signature, 0, sig_binary, sig_decode_len);
    if (decoded_len <= 0) {
        platform_crypto_buffer_free(sig_buffer);
        return -1;
    }

    // Create verification context
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        platform_crypto_buffer_free(sig_buffer);
        return -1;
    }

    // Initialize verification
    if (EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(),
                                           NULL, keypair->public_key) != 1) {
        platform_crypto_buffer_free(sig_buffer);
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    // Update with signing input
    if (EVP_DigestVerifyUpdate(ctx, signing_input, strlen(signing_input)) != 1) {
        platform_crypto_buffer_free(sig_buffer);
        EVP_MD_CTX_free(ctx);
        return -1;
    }

    // Verify signature
    int verify_result = EVP_DigestVerifyFinal(ctx, sig_binary, decoded_len);

    platform_crypto_buffer_free(sig_buffer);
    EVP_MD_CTX_free(ctx);

    return verify_result == 1 ? 1 : 0;
}

int jwt_create_access_token(jwt_claims_t *claims,
                           const rsa_keypair_t *keypair,
                           char *token_out) {
    if (!claims || !keypair || !token_out) {
        return -1;
    }

    // Set token type
    strcpy(claims->token_type, "access");

    // Set default expiration if not set
    if (claims->expires_at == 0) {
        claims->expires_at = claims->issued_at + JWT_ACCESS_TOKEN_LIFETIME;
    }

    // Create header
    char header_json[256];
    snprintf(header_json, sizeof(header_json),
             "{\"alg\":\"RS256\",\"typ\":\"JWT\",\"kid\":\"%s\"}",
             keypair->key_id);

    // Create payload
    char payload_json[1024];
    if (jwt_format_claims_json(claims, payload_json, sizeof(payload_json)) != 0) {
        return -1;
    }

    // Encode header and payload
    char header_b64[512];
    char payload_b64[1024];

    if (base64url_encode_json(header_json, header_b64, sizeof(header_b64)) <= 0) {
        return -1;
    }

    if (base64url_encode_json(payload_json, payload_b64, sizeof(payload_b64)) <= 0) {
        return -1;
    }

    // Create signing input (header.payload)
    char signing_input[1536];
    snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);

    // Create signature
    char signature_b64[512];
    if (jwt_create_signature(signing_input, keypair, signature_b64, sizeof(signature_b64)) != 0) {
        return -1;
    }

    // Combine into final JWT
    int written = snprintf(token_out, JWT_MAX_LENGTH, "%s.%s.%s",
                          header_b64, payload_b64, signature_b64);

    if (written >= JWT_MAX_LENGTH || written < 0) {
        return -1;
    }

    return 0;
}

int jwt_create_refresh_token(jwt_claims_t *claims,
                            const rsa_keypair_t *keypair,
                            char *token_out) {
    if (!claims || !keypair || !token_out) {
        return -1;
    }

    // Set token type
    strcpy(claims->token_type, "refresh");

    // Set longer expiration for refresh token
    if (claims->expires_at == 0) {
        claims->expires_at = claims->issued_at + JWT_REFRESH_TOKEN_LIFETIME;
    }

    // Save token type before calling jwt_create_access_token (which overwrites it)
    char saved_token_type[32];
    strcpy(saved_token_type, claims->token_type);

    int result = jwt_create_access_token(claims, keypair, token_out);

    // Restore the correct token type
    strcpy(claims->token_type, saved_token_type);

    return result;
}

int jwt_extract_header(const char *token,
                      char *alg_out, size_t alg_len,
                      char *kid_out, size_t kid_len) {
    if (!token || !alg_out || !kid_out) {
        return -1;
    }

    // Find first dot
    const char *first_dot = strchr(token, '.');
    if (!first_dot) return -1;

    // Extract and decode header
    size_t header_len = first_dot - token;
    char header_b64[512];
    if (header_len >= sizeof(header_b64)) return -1;

    strncpy(header_b64, token, header_len);
    header_b64[header_len] = '\0';

    char header_json[512];
    if (base64url_decode_json(header_b64, header_json, sizeof(header_json)) <= 0) {
        return -1;
    }

    // Parse algorithm
    char *alg_start = strstr(header_json, "\"alg\":\"");
    if (alg_start) {
        alg_start += 7;
        char *alg_end = strchr(alg_start, '"');
        if (alg_end && (size_t)(alg_end - alg_start) < alg_len) {
            strncpy(alg_out, alg_start, alg_end - alg_start);
            alg_out[alg_end - alg_start] = '\0';
        }
    }

    // Parse key ID (optional)
    char *kid_start = strstr(header_json, "\"kid\":\"");
    if (kid_start) {
        kid_start += 7;
        char *kid_end = strchr(kid_start, '"');
        if (kid_end && (size_t)(kid_end - kid_start) < kid_len) {
            strncpy(kid_out, kid_start, kid_end - kid_start);
            kid_out[kid_end - kid_start] = '\0';
        }
    } else {
        kid_out[0] = '\0';
    }

    return 0;
}

int jwt_validate_token(const char *token,
                      const rsa_keypair_t *keypair,
                      jwt_validation_result_t *result) {
    printf("\n🔍 JWT TOKEN VALIDATION DEBUG:\n");

    if (!token || !keypair || !result) {
        printf("❌ Parameter validation failed:\n");
        printf("  token: %s\n", token ? "OK" : "NULL");
        printf("  keypair: %s\n", keypair ? "OK" : "NULL");
        printf("  result: %s\n", result ? "OK" : "NULL");
        return -1;
    }

    printf("✅ Parameters validated successfully\n");
    printf("📝 JWT token length: %zu\n", strlen(token));
    printf("📝 JWT token preview: %.100s%s\n", token, strlen(token) > 100 ? "..." : "");

    // Initialize result
    memset(result, 0, sizeof(jwt_validation_result_t));
    jwt_init_claims(&result->claims);

    // Parse JWT components
    printf("🔍 Parsing JWT components...\n");
    char header_b64[512];
    char payload_b64[1024];
    char signature_b64[512];

    if (jwt_parse_components(token, header_b64, payload_b64, signature_b64,
                            sizeof(header_b64)) != 0) {
        printf("❌ JWT component parsing failed\n");
        strcpy(result->error_message, "Invalid JWT format");
        return 0;
    }

    printf("✅ JWT components parsed successfully\n");
    printf("📝 Header: %s\n", header_b64);
    printf("📝 Payload length: %zu\n", strlen(payload_b64));
    printf("📝 Signature: %s\n", signature_b64);

    // Verify header algorithm
    printf("🔍 Extracting and verifying header information...\n");
    char algorithm[32];
    char key_id[64];
    if (jwt_extract_header(token, algorithm, sizeof(algorithm),
                          key_id, sizeof(key_id)) != 0) {
        printf("❌ JWT header extraction failed\n");
        strcpy(result->error_message, "Invalid JWT header");
        return 0;
    }

    printf("✅ Header information extracted successfully\n");
    printf("📝 Algorithm: %s\n", algorithm);
    printf("📝 Key ID: %s\n", strlen(key_id) > 0 ? key_id : "(none)");

    if (strcmp(algorithm, "RS256") != 0) {
        printf("❌ Unsupported algorithm: %s (expected RS256)\n", algorithm);
        strcpy(result->error_message, "Unsupported algorithm");
        return 0;
    }
    printf("✅ Algorithm RS256 verified\n");

    // Check key ID if present
    printf("🔍 Verifying key ID...\n");
    printf("📝 Token key ID: '%s'\n", key_id);
    printf("📝 Keypair key ID: '%s'\n", keypair->key_id);

    if (strlen(key_id) > 0 && strcmp(key_id, keypair->key_id) != 0) {
        printf("❌ Key ID mismatch: token='%s', keypair='%s'\n", key_id, keypair->key_id);
        strcpy(result->error_message, "Key ID mismatch");
        return 0;
    }
    printf("✅ Key ID validation passed\n");

    // Verify signature
    printf("🔍 Constructing signing input for verification...\n");
    char signing_input[1536];
    snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);

    printf("📝 Signing input constructed: %.200s%s\n", signing_input, strlen(signing_input) > 200 ? "..." : "");
    printf("📝 Signing input length: %zu bytes\n", strlen(signing_input));

    printf("🔍 Calling jwt_verify_signature...\n");
    int sig_valid = jwt_verify_signature(signing_input, signature_b64, keypair);

    printf("📊 JWT signature verification result: %d\n", sig_valid);
    if (sig_valid != 1) {
        printf("❌ JWT VALIDATION FAILED: Signature verification returned %d\n", sig_valid);
        strcpy(result->error_message, "Invalid signature");
        printf("🔍 === JWT TOKEN VALIDATION DEBUG END ===\n\n");
        return 0;
    }

    printf("✅ JWT signature verification SUCCESSFUL!\n");

    // Decode and parse payload
    char payload_json[1024];
    if (base64url_decode_json(payload_b64, payload_json, sizeof(payload_json)) <= 0) {
        strcpy(result->error_message, "Invalid payload encoding");
        return 0;
    }

    if (jwt_parse_claims_json(payload_json, &result->claims) != 0) {
        strcpy(result->error_message, "Invalid claims format");
        return 0;
    }

    // Check expiration
    result->is_expired = jwt_is_expired(result->claims.expires_at);
    if (result->is_expired) {
        strcpy(result->error_message, "Token expired");
        result->is_valid = 0;
        return 0;
    }

    // All checks passed
    result->is_valid = 1;
    strcpy(result->error_message, "Token is valid");
    return 0;
}

#ifdef JWT_RS256_TEST_MAIN
/**
 * Test program for JWT RS256 implementation
 * Compile with: gcc -DJWT_RS256_TEST_MAIN jwt_rs256.c -o test_jwt
 */
#include <stdio.h>

int main() {
    printf("Testing JWT RS256 Implementation\n");
    printf("===============================\n\n");

    // Initialize RSA keypair
    rsa_keypair_t keypair;
    rsa_init_keypair(&keypair);

    if (rsa_generate_keypair(&keypair) != 0) {
        printf("✗ Failed to generate RSA keypair\n");
        return 1;
    }

    printf("✓ Generated RSA keypair (Key ID: %s)\n\n", keypair.key_id);

    // Test access token creation
    printf("Testing access token creation:\n");
    jwt_claims_t claims;
    if (jwt_set_user_claims(&claims, "xK8fG2mNpQrS7vW9yB4cD6eH8jL",
                           "user@example.com", 1,
                           "https://auth.example.com",
                           "example-app", 3600) != 0) {
        printf("✗ Failed to set user claims\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    char access_token[JWT_MAX_LENGTH];
    if (jwt_create_access_token(&claims, &keypair, access_token) != 0) {
        printf("✗ Failed to create access token\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    printf("✓ Created access token\n");
    printf("  Length: %zu characters\n", strlen(access_token));
    printf("  Token: %.60s...\n\n", access_token);

    // Test token validation
    printf("Testing token validation:\n");
    jwt_validation_result_t result;
    if (jwt_validate_token(access_token, &keypair, &result) != 0) {
        printf("✗ Failed to validate token\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    if (result.is_valid && !result.is_expired) {
        printf("✓ Token is valid\n");
        printf("  Subject: %s\n", result.claims.subject);
        printf("  Email: %s\n", result.claims.email);
        printf("  Issuer: %s\n", result.claims.issuer);
        printf("  JWT ID: %s\n", result.claims.jwt_id);
        printf("  Expires: %ld\n", result.claims.expires_at);
    } else {
        printf("✗ Token validation failed: %s\n", result.error_message);
    }

    // Test refresh token
    printf("\nTesting refresh token:\n");
    char refresh_token[JWT_MAX_LENGTH];
    if (jwt_create_refresh_token(&claims, &keypair, refresh_token) != 0) {
        printf("✗ Failed to create refresh token\n");
        rsa_free_keypair(&keypair);
        return 1;
    }

    printf("✓ Created refresh token\n");
    printf("  Length: %zu characters\n", strlen(refresh_token));

    jwt_validation_result_t refresh_result;
    if (jwt_validate_token(refresh_token, &keypair, &refresh_result) == 0 &&
        refresh_result.is_valid && !refresh_result.is_expired) {
        printf("✓ Refresh token is valid\n");
        printf("  Token type: %s\n", refresh_result.claims.token_type);
    } else {
        printf("✗ Refresh token validation failed\n");
    }

    // Test header extraction
    printf("\nTesting header extraction:\n");
    char alg[32], kid[64];
    if (jwt_extract_header(access_token, alg, sizeof(alg), kid, sizeof(kid)) == 0) {
        printf("✓ Extracted header\n");
        printf("  Algorithm: %s\n", alg);
        printf("  Key ID: %s\n", kid);
    } else {
        printf("✗ Failed to extract header\n");
    }

    rsa_free_keypair(&keypair);
    printf("\n✓ All tests completed\n");
    return 0;
}
#endif