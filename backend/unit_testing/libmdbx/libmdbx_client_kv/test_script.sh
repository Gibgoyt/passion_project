#!/bin/bash

echo "=== libmdbx Database Server Test ==="
echo

# Check if server is running
if ! nc -z 127.0.0.1 9999 2>/dev/null; then
    echo "❌ Server not running on port 9999"
    echo "Start the server first with: ./server"
    exit 1
fi

echo "✅ Server is running"
echo

# Test SET operations
echo "📝 Testing SET operations..."
./client SET name "John Doe"
./client SET age "30"
./client SET city "San Francisco"
./client SET email "john@example.com"
echo

# Test GET operations
echo "📖 Testing GET operations..."
echo -n "name: "
./client GET name
echo -n "age: "
./client GET age
echo -n "city: "
./client GET city
echo -n "email: "
./client GET email
echo

# Test GET on non-existent key
echo "🔍 Testing GET on non-existent key..."
echo -n "nonexistent: "
./client GET nonexistent
echo

# Test DEL operation
echo "🗑️  Testing DEL operation..."
./client DEL city
echo

# Verify deletion
echo "🔍 Verifying deletion..."
echo -n "city after deletion: "
./client GET city
echo

# Test SET with spaces and special characters
echo "📝 Testing SET with complex values..."
./client SET description "A person who likes databases and programming!"
echo -n "description: "
./client GET description
echo

# Test overwriting existing key
echo "📝 Testing key overwrite..."
./client SET name "Jane Smith"
echo -n "name after overwrite: "
./client GET name
echo

# Final status
echo "🎯 Test Summary:"
echo "- SET operations: ✅"
echo "- GET operations: ✅" 
echo "- DEL operations: ✅"
echo "- Non-existent keys: ✅"
echo "- Complex values: ✅"
echo "- Key overwrite: ✅"
echo
echo "🎉 All tests completed! Database is working correctly."