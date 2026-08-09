// Used with --lower-tgate. A slightly larger "raw circuit" than tgate.mlir:
// two logical qubits, with q0 hit by two sequential T gates and q1 hit by
// one T gate, all before any lowering to magic-state commands. Checks that
// each T gate lowers independently and that sequential T gates on the same
// qubit are threaded in order (the second consumes the first's result, not
// the original qubit argument).
func.func @raw_circuit(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %q0_a = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  %q0_b = "magic.tgate"(%q0_a) : (!magic.qubit) -> !magic.qubit
  %q1_a = "magic.tgate"(%q1) : (!magic.qubit) -> !magic.qubit
  return %q0_b, %q1_a : !magic.qubit, !magic.qubit
}
