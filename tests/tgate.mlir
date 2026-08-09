// Used with --lower-tgate to verify T(q) expands into create/move/consume.
func.func @test_tgate(%q0: !magic.qubit) -> !magic.qubit {
  %q1 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  return %q1 : !magic.qubit
}