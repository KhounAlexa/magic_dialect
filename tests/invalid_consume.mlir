// Expected: FAIL -- consume's magic-state operand skips the move stage,
// coming directly from create instead.
func.func @invalid(%q0: !magic.qubit) -> (!magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %q1, %c = "magic.consume"(%m0, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}