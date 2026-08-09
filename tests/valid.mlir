// Expected: PASS (parses clean) -- a correct create -> move -> consume chain.
func.func @valid(%q0: !magic.qubit) -> (!magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %q1, %c = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}