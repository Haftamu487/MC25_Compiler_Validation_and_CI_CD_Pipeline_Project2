#!/bin/bash

echo "========================================"
echo "       MC-25 Compiler Test Suite"
echo "========================================"

run_valid_test() {
    input="$1"
    description="$2"

    echo
    echo "[VALID TEST] $description"
    echo "Input: $input"

    echo "$input" | ./mc25 > /dev/null

    if [ $? -eq 0 ]; then
        echo "PASS ✓"
    else
        echo "FAIL ✗"
    fi
}

run_invalid_test() {
    input="$1"
    description="$2"

    echo
    echo "[INVALID TEST] $description"
    echo "Input: $input"

    echo "$input" | ./mc25 > /dev/null 2>&1

    if [ $? -ne 0 ]; then
        echo "PASS ✓ (Correctly rejected)"
    else
        echo "FAIL ✗ (Incorrectly accepted)"
    fi
}

echo
echo "Running Valid Tests..."
echo "--------------------------------"

run_valid_test \
"int main() { return 42; }" \
"Standard MC-25 program"

run_valid_test \
"int test() { return 100; }" \
"Different identifier and number"

run_valid_test \
"int hello() { return 0; }" \
"Return zero"

echo
echo "Running Invalid Tests..."
echo "--------------------------------"

run_invalid_test \
"int main() { return 42 }" \
"Missing semicolon"

run_invalid_test \
"int main() { return ; }" \
"Missing number"

run_invalid_test \
"float main() { return 42; }" \
"Invalid data type"

run_invalid_test \
"int main( { return 42; }" \
"Missing closing parenthesis"

run_invalid_test \
"int main() return 42;" \
"Missing braces"

echo
echo "========================================"
echo "         ALL TESTS COMPLETED"
echo "========================================"
