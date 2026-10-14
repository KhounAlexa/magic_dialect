add_mlir_library(MLIRMagicAnalysis
  DependencyGraph.cpp

  ADDITIONAL_HEADER_DIRS
  ${MAGIC_SOURCE_DIR}/include/magic/Dialect/Magic/Analysis

  DEPENDS
  MLIRMagicOpsIncGen

  LINK_LIBS PUBLIC
  MLIRFuncDialect
  MLIRIR
  MLIRMagicDialect
  )
