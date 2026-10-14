#!/bin/bash
# Simple test runner for the magic dialect. Each test declares whether it
# should PASS (parse clean) or FAIL (be rejected by a verifier), and this
# script checks the actual result matches, reporting a summary at the end.

# Resolve the binary path relative to this script's own location, so the
# script works no matter which directory you run it from.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Override with MAGIC_OPT=/path/to/magic-opt for out-of-tree build dirs.
TOY_OPT="${MAGIC_OPT:-$SCRIPT_DIR/../build/magic-opt}"

if [ ! -x "$TOY_OPT" ]; then
  echo "ERROR: could not find magic-opt at $TOY_OPT"
  echo "Build it first with: cmake -S .. -B ../build -G Ninja && cmake --build ../build"
  exit 1
fi

cd "$SCRIPT_DIR"

# All generated output (captured stdout/stderr, DOT graphs, PNGs) goes into a
# private temp dir so the source tree stays clean and parallel runs of this
# script cannot clobber each other.
OUT_DIR="$(mktemp -d "${TMPDIR:-/tmp}/magic-tests.XXXXXX")"
trap 'rm -rf "$OUT_DIR"' EXIT

PASS_COUNT=0
FAIL_COUNT=0
check() {
  local description="$1"
  local expect="$2"   # "pass" or "fail"
  shift 2
  local cmd=("$@")

  "${cmd[@]}" > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
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
    sed 's/^/         /' "$OUT_DIR/stderr"
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
check "invalid_forked_qubit.mlir should be rejected"     fail "$TOY_OPT" invalid_forked_qubit.mlir

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
echo "-- Description 3: dependency graph (DAG) export --"
# By default the pass must not touch the filesystem.
rm -f magic-deps.dot
"$TOY_OPT" --print-magic-deps raw_circuit.mlir > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
if [ $? -eq 0 ] && [ ! -e magic-deps.dot ]; then
  echo "  OK   - --print-magic-deps writes no file unless dot-file is given"
  PASS_COUNT=$((PASS_COUNT+1))
else
  echo "  FAIL - --print-magic-deps without dot-file failed or wrote magic-deps.dot"
  sed 's/^/         /' "$OUT_DIR/stderr"
  FAIL_COUNT=$((FAIL_COUNT+1))
fi

# Two circuits exported to two distinct paths must not overwrite each other.
"$TOY_OPT" --print-magic-deps="dot-file=$OUT_DIR/raw_circuit.dot" raw_circuit.mlir > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
raw_exit=$?
"$TOY_OPT" --print-magic-deps="dot-file=$OUT_DIR/multi.dot" multi.mlir > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
multi_exit=$?
if [ $raw_exit -eq 0 ] && [ $multi_exit -eq 0 ] \
   && grep -q 'raw_circuit' "$OUT_DIR/raw_circuit.dot" \
   && grep -q 'two_tgates' "$OUT_DIR/multi.dot" \
   && ! grep -q 'two_tgates' "$OUT_DIR/raw_circuit.dot"; then
  echo "  OK   - dot-file=<path> writes each circuit's DAG to its own file"
  PASS_COUNT=$((PASS_COUNT+1))
else
  echo "  FAIL - dot-file=<path> did not produce separate per-circuit DOT files"
  sed 's/^/         /' "$OUT_DIR/stderr"
  FAIL_COUNT=$((FAIL_COUNT+1))
fi

# dot-file=- streams the graph to stdout (IR goes to -o).
"$TOY_OPT" --print-magic-deps="dot-file=-" -o "$OUT_DIR/ir.mlir" raw_circuit.mlir > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
if [ $? -eq 0 ] && grep -q '^digraph MagicDeps' "$OUT_DIR/stdout"; then
  echo "  OK   - dot-file=- writes the DAG to stdout"
  PASS_COUNT=$((PASS_COUNT+1))
else
  echo "  FAIL - dot-file=- did not write the DAG to stdout"
  sed 's/^/         /' "$OUT_DIR/stderr"
  FAIL_COUNT=$((FAIL_COUNT+1))
fi

if command -v dot >/dev/null 2>&1; then
  dot -Tpng "$OUT_DIR/raw_circuit.dot" -o "$OUT_DIR/raw_circuit.png" > "$OUT_DIR/stdout" 2>"$OUT_DIR/stderr"
  if [ $? -eq 0 ] && [ -s "$OUT_DIR/raw_circuit.png" ]; then
    echo "  OK   - raw_circuit.mlir dependency DAG renders to a valid PNG"
    PASS_COUNT=$((PASS_COUNT+1))
  else
    echo "  FAIL - dot failed to render raw_circuit.dot to PNG"
    sed 's/^/         /' "$OUT_DIR/stderr"
    FAIL_COUNT=$((FAIL_COUNT+1))
  fi
else
  echo "  SKIP - graphviz 'dot' not installed; skipping DAG render check"
fi

echo ""
echo "=== Summary: $PASS_COUNT passed, $FAIL_COUNT failed ==="
[ $FAIL_COUNT -eq 0 ]