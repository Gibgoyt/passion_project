#!/bin/bash

# Build script for HTTPS-only server using uSockets with OpenSSL
# This script creates a self-signed certificate and builds an HTTPS server
# ONLY HTTPS works - HTTP is disabled

set -e  # Exit on any error

echo "Building HTTPS-only server with uSockets..."
echo "==========================================="

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Warning: This script is designed for macOS. You may need to modify it for other platforms."
fi

# Check if OpenSSL is installed via Homebrew
OPENSSL_PREFIX=$(brew --prefix openssl 2>/dev/null)
if [[ -z "$OPENSSL_PREFIX" ]]; then
    echo "Error: OpenSSL not found. Please run the main build.sh script first."
    exit 1
fi

echo "Using OpenSSL from: $OPENSSL_PREFIX"

# Check if certificates exist
if [[ ! -f "certs/server.key" ]] || [[ ! -f "certs/server.crt" ]]; then
    echo "❌ ERROR: SSL certificates not found!"
    echo "Please run ./gen_certs.sh first to generate certificates."
    echo ""
    echo "Required files:"
    echo "  - certs/server.crt      (SSL certificate)"
    echo "  - certs/server.key      (private key)"
    echo ""
    exit 1
else
    echo "✅ SSL certificates found!"
    echo "  - certs/server.crt"
    echo "  - certs/server.key"
fi

# Create the HTTPS-only server code
cat > https_server.c << 'EOF'
/* HTTPS-only server using uSockets - HTTP is NOT supported */
#include <libusockets.h>
/* SSL is always enabled for HTTPS-only server */
const int SSL = 1;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct http_socket {
    /* How far we have streamed our response */
    int offset;
};

struct http_context {
    /* The shared response */
    char *response;
    int length;
};

/* Event handlers */
void on_wakeup(struct us_loop_t *loop) {}
void on_pre(struct us_loop_t *loop) {}
void on_post(struct us_loop_t *loop) {}

struct us_socket_t *on_http_socket_writable(struct us_socket_t *s) {
    struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);
    struct http_context *http_context = (struct http_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

    /* Stream whatever is remaining of the response */
    http_socket->offset += us_socket_write(SSL, s, http_context->response + http_socket->offset, http_context->length - http_socket->offset, 0);

    return s;
}

struct us_socket_t *on_http_socket_close(struct us_socket_t *s, int code, void *reason) {
    printf("HTTPS client disconnected\n");
    return s;
}

struct us_socket_t *on_http_socket_end(struct us_socket_t *s) {
    /* HTTPS does not support half-closed sockets */
    us_socket_shutdown(SSL, s);
    return us_socket_close(SSL, s, 0, NULL);
}

struct us_socket_t *on_http_socket_data(struct us_socket_t *s, char *data, int length) {
    /* Get socket extension and the socket's context's extension */
    struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);
    struct http_context *http_context = (struct http_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

    /* We treat all data events as a request */
    http_socket->offset = us_socket_write(SSL, s, http_context->response, http_context->length, 0);

    /* Reset idle timer */
    us_socket_timeout(SSL, s, 30);

    return s;
}

struct us_socket_t *on_http_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    struct http_socket *http_socket = (struct http_socket *) us_socket_ext(SSL, s);

    /* Reset offset */
    http_socket->offset = 0;

    /* Timeout idle HTTPS connections */
    us_socket_timeout(SSL, s, 30);

    printf("HTTPS client connected from IP: %.*s\n", ip_length, ip);

    return s;
}

struct us_socket_t *on_http_socket_timeout(struct us_socket_t *s) {
    /* Close idle HTTPS sockets */
    printf("HTTPS connection timed out, closing...\n");
    return us_socket_close(SSL, s, 0, NULL);
}

int main() {
    /* Create the event loop with Kqueue support */
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    /* Create a socket context for HTTPS ONLY */
    struct us_socket_context_options_t options = {};
    options.key_file_name = "certs/server.key";
    options.cert_file_name = "certs/server.crt";
    options.passphrase = NULL; /* No passphrase for our generated key */

    printf("Creating HTTPS-only socket context...\n");
    struct us_socket_context_t *https_context = us_create_socket_context(SSL, loop, sizeof(struct http_context), options);

    if (!https_context) {
        printf("ERROR: Could not create HTTPS context with SSL cert/key\n");
        printf("Make sure the certificate files exist:\n");
        printf("  - certs/server.crt\n");
        printf("  - certs/server.key\n");
        exit(1);
    }

    printf("HTTPS context created successfully!\n");

    /* Generate the shared response */
    const char body[] = "<!DOCTYPE html><html><head><title>HTTPS-Only Server</title></head>"
                       "<body style='font-family: Arial, sans-serif; text-align: center; padding: 50px;'>"
                       "<h1 style='color: green;'>🔒 HTTPS-Only Server Working!</h1>"
                       "<p>This server <strong>ONLY</strong> supports HTTPS connections.</p>"
                       "<p>HTTP connections are <strong>NOT SUPPORTED</strong>.</p>"
                       "<p>Built with uSockets + OpenSSL + Kqueue on macOS</p>"
                       "<hr><small>SSL/TLS encryption is active</small>"
                       "</body></html>";

    struct http_context *http_context_ext = (struct http_context *) us_socket_context_ext(SSL, https_context);
    http_context_ext->response = (char *) malloc(256 + sizeof(body) - 1);
    http_context_ext->length = snprintf(http_context_ext->response, 256 + sizeof(body) - 1,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %ld\r\n"
        "Strict-Transport-Security: max-age=31536000\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\n"
        "\r\n%s", sizeof(body) - 1, body);

    /* Set up event handlers for HTTPS */
    us_socket_context_on_open(SSL, https_context, on_http_socket_open);
    us_socket_context_on_data(SSL, https_context, on_http_socket_data);
    us_socket_context_on_writable(SSL, https_context, on_http_socket_writable);
    us_socket_context_on_close(SSL, https_context, on_http_socket_close);
    us_socket_context_on_timeout(SSL, https_context, on_http_socket_timeout);
    us_socket_context_on_end(SSL, https_context, on_http_socket_end);

    /* Start serving HTTPS connections ONLY on port 3000 */
    const int HTTPS_PORT = 3000;
    printf("Starting HTTPS-only server on port %d...\n", HTTPS_PORT);

    struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, https_context, 0, HTTPS_PORT, 0, sizeof(struct http_socket));

    if (listen_socket) {
        printf("\n========================================\n");
        printf("🔒 HTTPS-Only Server is running!\n");
        printf("========================================\n");
        printf("HTTPS URL: https://localhost:%d\n", HTTPS_PORT);
        printf("Features:\n");
        printf("  - ✅ HTTPS/SSL/TLS support (OpenSSL)\n");
        printf("  - ✅ Kqueue event loop (macOS)\n");
        printf("  - ❌ HTTP support (disabled)\n");
        printf("  - 🔒 Self-signed certificate\n");
        printf("\nPress Ctrl+C to stop the server\n");
        printf("========================================\n\n");

        /* Run the event loop */
        us_loop_run(loop);
    } else {
        printf("❌ ERROR: Failed to listen on HTTPS port %d!\n", HTTPS_PORT);
        printf("Make sure the port is not already in use.\n");
        exit(1);
    }

    return 0;
}
EOF

echo ""
echo "Building HTTPS-only server..."
echo "Using SSL certificates from certs/ directory"

# Build the HTTPS server
USOCKETS_PATH="/Users/ahmed/dev/uWebSockets/uSockets"
CFLAGS="-I$OPENSSL_PREFIX/include" \
LDFLAGS="-L$OPENSSL_PREFIX/lib" \
cc -O3 -flto -DLIBUS_USE_OPENSSL -std=c11 -I$USOCKETS_PATH/src \
   -o https_server https_server.c \
   -L$OPENSSL_PREFIX/lib -lssl -lcrypto -lstdc++ $USOCKETS_PATH/uSockets.a

echo ""
echo "✅ HTTPS-only server built successfully!"
echo "========================================="
echo ""
echo "📁 Generated files:"
echo "  - https_server          (executable)"
echo "  - https_server.c        (source code)"
echo "  - certs/server.crt      (SSL certificate)"
echo "  - certs/server.key      (private key)"
echo ""
echo "🚀 To run the HTTPS server:"
echo "  ./https_server"
echo ""
echo "🌐 Then visit: https://localhost:3000"
echo "   (You'll need to accept the self-signed certificate warning)"
echo ""
echo "⚠️  IMPORTANT: This server ONLY supports HTTPS"
echo "   HTTP requests to http://localhost:3000 will NOT work!"
echo ""
echo "🔒 Security features enabled:"
echo "  - SSL/TLS encryption"
echo "  - HSTS (Strict Transport Security)"
echo "  - Security headers (X-Content-Type-Options, X-Frame-Options)"
echo ""