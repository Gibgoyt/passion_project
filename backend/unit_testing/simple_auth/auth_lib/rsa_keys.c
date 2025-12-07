#include "rsa_keys.h"
#include "base64url.h"
#include "../memory/platform_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// OpenSSL includes
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

void rsa_init_keypair(rsa_keypair_t *keypair) {
    if (!keypair) return;

    keypair->private_key = NULL;
    keypair->public_key = NULL;
    keypair->key_id[0] = '\0';
    keypair->is_loaded = 0;
}

void rsa_free_keypair(rsa_keypair_t *keypair) {
    if (!keypair) return;

    if (keypair->private_key) {
        EVP_PKEY_free(keypair->private_key);
        keypair->private_key = NULL;
    }

    if (keypair->public_key) {
        EVP_PKEY_free(keypair->public_key);
        keypair->public_key = NULL;
    }

    keypair->key_id[0] = '\0';
    keypair->is_loaded = 0;
}

int rsa_ensure_keys_directory(void) {
    struct stat st = {0};

    // Check if keys directory exists
    if (stat("keys", &st) == -1) {
        // Create directory with mode 700 (owner read/write/execute only)
        if (mkdir("keys", 0700) != 0) {
            return -1;
        }
    }

    return 0;
}

int rsa_generate_keypair(rsa_keypair_t *keypair) {
    if (!keypair) {
        return -1;
    }

    rsa_free_keypair(keypair);
    rsa_init_keypair(keypair);

    // Generate RSA key pair
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        return -1;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEY_BITS) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY *pkey = NULL;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);

    // Set private key
    keypair->private_key = pkey;

    // Extract public key
    if (rsa_extract_public_key(pkey, &keypair->public_key) != 0) {
        rsa_free_keypair(keypair);
        return -1;
    }

    // Generate key ID
    if (rsa_generate_key_id(keypair) != 0) {
        rsa_free_keypair(keypair);
        return -1;
    }

    keypair->is_loaded = 1;
    return 0;
}

int rsa_load_keys(rsa_keypair_t *keypair) {
    printf("\n🔍 RSA KEY LOADING DEBUG:\n");

    if (!keypair) {
        printf("❌ Keypair parameter is NULL\n");
        return -1;
    }

    printf("✅ Starting key loading process\n");
    printf("📝 Private key path: %s\n", PRIVATE_KEY_PATH);
    printf("📝 Public key path: %s\n", PUBLIC_KEY_PATH);

    rsa_free_keypair(keypair);
    rsa_init_keypair(keypair);

    // Try to load private key
    printf("🔍 Loading private key from %s...\n", PRIVATE_KEY_PATH);
    FILE *private_file = fopen(PRIVATE_KEY_PATH, "r");
    if (!private_file) {
        printf("❌ Failed to open private key file: %s\n", PRIVATE_KEY_PATH);
        return -1; // Private key file doesn't exist
    }
    printf("✅ Private key file opened successfully\n");

    EVP_PKEY *private_key = PEM_read_PrivateKey(private_file, NULL, NULL, NULL);
    fclose(private_file);

    if (!private_key) {
        printf("❌ Failed to parse private key from PEM file\n");
        return -1; // Failed to load private key
    }
    printf("✅ Private key loaded and parsed successfully\n");

    keypair->private_key = private_key;

    // Try to load public key, or extract from private key
    printf("🔍 Loading public key from %s...\n", PUBLIC_KEY_PATH);
    FILE *public_file = fopen(PUBLIC_KEY_PATH, "r");
    if (public_file) {
        printf("✅ Public key file found, loading...\n");
        EVP_PKEY *public_key = PEM_read_PUBKEY(public_file, NULL, NULL, NULL);
        fclose(public_file);

        if (public_key) {
            printf("✅ Public key loaded from file successfully\n");
            keypair->public_key = public_key;
        } else {
            printf("❌ Failed to parse public key from file, extracting from private key...\n");
            // Extract from private key
            if (rsa_extract_public_key(private_key, &keypair->public_key) != 0) {
                printf("❌ Failed to extract public key from private key\n");
                rsa_free_keypair(keypair);
                return -1;
            }
            printf("✅ Public key extracted from private key successfully\n");
        }
    } else {
        printf("⚠️ Public key file not found, extracting from private key...\n");
        // Extract public key from private key
        if (rsa_extract_public_key(private_key, &keypair->public_key) != 0) {
            printf("❌ Failed to extract public key from private key\n");
            rsa_free_keypair(keypair);
            return -1;
        }
        printf("✅ Public key extracted from private key successfully\n");
    }

    // Generate key ID
    printf("🔍 Generating key ID...\n");
    if (rsa_generate_key_id(keypair) != 0) {
        printf("❌ Failed to generate key ID\n");
        rsa_free_keypair(keypair);
        return -1;
    }

    printf("✅ Key ID generated successfully: %s\n", keypair->key_id);
    printf("📝 Key fingerprint (first 32 chars): %.32s\n", keypair->key_id);

    keypair->is_loaded = 1;
    printf("✅ RSA keypair loading completed successfully\n");
    printf("🔍 === RSA KEY LOADING DEBUG END ===\n\n");
    return 0;
}

int rsa_save_keys(const rsa_keypair_t *keypair) {
    if (!keypair || !keypair->is_loaded) {
        return -1;
    }

    // Ensure keys directory exists
    if (rsa_ensure_keys_directory() != 0) {
        return -1;
    }

    // Save private key with restrictive permissions
    FILE *private_file = fopen(PRIVATE_KEY_PATH, "w");
    if (!private_file) {
        return -1;
    }

    int private_result = PEM_write_PrivateKey(private_file, keypair->private_key,
                                                           NULL, NULL, 0, NULL, NULL);
    fclose(private_file);

    if (private_result != 1) {
        return -1;
    }

    // Set private key file permissions (owner read/write only)
    if (chmod(PRIVATE_KEY_PATH, 0600) != 0) {
        return -1;
    }

    // Save public key
    FILE *public_file = fopen(PUBLIC_KEY_PATH, "w");
    if (!public_file) {
        return -1;
    }

    int public_result = PEM_write_PUBKEY(public_file, keypair->public_key);
    fclose(public_file);

    if (public_result != 1) {
        return -1;
    }

    // Set public key file permissions (readable by others)
    chmod(PUBLIC_KEY_PATH, 0644);

    return 0;
}

int rsa_get_or_generate_keys(rsa_keypair_t *keypair) {
    if (!keypair) {
        return -1;
    }

    // Try to load existing keys first
    if (rsa_load_keys(keypair) == 0) {
        // Validate loaded keys
        if (rsa_validate_keypair(keypair) == 1) {
            return 0; // Successfully loaded and validated
        }

        // Keys are invalid, fall through to generation
        rsa_free_keypair(keypair);
    }

    // Generate new keys
    if (rsa_generate_keypair(keypair) != 0) {
        return -1;
    }

    // Save the new keys
    if (rsa_save_keys(keypair) != 0) {
        rsa_free_keypair(keypair);
        return -1;
    }

    return 0;
}

int rsa_validate_keypair(const rsa_keypair_t *keypair) {
    if (!keypair || !keypair->is_loaded ||
        !keypair->private_key || !keypair->public_key) {
        return 0;
    }

    // Test data for sign/verify test
    const char *test_data = "RSA keypair validation test";
    unsigned char signature[256];
    size_t sig_len = sizeof(signature);

    // Create signing context
    EVP_MD_CTX *sign_ctx = EVP_MD_CTX_new();
    if (!sign_ctx) {
        return -1;
    }

    // Initialize signing
    if (EVP_DigestSignInit(sign_ctx, NULL, EVP_sha256(),
                                         NULL, keypair->private_key) != 1) {
        EVP_MD_CTX_free(sign_ctx);
        return -1;
    }

    // Update with data
    if (EVP_DigestSignUpdate(sign_ctx, test_data, strlen(test_data)) != 1) {
        EVP_MD_CTX_free(sign_ctx);
        return -1;
    }

    // Finalize signature
    if (EVP_DigestSignFinal(sign_ctx, signature, &sig_len) != 1) {
        EVP_MD_CTX_free(sign_ctx);
        return -1;
    }

    EVP_MD_CTX_free(sign_ctx);

    // Create verification context
    EVP_MD_CTX *verify_ctx = EVP_MD_CTX_new();
    if (!verify_ctx) {
        return -1;
    }

    // Initialize verification
    if (EVP_DigestVerifyInit(verify_ctx, NULL, EVP_sha256(),
                                           NULL, keypair->public_key) != 1) {
        EVP_MD_CTX_free(verify_ctx);
        return -1;
    }

    // Update with data
    if (EVP_DigestVerifyUpdate(verify_ctx, test_data, strlen(test_data)) != 1) {
        EVP_MD_CTX_free(verify_ctx);
        return -1;
    }

    // Verify signature
    int verify_result = EVP_DigestVerifyFinal(verify_ctx, signature, sig_len);
    EVP_MD_CTX_free(verify_ctx);

    return verify_result == 1 ? 1 : 0;
}

int rsa_extract_public_key(EVP_PKEY *private_key, EVP_PKEY **public_key_out) {
    if (!private_key || !public_key_out) {
        return -1;
    }

    // Create a new EVP_PKEY for the public key
    EVP_PKEY *public_key = EVP_PKEY_new();
    if (!public_key) {
        return -1;
    }

    // Get the RSA key from the private key
    RSA *rsa_private = EVP_PKEY_get1_RSA(private_key);
    if (!rsa_private) {
        EVP_PKEY_free(public_key);
        return -1;
    }

    // Create a new RSA key with only public components
    RSA *rsa_public = RSA_new();
    if (!rsa_public) {
        RSA_free(rsa_private);
        EVP_PKEY_free(public_key);
        return -1;
    }

    // Get the public components (n and e)
    const BIGNUM *n, *e;
    RSA_get0_key(rsa_private, &n, &e, NULL);

    // Duplicate the public components
    BIGNUM *n_dup = BN_dup(n);
    BIGNUM *e_dup = BN_dup(e);

    if (!n_dup || !e_dup) {
        if (n_dup) BN_free(n_dup);
        if (e_dup) BN_free(e_dup);
        RSA_free(rsa_public);
        RSA_free(rsa_private);
        EVP_PKEY_free(public_key);
        return -1;
    }

    // Set the public components
    if (RSA_set0_key(rsa_public, n_dup, e_dup, NULL) != 1) {
        BN_free(n_dup);
        BN_free(e_dup);
        RSA_free(rsa_public);
        RSA_free(rsa_private);
        EVP_PKEY_free(public_key);
        return -1;
    }

    // Assign the RSA public key to the EVP_PKEY
    if (EVP_PKEY_assign_RSA(public_key, rsa_public) != 1) {
        RSA_free(rsa_public);
        RSA_free(rsa_private);
        EVP_PKEY_free(public_key);
        return -1;
    }

    RSA_free(rsa_private);
    *public_key_out = public_key;
    return 0;
}

int rsa_get_key_bits(EVP_PKEY *key) {
    if (!key) {
        return -1;
    }

    return EVP_PKEY_bits(key);
}

int rsa_generate_key_id(rsa_keypair_t *keypair) {
    if (!keypair || !keypair->public_key) {
        return -1;
    }

    // Export public key to DER format for hashing
    unsigned char *der_data = NULL;
    int der_len = i2d_PUBKEY(keypair->public_key, &der_data);
    if (der_len <= 0 || !der_data) {
        return -1;
    }

    // Hash the DER data with SHA-256
    unsigned char hash[32];
    SHA256(der_data, der_len, hash);

    // Free the DER data
    OPENSSL_free(der_data);

    // Take first 16 bytes of hash and encode as base64url
    char key_id_b64[32];
    int encoded_len = base64url_encode(hash, 16, key_id_b64, sizeof(key_id_b64));

    if (encoded_len <= 0 || encoded_len > KEY_ID_MAX_LENGTH) {
        return -1;
    }

    // Copy to keypair
    strncpy(keypair->key_id, key_id_b64, KEY_ID_MAX_LENGTH);
    keypair->key_id[KEY_ID_MAX_LENGTH] = '\0';

    return 0;
}

int rsa_export_public_key_pem(EVP_PKEY *public_key, char *pem_out, size_t pem_len) {
    if (!public_key || !pem_out || pem_len == 0) {
        return -1;
    }

    // Create memory BIO
    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio) {
        return -1;
    }

    // Write public key to BIO
    if (PEM_write_bio_PUBKEY(bio, public_key) != 1) {
        BIO_free(bio);
        return -1;
    }

    // Get data from BIO
    char *bio_data;
    long bio_len = BIO_get_mem_data(bio, &bio_data);

    if (bio_len <= 0 || (size_t)bio_len >= pem_len) {
        BIO_free(bio);
        return -1;
    }

    // Copy to output buffer
    memcpy(pem_out, bio_data, bio_len);
    pem_out[bio_len] = '\0';

    BIO_free(bio);
    return (int)bio_len;
}

int rsa_get_public_key_components(EVP_PKEY *public_key,
                                 char *n_out, size_t n_len,
                                 char *e_out, size_t e_len) {
    if (!public_key || !n_out || !e_out) {
        return -1;
    }

    // Get RSA key
    RSA *rsa = EVP_PKEY_get1_RSA(public_key);
    if (!rsa) {
        return -1;
    }

    // Get public key components
    const BIGNUM *n, *e;
    RSA_get0_key(rsa, &n, &e, NULL);

    // Convert modulus (n) to binary using secure buffer
    int n_bytes = BN_num_bytes(n);
    crypto_buffer_t *n_buffer = platform_crypto_buffer_alloc(n_bytes);
    if (!n_buffer) {
        RSA_free(rsa);
        return -1;
    }
    unsigned char *n_bin = (unsigned char*)platform_crypto_buffer_get_data(n_buffer);

    BN_bn2bin(n, n_bin);

    // Encode modulus to base64url
    int n_encoded = base64url_encode(n_bin, n_bytes, n_out, n_len);
    platform_crypto_buffer_free(n_buffer);

    if (n_encoded <= 0) {
        RSA_free(rsa);
        return -1;
    }

    // Convert exponent (e) to binary using secure buffer
    int e_bytes = BN_num_bytes(e);
    crypto_buffer_t *e_buffer = platform_crypto_buffer_alloc(e_bytes);
    if (!e_buffer) {
        platform_crypto_buffer_free(n_buffer);
        RSA_free(rsa);
        return -1;
    }
    unsigned char *e_bin = (unsigned char*)platform_crypto_buffer_get_data(e_buffer);

    BN_bn2bin(e, e_bin);

    // Encode exponent to base64url
    int e_encoded = base64url_encode(e_bin, e_bytes, e_out, e_len);
    platform_crypto_buffer_free(e_buffer);

    RSA_free(rsa);

    if (e_encoded <= 0) {
        return -1;
    }

    return 0;
}

#ifdef RSA_KEYS_TEST_MAIN
/**
 * Test program for RSA key management
 * Compile with: gcc -DRSA_KEYS_TEST_MAIN rsa_keys.c -o test_rsa_keys
 */
#include <stdio.h>

int main() {
    printf("Testing RSA Key Management\n");
    printf("=========================\n\n");

    rsa_keypair_t keypair;
    rsa_init_keypair(&keypair);

    // Test key generation
    printf("Testing key generation...\n");
    if (rsa_generate_keypair(&keypair) == 0) {
        printf("✓ Generated RSA keypair\n");
        printf("  Key ID: %s\n", keypair.key_id);
        printf("  Key bits: %d\n", rsa_get_key_bits(keypair.private_key));

        // Test validation
        printf("\nTesting validation...\n");
        int valid = rsa_validate_keypair(&keypair);
        printf("  Validation result: %s\n", valid == 1 ? "✓ Valid" : "✗ Invalid");

        // Test saving
        printf("\nTesting key saving...\n");
        if (rsa_save_keys(&keypair) == 0) {
            printf("✓ Saved keys to files\n");

            // Test loading
            printf("\nTesting key loading...\n");
            rsa_keypair_t loaded_keypair;
            rsa_init_keypair(&loaded_keypair);

            if (rsa_load_keys(&loaded_keypair) == 0) {
                printf("✓ Loaded keys from files\n");
                printf("  Loaded Key ID: %s\n", loaded_keypair.key_id);

                // Validate loaded keys
                int loaded_valid = rsa_validate_keypair(&loaded_keypair);
                printf("  Loaded validation: %s\n", loaded_valid == 1 ? "✓ Valid" : "✗ Invalid");

                rsa_free_keypair(&loaded_keypair);
            } else {
                printf("✗ Failed to load keys\n");
            }

        } else {
            printf("✗ Failed to save keys\n");
        }

        // Test public key export
        printf("\nTesting public key export...\n");
        char pem[2048];
        if (rsa_export_public_key_pem(keypair.public_key, pem, sizeof(pem)) > 0) {
            printf("✓ Exported public key to PEM\n");
            printf("PEM preview: %.50s...\n", pem);
        } else {
            printf("✗ Failed to export public key\n");
        }

        // Test public key components
        printf("\nTesting public key components...\n");
        char n_b64[512], e_b64[64];
        if (rsa_get_public_key_components(keypair.public_key, n_b64, sizeof(n_b64),
                                         e_b64, sizeof(e_b64)) == 0) {
            printf("✓ Extracted public key components\n");
            printf("  Modulus (n): %.40s...\n", n_b64);
            printf("  Exponent (e): %s\n", e_b64);
        } else {
            printf("✗ Failed to extract components\n");
        }

        rsa_free_keypair(&keypair);
    } else {
        printf("✗ Failed to generate RSA keypair\n");
    }

    return 0;
}
#endif