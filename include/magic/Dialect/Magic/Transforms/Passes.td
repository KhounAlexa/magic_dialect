//===- Passes.td - Magic dialect passes --------------------*- tablegen -*-===//

#ifndef MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_TD
#define MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_TD

include "mlir/Pass/PassBase.td"

def LowerTGate : Pass<"lower-tgate", "::mlir::ModuleOp"> {
  let summary = "Lower magic.tgate ops into magic.create/move/consume sequences";
  let description = [{
    Rewrites every `magic.tgate` into an explicit magic-state lifecycle:
    `magic.create` -> `magic.move` -> `magic.consume`. The updated qubit
    produced by `magic.consume` replaces all uses of the T gate's result, so
    sequential T gates on one qubit stay threaded in order.
  }];
}

def PrintMagicDeps : Pass<"print-magic-deps", "::mlir::ModuleOp"> {
  let summary = "Report magic-state dependencies and optionally export the DAG";
  let description = [{
    Emits a remark at every point where a magic state is required, both for
    raw circuits (`magic.tgate`) and lowered ones
    (`magic.create`/`move`/`consume`). When `dot-file` is set, the
    dependency DAG computed by `magic::DependencyGraph` is written in
    Graphviz DOT format to that path (`-` for stdout). Without it, the pass
    has no filesystem side effects.
  }];
  let options = [
    Option<"dotFile", "dot-file", "std::string", /*default=*/"",
           "Write the dependency DAG in Graphviz DOT format to this path "
           "('-' for stdout). No file is written when empty.">
  ];
}

#endif // MAGIC_DIALECT_MAGIC_TRANSFORMS_PASSES_TD
