# Simple Authentication System Makefile
# Follows established patterns from other unit tests

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -march=native -flto -DBORINGSSL_PREFIX=USOCKETS_BSSL
CFLAGS_DEBUG = -Wall -Wextra -g -std=c99 -DBORINGSSL_PREFIX=USOCKETS_BSSL
INCLUDES = -I../../uSockets/src -I../../uSockets/boringssl_safe/symbol_prefix_include -I../../uSockets/boringssl_safe/include -Iauth_lib

# BoringSSL and uSockets libraries
LDFLAGS = ../../uSockets/uSockets.a \
          ../../uSockets/boringssl_safe/build/ssl/libssl.a \
          ../../uSockets/boringssl_safe/build/crypto/libcrypto.a \
          -lcjson -luring -lpthread -ldl -lstdc++ -lz -lm

# libmdbx library
MDBX_INCLUDES = -I../../libmdbx/
MDBX_LDFLAGS = -L../../libmdbx/build -lmdbx-static -Wl,-rpath,../../libmdbx/build

# Auth library sources
AUTH_LIB_SRCS = auth_lib/base64url.c auth_lib/userid.c auth_lib/email.c \
                auth_lib/password.c auth_lib/rsa_keys.c auth_lib/jwt_rs256.c \
                auth_lib/jwks.c auth_lib/auth.c auth_lib/json_utils.c \
                auth_lib/pkce.c auth_lib/oauth.c

# Memory management sources for Oracle A1.Flex page allocation
MEMORY_SRCS = memory/page_allocator.c memory/jwt_storage.c memory/memory_validation.c \
              memory/a1_flex_optimizations.c

AUTH_LIB_OBJS = $(AUTH_LIB_SRCS:.c=.o)
MEMORY_OBJS = $(MEMORY_SRCS:.c=.o)

# Test sources
TEST_SRCS = tests/test_userid.c tests/test_email.c tests/test_password.c \
            tests/test_base64url.c tests/test_jwt.c tests/test_auth.c

TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

# Main targets
TARGETS = server client init_oauth_client $(TEST_BINS)
DEBUG_TARGETS = server_debug

.PHONY: all clean test check-deps run-server kill-server server_debug init-oauth-clients

all: check-deps $(TARGETS)

# Auth library compilation
auth_lib/%.o: auth_lib/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

# Memory management compilation
memory/%.o: memory/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

# Server compilation
server.o: server.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

server: server.o $(AUTH_LIB_OBJS) $(MEMORY_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(MDBX_LDFLAGS)

# Debug server compilation
server_debug.o: server.c
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

auth_lib/%_debug.o: auth_lib/%.c
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

memory/%_debug.o: memory/%.c
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

AUTH_LIB_DEBUG_OBJS = $(AUTH_LIB_SRCS:.c=_debug.o)
MEMORY_DEBUG_OBJS = $(MEMORY_SRCS:.c=_debug.o)

server_debug: server_debug.o $(AUTH_LIB_DEBUG_OBJS) $(MEMORY_DEBUG_OBJS)
	$(CC) $(CFLAGS_DEBUG) $^ -o $@ $(LDFLAGS) $(MDBX_LDFLAGS)

# Client compilation
client.o: client.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

client: client.o $(AUTH_LIB_OBJS) $(MEMORY_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(MDBX_LDFLAGS)

# OAuth client initializer compilation
init_oauth_client.o: init_oauth_client.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

init_oauth_client: init_oauth_client.o $(AUTH_LIB_OBJS) $(MEMORY_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(MDBX_LDFLAGS)

# Test compilation
tests/%.o: tests/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(MDBX_INCLUDES) -c $< -o $@

tests/%: tests/%.o $(AUTH_LIB_OBJS) $(MEMORY_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(MDBX_LDFLAGS)

# Dependency checking
check-deps:
	@echo "🔍 Checking dependencies..."
	@if [ ! -f "../../uSockets/uSockets.a" ]; then \
		echo "❌ uSockets not built. Run: cd ../../uSockets && make"; \
		exit 1; \
	fi
	@if [ ! -f "../../uSockets/boringssl_safe/build/crypto/libcrypto.a" ]; then \
		echo "❌ BoringSSL not built. Run: cd ../../uSockets && make"; \
		exit 1; \
	fi
	@if [ ! -f "../../libmdbx/build/libmdbx-static.a" ]; then \
		echo "❌ libmdbx not built. Run: cd ../../libmdbx && make"; \
		exit 1; \
	fi
	@echo "✅ All dependencies found"

# Test execution
test: all
	@echo "🧪 Running unit tests..."
	@./test_script.sh

# Server management
run-server: server
	@echo "🚀 Starting auth server on port 2053..."
	@./server

kill-server:
	@echo "🛑 Stopping auth server..."
	@pkill -f "./server" || true

# Cleanup
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(TARGETS) $(DEBUG_TARGETS) *.o auth_lib/*.o auth_lib/*_debug.o memory/*.o memory/*_debug.o tests/*.o
	rm -rf data/* keys/*.pem test_logs/
	@echo "✅ Clean complete"

# Create necessary directories
setup-dirs:
	mkdir -p data keys test_logs

# Generate RSA keys if they don't exist
setup-keys: setup-dirs server
	@if [ ! -f "keys/private_key.pem" ]; then \
		echo "🔐 Generating RSA key pair..."; \
		./server --generate-keys; \
	else \
		echo "🔑 RSA keys already exist"; \
	fi

# Initialize OAuth clients for testing
init-oauth-clients: init_oauth_client setup-keys
	@echo "🔧 Initializing OAuth test clients..."
	@./init_oauth_client

# Help target
help:
	@echo "Simple Authentication System - Available targets:"
	@echo "  all               - Build all targets"
	@echo "  server            - Build auth server"
	@echo "  server_debug      - Build auth server with debug symbols (-g)"
	@echo "  client            - Build test client"
	@echo "  init_oauth_client - Build OAuth client initializer"
	@echo "  test              - Run all tests"
	@echo "  check-deps        - Check dependencies"
	@echo "  setup-keys        - Generate RSA keys"
	@echo "  init-oauth-clients - Initialize OAuth test clients in database"
	@echo "  run-server        - Start server"
	@echo "  kill-server       - Stop server"
	@echo "  clean             - Clean build artifacts"
	@echo "  help              - Show this help"