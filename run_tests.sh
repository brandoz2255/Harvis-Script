#!/bin/bash
# Test runner for Harvis Script Compiler

COMPILER="/workspace/compiler/build/hs"
PASS=0
FAIL=0
TOTAL=0

run_test() {
    TOTAL=$((TOTAL + 1))
    local test_file=$1
    local name=$(basename "$test_file" .hs)
    
    echo "=== Test: $name ==="
    local output=$($COMPILER "$test_file" 2>&1)
    
    if echo "$output" | grep -q "tests passed"; then
        echo "✅ PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "❌ FAIL: $name"
        echo "$output" | tail -10
        FAIL=$((FAIL + 1))
    fi
    echo ""
}

# Basic tests
if [ -d "test_basic" ]; then
    for f in test_basic/*.hs; do
        run_test "$f"
    done
fi

# Exception tests
if [ -d "test_exceptions" ]; then
    for f in test_exceptions/*.hs; do
        run_test "$f"
    done
fi

# Concurrency tests
if [ -d "test_concurrency" ]; then
    for f in test_concurrency/*.hs; do
        run_test "$f"
    done
fi

# Module tests
if [ -d "test_modules" ]; then
    for f in test_modules/*.hs; do
        run_test "$f"
    done
fi

# Classes tests
if [ -d "test_classes" ]; then
    for f in test_classes/*.hs; do
        run_test "$f"
    done
fi

# Generics tests
if [ -d "test_generics" ]; then
    for f in test_generics/*.hs; do
        run_test "$f"
    done
fi

echo "================================"
echo "Test Results: $PASS passed, $FAIL failed, $TOTAL total"
echo "================================"
