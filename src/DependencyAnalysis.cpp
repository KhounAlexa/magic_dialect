#include "magic/DependencyGraph.h"
#include "magic/Dialect.h"
#include "magic/Passes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/FileUtilities.h"

#include "llvm/Support/ToolOutputFile.h"

#include <string>

namespace {

struct DependencyAnalysisPass
    : public mlir::PassWrapper<DependencyAnalysisPass,
                                mlir::OperationPass<mlir::ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(DependencyAnalysisPass)

  DependencyAnalysisPass() = default;
  DependencyAnalysisPass(const DependencyAnalysisPass &pass)
      : PassWrapper(pass) {}

  llvm::StringRef getArgument() const final { return "print-magic-deps"; }

  llvm::StringRef getDescription() const final {
    return "Analyze gate-execution / magic-state dependencies and emit "
           "per-op diagnostics; optionally export the DAG as Graphviz DOT "
           "(dot-file=<path>, '-' for stdout). Works on both a raw circuit "
           "(magic.tgate) and a lowered one (magic.create/move/consume).";
  }

  Option<std::string> dotFile{
      *this, "dot-file",
      llvm::cl::desc("Write the dependency DAG in Graphviz DOT format to "
                     "this path ('-' for stdout). No file is written when "
                     "empty."),
      llvm::cl::init("")};

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

    // Emit per-op diagnostics identifying where a magic state is required,
    // whether the circuit is still raw (magic.tgate not yet lowered) or
    // already lowered (magic.create/move/consume present).
    module.walk([&](magic::TGateOp tgateOp) {
      tgateOp.emitRemark()
          << "magic state required HERE (not yet lowered): this T gate "
             "will need exactly 1 magic state qubit, produced via "
             "create -> move -> consume once --lower-tgate runs";
    });

    module.walk([&](magic::ConsumeOp consumeOp) {
      auto moveOp = consumeOp.getMagicState().getDefiningOp<magic::MoveOp>();
      if (!moveOp) {
        consumeOp.emitRemark() << "consume has no traceable move op";
        return;
      }
      auto createOp = moveOp.getValue().getDefiningOp<magic::CreateOp>();
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

    const auto &graph = getAnalysis<magic::DependencyGraph>();

    std::string errorMessage;
    std::unique_ptr<llvm::ToolOutputFile> output =
        mlir::openOutputFile(path, &errorMessage);
    if (!output) {
      module.emitError() << "failed to open dependency graph output '"
                         << path << "': " << errorMessage;
      return signalPassFailure();
    }
    magic::writeDependencyGraphDot(graph, output->os());
    output->keep();

    if (path != "-")
      module.emitRemark() << "wrote magic-state dependency DAG to '" << path
                          << "' (render with: dot -Tpng " << path
                          << " -o <out>.png)";
  }
};

} // namespace

namespace magic {
void registerDependencyAnalysisPass() {
  mlir::PassRegistration<DependencyAnalysisPass>();
}
} // namespace magic
