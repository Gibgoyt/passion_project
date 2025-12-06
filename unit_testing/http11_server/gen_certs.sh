#!/bin/bash

# Certificate generation script for HTTPS server using OpenSSL
# This script deletes existing certificates and creates fresh ones

set -e  # Exit on any error

echo "Generating fresh SSL certificates for HTTPS server..."
echo "===================================================="

# Get the OpenSSL binary path from Homebrew
OPENSSL_PREFIX=$(brew --prefix openssl 2>/dev/null)
if [[ -z "$OPENSSL_PREFIX" ]]; then
    echo "Error: OpenSSL not found via Homebrew."
    echo "Please install it first: brew install openssl"
    exit 1
fi

OPENSSL_BIN="$OPENSSL_PREFIX/bin/openssl"
echo "Using OpenSSL binary: $OPENSSL_BIN"

# Verify OpenSSL binary exists and works
if [[ ! -f "$OPENSSL_BIN" ]]; then
    echo "Error: OpenSSL binary not found at $OPENSSL_BIN"
    exit 1
fi

# Test OpenSSL version
echo "OpenSSL version:"
"$OPENSSL_BIN" version

# Remove existing certificates directory if it exists
if [[ -d "certs" ]]; then
    echo "Removing existing certs directory..."
    rm -rf certs
fi

# Create fresh certs directory
echo "Creating fresh certs directory..."
mkdir -p certs

# Generate private key (RSA 2048-bit)
echo "Generating RSA private key (2048-bit)..."
"$OPENSSL_BIN" genrsa -out certs/server.key 2048

# Set proper permissions on private key
chmod 600 certs/server.key

# Generate self-signed certificate (valid for 365 days)
echo "Generating self-signed certificate..."
"$OPENSSL_BIN" req -new -x509 -key certs/server.key -out certs/server.crt -days 365 \
    -subj "/C=US/ST=CA/L=San Francisco/O=uSockets HTTPS Server/OU=Development/CN=localhost" \
    -extensions v3_req \
    -config <(cat <<EOF
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req
prompt = no

[req_distinguished_name]
C = US
ST = CA
L = San Francisco
O = uSockets HTTPS Server
OU = Development
CN = localhost

[v3_req]
basicConstraints = CA:FALSE
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
DNS.2 = *.localhost
IP.1 = 127.0.0.1
IP.2 = ::1
EOF
)

# Set proper permissions on certificate
chmod 644 certs/server.crt

echo ""
echo "✅ Fresh SSL certificates generated successfully!"
echo "=============================================="
echo ""
echo "📁 Generated files:"
echo "  - certs/server.key     (Private key - RSA 2048-bit)"
echo "  - certs/server.crt     (Certificate - Valid for 365 days)"
echo ""

# Display certificate information
echo "🔍 Certificate details:"
echo "----------------------"
"$OPENSSL_BIN" x509 -in certs/server.crt -text -noout | grep -A 1 "Subject:"
"$OPENSSL_BIN" x509 -in certs/server.crt -text -noout | grep -A 1 "Validity"
"$OPENSSL_BIN" x509 -in certs/server.crt -text -noout | grep -A 5 "Subject Alternative Name"

echo ""
echo "🔒 Certificate features:"
echo "  - Self-signed certificate for localhost"
echo "  - Valid for HTTPS on localhost, 127.0.0.1, and ::1"
echo "  - RSA 2048-bit encryption"
echo "  - Includes Subject Alternative Names (SAN)"
echo "  - Server authentication enabled"
echo ""
echo "⚡ Ready to use with HTTPS server!"
echo "   Run: ./build_http_server.sh"
echo ""