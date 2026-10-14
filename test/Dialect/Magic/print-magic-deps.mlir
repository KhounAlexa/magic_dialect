// Remarks only; by default the pass writes no file.
// RUN: rm -rf %t && mkdir -p %t && cd %t
// RUN: magic-opt --print-magic-deps %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=REMARK
// RUN: not ls %t/magic-deps.dot

// Export to an explicit path, and to stdout.
// RUN: magic-opt --print-magic-deps="dot-file=%t/deps.dot" %s -o /dev/null
// RUN: FileCheck %s --check-prefix=DOT < %t/deps.dot
// RUN: magic-opt --print-magic-deps="dot-file=-" %s -o /dev/null | FileCheck %s --check-prefix=DOT

// An unwritable destination fails the pass.
// RUN: not magic-opt --print-magic-deps="dot-file=%t/missing/deps.dot" %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR

// REMARK-COUNT-3: remark: magic state required HERE (not yet lowered)
// REMARK-COUNT-2: remark: magic state required HERE: this consume op depends on the create op at

// ERR: error: failed to open dependency graph output

// One cluster per function, in source order.
// DOT-LABEL: digraph MagicDeps {
// DOT:         subgraph cluster_0 {
// DOT-NEXT:      label="raw_circuit";
// DOT:           n0 [label="T-gate\n(magic state required\nonce lowered)\n{{.*}}print-magic-deps.mlir:{{[0-9]+}}:{{[0-9]+}}", shape=doubleoctagon, fillcolor=orange];
// DOT:           n1 [label="arg0\n(logical qubit)", shape=ellipse, fillcolor=lightblue];
// DOT:           n5 [label="return", shape=cds, fillcolor=white];
// DOT:         subgraph cluster_1 {
// DOT-NEXT:      label="two_tgates";
// DOT:           n6 [label="create\n{{.*}}", shape=box, fillcolor=palegreen];
// DOT:           n7 [label="move\n{{.*}}", shape=box, fillcolor=khaki];
// DOT:           n8 [label="consume\n(magic state required)\n{{.*}}", shape=doubleoctagon, fillcolor=salmon];

// Raw circuit: q0 -> T -> T -> return, q1 -> T -> return.
// DOT:         n1 -> n0;
// DOT-NEXT:    n0 -> n2;
// DOT-NEXT:    n4 -> n3;
// DOT-NEXT:    n2 -> n5;
// DOT-NEXT:    n3 -> n5;
// Lowered circuit: two independent create -> move -> consume chains.
// DOT-NEXT:    n6 -> n7;
// DOT-NEXT:    n7 -> n8;
// DOT-NEXT:    n9 -> n8;
// DOT-NEXT:    n10 -> n11;
// DOT-NEXT:    n11 -> n12;
// DOT-NEXT:    n13 -> n12;
// DOT-NEXT:    n8 -> n14;
// DOT-NEXT:    n12 -> n14;
// DOT-NEXT:  }

func.func @raw_circuit(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %q0_a = "magic.tgate"(%q0) : (!magic.qubit) -> !magic.qubit
  %q0_b = "magic.tgate"(%q0_a) : (!magic.qubit) -> !magic.qubit
  %q1_a = "magic.tgate"(%q1) : (!magic.qubit) -> !magic.qubit
  return %q0_b, %q1_a : !magic.qubit, !magic.qubit
}

func.func @two_tgates(%q0: !magic.qubit, %q1: !magic.qubit) -> (!magic.qubit, !magic.qubit) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %qa, %ca = "magic.consume"(%m1, %q0) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  %m2 = "magic.create"() : () -> !magic.state
  %m3 = "magic.move"(%m2) : (!magic.state) -> !magic.state
  %qb, %cb = "magic.consume"(%m3, %q1) : (!magic.state, !magic.qubit) -> (!magic.qubit, i1)
  return %qa, %qb : !magic.qubit, !magic.qubit
}
