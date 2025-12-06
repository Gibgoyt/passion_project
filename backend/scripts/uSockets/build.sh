#!/bin/bash

# if no boringssl built, then build

if [ ! -f ./boringssl/build/ssl/libssl.a ]; then
	echo "no libssl.a found"
	echo "boringssl not built, please run 'make boringssl' first"
	exit 1
fi

if [ ! -f ./boringssl/build/crypto/libcrypto.a ]; then
	echo "no libssl.a found"
        echo "boringssl not built, please run 'make boringssl' first"
	exit 1
fi

if [ ! -f lsquic/src/liblsquic/liblsquic.a]; then
	echo "no liblsquic.a found"
	echo "lsquic not built, please build lsquic first"
	exit 1
fi

WITH_QUIC=1 WITH_BORINGSSL=1 make -j4
