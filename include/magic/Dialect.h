#ifndef MAGIC_DIALECT_H
#define MAGIC_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"

#include "magic/MagicDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "magic/MagicTypes.h.inc"

#define GET_OP_CLASSES
#include "magic/MagicOps.h.inc"

#endif
