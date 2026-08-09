include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"

def Magic_Dialect : Dialect {
  let name = "magic";
  let cppNamespace = "::magic";
  let useDefaultTypePrinterParser = 1;
}

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

class Magic_Op<string mnemonic, list<Trait> traits = []> :
    Op<Magic_Dialect, mnemonic, traits>;

def CreateOp : Magic_Op<"create"> {
  let summary = "Create a magic state qubit resource";
  let results = (outs MagicStateType:$result);
  let hasVerifier = 1;
}

def MoveOp : Magic_Op<"move"> {
  let summary = "Move/relocate a magic state qubit resource";
  let arguments = (ins MagicStateType:$value);
  let results = (outs MagicStateType:$result);
  let hasVerifier = 1;
}

def ConsumeOp : Magic_Op<"consume"> {
  let summary = "Consume a magic state qubit resource";
  let arguments = (ins MagicStateType:$magic_state, QubitType:$qubit);
  let results = (outs QubitType:$updated_qubit, I1:$classical_bit);
  let hasVerifier = 1;
}

def TGateOp : Magic_Op<"tgate"> {
  let summary = "A T gate acting on a logical qubit (pre-lowering form)";
  let arguments = (ins QubitType:$qubit);
  let results = (outs QubitType:$result);
}
