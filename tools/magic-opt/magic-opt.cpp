//===- magic-opt.cpp - Magic dialect optimizer driver ---------------------===//

#include "magic/Dialect/Magic/IR/Magic.h"
#include "magic/Dialect/Magic/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<mlir::magic::MagicDialect, mlir::func::FuncDialect>();

  mlir::magic::registerMagicPasses();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "Magic State Qubit dialect tool\n", registry));
}
