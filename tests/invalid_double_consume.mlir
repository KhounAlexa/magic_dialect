// Expected: FAIL -- the same moved magic state is consumed twice.
// A magic state resource is linear: it must be consumed exactly once.
func.func @double_consume(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, i1, !magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %qa, %ca = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  %qb, %cb = "magic.consume"(%m1, %q1) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %qa, %ca, %qb, %cb : !magic.qubit, i1, !magic.qubit, i1
}
