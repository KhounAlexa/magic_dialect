// Remarks only; by default the pass writes no file.
// RUN: rm -rf %t && mkdir -p %t && cd %t
// RUN: magic-opt --print-magic-deps %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=REMARK
// RUN: not ls %t/magic-deps.dot

// Export to an explicit path, and to stdout.
// RUN: magic-opt --print-magic-deps="dot-file=%t/deps.dot" %s -o /dev/null
// RUN: FileCheck %s --check-prefix=DOT < %t/deps.dot
// RUN: magic-opt --print-magic-deps="dot-file=-" %s -o /dev/null | FileCheck %s --check-prefix=DOT

// The classic (left-to-right, boxes with locations) style is still available.
// RUN: magic-opt --print-magic-deps="dot-file=- dot-style=classic" %s -o /dev/null | FileCheck %s --check-prefix=CLASSIC

// An unwritable destination fails the pass, as does an unknown style.
// RUN: not magic-opt --print-magic-deps="dot-file=%t/missing/deps.dot" %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR
// RUN: not magic-opt --print-magic-deps="dot-file=- dot-style=bogus" %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR-STYLE

// REMARK-COUNT-3: remark: magic state required HERE (not yet lowered)
// REMARK-COUNT-2: remark: magic state required HERE: this consume op depends on the create op at

// ERR: error: failed to open dependency graph output
// ERR-STYLE: error: unknown dot-style 'bogus'; expected 'qiskit' or 'classic'

// The DOT output follows Qiskit's dag_drawer: one dashed cluster per
// function (in source order), each with a Source and a Sink, green input
// nodes, lightblue op nodes and red output nodes, all labelled by wire.
// Op nodes carry their source location as a tooltip.
// DOT-LABEL: digraph MagicDeps {
// DOT:         rankdir=TB;
// DOT:         subgraph cluster_0 {
// DOT-NEXT:      label="raw_circuit";
// DOT:           n0 [label="Source", fillcolor=darkseagreen
// DOT:           n1 [label="Sink", fillcolor=red
// DOT:           n2 [label="tgate", fillcolor=lightblue, tooltip="{{.*}}print-magic-deps.mlir:{{[0-9]+}}:{{[0-9]+}}"];
// DOT:           n3 [label="q[0]", fillcolor=green];
// DOT:           n6 [label="q[1]", fillcolor=green];
// DOT:           n7 [label="q[0]", fillcolor=red];
// DOT:           n8 [label="q[1]", fillcolor=red];
// DOT:         subgraph cluster_1 {
// DOT-NEXT:      label="two_tgates";
// DOT:           n11 [label="create", fillcolor=lightblue, tooltip="{{.*}}print-magic-deps.mlir:{{[0-9]+}}:{{[0-9]+}}"];
// DOT:           n12 [label="move", fillcolor=lightblue
// DOT:           n13 [label="consume", fillcolor=lightblue
// DOT:           n14 [label="q[0]", fillcolor=green];
// DOT:           n19 [label="q[0]", fillcolor=red];
// DOT:           n21 [label="c[0]", fillcolor=red];
// DOT:           n22 [label="c[1]", fillcolor=red];

// Raw circuit: Source -> q[0] -> tgate -> tgate -> q[0] -> Sink, and
// Source -> q[1] -> tgate -> q[1] -> Sink. Edges carry the wire name;
// Source/Sink attachment edges are unlabelled.
// DOT:         n0 -> n3;
// DOT-NEXT:    n3 -> n2 [label="q[0]"];
// DOT-NEXT:    n2 -> n4 [label="q[0]"];
// DOT-NEXT:    n0 -> n6;
// DOT-NEXT:    n6 -> n5 [label="q[1]"];
// DOT-NEXT:    n4 -> n7 [label="q[0]"];
// DOT-NEXT:    n7 -> n1;
// DOT-NEXT:    n5 -> n8 [label="q[1]"];
// DOT-NEXT:    n8 -> n1;
// Lowered circuit: two independent create -> move -> consume chains, each
// on its own magic-state wire m[i]; create hangs off Source. The classical
// bits are never used but still leave the circuit through an output node.
// DOT-NEXT:    n9 -> n11;
// DOT-NEXT:    n11 -> n12 [label="m[0]"];
// DOT-NEXT:    n12 -> n13 [label="m[0]"];
// DOT-NEXT:    n9 -> n14;
// DOT-NEXT:    n14 -> n13 [label="q[0]"];
// DOT-NEXT:    n9 -> n15;
// DOT-NEXT:    n15 -> n16 [label="m[1]"];
// DOT-NEXT:    n16 -> n17 [label="m[1]"];
// DOT-NEXT:    n9 -> n18;
// DOT-NEXT:    n18 -> n17 [label="q[1]"];
// DOT-NEXT:    n13 -> n19 [label="q[0]"];
// DOT-NEXT:    n19 -> n10;
// DOT-NEXT:    n17 -> n20 [label="q[1]"];
// DOT-NEXT:    n20 -> n10;
// DOT-NEXT:    n13 -> n21 [label="c[0]"];
// DOT-NEXT:    n21 -> n10;
// DOT-NEXT:    n17 -> n22 [label="c[1]"];
// DOT-NEXT:    n22 -> n10;
// DOT-NEXT:  }

// Classic style: same graph, drawn left to right with the op's location in
// the label. Source and output nodes are hidden and returned values flow
// straight into a "return" node; the unused classical bits draw no edge.
// CLASSIC-LABEL: digraph MagicDeps {
// CLASSIC:         rankdir=LR;
// CLASSIC:         subgraph cluster_0 {
// CLASSIC-NEXT:      label="raw_circuit";
// CLASSIC-NOT:       Source
// CLASSIC:           n1 [label="return", shape=cds, fillcolor=white];
// CLASSIC:           n2 [label="T-gate\n(magic state required\nonce lowered)\n{{.*}}print-magic-deps.mlir:{{[0-9]+}}:{{[0-9]+}}", shape=doubleoctagon, fillcolor=orange];
// CLASSIC:           n3 [label="arg0\n(logical qubit)", shape=ellipse, fillcolor=lightblue];
// CLASSIC:         subgraph cluster_1 {
// CLASSIC-NEXT:      label="two_tgates";
// CLASSIC:           n11 [label="create\n{{.*}}", shape=box, fillcolor=palegreen];
// CLASSIC:           n12 [label="move\n{{.*}}", shape=box, fillcolor=khaki];
// CLASSIC:           n13 [label="consume\n(magic state required)\n{{.*}}", shape=doubleoctagon, fillcolor=salmon];
// CLASSIC-NOT:       c[0]
// CLASSIC:         n3 -> n2;
// CLASSIC-NEXT:    n2 -> n4;
// CLASSIC-NEXT:    n6 -> n5;
// CLASSIC-NEXT:    n4 -> n1;
// CLASSIC-NEXT:    n5 -> n1;
// CLASSIC-NEXT:    n11 -> n12;
// CLASSIC-NEXT:    n12 -> n13;
// CLASSIC-NEXT:    n14 -> n13;
// CLASSIC-NEXT:    n15 -> n16;
// CLASSIC-NEXT:    n16 -> n17;
// CLASSIC-NEXT:    n18 -> n17;
// CLASSIC-NEXT:    n13 -> n10;
// CLASSIC-NEXT:    n17 -> n10;
// CLASSIC-NEXT:  }

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
