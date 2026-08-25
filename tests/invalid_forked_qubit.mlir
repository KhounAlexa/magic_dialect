// Expected: FAIL -- the same qubit value is fed into two independent T
// gates instead of being threaded through (the second gate should consume
// the first gate's output). A logical qubit value is linear, just like a
// magic state: it cannot be forked into two unrelated gate operations.
func.func @forked_qubit(%q0: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %q1 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  %q2 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  return %q1, %q2 : !magic.qubit, !magic.qubit
}
