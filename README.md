# magic_dialect

An MLIR dialect that models magic states as explicit SSA resources and lowers
logical T gates into `magic.create -> magic.move -> magic.consume`.

## Layout

```text
include/magic/   MagicOps.td (dialect/type/op definitions) and public headers
src/             dialect implementation, verifiers and passes
tools/           magic-opt driver
tests/           .mlir test inputs and run_tests.sh
```

The `*.inc` files are generated from `include/magic/MagicOps.td` by
`mlir-tblgen` at build time (into `<build>/include/magic/`); they are not
checked in. Editing the `.td` file re-generates them on the next build.

## Build

Requires an installed LLVM/MLIR (developed against LLVM 18).

```bash
cmake -S . -B build -G Ninja -DMLIR_DIR=/usr/lib/llvm-18/lib/cmake/mlir
cmake --build build
```

## Test

```bash
tests/run_tests.sh
```

Set `MAGIC_OPT=/path/to/magic-opt` to test a binary from another build dir.

## Passes

- `--lower-tgate`: rewrite each `magic.tgate` into a create/move/consume chain.
- `--print-magic-deps`: emit per-op remarks describing where magic states are
  required. The dependency DAG is exported only on request:

  ```bash
  build/magic-opt --print-magic-deps="dot-file=deps.dot" tests/raw_circuit.mlir
  build/magic-opt --print-magic-deps="dot-file=-" -o /dev/null tests/raw_circuit.mlir
  ```

  `dot-file=-` writes the DOT graph to stdout. The graph itself is built by
  `magic::DependencyGraph` (an in-memory MLIR analysis) and rendered by
  `magic::writeDependencyGraphDot`, so other exporters can reuse it.
