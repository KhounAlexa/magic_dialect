// RUN: magic-opt %s | magic-opt | FileCheck %s

// magic.consume has two results. Older LLVM (e.g. 18) prints them as
// `%N:2` / `%N#0`; LLVM 20 prints the ODS result names
// (`%updated_qubit, %classical_bit`).
// The patterns below accept both forms.

// A correct create -> move -> consume chain parses, verifies and round-trips.
// CHECK-LABEL: func.func @valid(
// CHECK:         %[[C:[0-9]+]] = "magic.create"() : () -> !magic.state
// CHECK:         %[[M:[0-9]+]] = "magic.move"(%[[C]]) : (!magic.state) -> !magic.state
// CHECK:         %[[R:[^ ,:]+]]{{(:2)?}}{{(, %[^ ]+)?}} = "magic.consume"(%[[M]], %arg0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
// CHECK:         return %[[R]]{{(#0)?}}, %{{[^ ]+}} : !magic.qubit, i1
func.func @valid(%q0: !magic.qubit) -> (!magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %q1, %c = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}

// A raw (not yet lowered) T gate is valid IR on its own.
// CHECK-LABEL: func.func @tgate(
// CHECK:         %[[Q:[0-9]+]] = "magic.tgate"(%arg0) : (!magic.qubit) -> !magic.qubit
// CHECK:         return %[[Q]] : !magic.qubit
func.func @tgate(%q0: !magic.qubit) -> !magic.qubit {
  %q1 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  return %q1 : !magic.qubit
}
