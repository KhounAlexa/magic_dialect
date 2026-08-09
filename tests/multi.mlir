// Used with --print-magic-deps to verify two independent T-gate chains
// are tracked separately, without cross-contamination.
func.func @two_tgates(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %qa, %ca = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  %m2 = "magic.create"() : () -> !magic.state
  %m3 = "magic.move"(%m2) : (!magic.state) -> !magic.state
  %qb, %cb = "magic.consume"(%m3, %q1) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %qa, %qb : !magic.qubit, !magic.qubit
}