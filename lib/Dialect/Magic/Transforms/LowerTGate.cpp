//===- LowerTGate.cpp - Lower magic.tgate to create/move/consume ----------===//

#include "magic/Dialect/Magic/IR/Magic.h"
#include "magic/Dialect/Magic/Transforms/Passes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"

namespace mlir {
namespace magic {
#define GEN_PASS_DEF_LOWERTGATE
#include "magic/Dialect/Magic/Transforms/Passes.h.inc"
} // namespace magic
} // namespace mlir

using namespace mlir;
using namespace mlir::magic;

namespace {

struct LowerTGatePass : public impl::LowerTGateBase<LowerTGatePass> {
  using LowerTGateBase::LowerTGateBase;

  void runOnOperation() override {
    ModuleOp module = getOperation();

    module.walk([&](TGateOp op) {
      OpBuilder builder(op);
      auto stateType = MagicStateType::get(builder.getContext());
      auto qubitType = QubitType::get(builder.getContext());
      auto classicalBitType = builder.getI1Type();

      auto createOp = builder.create<CreateOp>(op.getLoc(), stateType);
      auto moveOp =
          builder.create<MoveOp>(op.getLoc(), stateType, createOp.getResult());
      auto consumeOp = builder.create<ConsumeOp>(
          op.getLoc(), qubitType, classicalBitType, moveOp.getResult(),
          op.getQubit());

      op.getResult().replaceAllUsesWith(consumeOp.getUpdatedQubit());
      op.erase();
    });
  }
};

} // namespace
