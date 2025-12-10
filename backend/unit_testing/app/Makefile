# App Makefile (uSockets + libmdbx + cJSON + AuthLibs)

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

# New Library Paths
AUTH_LIB_DIR = ../../libraries/auth_lib
MEMORY_LIB_DIR = ../../libraries/memory

INCLUDES = -I$(USOCKETS_INCLUDE) \
           -I$(MDBX_INCLUDE) \
           -I$(OPENSSL_INCLUDE) \
           -I$(AUTH_LIB_DIR) \
           -I$(MEMORY_LIB_DIR)

# Linking: uSockets, mdbx, openssl, cjson, stdc++ (for uSockets), pthread, z, m
LDFLAGS = $(USOCKETS_LIB) \
          $(MDBX_LIB) \
          -L$(OPENSSL_LIB_DIR) -lssl -lcrypto \
          -lcjson \
          -lpthread -lstdc++ -lz -lm

# Source files
SRCS = server.c \
       $(AUTH_LIB_DIR)/auth.c \
       $(AUTH_LIB_DIR)/oauth.c \
       $(AUTH_LIB_DIR)/json_utils.c \
       $(AUTH_LIB_DIR)/base64url.c \
       $(AUTH_LIB_DIR)/email.c \
       $(AUTH_LIB_DIR)/jwks.c \
       $(AUTH_LIB_DIR)/jwt_rs256.c \
       $(AUTH_LIB_DIR)/password.c \
       $(AUTH_LIB_DIR)/pkce.c \
       $(AUTH_LIB_DIR)/rsa_keys.c \
       $(AUTH_LIB_DIR)/userid.c \
       $(MEMORY_LIB_DIR)/platform_detection.c \
       $(MEMORY_LIB_DIR)/jwt_storage_compat.c \
       $(MEMORY_LIB_DIR)/mac_m1_2022/jwt_storage.c \
       $(MEMORY_LIB_DIR)/mac_m1_2022/memory_validation.c \
       $(MEMORY_LIB_DIR)/mac_m1_2022/page_allocator.c

# Object files (auto-generated)
OBJS = $(SRCS:.c=.o)

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -f $(AUTH_LIB_DIR)/*.o
	rm -f $(MEMORY_LIB_DIR)/*.o
	rm -rf data/ auth_data/

run: server
	./server

.PHONY: all clean run
