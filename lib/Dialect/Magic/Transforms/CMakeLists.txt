add_mlir_dialect_library(MLIRMagicTransforms
  LowerTGate.cpp
  PrintMagicDeps.cpp

  ADDITIONAL_HEADER_DIRS
  ${MAGIC_SOURCE_DIR}/include/magic/Dialect/Magic/Transforms

  DEPENDS
  MLIRMagicOpsIncGen
  MLIRMagicPassIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRMagicAnalysis
  MLIRMagicDialect
  MLIRPass
  MLIRSupport
  )
