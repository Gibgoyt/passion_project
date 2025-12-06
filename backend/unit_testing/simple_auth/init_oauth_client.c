/**
 * OAuth Client Initialization Utility
 *
 * This utility creates sample OAuth clients for testing the PKCE implementation.
 * Run this once to set up test clients in the database.
 */

#include "auth_lib/auth.h"
#include "auth_lib/oauth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int check_clients_exist(auth_context_t *ctx) {
    // Check if all required test clients exist
    const char *required_clients[] = {
        "astro-pkce-test",
        "spa-mobile-test",
        "dev-localhost"
    };
    const int num_clients = sizeof(required_clients) / sizeof(required_clients[0]);

    for (int i = 0; i < num_clients; i++) {
        client_data_t client;
        if (oauth_get_client(ctx, required_clients[i], &client) != 0) {
            return 0; // Client not found
        }
        if (!client.is_active) {
            return 0; // Client exists but is inactive
        }
    }

    return 1; // All clients exist and are active
}

int main(int argc, char *argv[]) {
    int check_mode = 0;

    // Parse command line arguments
    if (argc > 1 && strcmp(argv[1], "--check-clients") == 0) {
        check_mode = 1;
    }

    if (!check_mode) {
        printf("🔧 Initializing OAuth clients for testing...\n");
    }

    // Initialize authentication context
    auth_context_t auth_ctx;
    auth_config_t config;
    auth_init_config(&config);

    if (auth_initialize(&auth_ctx, &config) != 0) {
        if (!check_mode) {
            printf("❌ Failed to initialize authentication system\n");
        }
        return 1;
    }

    // If in check mode, just verify clients exist and exit
    if (check_mode) {
        int clients_exist = check_clients_exist(&auth_ctx);
        auth_cleanup(&auth_ctx);
        return clients_exist ? 0 : 1; // Exit code 0 if all clients exist, 1 if any missing
    }

    printf("✅ Authentication system initialized\n");

    // Create test client for Astro application
    client_data_t astro_client;
    memset(&astro_client, 0, sizeof(client_data_t));

    strcpy(astro_client.client_id, "astro-pkce-test");
    strcpy(astro_client.client_name, "Astro PKCE Test Application");
    strcpy(astro_client.redirect_uri, "http://localhost:3000/callback");
    astro_client.client_type = 1; // Public client (requires PKCE)
    astro_client.created_at = time(NULL);
    astro_client.is_active = 1;

    if (oauth_register_client(&auth_ctx, &astro_client) == 0) {
        printf("✅ Created Astro test client: %s\n", astro_client.client_id);
        printf("   Redirect URI: %s\n", astro_client.redirect_uri);
    } else {
        printf("❌ Failed to create Astro test client\n");
        auth_cleanup(&auth_ctx);
        return 1;
    }

    // Create additional test client for mobile/SPA testing
    client_data_t spa_client;
    memset(&spa_client, 0, sizeof(client_data_t));

    strcpy(spa_client.client_id, "spa-mobile-test");
    strcpy(spa_client.client_name, "SPA/Mobile Test Application");
    strcpy(spa_client.redirect_uri, "http://localhost:3001/oauth/callback");
    spa_client.client_type = 1; // Public client (requires PKCE)
    spa_client.created_at = time(NULL);
    spa_client.is_active = 1;

    if (oauth_register_client(&auth_ctx, &spa_client) == 0) {
        printf("✅ Created SPA/Mobile test client: %s\n", spa_client.client_id);
        printf("   Redirect URI: %s\n", spa_client.redirect_uri);
    } else {
        printf("❌ Failed to create SPA/Mobile test client\n");
        auth_cleanup(&auth_ctx);
        return 1;
    }

    // Create development client with localhost variations
    client_data_t dev_client;
    memset(&dev_client, 0, sizeof(client_data_t));

    strcpy(dev_client.client_id, "dev-localhost");
    strcpy(dev_client.client_name, "Development Localhost Client");
    strcpy(dev_client.redirect_uri, "http://127.0.0.1:4321/auth/callback");
    dev_client.client_type = 1; // Public client (requires PKCE)
    dev_client.created_at = time(NULL);
    dev_client.is_active = 1;

    if (oauth_register_client(&auth_ctx, &dev_client) == 0) {
        printf("✅ Created development client: %s\n", dev_client.client_id);
        printf("   Redirect URI: %s\n", dev_client.redirect_uri);
    } else {
        printf("❌ Failed to create development client\n");
        auth_cleanup(&auth_ctx);
        return 1;
    }

    // Verify clients were created
    printf("\n🔍 Verifying created clients...\n");

    client_data_t test_client;
    if (oauth_get_client(&auth_ctx, "astro-pkce-test", &test_client) == 0) {
        printf("✅ Verified Astro client exists\n");
        printf("   Name: %s\n", test_client.client_name);
        printf("   Type: %s\n", test_client.client_type == 1 ? "Public (PKCE Required)" : "Confidential");
        printf("   Status: %s\n", test_client.is_active ? "Active" : "Inactive");
    } else {
        printf("❌ Could not verify Astro client\n");
    }

    if (oauth_get_client(&auth_ctx, "spa-mobile-test", &test_client) == 0) {
        printf("✅ Verified SPA/Mobile client exists\n");
    } else {
        printf("❌ Could not verify SPA/Mobile client\n");
    }

    if (oauth_get_client(&auth_ctx, "dev-localhost", &test_client) == 0) {
        printf("✅ Verified development client exists\n");
    } else {
        printf("❌ Could not verify development client\n");
    }

    // Cleanup
    auth_cleanup(&auth_ctx);

    printf("\n🎉 OAuth client initialization complete!\n");
    printf("\n💡 Available test clients:\n");
    printf("   1. astro-pkce-test      -> http://localhost:3000/callback\n");
    printf("   2. spa-mobile-test      -> http://localhost:3001/oauth/callback\n");
    printf("   3. dev-localhost        -> http://127.0.0.1:4321/auth/callback\n");
    printf("\n📚 Use these client_ids in your OAuth flows for testing.\n");

    return 0;
}