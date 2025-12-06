#!/bin/bash

# add --clean to rm -rf ./lsquic/build/

if [ -f ./lsquic/src/liblsquic/liblsquic.a ]; then
	echo "liblsquic found"
fi

cd lsquic
if [ ! -d build ]; then
	mkdir build
fi

cd build

cmake \
	-DBORINGSSL_INCLUDE=/home/opc/splitdo/new/uSockets/boringssl/include\
	-DBORINGSSL_LIB=/home/opc/splitdo/new/uSockets/boringssl/build \
	-DZLIB_LIB=/usr/lib64/libz.so \
	-DCMAKE_EXE_LINKER_FLAGS="-lstdc++" \
	..

make -j4

mkdir -p lsquic/src/liblsquic
cp lsquic/build/src/liblsquic/liblsquic.a lsquic/src/liblsquic/liblsquic.a
