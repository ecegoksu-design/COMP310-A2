#!/bin/bash

echo "===Starting mysh==="

cd ../src || exit 1
make clean
make mysh

cd ../test-cases || exit 1


tests=()
for f in T_*.txt; do

    if [[ "$f" != *_result*.txt ]]; then
        tests+=("${f%.txt}")
    fi
done

passed=0
total=${#tests[@]}

for test in "${tests[@]}"; do
    echo -n "Testing $test... "

    ../src/mysh < "$test.txt" > temp_output.txt

    if [ -f "${test}_result2.txt" ]; then

        if diff -iw temp_output.txt "${test}_result.txt" > /dev/null; then
            echo "PASS (matches result1)"
            ((passed++))
        elif diff -iw temp_output.txt "${test}_result2.txt" > /dev/null; then
            echo "PASS (matches result2)"
            ((passed++))
        else
            echo "FAIL"
            echo "Differences with result1:"
            diff -iw temp_output.txt "${test}_result.txt"
            echo "Differences with result2:"
            diff -iw temp_output.txt "${test}_result2.txt"
        fi
    else

        if diff -iw temp_output.txt "${test}_result.txt" > /dev/null; then
            echo "PASS"
            ((passed++))
        else
            echo "FAIL"
            echo "Differences:"
            diff -iw temp_output.txt "${test}_result.txt"
        fi
    fi

    echo "**********************"
    rm -f temp_output.txt
done

echo "Passed: $passed/$total"
echo "Failed: $((total - passed))/$total"

echo "===Making clean==="
cd ../src || exit 1
make clean