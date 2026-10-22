//===- MagicOps.td - Magic dialect operations --------------*- tablegen -*-===//

#ifndef MAGIC_DIALECT_MAGIC_IR_MAGICOPS_TD
#define MAGIC_DIALECT_MAGIC_IR_MAGICOPS_TD

include "magic/Dialect/Magic/IR/MagicBase.td"
include "magic/Dialect/Magic/IR/MagicTypes.td"

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
  let hasVerifier = 1;
}

#endif // MAGIC_DIALECT_MAGIC_IR_MAGICOPS_TD
