#!/bin/bash
# Simple test runner for the magic dialect. Each test declares whether it
# should PASS (parse clean) or FAIL (be rejected by a verifier), and this
# script checks the actual result matches, reporting a summary at the end.

# Resolve the binary path relative to this script's own location, so the
# script works no matter which directory you run it from.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOY_OPT="$SCRIPT_DIR/../build/magic-opt"

if [ ! -x "$TOY_OPT" ]; then
  echo "ERROR: could not find magic-opt at $TOY_OPT"
  echo "Build it first with: cd ../build && ninja"
  exit 1
fi

cd "$SCRIPT_DIR"

PASS_COUNT=0
FAIL_COUNT=0
check() {
  local description="$1"
  local expect="$2"   # "pass" or "fail"
  shift 2
  local cmd=("$@")

  "${cmd[@]}" > /tmp/test_stdout 2>/tmp/test_stderr
  local exit_code=$?

  if [ "$expect" = "pass" ] && [ $exit_code -eq 0 ]; then
    echo "  OK   - $description"
    PASS_COUNT=$((PASS_COUNT+1))
  elif [ "$expect" = "fail" ] && [ $exit_code -ne 0 ]; then
    echo "  OK   - $description (correctly rejected)"
    PASS_COUNT=$((PASS_COUNT+1))
  else
    echo "  FAIL - $description (expected exit=$expect, got exit_code=$exit_code)"
    echo "         --- stderr ---"
    sed 's/^/         /' /tmp/test_stderr
    FAIL_COUNT=$((FAIL_COUNT+1))
  fi
}

echo "=== Running magic dialect test suite ==="
echo ""

echo "-- Description 1/2: basic parsing + state verification --"
check "valid.mlir should parse cleanly"                pass "$TOY_OPT" valid.mlir
check "invalid_consume.mlir should be rejected"         fail "$TOY_OPT" invalid_consume.mlir
check "invalid_move.mlir should be rejected"             fail "$TOY_OPT" invalid_move.mlir
check "invalid_double_consume.mlir should be rejected"   fail "$TOY_OPT" invalid_double_consume.mlir
check "invalid_double_move.mlir should be rejected"      fail "$TOY_OPT" invalid_double_move.mlir
check "invalid_dangling_create.mlir should be rejected"  fail "$TOY_OPT" invalid_dangling_create.mlir
check "invalid_dangling_move.mlir should be rejected"    fail "$TOY_OPT" invalid_dangling_move.mlir
check "invalid_type_confusion.mlir should be rejected"   fail "$TOY_OPT" invalid_type_confusion.mlir

echo ""
echo "-- Description 4: tgate lowering --"
check "tgate.mlir parses as-is without the flag"         pass "$TOY_OPT" tgate.mlir
check "tgate.mlir lowers successfully with --lower-tgate" pass "$TOY_OPT" --lower-tgate tgate.mlir
check "raw_circuit.mlir parses as-is without the flag"    pass "$TOY_OPT" raw_circuit.mlir
check "raw_circuit.mlir lowers successfully with --lower-tgate" pass "$TOY_OPT" --lower-tgate raw_circuit.mlir

echo ""
echo "-- Description 3: dependency analysis --"
check "multi.mlir dependency analysis runs cleanly"       pass "$TOY_OPT" --print-magic-deps multi.mlir

echo ""
echo "=== Summary: $PASS_COUNT passed, $FAIL_COUNT failed ==="
[ $FAIL_COUNT -eq 0 ]