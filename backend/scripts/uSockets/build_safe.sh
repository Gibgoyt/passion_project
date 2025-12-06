#!/bin/bash
set -e

echo "Building uSockets with symbol prefixing to avoid OpenSSL conflicts..."
echo "This will prefix BoringSSL symbols with USOCKETS_BSSL_ and lsquic symbols with USOCKETS_LSQUIC_"

# Check dependencies
echo "Checking dependencies..."

# Check for Go (required for BoringSSL symbol prefixing)
if ! command -v go &> /dev/null; then
    echo "Error: Go is required for BoringSSL symbol prefixing but is not installed."
    echo "Please install Go and try again."
    exit 1
fi

# Check for required build tools
for tool in cmake make ar nm objcopy; do
    if ! command -v $tool &> /dev/null; then
        echo "Error: $tool is required but not found."
        exit 1
    fi
done

echo "All dependencies found."

# Create working directories
echo "Setting up build directories..."
mkdir -p build_safe_temp
cd build_safe_temp

# Step 1: Build BoringSSL without prefix first (to extract symbols)
echo "Step 1: Building BoringSSL to extract symbols..."
if [ ! -f "../boringssl/build/ssl/libssl.a" ] || [ ! -f "../boringssl/build/crypto/libcrypto.a" ]; then
    echo "Building BoringSSL for symbol extraction..."
    cd ../boringssl
    rm -rf build
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    cd ../../build_safe_temp
else
    echo "BoringSSL already built, using existing build for symbol extraction..."
fi

# Step 2: Extract BoringSSL symbols
echo "Step 2: Extracting BoringSSL symbols..."
cd ../boringssl
go run util/read_symbols.go \
    -out ../build_safe_temp/boringssl_symbols.txt \
    build/crypto/libcrypto.a \
    build/ssl/libssl.a
cd ../build_safe_temp

echo "Extracted $(wc -l < boringssl_symbols.txt) BoringSSL symbols"

# Step 3: Rebuild BoringSSL with USOCKETS_BSSL prefix
echo "Step 3: Rebuilding BoringSSL with USOCKETS_BSSL_ prefix..."
cd ../boringssl
rm -rf build_prefixed
mkdir build_prefixed
cd build_prefixed
cmake -DCMAKE_BUILD_TYPE=Release \
    -DBORINGSSL_PREFIX=USOCKETS_BSSL \
    -DBORINGSSL_PREFIX_SYMBOLS="$(pwd)/../../build_safe_temp/boringssl_symbols.txt" \
    ..
make -j$(nproc)
cd ../../build_safe_temp

echo "BoringSSL rebuilt with symbol prefixes"

# Step 4: Build lsquic with prefixed BoringSSL
echo "Step 4: Building lsquic with prefixed BoringSSL..."
cd ../lsquic
rm -rf build_safe
mkdir build_safe
cd build_safe

# Use the prefixed BoringSSL - pass library paths directly
cmake -DCMAKE_BUILD_TYPE=Release \
    -DBORINGSSL_INCLUDE="$(pwd)/../../boringssl/include" \
    -DBORINGSSL_LIB_ssl="$(pwd)/../../boringssl/build_prefixed/ssl/libssl.a" \
    -DBORINGSSL_LIB_crypto="$(pwd)/../../boringssl/build_prefixed/crypto/libcrypto.a" \
    -DZLIB_LIB=/usr/lib64/libz.so \
    ..
make -j$(nproc)
cd ../../build_safe_temp

echo "lsquic built with prefixed BoringSSL"

# Step 5: Extract lsquic symbols and create prefix mapping
echo "Step 5: Extracting lsquic symbols for prefixing..."

# Extract symbols from lsquic
nm -g ../lsquic/build_safe/src/liblsquic/liblsquic.a | \
    grep " T " | \
    awk '{print $3}' | \
    grep "^lsquic_\|^lsxpack_" > lsquic_symbols.txt

echo "Extracted $(wc -l < lsquic_symbols.txt) lsquic symbols"

# Create objcopy redefine file for lsquic
echo "Creating symbol redefine mapping..."
while read -r sym; do
    echo "$sym USOCKETS_LSQUIC_$sym"
done < lsquic_symbols.txt > lsquic_redefine.txt

# Step 6: Apply prefix to lsquic using objcopy
echo "Step 6: Applying USOCKETS_LSQUIC_ prefix to lsquic symbols..."
mkdir -p prefixed_libs
cp ../lsquic/build_safe/src/liblsquic/liblsquic.a prefixed_libs/

cd prefixed_libs
# Extract all object files from the archive
ar x liblsquic.a

# Apply symbol renaming to each object file
for obj in *.o; do
    if [ -f "$obj" ]; then
        objcopy --redefine-syms=../lsquic_redefine.txt "$obj"
    fi
done

# Recreate the archive with prefixed symbols
rm -f liblsquic.a
ar rcs liblsquic.a *.o

echo "lsquic symbols successfully prefixed"
cd ..

# Step 7: Build uSockets with prefixed libraries
echo "Step 7: Building uSockets with prefixed libraries..."
cd ..

# Create temporary copies of prefixed libraries in expected locations
echo "Setting up prefixed library paths..."
mkdir -p boringssl_safe/build/ssl boringssl_safe/build/crypto
cp boringssl/build_prefixed/ssl/libssl.a boringssl_safe/build/ssl/
cp boringssl/build_prefixed/crypto/libcrypto.a boringssl_safe/build/crypto/
cp -r boringssl/include boringssl_safe/

mkdir -p lsquic_safe/src/liblsquic
cp build_safe_temp/prefixed_libs/liblsquic.a lsquic_safe/src/liblsquic/
cp -r lsquic/include lsquic_safe/

# Backup original libraries
if [ ! -d "boringssl_original" ]; then
    mv boringssl boringssl_original
    mv lsquic lsquic_original
fi

# Symlink to our prefixed versions
ln -sf boringssl_safe boringssl
ln -sf lsquic_safe lsquic

# Build uSockets
echo "Building uSockets.a with prefixed symbols..."
WITH_QUIC=1 WITH_BORINGSSL=1 make clean
WITH_QUIC=1 WITH_BORINGSSL=1 make -j$(nproc)

# Restore original symlinks
rm -f boringssl lsquic
ln -sf boringssl_original boringssl
ln -sf lsquic_original lsquic

echo "Build completed!"

# Step 8: Generate reference files
echo "Step 8: Generating symbol reference files..."
cp build_safe_temp/boringssl_symbols.txt ./
cp build_safe_temp/lsquic_symbols.txt ./

echo ""
echo "=== Build Summary ==="
echo "✓ BoringSSL symbols prefixed with: USOCKETS_BSSL_"
echo "✓ lsquic symbols prefixed with: USOCKETS_LSQUIC_"
echo "✓ Generated reference files:"
echo "  - boringssl_symbols.txt ($(wc -l < boringssl_symbols.txt) symbols)"
echo "  - lsquic_symbols.txt ($(wc -l < lsquic_symbols.txt) symbols)"
echo "✓ Built uSockets.a with prefixed dependencies"
echo ""
echo "You can now safely link uSockets.a with OpenSSL without symbol conflicts."
echo ""
echo "To verify prefixing worked, run:"
echo "  nm -g uSockets.a | grep -E 'USOCKETS_(BSSL|LSQUIC)_'"
echo ""

# Cleanup temp directory
echo "Cleaning up temporary files..."
rm -rf build_safe_temp

echo "build_safe.sh completed successfully!"