#!/bin/bash

# Check if data is a file (legacy MDBX artifact) and remove it
if [ -f data ] && [ ! -d data ]; then
    echo "⚠️  Found legacy 'data' file. Removing it to create directory structure..."
    rm data
fi

# Setup directories
echo "🔧 Setting up directories..."
mkdir -p data
mkdir -p keys

# Generate keys if they don't exist
if [ ! -f keys/private_key.pem ]; then
    echo "⚠️  RSA keys not found. Generating new key pair..."
    ./server --generate-keys
    if [ $? -ne 0 ]; then
        echo "❌ Failed to generate keys"
        exit 1
    fi
fi

# Run the server
echo "🚀 Starting server..."
./server