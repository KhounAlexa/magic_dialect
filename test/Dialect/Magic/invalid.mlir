// RUN: magic-opt %s -split-input-file -verify-diagnostics

// consume's magic-state operand skips the move stage, coming directly from
// create instead.
func.func @skip_move(%q0: !magic.qubit) -> (!magic.qubit, i1) {
  // expected-error @+1 {{created magic state escapes to a non-'magic.move' use}}
  %m0 = "magic.create"() : () -> !magic.state
  %q1, %c = "magic.consume"(%m0, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}

// -----

// move's operand is a plain function argument, not produced by magic.create.
func.func @move_of_argument(%not_magic: !magic.state) -> !magic.state {
  // expected-error @+1 {{operand must be produced by a 'magic.create' op}}
  %m1 = "magic.move"(%not_magic) : (!magic.state) -> !magic.state
  return %m1 : !magic.state
}

// -----

// The same moved magic state is consumed twice.
func.func @double_consume(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, i1, !magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  // expected-error @+1 {{moved magic state is consumed more than once}}
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %qa, %ca = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  %qb, %cb = "magic.consume"(%m1, %q1) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %qa, %ca, %qb, %cb : !magic.qubit, i1, !magic.qubit, i1
}

// -----

// The same created magic state is moved twice, forking one resource.
func.func @double_move() -> (!magic.state, !magic.state) {
  // expected-error @+1 {{created magic state is used more than once}}
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %m2 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  return %m1, %m2 : !magic.state, !magic.state
}

// -----

// A magic state is created and then abandoned: it is never moved.
func.func @dangling_create(%q0: !magic.qubit) -> !magic.qubit {
  // expected-error @+1 {{created magic state is never moved}}
  %m0 = "magic.create"() : () -> !magic.state
  return %q0 : !magic.qubit
}

// -----

// A moved magic state escapes via return without ever being consumed.
func.func @dangling_move() -> !magic.state {
  %m0 = "magic.create"() : () -> !magic.state
  // expected-error @+1 {{moved magic state escapes to a non-'magic.consume' use}}
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  return %m1 : !magic.state
}

// -----

// A magic-state value is passed where a logical qubit is expected; rejected
// by ordinary ODS operand type checking.
func.func @type_confusion() -> (!magic.qubit, i1) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %m2 = "magic.create"() : () -> !magic.state
  %m3 = "magic.move"(%m2) : (!magic.state) -> !magic.state
  // expected-error @+1 {{operand #1 must be}}
  %q1, %c = "magic.consume"(%m1, %m3) : (!magic.state, !magic.state) -> (!magic.qubit, i1)
  return %q1, %c : !magic.qubit, i1
}

// -----

// The same qubit value is fed into two independent T gates instead of being
// threaded through.
func.func @forked_qubit(%q0: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  // expected-error @+1 {{qubit operand is used more than once}}
  %q1 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  %q2 = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  return %q1, %q2 : !magic.qubit, !magic.qubit
}
