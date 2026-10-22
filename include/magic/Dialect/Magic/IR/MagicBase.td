//===- MagicBase.td - Magic dialect definition -------------*- tablegen -*-===//
//
// Dialect definition shared by MagicTypes.td and MagicOps.td.
//
//===----------------------------------------------------------------------===//

#ifndef MAGIC_DIALECT_MAGIC_IR_MAGICBASE_TD
#define MAGIC_DIALECT_MAGIC_IR_MAGICBASE_TD

include "mlir/IR/OpBase.td"

def Magic_Dialect : Dialect {
  let name = "magic";
  let summary = "Magic states as explicit, linear SSA resources";
  let cppNamespace = "::mlir::magic";
  let useDefaultTypePrinterParser = 1;
}

#endif // MAGIC_DIALECT_MAGIC_IR_MAGICBASE_TD
