// Expected: FAIL -- a magic-state value is passed where a logical qubit is
// expected. Qubit, magic-state, and classical-bit are distinct MLIR types,
// so this must be rejected by ordinary operand type checking.
func.func @confuse() -> (!magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %m2 = "magic.create"() : () -> !magic.state
  %m3 = "magic.move"(%m2) : (!magic.state) -> !magic.state
  %q1, %c = "magic.consume"(%m1, %m3) : (!magic.state, !magic.state) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}
