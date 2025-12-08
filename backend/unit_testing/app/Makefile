# App Makefile (uSockets + libmdbx + cJSON)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=native -flto
# Debug flags (uncomment to use)
# CFLAGS = -Wall -Wextra -g -O0 -std=c99

# Paths (adapted from sibling projects)
USOCKETS_INCLUDE = /Users/ahmed/dev/uWebSockets/uSockets/src
USOCKETS_LIB = /Users/ahmed/dev/uWebSockets/uSockets/uSockets.a

MDBX_INCLUDE = /Users/ahmed/dev/libmdbx/build-local/include
MDBX_LIB = /Users/ahmed/dev/libmdbx/build-local/lib/libmdbx.a

OPENSSL_INCLUDE = /opt/homebrew/include
OPENSSL_LIB_DIR = /opt/homebrew/lib

INCLUDES = -I$(USOCKETS_INCLUDE) \
           -I$(MDBX_INCLUDE) \
           -I$(OPENSSL_INCLUDE)

# Linking: uSockets, mdbx, openssl, cjson, stdc++ (for uSockets), pthread, z, m
LDFLAGS = $(USOCKETS_LIB) \
          $(MDBX_LIB) \
          -L$(OPENSSL_LIB_DIR) -lssl -lcrypto \
          -lcjson \
          -lpthread -lstdc++ -lz -lm

TARGET = server

all: $(TARGET)

server: server.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -rf data/

run: server
	./server

.PHONY: all clean run