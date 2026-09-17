#!/bin/bash

echo "=== MC-25 Compiler Validation ==="

echo "[1] Building compiler..."
make clean
make

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Compiler build successful."

echo "[2] Running test suite..."
./test_suite.sh

if [ $? -ne 0 ]; then
    echo "Test suite failed."
    exit 1
fi

echo "Test suite passed."

echo "[3] Running MARS simulation..."
java -jar Mars4_5.jar nc output.s > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "MARS Simulation Passed"
else
    echo "MARS Simulation Failed"
    exit 1
fi

echo "=== Validation Successful ==="