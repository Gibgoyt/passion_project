OPENSSL_INCLUDE = ../../../openssl/include
OPENSSL_LIBS = ../../../openssl
USOCKETS_INCLUDE = ../uSockets/src
USOCKETS_LIB = ../uSockets/uSockets.a
BORINGSSL_DIR = ../uSockets/boringssl_safe
LSQUIC_DIR = ../uSockets/lsquic_safe
CC = gcc
CFLAGS = -O3 -march=native -flto -DBORINGSSL_PREFIX=USOCKETS_BSSL
LDFLAGS = -I$(USOCKETS_INCLUDE) -I$(BORINGSSL_DIR)/symbol_prefix_include -I$(BORINGSSL_DIR)/include -I$(LSQUIC_DIR)/include $(USOCKETS_LIB) $(BORINGSSL_DIR)/build/ssl/libssl.a $(BORINGSSL_DIR)/build/crypto/libcrypto.a $(LSQUIC_DIR)/src/liblsquic/liblsquic.a -luring -lpthread -ldl -lstdc++ -lz -lm

.PHONY: clean help

# Pattern rule: make <name> will compile <name>.c into <name>
%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f main liburing_bio_interface_tls

help:
	@echo "Usage: make <target>"
	@echo "Targets:"
	@echo "  make main                          - Compile main.c"
	@echo "  make liburing_bio_interface_tls    - Compile liburing_bio_interface_tls.c"
	@echo "  make clean                         - Remove compiled binaries"
	@echo "  make help                          - Show this help message"
