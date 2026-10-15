//===- PrintMagicDeps.cpp - Report and export magic-state dependencies ----===//

#include "magic/Dialect/Magic/Analysis/DependencyGraph.h"
#include "magic/Dialect/Magic/IR/Magic.h"
#include "magic/Dialect/Magic/Transforms/Passes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Support/FileUtilities.h"

#include "llvm/Support/ToolOutputFile.h"

namespace mlir {
namespace magic {
#define GEN_PASS_DEF_PRINTMAGICDEPS
#include "magic/Dialect/Magic/Transforms/Passes.h.inc"
} // namespace magic
} // namespace mlir

using namespace mlir;
using namespace mlir::magic;

namespace {

// The `dot-file` option, pass name and description are generated from
// Passes.td into PrintMagicDepsBase.
struct PrintMagicDepsPass
    : public mlir::magic::impl::PrintMagicDepsBase<PrintMagicDepsPass> {
  using PrintMagicDepsBase::PrintMagicDepsBase;

  void runOnOperation() override {
    ModuleOp module = getOperation();

    // Emit per-op diagnostics identifying where a magic state is required,
    // whether the circuit is still raw (magic.tgate not yet lowered) or
    // already lowered (magic.create/move/consume present).
    module.walk([&](TGateOp tgateOp) {
      tgateOp.emitRemark()
          << "magic state required HERE (not yet lowered): this T gate "
             "will need exactly 1 magic state qubit, produced via "
             "create -> move -> consume once --lower-tgate runs";
    });

    module.walk([&](ConsumeOp consumeOp) {
      auto moveOp = consumeOp.getMagicState().getDefiningOp<MoveOp>();
      if (!moveOp) {
        consumeOp.emitRemark() << "consume has no traceable move op";
        return;
      }
      auto createOp = moveOp.getValue().getDefiningOp<CreateOp>();
      if (!createOp) {
        consumeOp.emitRemark() << "consume's move has no traceable create op";
        return;
      }
      consumeOp.emitRemark()
          << "magic state required HERE: this consume op depends on the "
             "create op at "
          << createOp.getLoc() << " (via move op at " << moveOp.getLoc()
          << ")";
    });

    // This pass only observes the IR.
    markAllAnalysesPreserved();

    // Exporting is opt-in: the pass has no filesystem side effect unless a
    // destination is explicitly requested, so several runs (or several
    // circuits) never clobber a shared, hard-coded output file.
    const std::string path = dotFile.getValue();
    if (path.empty())
      return;

    const auto &graph = getAnalysis<DependencyGraph>();

    std::string errorMessage;
    std::unique_ptr<llvm::ToolOutputFile> output =
        openOutputFile(path, &errorMessage);
    if (!output) {
      module.emitError() << "failed to open dependency graph output '"
                         << path << "': " << errorMessage;
      return signalPassFailure();
    }
    writeDependencyGraphDot(graph, output->os());
    output->keep();

    if (path != "-")
      module.emitRemark() << "wrote magic-state dependency DAG to '" << path
                          << "' (render with: dot -Tpng " << path
                          << " -o <out>.png)";
  }
};

} // namespace
