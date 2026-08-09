// Expected: FAIL -- a magic state is created and then abandoned: it is
// never moved. A magic state resource must not be dropped.
func.func @dangling_create(%q0: !magic.qubit) -> !magic.qubit {
  %m0 = "magic.create"() : () -> !magic.state
  return %q0 : !magic.qubit
}
