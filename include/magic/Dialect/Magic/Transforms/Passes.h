//===- Passes.h - Magic dialect passes --------------------------*- C++ -*-===//

#ifndef MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_H
#define MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_H

#include "mlir/Pass/Pass.h"

#include <memory>
#include <string>

namespace mlir {
namespace magic {

// Declares createLowerTGate(), createPrintMagicDeps(...) and the
// PrintMagicDepsOptions struct, all generated from Passes.td.
#define GEN_PASS_DECL
#include "magic/Dialect/Magic/Transforms/Passes.h.inc"

// Declares registerMagicPasses() and per-pass register functions.
#define GEN_PASS_REGISTRATION
#include "magic/Dialect/Magic/Transforms/Passes.h.inc"

} // namespace magic
} // namespace mlir

#endif // MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_H
