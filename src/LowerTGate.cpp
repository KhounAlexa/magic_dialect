#include "magic/Dialect.h"
#include "magic/Passes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

namespace {

struct LowerTGatePass
    : public mlir::PassWrapper<LowerTGatePass,
                                mlir::OperationPass<mlir::ModuleOp>> {

  llvm::StringRef getArgument() const final { return "lower-tgate"; }

  llvm::StringRef getDescription() const final {
    return "Lower magic.tgate ops into magic.create/move/consume sequences";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

    module.walk([&](magic::TGateOp op) {
      mlir::OpBuilder builder(op);
      auto stateType = magic::MagicStateType::get(builder.getContext());
      auto qubitType = magic::QubitType::get(builder.getContext());
      auto classicalBitType = builder.getI1Type();

      auto createOp = builder.create<magic::CreateOp>(op.getLoc(), stateType);
      auto moveOp = builder.create<magic::MoveOp>(
          op.getLoc(), stateType, createOp.getResult());
      auto consumeOp = builder.create<magic::ConsumeOp>(
          op.getLoc(), qubitType, classicalBitType,
          moveOp.getResult(), op.getQubit());

      op.getResult().replaceAllUsesWith(consumeOp.getUpdatedQubit());
      op.erase();
    });
  }
};

} // namespace

namespace magic {
void registerLowerTGatePass() {
  mlir::PassRegistration<LowerTGatePass>();
}
} // namespace magic
