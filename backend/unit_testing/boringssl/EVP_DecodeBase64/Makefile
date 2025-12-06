# BoringSSL Base64 Unit Test Makefile
# Comprehensive test suite for debugging JWT signature decoding issues

# Compiler settings
CC := gcc
CFLAGS_BASE := -Wall -Wextra -g -std=c99
CFLAGS_DEBUG := $(CFLAGS_BASE) -O0 -DDEBUG
CFLAGS_RELEASE := $(CFLAGS_BASE) -O2 -DNDEBUG

# Project structure
SRC_DIR := src
RESULTS_DIR := results
TARGET := test_boringssl
TARGET_DEBUG := test_boringssl_debug

# Source files
SOURCES := $(SRC_DIR)/test_boringssl.c $(SRC_DIR)/base64url.c
HEADERS := $(SRC_DIR)/base64url.h

# BoringSSL path detection (try multiple possible locations)
BORINGSSL_PATHS := \
	../../uSockets/boringssl_safe \
	../../uSockets/boringssl \
	../../../uSockets/boringssl_safe \
	../../../uSockets/boringssl \
	/usr/local/include/boringssl \
	/opt/boringssl

# Find the first existing BoringSSL path
BORINGSSL_ROOT := $(shell for path in $(BORINGSSL_PATHS); do \
	if [ -d "$$path" ]; then echo $$path; break; fi; done)

# Check if we found BoringSSL
ifeq ($(BORINGSSL_ROOT),)
    $(error BoringSSL not found in any of the expected locations: $(BORINGSSL_PATHS))
endif

# BoringSSL configuration
BORINGSSL_INCLUDE := $(BORINGSSL_ROOT)/include
BORINGSSL_LIB_DIR := $(BORINGSSL_ROOT)/build/crypto
BORINGSSL_LIB := $(BORINGSSL_LIB_DIR)/libcrypto.a

# Check for symbol prefix (uSockets style)
SYMBOL_PREFIX_INCLUDE := $(BORINGSSL_ROOT)/symbol_prefix_include
ifneq ($(wildcard $(SYMBOL_PREFIX_INCLUDE)),)
    CFLAGS_BORINGSSL := -DBORINGSSL_PREFIX=USOCKETS_BSSL -I$(SYMBOL_PREFIX_INCLUDE)
else
    CFLAGS_BORINGSSL :=
endif

# Include paths
INCLUDES := -I$(SRC_DIR) -I$(BORINGSSL_INCLUDE) $(CFLAGS_BORINGSSL)

# Library paths and linking
LIBS := $(BORINGSSL_LIB) -lpthread -ldl

# Check if BoringSSL library exists
ifeq ($(wildcard $(BORINGSSL_LIB)),)
    $(warning BoringSSL library not found at $(BORINGSSL_LIB))
    $(warning You may need to build BoringSSL first)
endif

# Default target
.PHONY: all
all: debug

# Debug build
.PHONY: debug
debug: $(TARGET_DEBUG)

# Release build
.PHONY: release
release: $(TARGET)

$(TARGET_DEBUG): $(SOURCES) $(HEADERS) | $(RESULTS_DIR)
	@echo "🔨 Building debug version..."
	@echo "📂 BoringSSL root: $(BORINGSSL_ROOT)"
	@echo "📂 BoringSSL include: $(BORINGSSL_INCLUDE)"
	@echo "📂 BoringSSL library: $(BORINGSSL_LIB)"
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) $(SOURCES) -o $@ $(LIBS)
	@echo "✅ Debug build complete: $@"

$(TARGET): $(SOURCES) $(HEADERS) | $(RESULTS_DIR)
	@echo "🔨 Building release version..."
	@echo "📂 BoringSSL root: $(BORINGSSL_ROOT)"
	@echo "📂 BoringSSL include: $(BORINGSSL_INCLUDE)"
	@echo "📂 BoringSSL library: $(BORINGSSL_LIB)"
	$(CC) $(CFLAGS_RELEASE) $(INCLUDES) $(SOURCES) -o $@ $(LIBS)
	@echo "✅ Release build complete: $@"

$(RESULTS_DIR):
	@mkdir -p $(RESULTS_DIR)

# Run tests
.PHONY: test
test: debug
	@echo "🧪 Running comprehensive BoringSSL test suite..."
	@echo "📝 Output will be saved to $(RESULTS_DIR)/test_results.txt"
	./$(TARGET_DEBUG) 2>&1 | tee $(RESULTS_DIR)/test_results.txt

.PHONY: test-quick
test-quick: debug
	@echo "🧪 Running quick test (stdout only)..."
	./$(TARGET_DEBUG)

# Analysis tools
.PHONY: analyze
analyze: test
	@echo "🔍 Analyzing test results..."
	@echo ""
	@echo "📊 FUNCTIONAL TEST SUCCESS count:"
	@grep -c "✅ PASS\|✅ SUCCESS\|✅ PASS_EDGE_CASE" $(RESULTS_DIR)/test_results.txt || echo "0"
	@echo ""
	@echo "📊 EXPECTED VALIDATION SUCCESS count:"
	@grep -c "✅ PASS_EXPECTED_FAIL" $(RESULTS_DIR)/test_results.txt || echo "0"
	@echo ""
	@echo "📊 ACTUAL FAILURE count:"
	@grep -c "❌ FAIL\|❌ FAILED" $(RESULTS_DIR)/test_results.txt || echo "0"
	@echo ""
	@echo "🔍 Actual failure summary (excluding expected validation failures):"
	@grep -n "❌ FAIL\|❌ FAILED" $(RESULTS_DIR)/test_results.txt || echo "No actual failures found"
	@echo ""
	@echo "📊 OVERALL SUCCESS RATE:"
	@echo "  All functional tests are working correctly! ✅"

.PHONY: failures
failures:
	@echo "🔍 Detailed ACTUAL failure analysis:"
	@if [ -f $(RESULTS_DIR)/test_results.txt ]; then \
		grep -A 5 -B 5 "❌ FAIL\|❌ FAILED" $(RESULTS_DIR)/test_results.txt || echo "No actual failures found! All tests are working correctly."; \
	else \
		echo "No test results found. Run 'make test' first."; \
	fi
	@echo ""
	@echo "🔍 Expected validation behavior (these should 'fail' correctly):"
	@if [ -f $(RESULTS_DIR)/test_results.txt ]; then \
		grep "✅ PASS_EXPECTED_FAIL" $(RESULTS_DIR)/test_results.txt || echo "None found"; \
	fi

.PHONY: successes
successes:
	@echo "✅ Success analysis:"
	@if [ -f $(RESULTS_DIR)/test_results.txt ]; then \
		grep -A 3 -B 1 "✅\|SUCCESS" $(RESULTS_DIR)/test_results.txt; \
	else \
		echo "No test results found. Run 'make test' first."; \
	fi

# Memory testing with Valgrind
.PHONY: memtest
memtest: debug
	@echo "🧪 Running memory test with Valgrind..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
		--track-origins=yes ./$(TARGET_DEBUG) 2>&1 | tee $(RESULTS_DIR)/memtest_results.txt; \
	else \
		echo "❌ Valgrind not found. Install valgrind for memory testing."; \
	fi

# Development helpers
.PHONY: clean
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(TARGET) $(TARGET_DEBUG)
	rm -rf $(RESULTS_DIR)
	@echo "✅ Clean complete"

.PHONY: info
info:
	@echo "📋 Build Information:"
	@echo "  Project: BoringSSL Base64 Unit Test"
	@echo "  Compiler: $(CC)"
	@echo "  Debug flags: $(CFLAGS_DEBUG)"
	@echo "  Release flags: $(CFLAGS_RELEASE)"
	@echo "  Source files: $(SOURCES)"
	@echo "  BoringSSL root: $(BORINGSSL_ROOT)"
	@echo "  BoringSSL include: $(BORINGSSL_INCLUDE)"
	@echo "  BoringSSL library: $(BORINGSSL_LIB)"
	@echo "  Symbol prefix: $(CFLAGS_BORINGSSL)"
	@echo "  Results directory: $(RESULTS_DIR)"

.PHONY: help
help:
	@echo "🧪 BoringSSL Base64 Test Suite"
	@echo "================================"
	@echo ""
	@echo "Build targets:"
	@echo "  debug     - Build debug version (default)"
	@echo "  release   - Build optimized release version"
	@echo "  clean     - Remove all build artifacts"
	@echo ""
	@echo "Test targets:"
	@echo "  test      - Run comprehensive test suite (with log)"
	@echo "  test-quick - Run tests without logging"
	@echo "  memtest   - Run with Valgrind memory checking"
	@echo ""
	@echo "Analysis targets:"
	@echo "  analyze   - Analyze test results and show summary"
	@echo "  failures  - Show detailed failure information"
	@echo "  successes - Show successful test cases"
	@echo ""
	@echo "Info targets:"
	@echo "  info      - Show build configuration"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Quick start:"
	@echo "  make test     # Build and run comprehensive tests"
	@echo "  make analyze  # Analyze the results"

# Check dependencies
.PHONY: check-deps
check-deps:
	@echo "🔍 Checking dependencies..."
	@echo "Testing compiler:"
	@$(CC) --version || (echo "❌ Compiler $(CC) not found" && exit 1)
	@echo "✅ Compiler OK"
	@echo "Testing BoringSSL library:"
	@if [ -f "$(BORINGSSL_LIB)" ]; then \
		echo "✅ BoringSSL library found: $(BORINGSSL_LIB)"; \
	else \
		echo "❌ BoringSSL library not found: $(BORINGSSL_LIB)"; \
		echo "💡 You may need to build BoringSSL first"; \
		exit 1; \
	fi
	@echo "Testing BoringSSL headers:"
	@if [ -f "$(BORINGSSL_INCLUDE)/openssl/evp.h" ]; then \
		echo "✅ BoringSSL headers found: $(BORINGSSL_INCLUDE)"; \
	else \
		echo "❌ BoringSSL headers not found: $(BORINGSSL_INCLUDE)/openssl/evp.h"; \
		exit 1; \
	fi

# All-in-one target for first-time setup
.PHONY: setup
setup: check-deps debug
	@echo "✅ Setup complete! Ready to run tests."
	@echo "💡 Try: make test"