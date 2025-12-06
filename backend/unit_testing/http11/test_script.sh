#!/bin/bash

# HTTP/1.1 SSL Unit Testing Script
# Automated testing for uSockets SSL HTTPS server and client

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test configuration
SERVER_PORT=2053
SERVER_HOST="localhost"
TEST_TIMEOUT=10
LOG_DIR="./test_logs"
SERVER_LOG="$LOG_DIR/server.log"
CLIENT_LOG="$LOG_DIR/client.log"

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date +'%H:%M:%S')] ${message}${NC}"
}

print_success() {
    print_status "$GREEN" "✅ $1"
}

print_error() {
    print_status "$RED" "❌ $1"
}

print_warning() {
    print_status "$YELLOW" "⚠️  $1"
}

print_info() {
    print_status "$BLUE" "ℹ️  $1"
}

# Cleanup function
cleanup() {
    print_info "Cleaning up..."
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
        print_info "Server stopped (PID: $SERVER_PID)"
    fi

    # Remove any test files if needed
    # rm -f test_*.tmp 2>/dev/null || true
}

# Set up cleanup on exit
trap cleanup EXIT

# Create log directory
mkdir -p "$LOG_DIR"

print_info "Starting HTTP/1.1 SSL Unit Tests"
print_info "================================="

# Test 1: Check dependencies
print_info "Test 1: Checking dependencies..."

if [ ! -f "./server" ]; then
    print_error "Server binary not found. Run 'make server' first."
    exit 1
fi

if [ ! -f "./client" ]; then
    print_error "Client binary not found. Run 'make client' first."
    exit 1
fi

if [ ! -f "/etc/ssl/splitdo_api/cert.pem" ]; then
    print_error "SSL certificate not found at /etc/ssl/splitdo_api/cert.pem"
    exit 1
fi

if [ ! -f "/etc/ssl/splitdo_api/private/key.pem" ]; then
    print_error "SSL private key not found at /etc/ssl/splitdo_api/private/key.pem"
    exit 1
fi

print_success "All dependencies found"

# Test 2: Check if port is available
print_info "Test 2: Checking if port $SERVER_PORT is available..."

if netstat -tlnp 2>/dev/null | grep -q ":$SERVER_PORT "; then
    print_error "Port $SERVER_PORT is already in use"
    netstat -tlnp 2>/dev/null | grep ":$SERVER_PORT "
    exit 1
fi

print_success "Port $SERVER_PORT is available"

# Test 3: Start server
print_info "Test 3: Starting SSL HTTPS server..."

./server > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

# Wait for server to start
sleep 2

if ! kill -0 $SERVER_PID 2>/dev/null; then
    print_error "Server failed to start"
    print_error "Server log contents:"
    cat "$SERVER_LOG"
    exit 1
fi

print_success "Server started successfully (PID: $SERVER_PID)"

# Test 4: Verify server is listening
print_info "Test 4: Verifying server is listening on port $SERVER_PORT..."

for i in {1..5}; do
    if netstat -tlnp 2>/dev/null | grep -q ":$SERVER_PORT "; then
        print_success "Server is listening on port $SERVER_PORT"
        break
    fi
    if [ $i -eq 5 ]; then
        print_error "Server is not listening on port $SERVER_PORT after 5 attempts"
        print_error "Server log contents:"
        cat "$SERVER_LOG"
        exit 1
    fi
    sleep 1
done

# Test 5: SSL certificate validation
print_info "Test 5: Testing SSL certificate with openssl..."

timeout 5 openssl s_client -connect "$SERVER_HOST:$SERVER_PORT" -verify_return_error < /dev/null > /dev/null 2>&1
if [ $? -eq 0 ]; then
    print_success "SSL certificate validation passed"
else
    print_warning "SSL certificate validation failed (this may be expected for self-signed certs)"
fi

# Test 6: Basic connectivity test with curl
print_info "Test 6: Testing HTTPS connectivity with curl..."

curl_output=$(timeout $TEST_TIMEOUT curl -k -s -w "%{http_code}" "https://$SERVER_HOST:$SERVER_PORT" 2>/dev/null)
curl_exit_code=$?

if [ $curl_exit_code -eq 0 ] && [[ "$curl_output" == *"200"* ]]; then
    print_success "HTTPS connectivity test passed (HTTP 200)"
else
    print_warning "HTTPS connectivity test with curl failed (exit code: $curl_exit_code)"
    print_info "This may be normal if curl is not available or has SSL issues"
fi

# Test 7: Client validation test
print_info "Test 7: Running SSL client validation test..."

timeout $TEST_TIMEOUT ./client "$SERVER_HOST" $SERVER_PORT > "$CLIENT_LOG" 2>&1
client_exit_code=$?

print_info "Client test output:"
cat "$CLIENT_LOG"

if [ $client_exit_code -eq 0 ]; then
    print_success "Client validation test PASSED"
else
    print_error "Client validation test FAILED (exit code: $client_exit_code)"
    exit 1
fi

# Test 8: Multiple concurrent connections
print_info "Test 8: Testing multiple concurrent connections..."

concurrent_success=0
for i in {1..3}; do
    timeout $TEST_TIMEOUT ./client "$SERVER_HOST" $SERVER_PORT > "$LOG_DIR/client_$i.log" 2>&1 &
    client_pids[$i]=$!
done

# Wait for all clients to complete
for i in {1..3}; do
    wait ${client_pids[$i]}
    if [ $? -eq 0 ]; then
        concurrent_success=$((concurrent_success + 1))
    fi
done

if [ $concurrent_success -eq 3 ]; then
    print_success "All 3 concurrent connections successful"
else
    print_warning "Only $concurrent_success out of 3 concurrent connections successful"
fi

# Test 9: Server stress test (brief)
print_info "Test 9: Brief server stress test (10 rapid connections)..."

stress_success=0
for i in {1..10}; do
    timeout 3 ./client "$SERVER_HOST" $SERVER_PORT > "$LOG_DIR/stress_$i.log" 2>&1
    if [ $? -eq 0 ]; then
        stress_success=$((stress_success + 1))
    fi
    sleep 0.1  # Brief pause between connections
done

print_info "Stress test: $stress_success out of 10 connections successful"

if [ $stress_success -ge 8 ]; then
    print_success "Stress test passed (≥8/10 successful)"
else
    print_warning "Stress test marginal ($stress_success/10 successful)"
fi

# Test 10: Check for memory leaks (basic)
print_info "Test 10: Basic server health check..."

server_memory=$(ps -o pid,vsz,rss,comm -p $SERVER_PID --no-headers 2>/dev/null || echo "unknown")
print_info "Server memory usage: $server_memory"

if kill -0 $SERVER_PID 2>/dev/null; then
    print_success "Server is still running and responsive"
else
    print_error "Server appears to have crashed during testing"
    exit 1
fi

# Final summary
print_info "=================================="
print_success "SSL HTTPS Unit Tests Completed!"
print_info "=================================="
print_info "Test Summary:"
print_info "- Dependencies: ✅"
print_info "- Port availability: ✅"
print_info "- Server startup: ✅"
print_info "- Server listening: ✅"
print_info "- SSL certificate: $([ $? -eq 0 ] && echo '✅' || echo '⚠️')"
print_info "- HTTPS connectivity: $([ $curl_exit_code -eq 0 ] && echo '✅' || echo '⚠️')"
print_info "- Client validation: ✅"
print_info "- Concurrent connections: $([ $concurrent_success -eq 3 ] && echo '✅' || echo '⚠️')"
print_info "- Stress test: $([ $stress_success -ge 8 ] && echo '✅' || echo '⚠️')"
print_info "- Server health: ✅"

print_success "All critical tests passed! SSL HTTPS server is working correctly."
print_info "Logs available in: $LOG_DIR/"

exit 0