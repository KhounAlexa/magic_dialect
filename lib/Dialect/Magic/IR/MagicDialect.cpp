//===- MagicDialect.cpp - Magic dialect -----------------------------------===//

#include "magic/Dialect/Magic/IR/Magic.h"

#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::magic;

// Constructor, destructor and TypeID definition are generated from
// MagicBase.td (-gen-dialect-defs); only initialize() is hand-written.
#include "magic/Dialect/Magic/IR/MagicOpsDialect.cpp.inc"

void MagicDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "magic/Dialect/Magic/IR/MagicOps.cpp.inc"
      >();
  addTypes<
#define GET_TYPEDEF_LIST
#include "magic/Dialect/Magic/IR/MagicOpsTypes.cpp.inc"
      >();
}

#define GET_TYPEDEF_CLASSES
#include "magic/Dialect/Magic/IR/MagicOpsTypes.cpp.inc"
