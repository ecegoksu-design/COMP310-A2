#!/bin/bash
set -e

echo "===Starting mysh==="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
TEST_DIR="$PROJECT_ROOT/test-cases"

tests="tc1 tc2 tc3 tc4 tc5"

passed=0
total=5

for test in $tests; do
  echo "**********************"
  echo -n "Testing $test... "

  case "$test" in
    tc1|tc2|tc4) size=18 ;;
    tc3) size=21 ;;
    tc5) size=6 ;;
    *) echo "Unknown test $test"; exit 1 ;;
  esac

  (cd "$SRC_DIR" && make clean >/dev/null && make framesize="$size" mysh >/dev/null)

  (cd "$TEST_DIR" && "$SRC_DIR/mysh" < "$test.txt" > temp_output.txt)

  if diff -iw "$TEST_DIR/temp_output.txt" "$TEST_DIR/${test}_result.txt" >/dev/null; then
    echo "PASS"
    passed=$((passed+1))
  else
    echo "FAIL"
    echo "Differences:"
    diff -iw "$TEST_DIR/temp_output.txt" "$TEST_DIR/${test}_result.txt"
  fi

  rm -f "$TEST_DIR/temp_output.txt"
done

echo "**********************"
echo "Passed: $passed/$total"
echo "Failed: $((total - passed))/$total"

echo "===Making clean==="
(cd "$SRC_DIR" && make clean >/dev/null)
