// RUN: magic-opt --lower-tgate %s | FileCheck %s

// magic.consume has two results. Older LLVM (e.g. 18) prints them as
// `%N:2` / `%N#0`; LLVM 20 prints the ODS result names
// (`%updated_qubit, %classical_bit`).
// The patterns below accept both forms.

// A single T gate expands into create -> move -> consume.
// CHECK-LABEL: func.func @single_tgate(
// CHECK-NOT:     magic.tgate
// CHECK:         %[[C:[0-9]+]] = "magic.create"() : () -> !magic.state
// CHECK:         %[[M:[0-9]+]] = "magic.move"(%[[C]]) : (!magic.state) -> !magic.state
// CHECK:         %[[R:[^ ,:]+]]{{(:2)?}}{{(, %[^ ]+)?}} = "magic.consume"(%[[M]], %arg0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
// CHECK:         return %[[R]]{{(#0)?}} : !magic.qubit
func.func @single_tgate(%q0: !magic.qubit) -> !magic.qubit {
  %q1 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  return %q1 : !magic.qubit
}

// Two logical qubits: q0 hit by two sequential T gates, q1 by one. Each T
// gate lowers independently, and the second T gate on q0 consumes the first
// one's updated qubit rather than the original argument.
// CHECK-LABEL: func.func @raw_circuit(
// CHECK-NOT:     magic.tgate
// CHECK:         %[[C0:[0-9]+]] = "magic.create"()
// CHECK:         %[[M0:[0-9]+]] = "magic.move"(%[[C0]])
// CHECK:         %[[R0:[^ ,:]+]]{{(:2)?}}{{(, %[^ ]+)?}} = "magic.consume"(%[[M0]], %arg0)
// CHECK:         %[[C1:[0-9]+]] = "magic.create"()
// CHECK:         %[[M1:[0-9]+]] = "magic.move"(%[[C1]])
// CHECK:         %[[R1:[^ ,:]+]]{{(:2)?}}{{(, %[^ ]+)?}} = "magic.consume"(%[[M1]], %[[R0]]{{(#0)?}})
// CHECK:         %[[C2:[0-9]+]] = "magic.create"()
// CHECK:         %[[M2:[0-9]+]] = "magic.move"(%[[C2]])
// CHECK:         %[[R2:[^ ,:]+]]{{(:2)?}}{{(, %[^ ]+)?}} = "magic.consume"(%[[M2]], %arg1)
// CHECK:         return %[[R1]]{{(#0)?}}, %[[R2]]{{(#0)?}} : !magic.qubit, !magic.qubit
// CHECK-NOT:     magic.tgate
func.func @raw_circuit(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %q0_a = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  %q0_b = "magic.tgate"(%q0_a) : (!magic.qubit) -> !magic.qubit
  %q1_a = "magic.tgate"(%q1) : (!magic.qubit) -> !magic.qubit
  return %q0_b, %q1_a : !magic.qubit, !magic.qubit
}
