#include "Dialect.h"
#include "Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<magic::MagicDialect>();
  registry.insert<mlir::func::FuncDialect>();

  magic::registerLowerTGatePass();
  magic::registerDependencyAnalysisPass();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Magic State Qubit dialect tool\n", registry));
}
