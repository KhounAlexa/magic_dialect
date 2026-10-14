//===- MagicTypes.td - Magic dialect types -----------------*- tablegen -*-===//

#ifndef MAGIC_DIALECT_MAGIC_IR_MAGICTYPES_TD
#define MAGIC_DIALECT_MAGIC_IR_MAGICTYPES_TD

include "magic/Dialect/Magic/IR/MagicBase.td"
include "mlir/IR/AttrTypeBase.td"

class Magic_Type<string name, string typeMnemonic> :
    TypeDef<Magic_Dialect, name> {
  let mnemonic = typeMnemonic;
}

def MagicStateType : Magic_Type<"MagicState", "state"> {
  let summary = "A magic state qubit resource (logical level)";
}

def QubitType : Magic_Type<"Qubit", "qubit"> {
  let summary = "A logical qubit";
}

#endif // MAGIC_DIALECT_MAGIC_IR_MAGICTYPES_TD
