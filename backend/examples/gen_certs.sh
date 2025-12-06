#!/bin/bash
# Generate self-signed certificates for HTTPS server testing

OPENSSL_BIN="../openssl/apps/openssl"
OPENSSL_CONF="../openssl/apps/openssl.cnf"

if [ ! -f "$OPENSSL_BIN" ]; then
    echo "ERROR: OpenSSL binary not found at $OPENSSL_BIN"
    exit 1
fi

if [ ! -f "$OPENSSL_CONF" ]; then
    echo "ERROR: OpenSSL config not found at $OPENSSL_CONF"
    exit 1
fi

echo "Generating self-signed certificate for testing..."
echo "Using OpenSSL: $($OPENSSL_BIN version)"
echo ""

OPENSSL_CONF="$OPENSSL_CONF" $OPENSSL_BIN req -x509 -newkey rsa:2048 -nodes \
    -keyout server.key \
    -out server.crt \
    -days 365 \
    -subj "/C=US/ST=State/L=City/O=Organization/CN=localhost"

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Certificate and key generated successfully"
    echo "  - Certificate: ./server.crt"
    echo "  - Private key: ./server.key"
    echo ""
    echo "You can now run: ./main"
else
    echo ""
    echo "ERROR: Failed to generate certificates"
    exit 1
fi
