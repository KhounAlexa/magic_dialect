//===- Magic.h - Magic dialect ----------------------------------*- C++ -*-===//

#ifndef MAGIC_DIALECT_MAGIC_IR_MAGIC_H
#define MAGIC_DIALECT_MAGIC_IR_MAGIC_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"

#include "magic/Dialect/Magic/IR/MagicOpsDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "magic/Dialect/Magic/IR/MagicOpsTypes.h.inc"

#define GET_OP_CLASSES
#include "magic/Dialect/Magic/IR/MagicOps.h.inc"

#endif // MAGIC_DIALECT_MAGIC_IR_MAGIC_H
