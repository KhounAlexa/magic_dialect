#ifndef MAGIC_DIALECT_H
#define MAGIC_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"

#include "MagicDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "MagicTypes.h.inc"

#define GET_OP_CLASSES
#include "MagicOps.h.inc"

#endif
