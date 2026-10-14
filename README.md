# magic_dialect

An MLIR dialect that models magic states as explicit SSA resources and lowers
logical T gates into `magic.create -> magic.move -> magic.consume`.

## Layout

The tree mirrors the MLIR repository (and `mlir/examples/standalone`), so
the dialect can be moved in-tree with mostly mechanical renames.

```text
include/magic/Dialect/Magic/
  IR/           MagicBase.td, MagicTypes.td, MagicOps.td, Magic.h
  Analysis/     DependencyGraph.h
  Transforms/   Passes.td, Passes.h
lib/Dialect/Magic/
  IR/           MagicDialect.cpp, MagicOps.cpp (verifiers)   -> MLIRMagicDialect
  Analysis/     DependencyGraph.cpp                          -> MLIRMagicAnalysis
  Transforms/   LowerTGate.cpp, PrintMagicDeps.cpp           -> MLIRMagicTransforms
tools/magic-opt/  mlir-opt style driver
test/             lit + FileCheck regression tests (check-magic)
```

All `*.inc` files (dialect, types, ops and pass declarations) are generated
by `mlir-tblgen` at build time into `<build>/include/magic/...`; they are not
checked in.

## Build

Requires an installed LLVM/MLIR (developed against LLVM 18), plus `lit` and
`FileCheck` for the tests.

```bash
cmake -S . -B build -G Ninja \
  -DMLIR_DIR=/usr/lib/llvm-18/lib/cmake/mlir \
  -DLLVM_EXTERNAL_LIT=$(which lit)
cmake --build build
```

`lit` can be installed with `pip install lit`; `FileCheck` and `not` are
taken from the LLVM tools directory (`llvm-18-tools` on Ubuntu). Pass
`-DMAGIC_INCLUDE_TESTS=OFF` to configure without the test target.

## Test

```bash
cmake --build build --target check-magic
```

## Passes

Passes are declared in `Passes.td` and registered via
`mlir::magic::registerMagicPasses()`.

- `--lower-tgate`: rewrite each `magic.tgate` into a create/move/consume chain.
- `--print-magic-deps`: emit per-op remarks describing where magic states are
  required. The dependency DAG is exported only on request:

  ```bash
  build/bin/magic-opt --print-magic-deps="dot-file=deps.dot" input.mlir
  build/bin/magic-opt --print-magic-deps="dot-file=-" -o /dev/null input.mlir
  ```

  The graph itself is built by `mlir::magic::DependencyGraph` (a
  side-effect-free MLIR analysis) and rendered by
  `mlir::magic::writeDependencyGraphDot`, so other exporters can reuse it.

## Moving into the MLIR tree

| Here | In-tree |
|---|---|
| `include/magic/Dialect/Magic/...` | `mlir/include/mlir/Dialect/Magic/...` |
| `lib/Dialect/Magic/...` | `mlir/lib/Dialect/Magic/...` |
| `test/Dialect/Magic/*.mlir` | `mlir/test/Dialect/Magic/*.mlir` (`magic-opt` -> `mlir-opt`) |
| `#include "magic/..."` | `#include "mlir/..."` |
| explicit `mlir_tablegen` block in `IR/CMakeLists.txt` | `add_mlir_dialect(MagicOps magic)` |

The C++ namespace is already `mlir::magic`, and the library names
(`MLIRMagic*`) follow MLIR's naming. In-tree, the dialect and passes would
also need to be registered in `mlir/include/mlir/InitAllDialects.h` and
`InitAllPasses.h`.
