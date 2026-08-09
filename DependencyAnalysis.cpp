#include "Dialect.h"
#include "Passes.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

namespace {

struct DependencyAnalysisPass
    : public mlir::PassWrapper<DependencyAnalysisPass,
                                mlir::OperationPass<mlir::ModuleOp>> {

  llvm::StringRef getArgument() const final { return "print-magic-deps"; }

  llvm::StringRef getDescription() const final {
    return "Report which magic.create each magic.consume depends on";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

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
          << "this consume op depends on create op at "
          << createOp.getLoc()
          << " (via move op at " << moveOp.getLoc() << ")";
    });
  }
};

} // namespace

namespace magic {
void registerDependencyAnalysisPass() {
  mlir::PassRegistration<DependencyAnalysisPass>();
}
} // namespace magic
